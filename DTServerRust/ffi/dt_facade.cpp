#include "dt_facade.h"

#include "dt-server/src/twin/ffi.rs.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <BECommunicationService.h>
#include <BaseFuctions/StringExtention.hpp>
#include <DigitalTwinManager.h>
#include <MQTT/entities/FunctionRegistrationRequest.hpp>
#include <MQTT/entities/values/BooleanValueEntity.h>
#include <MQTT/entities/values/ComplexValueEntity.h>
#include <MQTT/entities/values/IntegerValueEntity.h>
#include <MQTT/entities/values/PositiveValueEntity.h>
#include <MQTT/entities/values/RationalValueEntity.h>
#include <MQTT/entities/values/RealValueEntity.h>
#include <MQTT/entities/values/StringValueEntity.h>
#include <DigitalTwinStore.h>
#include <Model/DigitalTwinModel.h>
#include <Services/IMqttClientService.h>
#include <entities/DigitalTwin.h>
#include <sysmlv2/service/online/HttpException.h>

namespace dtrs {
    namespace {
        namespace History = DigitalTwin::Store;
        using Clock = std::chrono::system_clock;

        // The housekeeping thread looks for checkpoints that are due (new month, gap) this often.
        constexpr std::chrono::seconds HOUSEKEEPING_PERIOD{30};
        // A twin whose registration in the store failed is registered again on an update, at most this often.
        constexpr std::chrono::seconds REGISTRATION_RETRY{30};

        // The model library publishes through this interface only in DigitalTwinManager::generateMQTTInterface and
        // addDigitalTwinAndCreateModel, neither of which the facade calls. The manager still needs an instance.
        class NoopMqttClient : public DigitalTwin::Communication::IMqttClientService {
        public:
            void publish(std::string, std::string) override {}
            void subscribe(std::string, std::function<void(std::string, std::string)>) override {}
        };

        // One login. Models keep it alive through a shared_ptr, so a new login does not pull the manager away
        // from a model that is still in use.
        struct Session {
            std::unique_ptr<BACKEND_COMMUNICATION::CommunicationService> Communication;
            NoopMqttClient Mqtt;
            std::unique_ptr<DigitalTwin::DigitalTwinManager> Manager;
            // The session was created with an application key (the key is in State::ApplicationKeys).
            bool UsesApplicationKey = false;
        };

        struct ModelEntry {
            std::shared_ptr<Session> Owner;  // declared first: destroyed after the model
            std::unique_ptr<DigitalTwin::Model::DigitalTwinModel> Model;
            std::mutex ModelMutex;  // the model is not thread safe

            // What the registration in the store needs: set before the entry is published, never changed.
            std::string ProjectIdText;
            std::string TwinIdText;
            std::string TwinName;
            std::string CommitIdText;
            std::vector<DigitalTwin::Model::ModelElementInfo> Tree;

            // History state, guarded by ModelMutex.
            bool Registered = false;  // the twin is known to the store (VersionNo is valid)
            int32_t VersionNo = 0;
            bool RegistrationInProgress = false;
            std::chrono::steady_clock::time_point NextRegistrationAttempt{};
            bool ChangedSinceCheckpoint = false;
            // The model was replaced (load of the same twin) or released: nothing it does is recorded any more, and
            // an update that still reaches it is refused.
            bool Retired = false;
        };

