#include "dt_facade.h"

#include "dt-server/src/twin/ffi.rs.h"

#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <BECommunicationService.h>
#include <BaseFuctions/StringExtention.hpp>
#include <DigitalTwinManager.h>
#include <MQTT/entities/FunctionRegistrationRequest.hpp>
#include <Model/DigitalTwinModel.h>
#include <Services/IMqttClientService.h>
#include <entities/DigitalTwin.h>
#include <sysmlv2/service/online/HttpException.h>

namespace dtrs {
    namespace {
        // The model library publishes through this interface only in DigitalTwinManager::generateMQTTInterface and
        // addDigitalTwinAndCreateModel, neither of which the facade calls. The manager still needs an instance.
        class NoopMqttClient : public DigitalTwin::Communication::IMqttClientService {
        public:
            void publish(std::string, std::string) override {}
        };

        // One login. Models keep it alive through a shared_ptr, so a new login does not pull the manager away
        // from a model that is still in use.
        struct Session {
            std::unique_ptr<BACKEND_COMMUNICATION::CommunicationService> Communication;
            NoopMqttClient Mqtt;
            std::unique_ptr<DigitalTwin::DigitalTwinManager> Manager;
        };

        struct ModelEntry {
            std::shared_ptr<Session> Owner;  // declared first: destroyed after the model
            std::unique_ptr<DigitalTwin::Model::DigitalTwinModel> Model;
            std::mutex ModelMutex;  // the model is not thread safe
        };

        struct State {
            std::mutex Mutex;
            std::shared_ptr<Session> Current;
            std::map<boost::uuids::uuid, std::shared_ptr<ModelEntry>> Models;
        };

        // Deliberately leaked: a blocking call may still run on a Rust thread while the process exits, and
        // destroying the mutex, the sessions and the models (static destruction order, exit handlers) under it
        // would be a race. The operating system reclaims the memory.
        State& state = *new State;

        std::string describe(const std::exception& e) {
            try {
                return BACKEND_COMMUNICATION::CommunicationService::describeBackendError(e);
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
    }  // namespace

    CallResult facade_login(rust::Str url, uint16_t port, rust::Str username, rust::Str password) noexcept {
        return guarded([&]() -> CallResult {
            auto session = std::make_shared<Session>();
            session->Communication = std::make_unique<BACKEND_COMMUNICATION::CommunicationService>(
                std::string(url), static_cast<unsigned int>(port), "");
            session->Communication->setUserForLoginInBackend(std::string(username), std::string(password));
            session->Manager = std::make_unique<DigitalTwin::DigitalTwinManager>(
                session->Communication.get(), &session->Mqtt, false);
            std::shared_ptr<Session> previous;
            {
                std::lock_guard lock(state.Mutex);
                previous = std::move(state.Current);
                state.Current = std::move(session);
            }
            return result(CODE_OK, "");
        });
    }

    TwinLoad facade_load_twin(rust::Str project_id, rust::Str twin_id) noexcept {
        TwinLoad out{CODE_UNKNOWN_ERROR, rust::String(), rust::String(), rust::Vec<rust::String>()};
        auto fail = [&out](int32_t code, const std::string& message) {
            out.code = code;
            out.message = rust::String::lossy(message);
            return std::move(out);
        };
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
            const std::vector<std::string> elements = entry->Model->getElementStrings();

            out.name = rust::String::lossy(name);
            for (const auto& element : elements)
                out.elements.push_back(rust::String::lossy(element));

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
            out.code = CODE_OK;
            return std::move(out);
        } catch (const std::exception& e) {
            return fail(codeFor(e), describe(e));
        } catch (...) {
            return fail(CODE_UNKNOWN_ERROR, "unknown C++ exception");
        }
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
            return result(CODE_OK, "");
        });
    }
}  // namespace dtrs
