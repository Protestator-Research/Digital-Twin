//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttClientService.h"

#include <algorithm>
#include <iostream>
#include <exception>
#include <random>
#include <sstream>
#include <iomanip>
#include <utility>
#include <vector>

namespace DigitalTwin::Communication {
    namespace {
        constexpr int QUALITY_OF_SERVICE = 0;
        constexpr std::chrono::seconds KEEP_ALIVE(60);
        constexpr std::chrono::seconds DISCONNECT_TIMEOUT(2);
        constexpr std::chrono::seconds FIRST_RETRY_DELAY(1);
        constexpr std::chrono::seconds MAX_RETRY_DELAY(60);

        /**
         * Paho wants a client id for MQTT 3.1.1 and the server assigns different ones for every connection otherwise,
         * so an empty id is replaced by a unique one.
         */
        std::string makeClientId(std::string clientId) {
            if (!clientId.empty())
                return clientId;
            std::random_device device;
            std::ostringstream stream;
            stream << "dt-client-" << std::hex << std::setfill('0') << std::setw(8) << device() << std::setw(8) << device();
            return stream.str();
        }

        std::string makeServerUri(const std::string& server, const std::string& port) {
            std::string uri = server.find("://") == std::string::npos ? "tcp://" + server : server;
            if (!port.empty())
                uri += ":" + port;
            return uri;
        }

        mqtt::create_options makeCreateOptions(MqttProtocolVersion version) {
            if (version == MqttProtocolVersion::V5)
                return mqtt::create_options(MQTTVERSION_5);
            return mqtt::create_options();
        }

        mqtt::connect_options makeConnectOptions(MqttProtocolVersion version, const std::string& username,
                                                 const std::string& password) {
            mqtt::connect_options options;
            if (version == MqttProtocolVersion::V5) {
                options = mqtt::connect_options_builder::v5()
                    .keep_alive_interval(KEEP_ALIVE)
                    .clean_start(true)
                    .finalize();
            } else {
                // Only 3.1.1: no fallback to MQTT 3.1, the DT server does not speak it.
                options = mqtt::connect_options_builder::v3()
                    .mqtt_version(MQTTVERSION_3_1_1)
                    .keep_alive_interval(KEEP_ALIVE)
                    .clean_session(true)
                    .finalize();
            }
            // The DT server checks them at the SysML backend. Without a user name the CONNECT carries no
            // credentials at all (a password alone is not allowed in MQTT 3.1.1).
            if (!username.empty()) {
                options.set_user_name(username);
                if (!password.empty())
                    options.set_password(password);
            }
            return options;
        }

        /**
         * True if the CONNACK of a failed connect says that the login was refused: 3.1.1 return code 4 (bad user
         * name or password) or 5 (not authorized), MQTT 5 reason code 0x86 (bad user name or password) or 0x87
         * (not authorized). Return code 3 / reason 0x88 (server unavailable) is a temporary problem and no reason
         * to give up.
         */
        bool isLoginRefused(MqttProtocolVersion version, const mqtt::token& token) {
            const int returnCode = token.get_return_code();
            if (version == MqttProtocolVersion::V5) {
                const int reason = static_cast<int>(token.get_reason_code());
                return reason == 0x86 || reason == 0x87 || returnCode == 0x86 || returnCode == 0x87;
            }
            return returnCode == 4 || returnCode == 5;
        }
    }

    /**
     * Reports a failed connect attempt. With automatic reconnect Paho keeps retrying (also after a failed first
     * attempt), so this is a progress report, not the end.
     */
    class MqttClientService::ConnectListener : public mqtt::iaction_listener {
    public:
        explicit ConnectListener(std::function<void(const mqtt::token&)> failed) : Failed(std::move(failed)) {}

        void on_failure(const mqtt::token& token) override {
            Failed(token);
        }

        void on_success(const mqtt::token&) override {}

    private:
        std::function<void(const mqtt::token&)> Failed;
    };