        // Lock order. There are three kinds of lock and no code path holds two of the first two at once:
        //  - State::Mutex guards the maps and the store pointer. It is held only to copy or change them; no other
        //    lock is taken while it is held.
        //  - ModelEntry::ModelMutex guards one model and its history state. It is taken without any other of these
        //    locks held (callers first copy the entry out of the map under State::Mutex, then release it). While it
        //    is held, only the store's own mutex (a leaf: it calls nothing back) may be taken, and the model mutex
        //    is released around the one blocking database call (registration).
        //  - A per-twin load mutex (State::LoadLocks) is taken by facade_load_twin before anything else of this
        //    list is held and kept until the model is swapped in. Under it State::Mutex, ModelMutex of the old
        //    entry and the store's mutex are taken one after the other, never the other way round: nothing that
        //    holds State::Mutex or a ModelMutex ever waits for a load mutex.
        //  - HousekeepingMutex only protects the stop flag of the housekeeping thread and is held while waiting.
        // So State::Mutex is never taken under a ModelMutex. The housekeeping thread copies the list of entries
        // under State::Mutex and then locks the models one at a time, never two.
        struct State {
            std::mutex Mutex;
            std::shared_ptr<Session> Current;
            // Every application key that was ever used for a session, for removing it from error messages. The
            // sessions are shared with the models (a model keeps its session alive), so a message can come from an
            // older session than the current one; a key is never removed. Guarded by KeysMutex, a leaf lock.
            std::mutex KeysMutex;
            std::vector<std::string> ApplicationKeys;
            std::map<boost::uuids::uuid, std::shared_ptr<ModelEntry>> Models;
            // One mutex per twin id that serialises the retire-register-swap section of facade_load_twin. Entries
            // are never removed (a few bytes per twin). Guarded by Mutex; the mutexes themselves are not.
            std::map<boost::uuids::uuid, std::shared_ptr<std::mutex>> LoadLocks;

            // History store, guarded by Mutex.
            std::shared_ptr<History::DigitalTwinStore> HistoryStore;
            bool StoreConfigured = false;
            bool StoreShutdownStarted = false;
            bool StoreShutDown = false;
            std::thread Housekeeper;

            std::mutex HousekeepingMutex;
            std::condition_variable HousekeepingWake;
            bool HousekeepingStop = false;
        };

        // Deliberately leaked: a blocking call may still run on a Rust thread while the process exits, and
        // destroying the mutex, the sessions and the models (static destruction order, exit handlers) under it
        // would be a race. The operating system reclaims the memory.
        State& state = *new State;

        // Defence in depth: the text of a library exception for an HTTP status that describeBackendError does not
        // know is what() of the library, whose content is not under our control. A configured application key is
        // replaced in every message that crosses the bridge.
        std::string redactApplicationKeys(std::string message) {
            std::vector<std::string> keys;
            {
                std::lock_guard lock(state.KeysMutex);
                keys = state.ApplicationKeys;
            }
            for (const auto& key : keys) {
                if (key.empty())
                    continue;
                for (std::size_t at = message.find(key); at != std::string::npos; at = message.find(key, at)) {
                    message.replace(at, key.size(), "<application key>");
                    at += std::string("<application key>").size();
                }
            }
            return message;
        }

        // keySession: the failing request was made with an application key. Then an HTTP 401 means the server
        // rejected the key, and the advice of describeBackendError ("Your session has expired ... log in again") is
        // wrong: only the detail of the server's own answer, if there is one, is kept (the caller adds the rest).
        std::string describe(const std::exception& e, bool keySession = false) {
            try {
                if (keySession) {
                    if (auto http = dynamic_cast<const SysMLv2::API::EXCEPTIONS::HTTPException*>(&e))
                        if (http->httpErrorCode() == 401)
                            return redactApplicationKeys(http->serverErrorCode());
                }
                return redactApplicationKeys(BACKEND_COMMUNICATION::CommunicationService::describeBackendError(e));
            } catch (...) {
                return "backend error";
            }
        }

        // HTTP 401 means the session expired: the caller logs in again and retries once.
        int32_t codeFor(const std::exception& e) {
            if (dynamic_cast<const std::invalid_argument*>(&e))
                return CODE_INVALID_ARGUMENT;
            if (auto http = dynamic_cast<const SysMLv2::API::EXCEPTIONS::HTTPException*>(&e))
                if (http->httpErrorCode() == 401)
                    return CODE_UNAUTHORIZED;
            return CODE_BACKEND_ERROR;
        }

        CallResult result(int32_t code, const std::string& message) {
            return CallResult{code, rust::String::lossy(message)};
        }

        // Runs `body` (which returns a CallResult) and turns every exception into an error result.
        template <typename F>
        CallResult guarded(F&& body) noexcept {
            try {
                return body();
            } catch (const std::exception& e) {
                return result(codeFor(e), describe(e));
            } catch (...) {
                return result(CODE_UNKNOWN_ERROR, "unknown C++ exception");
            }
        }

        boost::uuids::uuid parseUuid(rust::Str text) {
            return boost::uuids::string_generator()(std::string(text));
        }

        // ---------------------------------------------------------------------------------------------
        // History
        // ---------------------------------------------------------------------------------------------

        // The messages of the store never contain the database URL or the password.
        void logHistory(const std::string& message) {
            try {
                std::cerr << "[history] " << message << std::endl;
            } catch (...) {
            }
        }

        // The store while it accepts rows, nullptr if none was configured or it is shut down.
        std::shared_ptr<History::DigitalTwinStore> currentStore() {
            std::lock_guard lock(state.Mutex);
            return state.StoreShutDown ? nullptr : state.HistoryStore;
        }

        Clock::time_point fromMillis(int64_t milliseconds) {
            if (milliseconds <= 0)
                return Clock::now();
            return Clock::time_point(std::chrono::duration_cast<Clock::duration>(std::chrono::milliseconds(milliseconds)));
        }

        // Registers the twin in the store (a database call that may take a while). Never throws.
        std::optional<History::TwinRegistration> tryRegister(History::DigitalTwinStore& store, const ModelEntry& entry,
                                                             bool readLastState = true) {
            try {
                return store.registerTwin(entry.ProjectIdText, entry.TwinIdText, entry.TwinName, entry.CommitIdText,
                                          entry.Tree, readLastState);
            } catch (const std::exception& e) {
                logHistory("registering twin " + entry.TwinIdText + " failed, it runs without history for now: " + e.what());
            } catch (...) {
                logHistory("registering twin " + entry.TwinIdText + " failed, it runs without history for now");
            }
            return std::nullopt;
        }

        // Writes a checkpoint of the current state of the model. The caller holds entry.ModelMutex: the snapshot
        // and the enqueue happen under it, so that no update falls between them. Never throws.
        bool writeCheckpoint(ModelEntry& entry, History::DigitalTwinStore& store, History::CheckpointReason reason,
                             Clock::time_point takenAt) {
            if (!entry.Registered)
                return false;
            try {
                const auto snapshot = entry.Model->snapshotState();
                const bool accepted = store.enqueueCheckpoint(entry.VersionNo, reason, takenAt, snapshot);
                if (accepted)
                    entry.ChangedSinceCheckpoint = false;
                return accepted;
            } catch (const std::exception& e) {
                logHistory(std::string("writing a checkpoint failed: ") + e.what());
            } catch (...) {
                logHistory("writing a checkpoint failed");
            }
            return false;
        }

        // A twin that the store does not know (the database was down when it was loaded): the housekeeping thread
        // tries again to register it, at most every 30 seconds, so that update_variable never makes a blocking
        // database call. On success a load checkpoint of the current state is written (the state is not restored:
        // the model has moved on). The registration runs without the model mutex, which is released and taken
        // again; the updates in between are in the checkpoint.
        void retryRegistration(ModelEntry& entry, History::DigitalTwinStore& store, std::unique_lock<std::mutex>& modelLock) {
            if (entry.TwinIdText.empty())
                return;  // the twin was loaded before the store existed: there is nothing to register
            if (entry.RegistrationInProgress || std::chrono::steady_clock::now() < entry.NextRegistrationAttempt)
                return;
            entry.RegistrationInProgress = true;
            modelLock.unlock();
            const auto registration = tryRegister(store, entry, false);  // the state is not restored here
            modelLock.lock();
            entry.RegistrationInProgress = false;
            entry.NextRegistrationAttempt = std::chrono::steady_clock::now() + REGISTRATION_RETRY;
            if (!registration || entry.Retired)
                return;
            entry.Registered = true;
            entry.VersionNo = registration->VersionNo;
            logHistory("twin " + entry.TwinIdText + " is registered now");
            writeCheckpoint(entry, store, History::CheckpointReason::Load, Clock::now());
        }