    MqttClientService::MqttClientService(std::string server, std::string port, std::string clientId,
                                         MqttProtocolVersion version, std::string username, std::string password) :
        Version(version),
        ClientId(makeClientId(std::move(clientId))),
        Username(std::move(username)),
        Password(std::move(password)),
        Started(false),
        AuthenticationFailed(false),
        Listener(std::make_unique<ConnectListener>([this](const mqtt::token& token) { onConnectFailed(token); })),
        Client(makeServerUri(server, port), ClientId, makeCreateOptions(version)) {
        Client.set_connected_handler([this](const std::string&) {
            onConnected();
        });
        Client.set_connection_lost_handler([this](const std::string& cause) {
            onConnectionLost(cause);
        });
        Client.set_disconnected_handler([](const mqtt::properties&, mqtt::ReasonCode reason) {
            // MQTT 5 only: the server announces why it ends the connection (e.g. 0x8B server shutting down).
            std::cerr << "MQTT server disconnected the client: " << mqtt::to_string(reason) << std::endl;
        });
        Client.set_message_callback([this](mqtt::const_message_ptr message) {
            onMessage(std::move(message));
        });
    }

    MqttClientService::~MqttClientService() {
        stop();
    }

    void MqttClientService::start() {
        if (Started.exchange(true))
            return;
        AuthenticationFailed = false;
        {
            std::lock_guard<std::mutex> lock(RetryMutex);
            Stopping = false;
            AttemptWanted = true;
            AttemptDue = std::chrono::steady_clock::now();
            Backoff = FIRST_RETRY_DELAY;
        }
        RetryThread = std::thread([this] { retryLoop(); });
    }

    void MqttClientService::retryLoop() {
        std::unique_lock<std::mutex> lock(RetryMutex);
        while (true) {
            RetryCv.wait(lock, [this] { return Stopping || AttemptWanted; });
            if (Stopping)
                return;
            // Waits for the due time; a stop() or a newer schedule wakes it up.
            if (RetryCv.wait_until(lock, AttemptDue, [this] { return Stopping; }))
                return;
            AttemptWanted = false;
            lock.unlock();
            attemptConnect();
            lock.lock();
        }
    }

    void MqttClientService::attemptConnect() {
        try {
            // Every attempt gets the listener, so that every refusal is seen.
            Client.connect(makeConnectOptions(Version, Username, Password), nullptr, *Listener);
        } catch (const std::exception& e) {
            reportError(std::string("MQTT connect failed: ") + e.what());
            scheduleRetry();
        }
    }

    void MqttClientService::scheduleRetry() {
        std::lock_guard<std::mutex> lock(RetryMutex);
        if (Stopping || AuthenticationFailed)
            return;
        AttemptDue = std::chrono::steady_clock::now() + Backoff;
        Backoff = std::min(Backoff * 2, MAX_RETRY_DELAY);
        AttemptWanted = true;
        RetryCv.notify_all();
    }

    void MqttClientService::onConnectionLost(const std::string& cause) {
        reportError("MQTT connection lost" + (cause.empty() ? std::string() : ": " + cause));
        scheduleRetry();
    }

    void MqttClientService::stop() {
        {
            std::lock_guard<std::mutex> lock(RetryMutex);
            Stopping = true;
            AttemptWanted = false;
            RetryCv.notify_all();
        }
        if (RetryThread.joinable())
            RetryThread.join();
        if (!Started.exchange(false))
            return;
        try {
            // Also ends a connect that is still running; on a client that is not connected this just fails.
            Client.disconnect()->wait_for(DISCONNECT_TIMEOUT);
        } catch (...) {
        }
    }

    void MqttClientService::publish(std::string topic, std::string payload) {
        if (topic.empty() || payload.empty())
            return;
        if (!Client.is_connected()) {
            std::cerr << "MQTT message for topic '" << topic << "' dropped: not connected" << std::endl;
            return;
        }
        try {
            auto message = mqtt::make_message(topic, payload);
            message->set_qos(QUALITY_OF_SERVICE);
            Client.publish(std::move(message));
        } catch (const std::exception& e) {
            std::cerr << "MQTT publish failed: " << e.what() << std::endl;
        }
    }