        // The history of one update: the change set, then a checkpoint if one is due. The caller holds the model
        // mutex through `modelLock`.
        void recordHistory(ModelEntry& entry, History::DigitalTwinStore& store, const DigitalTwin::Model::UpdateResult& update,
                           Clock::time_point receivedAt) {
            try {
                if (!entry.Registered)
                    return;  // not recorded; the registration is retried by the housekeeping thread
                if (!update.changes.empty()) {
                    store.enqueueChanges(entry.VersionNo, update.changes, receivedAt, update.timePoint.value_or(receivedAt));
                    entry.ChangedSinceCheckpoint = true;
                }
                if (const auto reason = store.checkpointDue(entry.VersionNo, receivedAt))
                    writeCheckpoint(entry, store, *reason, receivedAt);
            } catch (const std::exception& e) {
                logHistory(std::string("recording an update failed: ") + e.what());
            } catch (...) {
                logHistory("recording an update failed");
            }
        }

        // The model is replaced or released: a final checkpoint if it changed, then the entry records nothing and
        // refuses updates. Whatever it did before is queued in the store before this returns, so a registration that
        // follows can wait for it. Returns true if this call retired it.
        bool retireEntry(ModelEntry& entry, History::DigitalTwinStore* store) {
            std::lock_guard lock(entry.ModelMutex);
            if (entry.Retired)
                return false;
            if (store && entry.Registered && entry.ChangedSinceCheckpoint)
                writeCheckpoint(entry, *store, History::CheckpointReason::Shutdown, Clock::now());
            entry.Retired = true;
            return true;
        }

        // Undoes retireEntry if the load that retired the entry fails and the entry stays in use.
        // Only an entry that is still the one of the map is un-retired: if the twin was released meanwhile, the
        // entry stays retired.
        struct Retiree {
            std::shared_ptr<ModelEntry> Entry;
            boost::uuids::uuid TwinId{};
            bool Keep = false;
            ~Retiree() {
                if (!Entry || Keep)
                    return;
                bool inMap = false;
                {
                    std::lock_guard lock(state.Mutex);
                    const auto it = state.Models.find(TwinId);
                    inMap = it != state.Models.end() && it->second == Entry;
                }
                if (inMap) {
                    std::lock_guard lock(Entry->ModelMutex);
                    Entry->Retired = false;
                }
            }
        };

        // One pass of the housekeeping thread: checkpoints that are due although nothing changed (a new month, a
        // gap), and the registration of twins that the store does not know, for every loaded twin.
        void housekeepingPass() {
            const auto store = currentStore();
            if (!store)
                return;
            std::vector<std::shared_ptr<ModelEntry>> entries;
            {
                std::lock_guard lock(state.Mutex);
                for (const auto& model : state.Models)
                    entries.push_back(model.second);
            }
            const auto now = Clock::now();
            for (const auto& entry : entries) {
                std::unique_lock lock(entry->ModelMutex);
                if (entry->Retired)
                    continue;
                if (!entry->Registered) {
                    retryRegistration(*entry, *store, lock);
                    continue;
                }
                if (const auto reason = store->checkpointDue(entry->VersionNo, now))
                    writeCheckpoint(*entry, *store, *reason, now);
            }
        }

        void housekeepingLoop() {
            std::unique_lock lock(state.HousekeepingMutex);
            while (!state.HousekeepingStop) {
                state.HousekeepingWake.wait_for(lock, HOUSEKEEPING_PERIOD, [] { return state.HousekeepingStop; });
                if (state.HousekeepingStop)
                    break;
                lock.unlock();
                try {
                    housekeepingPass();
                } catch (const std::exception& e) {
                    logHistory(std::string("housekeeping failed: ") + e.what());
                } catch (...) {
                    logHistory("housekeeping failed");
                }
                lock.lock();
            }
        }
    }  // namespace