    void MqttClientService::subscribe(std::string topic, Callback callback) {
        {
            std::lock_guard<std::mutex> lock(SubscriptionsMutex);
            Subscriptions[topic] = std::move(callback);
        }
        // Not connected yet: the connected handler sends all subscriptions.
        if (Client.is_connected())
            subscribeOnServer(topic);
    }

    bool MqttClientService::isConnected() const {
        return Client.is_connected();
    }

    bool MqttClientService::isAuthenticationFailed() const {
        return AuthenticationFailed;
    }

    MqttProtocolVersion MqttClientService::protocolVersion() const {
        return Version;
    }

    void MqttClientService::setConnectedHandler(std::function<void()> handler) {
        ConnectedHandler = std::move(handler);
    }

    void MqttClientService::setErrorHandler(std::function<void(const std::string&)> handler) {
        ErrorHandler = std::move(handler);
    }

    void MqttClientService::onConnectFailed(const mqtt::token& token) {
        if (isLoginRefused(Version, token)) {
            // Do not try again: the server asks the SysML backend for every attempt, and the credentials will not
            // become valid by themselves. Neither the user name nor the password is part of the message.
            {
                // no further attempt, see scheduleRetry()
                std::lock_guard<std::mutex> lock(RetryMutex);
                AuthenticationFailed = true;
                AttemptWanted = false;
            }
            reportError("MQTT login at the DT server failed: user name or password refused (return code " +
                        std::to_string(token.get_return_code()) + ")");
            return;
        }
        reportError("MQTT connect failed: " + token.get_error_message() + " (return code " +
                    std::to_string(token.get_return_code()) + ")");
        scheduleRetry();
    }

    void MqttClientService::reportError(const std::string& message) {
        std::cerr << message << std::endl;
        if (ErrorHandler) {
            try {
                ErrorHandler(message);
            } catch (...) {
            }
        }
    }

    void MqttClientService::subscribeOnServer(const std::string& topic) {
        try {
            // Does not wait for the SUBACK, this is also called from the Paho callback thread.
            Client.subscribe(topic, QUALITY_OF_SERVICE);
        } catch (const std::exception& e) {
            std::cerr << "MQTT subscribe to '" << topic << "' failed: " << e.what() << std::endl;
        }
    }

    void MqttClientService::onConnected() {
        // Called after every successful connect (the session is clean, so the server has forgotten the
        // subscriptions).
        {
            std::lock_guard<std::mutex> lock(RetryMutex);
            Backoff = FIRST_RETRY_DELAY;
        }
        std::vector<std::string> topics;
        {
            std::lock_guard<std::mutex> lock(SubscriptionsMutex);
            topics.reserve(Subscriptions.size());
            for (const auto& entry : Subscriptions)
                topics.push_back(entry.first);
        }
        for (const auto& topic : topics)
            subscribeOnServer(topic);
        if (ConnectedHandler) {
            try {
                ConnectedHandler();
            } catch (const std::exception& e) {
                std::cerr << "Exception in the MQTT connected handler: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown exception in the MQTT connected handler" << std::endl;
            }
        }
    }

    void MqttClientService::onMessage(mqtt::const_message_ptr message) {
        const std::string topic = message->get_topic();
        const std::string payload = message->get_payload_str();

        // Exact match, like before. The callback is copied, so that it can subscribe again without a deadlock.
        Callback callback;
        {
            std::lock_guard<std::mutex> lock(SubscriptionsMutex);
            if (const auto it = Subscriptions.find(topic); it != Subscriptions.end())
                callback = it->second;
        }
        if (!callback)
            return;
        try {
            callback(topic, payload);
        } catch (const std::exception& e) {
            std::cerr << "Exception in subscription callback for topic '" << topic << "': " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in subscription callback for topic '" << topic << "'" << std::endl;
        }
    }
}