    namespace {
        // The session objects of both login methods are built the same way: only the way the communication
        // service gets its bearer token differs.
        std::shared_ptr<Session> newSession(rust::Str url, uint16_t port) {
            auto session = std::make_shared<Session>();
            session->Communication = std::make_unique<BACKEND_COMMUNICATION::CommunicationService>(
                std::string(url), static_cast<unsigned int>(port), "");
            return session;
        }

        // Adds the manager and replaces the current session (the old one is destroyed outside the lock).
        void installSession(std::shared_ptr<Session> session) {
            session->Manager = std::make_unique<DigitalTwin::DigitalTwinManager>(
                session->Communication.get(), &session->Mqtt, false);
            std::shared_ptr<Session> previous;
            {
                std::lock_guard lock(state.Mutex);
                previous = std::move(state.Current);
                state.Current = std::move(session);
            }
        }
    }  // namespace

    CallResult facade_login(rust::Str url, uint16_t port, rust::Str username, rust::Str password) noexcept {
        return guarded([&]() -> CallResult {
            auto session = newSession(url, port);
            session->Communication->setUserForLoginInBackend(std::string(username), std::string(password));
            installSession(std::move(session));
            return result(CODE_OK, "");
        });
    }

    CallResult facade_login_application_key(rust::Str url, uint16_t port, rust::Str application_key) noexcept {
        return guarded([&]() -> CallResult {
            auto session = newSession(url, port);
            // no request: the key is the bearer token; the exception text of a bad key never contains it
            session->Communication->setApplicationKey(std::string(application_key));
            session->UsesApplicationKey = true;
            {
                std::lock_guard lock(state.KeysMutex);
                const std::string key(application_key);
                if (std::find(state.ApplicationKeys.begin(), state.ApplicationKeys.end(), key) ==
                    state.ApplicationKeys.end())
                    state.ApplicationKeys.push_back(key);
            }
            installSession(std::move(session));
            return result(CODE_OK, "");
        });
    }

    TwinLoad facade_load_twin(rust::Str project_id, rust::Str twin_id) noexcept {
        TwinLoad out{CODE_UNKNOWN_ERROR, rust::String(), rust::String(), rust::Vec<TwinElement>()};
        auto fail = [&out](int32_t code, const std::string& message) {
            out.code = code;
            out.message = rust::String::lossy(message);
            return std::move(out);
        };
        bool keySession = false;
        try {
            const auto projectId = parseUuid(project_id);
            const auto twinId = parseUuid(twin_id);

            std::shared_ptr<Session> session;
            {
                std::lock_guard lock(state.Mutex);
                session = state.Current;
            }
            if (!session)
                return fail(CODE_NOT_LOGGED_IN, "not logged in to the backend");
            keySession = session->UsesApplicationKey;

            // The server has no endpoint for a single twin: list the twins of the project and pick one, like
            // DigitalTwinManager::downloadDigitalTwin.
            std::shared_ptr<SysMLv2::REST::DigitalTwin> twin;
            for (const auto& candidate : session->Communication->getAllDigitalTwinsForProjectWithId(projectId))
                if (candidate && candidate->getId() == twinId) {
                    twin = candidate;
                    break;
                }
            if (!twin)
                return fail(CODE_NOT_FOUND, "the project has no digital twin with this id");

            auto entry = std::make_shared<ModelEntry>();
            entry->Owner = session;
            entry->Model = std::make_unique<DigitalTwin::Model::DigitalTwinModel>(twin, session->Manager.get());
            const std::string name = entry->Model->digitalTwinName();

            // History: register the twin and its commit; for a commit that was loaded before, the last stored
            // state goes into the model BEFORE the initial values are read, so that Rust publishes the restored
            // values. A store that does not answer must not keep the twin from loading.
            const auto store = currentStore();
            // A load of a twin that is already loaded replaces its model. Two loads of the same twin must not
            // interleave (one could swap in a model and the other drop it unnoticed), so the section from here to
            // the swap is serialised per twin. The slow part, building the model, is already behind us.
            std::shared_ptr<std::mutex> loadMutex;
            {
                std::lock_guard lock(state.Mutex);
                auto& slot = state.LoadLocks[twinId];
                if (!slot)
                    slot = std::make_shared<std::mutex>();
                loadMutex = slot;
            }
            std::lock_guard loadLock(*loadMutex);

            // The old model is retired first: its final checkpoint is queued, and nothing it does afterwards is
            // recorded. If it is of the same commit, its (now frozen) state is the state of the twin: the new model
            // is restored from it, and the store need neither wait for its queue nor read the database. Otherwise
            // the state comes from the database, after registerTwin has waited for the queue to drain.
            Retiree retiree;
            retiree.TwinId = twinId;
            std::shared_ptr<ModelEntry> old;
            if (store) {
                {
                    std::lock_guard lock(state.Mutex);
                    const auto existing = state.Models.find(twinId);
                    if (existing != state.Models.end())
                        old = existing->second;
                }
                if (old && retireEntry(*old, store.get()))
                    retiree.Entry = old;
            }
            if (store) {
                try {
                    entry->ProjectIdText = boost::uuids::to_string(entry->Model->projectId());
                    entry->TwinIdText = boost::uuids::to_string(twinId);
                    entry->CommitIdText = boost::uuids::to_string(entry->Model->commitId());
                    entry->TwinName = name;
                    entry->Tree = entry->Model->getModelTree();

                    bool restoredFromOld = false;
                    if (old && boost::uuids::to_string(old->Model->commitId()) == entry->CommitIdText) {
                        std::vector<DigitalTwin::Model::VariableState> frozen;
                        {
                            std::lock_guard lock(old->ModelMutex);  // retired: it cannot change any more
                            frozen = old->Model->snapshotState();
                        }
                        const auto applied = entry->Model->restoreState(frozen);
                        restoredFromOld = true;
                        logHistory("twin " + entry->TwinIdText + ": " + std::to_string(applied) + " of " +
                                   std::to_string(frozen.size()) + " values taken over from the replaced model");
                    }

                    if (const auto registration = tryRegister(*store, *entry, !restoredFromOld)) {
                        entry->Registered = true;
                        entry->VersionNo = registration->VersionNo;
                        if (registration->KnownCommit && !restoredFromOld) {
                            const auto applied = entry->Model->restoreState(registration->LastState);
                            logHistory("twin " + entry->TwinIdText + ": restored " + std::to_string(applied) + " of " +
                                       std::to_string(registration->LastState.size()) + " stored values");
                        }
                    } else {
                        entry->NextRegistrationAttempt = {};  // the next housekeeping pass tries again
                    }
                } catch (const std::exception& e) {
                    entry->Registered = false;
                    entry->NextRegistrationAttempt = {};
                    logHistory(std::string("the twin runs without history for now: ") + e.what());
                }
            }

            const auto elements = entry->Model->getElementValueEntities();

            out.name = rust::String::lossy(name);
            for (const auto& [path, value] : elements) {
                if (!value)
                    continue;
                TwinElement element;
                element.path = rust::String::lossy(path);
                // One element that cannot be serialized must not fail the whole twin: it is passed on with an
                // empty initial value, which the Rust side rejects (classify), logs and counts as skipped.
                try {
                    element.initial_json = rust::String::lossy(value->getJson());
                } catch (const std::exception&) {
                    element.initial_json = rust::String();
                } catch (...) {
                    element.initial_json = rust::String();
                }
                out.elements.push_back(std::move(element));
            }

            // The state of the model after the restore is the first checkpoint of this load. (The entry is not
            // published yet, so nobody else uses it.)
            if (store && entry->Registered)
                writeCheckpoint(*entry, *store, History::CheckpointReason::Load, Clock::now());

            std::shared_ptr<ModelEntry> replaced;
            {
                std::lock_guard lock(state.Mutex);
                auto it = state.Models.find(twinId);
                if (it != state.Models.end()) {
                    replaced = std::move(it->second);
                    it->second = std::move(entry);
                } else {
                    state.Models.emplace(twinId, std::move(entry));
                }
            }
            retiree.Keep = true;
            out.code = CODE_OK;
            return std::move(out);
        } catch (const std::exception& e) {
            return fail(codeFor(e), describe(e, keySession));
        } catch (...) {
            return fail(CODE_UNKNOWN_ERROR, "unknown C++ exception");
        }
    }

    // Test/diagnostic support, no Structura server needed: one value entity per type, serialized by the same
    // getJson() the model uses, so the Rust tests can check the C++ -> classify contract offline.
    rust::Vec<TwinElement> facade_sample_initial_values() noexcept {
        rust::Vec<TwinElement> out;
        try {
            using namespace DigitalTwin::Communication;
            auto add = [&out](const char* path, const ValueEntity& value) {
                TwinElement element;
                element.path = rust::String(path);
                element.initial_json = rust::String::lossy(value.getJson());
                out.push_back(std::move(element));
            };
            add("real", RealValueEntity(1.5));
            add("real_zero", RealValueEntity(0.0));
            add("integer", IntegerValueEntity(-7));
            add("boolean", BooleanValueEntity(true));
            add("complex", ComplexValueEntity(1.0, -2.5));
            add("rational", RationalValueEntity(2L, 6L));
            add("positive", PositiveValueEntity(3u));
            add("string", StringValueEntity(std::string("hello")));
            add("string_empty", StringValueEntity(std::string()));
        } catch (...) {
            // a partial list makes the offline test fail on its element count
        }
        return out;
    }

    CallResult facade_register_function(rust::Str twin_id, rust::Str request_json) noexcept {
        return guarded([&]() -> CallResult {
            const auto twinId = parseUuid(twin_id);
            std::shared_ptr<ModelEntry> entry;
            {
                std::lock_guard lock(state.Mutex);
                auto it = state.Models.find(twinId);
                if (it != state.Models.end())
                    entry = it->second;
            }
            if (!entry)
                return result(CODE_NOT_FOUND, "no model loaded for this digital twin");

            DigitalTwin::Communication::FunctionRegistrationRequest request{std::string(request_json)};

            // DigitalTwinModel::registerFunction splits the name and indexes parts.size() - 1; an empty name
            // would underflow there, so it is rejected here.
            const auto qualifiedName = request.getQualifiedName();
            const auto parts = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(
                CPSBASELIB::STD_EXTENTION::StringExtention::replaceAll(qualifiedName, "::", "/"), '/');
            if (parts.empty())
                return result(CODE_INVALID_ARGUMENT, "the qualified name of the function is empty");

            std::lock_guard lock(entry->ModelMutex);
            entry->Model->registerFunction(&request);
            return result(CODE_OK, "");
        });
    }

    CallResult facade_release_twin(rust::Str twin_id) noexcept {
        return guarded([&]() -> CallResult {
            const auto twinId = parseUuid(twin_id);
            std::shared_ptr<ModelEntry> removed;
            {
                std::lock_guard lock(state.Mutex);
                auto it = state.Models.find(twinId);
                if (it != state.Models.end()) {
                    removed = std::move(it->second);
                    state.Models.erase(it);
                }
            }
            // Not needed to rebuild the state (the rows after the last checkpoint are part of it), but it keeps
            // the replay short.
            if (removed) {
                const auto store = currentStore();
                retireEntry(*removed, store.get());
            }
            return result(CODE_OK, "");
        });
    }

    CallResult facade_update_variable(rust::Str twin_id, rust::Str path, rust::Str payload_json,
                                      int64_t received_at_ms) noexcept {
        return guarded([&]() -> CallResult {
            const auto twinId = parseUuid(twin_id);
            std::shared_ptr<ModelEntry> entry;
            {
                std::lock_guard lock(state.Mutex);
                auto it = state.Models.find(twinId);
                if (it != state.Models.end())
                    entry = it->second;
            }
            if (!entry)
                return result(CODE_NOT_FOUND, "no model loaded for this digital twin");

            const auto receivedAt = fromMillis(received_at_ms);
            const auto store = currentStore();

            std::lock_guard modelLock(entry->ModelMutex);   // das Modell ist nicht thread-safe
            if (entry->Retired)
                return result(CODE_NOT_FOUND, "the model of this digital twin was replaced or released");
            const auto update = entry->Model->updateVariable(std::string(path), std::string(payload_json));
            // The change set is queued under the same lock, so the queue order is the order of the updates.
            if (store)
                recordHistory(*entry, *store, update, receivedAt);
            return result(CODE_OK, "");
        });
    }

    CallResult facade_store_configure(const StoreConfig& config) noexcept {
        return guarded([&]() -> CallResult {
            History::StoreSettings settings;
            settings.DatabaseUrl = std::string(config.database_url);
            if (settings.DatabaseUrl.empty())
                return result(CODE_INVALID_ARGUMENT, "the history database url is empty");
            settings.RetentionDays = config.retention_days;
            settings.CheckpointIntervalSeconds = config.checkpoint_interval_s;
            if (config.queue_capacity != 0)
                settings.QueueCapacityRows = config.queue_capacity;
            if (config.batch_size != 0)
                settings.BatchSizeRows = config.batch_size;
            if (config.flush_interval_ms != 0)
                settings.FlushIntervalMs = config.flush_interval_ms;

            std::lock_guard lock(state.Mutex);
            if (state.StoreConfigured)
                return result(CODE_INVALID_ARGUMENT, "the history store is already configured");
            // The constructor only starts the writer thread, it does not touch the database.
            auto store = std::make_shared<History::DigitalTwinStore>(std::move(settings));
            state.Housekeeper = std::thread([] { housekeepingLoop(); });
            state.HistoryStore = std::move(store);
            state.StoreConfigured = true;
            return result(CODE_OK, "");
        });
    }

    StoreStats facade_store_stats() noexcept {
        StoreStats out{};
        try {
            std::shared_ptr<History::DigitalTwinStore> store;
            {
                std::lock_guard lock(state.Mutex);
                store = state.HistoryStore;
            }
            if (!store)
                return out;
            const auto stats = store->stats();
            out.enabled = true;
            out.connected = stats.Connected;
            out.pending = stats.Pending;
            out.written = stats.Written;
            out.dropped = stats.Dropped;
            out.retries = stats.Retries;
            out.checkpoints = stats.Checkpoints;
        } catch (...) {
        }
        return out;
    }

    CallResult facade_store_shutdown(uint32_t timeout_ms) noexcept {
        return guarded([&]() -> CallResult {
            std::shared_ptr<History::DigitalTwinStore> store;
            std::thread housekeeper;
            {
                std::lock_guard lock(state.Mutex);
                if (!state.StoreConfigured || state.StoreShutdownStarted)
                    return result(CODE_OK, "");
                state.StoreShutdownStarted = true;
                store = state.HistoryStore;
                housekeeper = std::move(state.Housekeeper);
            }

            // 1. The housekeeping thread, so that it does not write while the store goes down. State::Mutex is not
            //    held: the thread takes it.
            {
                std::lock_guard lock(state.HousekeepingMutex);
                state.HousekeepingStop = true;
            }
            state.HousekeepingWake.notify_all();
            if (housekeeper.joinable())
                housekeeper.join();

            // 2. A final checkpoint of every twin that changed since its last one.
            std::vector<std::shared_ptr<ModelEntry>> entries;
            {
                std::lock_guard lock(state.Mutex);
                for (const auto& model : state.Models)
                    entries.push_back(model.second);
            }
            const auto now = Clock::now();
            for (const auto& entry : entries) {
                std::lock_guard lock(entry->ModelMutex);
                if (!entry->Retired && entry->Registered && entry->ChangedSinceCheckpoint)
                    writeCheckpoint(*entry, *store, History::CheckpointReason::Shutdown, now);
            }

            // 3. The store writes out its queue within the timeout and stops.
            store->shutdown(std::chrono::milliseconds(timeout_ms));
            {
                std::lock_guard lock(state.Mutex);
                state.StoreShutDown = true;
            }
            return result(CODE_OK, "");
        });
    }
}  // namespace dtrs
