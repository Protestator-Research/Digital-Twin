//
// Created by Moritz Herzog on 06.08.24.
//

#ifndef DIGITALTWIN_MQTTCLIENTSERVICE_H
#define DIGITALTWIN_MQTTCLIENTSERVICE_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <mqtt/async_client.h>
#include <mqtt/ssl_options.h>

#include "../cpp_physical_twin_communication_global.h"
#include "IMqttClientService.h"
#include "MqttTlsOptions.h"

namespace DigitalTwin::Communication {
    /**
     * MQTT client on top of the Paho C++ library (mqtt::async_client). Speaks MQTT 3.1.1 or MQTT 5, publishes and
     * subscribes with QoS 0. Subscriptions are remembered and sent again after every (re)connect.
     *
     * The reconnect is done here and not by Paho (automatic reconnect), because Paho reports only the first connect
     * attempt to a listener: a login that is refused on a later attempt would be retried forever without anybody
     * noticing, and every attempt costs the server a request at the SysML backend. A worker thread connects again
     * with growing pauses (1 s up to 60 s) after a failed connect or a lost connection, and stops for good when the
     * server refuses the login.
     *
     * publish(), subscribe(), start() and stop() can be called from any thread, none of them blocks (except stop(),
     * which waits a short time for the DISCONNECT). The callbacks run on threads of the Paho library.
     */
    class CPPPHYSICALTWINCOMMUNICATION_EXPORT MqttClientService : public IMqttClientService {
    public:
        /**
         * Deleted, because this is not needed!
         */
        MqttClientService() = delete;
        MqttClientService(const MqttClientService&) = delete;
        MqttClientService& operator=(const MqttClientService&) = delete;

        /**
         * Constructor needed for the connection to a Server. Nothing is connected before start().
         * @param server The Server URL or IP. A scheme (tcp://, ssl://, ...) is kept, without one tcp:// is used (or
         * ssl:// with TLS switched on). See makeMqttEndpoint() for the schemes and for a scheme that contradicts the
         * TLS option.
         * @param port The Port on the server, where the DT Server is running.
         * @param clientId The MQTT client id. An empty id is replaced by a generated unique one.
         * @param version The MQTT protocol version. It has to be enabled on the server
         * (run.mqtt.protocol_version of DTServerRust).
         * @param username User name of the login at the DT server (the SysML user). Not sent when empty.
         * @param password Password of the login. Not sent when empty. Never written to a log or an error message.
         * @param tls TLS settings. With the default (TLS off, no ssl:// scheme) the connection is plain TCP as before.
         * With TLS the server certificate is always verified, including the host name, there is no way to switch that
         * off. A problem with TLS (certificate not trusted, wrong host name, expired, no TLS on the server, TLS on a
         * plain server, unreadable CA file) is reported by the error handler with a text that starts with "TLS", and
         * the connect is retried with the usual growing pauses, except for an unusable CA file, which stops the
         * client (see isTlsConfigurationFailed()). IPv6 literals ("[::1]") do not work with TLS, see MqttEndpoint.
         */
        MqttClientService(std::string server, std::string port, std::string clientId,
                          MqttProtocolVersion version = MqttProtocolVersion::V3_1_1,
                          std::string username = std::string(), std::string password = std::string(),
                          MqttTlsOptions tls = MqttTlsOptions());

        ~MqttClientService() override;

        /**
         * Connects asynchronously and returns immediately. The connection is re-established automatically after it
         * was lost. Idempotent.
         */
        void start();
        /**
         * Disconnects (waits at most a few seconds for it) and cancels pending connect attempts. After it returns no
         * further attempt is made. Idempotent.
         */
        void stop();

        /**
         * Publishes with QoS 0. Empty topics and payloads are ignored; while not connected the message is dropped.
         */
        void publish(std::string topic, std::string payload) override;

        void subscribe(std::string topic, std::function<void(std::string topic, std::string payload)> callback) override;

        bool isConnected() const;

        /**
         * True if the server refused the login (wrong user name or password, not authorized). The client then
         * stops trying to connect, because every attempt is checked at the SysML backend by the server. A new
         * start() after stop() tries again.
         */
        bool isAuthenticationFailed() const;

        /**
         * True if the CA file given for TLS cannot be used (missing, unreadable, no certificate in it). The client
         * then stops trying to connect, because retrying cannot help. Once it has connected successfully, a CA file that
         * fails the check later (network share, virus scanner) is only reported and retried with the usual pauses. A new start() after stop() tries again.
         */
        bool isTlsConfigurationFailed() const;

        /**
         * True if a connect attempt failed because of TLS (certificate not trusted, wrong host name, expired
         * certificate, TLS handshake not possible, or the CA file is unusable) and there was no successful connect
         * since. A connect failure without a message of OpenSSL (server not reachable, host name not matching) only counts
         * while this object has never connected; after the first successful connect such failures are ordinary outages and
         * do not set it (a restart of the server must not look like a certificate problem). It stays set while the following
         * attempts are made, so that an application can tell "a TLS problem
         * is being retried" from "connected". The connect is retried with growing pauses (1 s up to 60 s) as for
         * any other failure.
         */
        bool hasTlsProblem() const;

        /** True if the connection uses TLS (ssl:// URI). */
        bool usesTls() const;

        /** The URI of the server as Paho gets it (scheme, host and port). */
        std::string serverUri() const;

        MqttProtocolVersion protocolVersion() const;

        /**
         * Handler that is called after every successful (re)connect, after the subscriptions were sent. It runs on
         * a thread of the Paho library. Has to be set before start().
         */
        void setConnectedHandler(std::function<void()> handler);

        /**
         * Handler for connection problems (failed connect, lost connection), with a text. Besides this they are
         * written to stderr. Runs on a thread of the Paho library. Has to be set before start().
         */
        void setErrorHandler(std::function<void(const std::string&)> handler);

    private:
        using Callback = std::function<void(std::string topic, std::string payload)>;

        class ConnectListener;

        void subscribeOnServer(const std::string& topic);
        void onConnected();
        void onConnectFailed(const mqtt::token& token);
        void onConnectionLost(const std::string& cause);
        void scheduleRetry();
        void retryLoop();
        void attemptConnect();
        void reportError(const std::string& message);
        bool onTlsConnectFailed(const mqtt::token& token);
        void reportTlsError(const std::string& message);
        void recordTlsDetail(const std::string& detail);
        void onMessage(mqtt::const_message_ptr message);

        // Everything the Paho callbacks touch is declared before Client, so that it still exists while the
        // client is destroyed (which waits for running callbacks).
        MqttProtocolVersion Version;
        std::string ClientId;
        std::string Username;
        std::string Password;
        std::atomic<bool> Started;
        std::atomic<bool> AuthenticationFailed;
        MqttTlsOptions Tls;
        MqttEndpoint Endpoint;
        std::atomic<bool> TlsConfigurationFailed{false};
        std::atomic<bool> TlsProblem{false};
        // Set by the first successful connect and kept: what looks like a TLS problem before it (nothing was ever trusted)
        // is only an outage after it.
        std::atomic<bool> EverConnected{false};
        // The messages of OpenSSL about the current connect attempt (set on a Paho thread).
        std::mutex TlsDetailMutex;
        std::string TlsDetail;
        // Reconnect state. The worker thread is joined in stop(), which the destructor calls, before Client goes away.
        std::mutex RetryMutex;
        std::condition_variable RetryCv;
        std::thread RetryThread;
        bool Stopping = true;
        bool AttemptWanted = false;
        std::chrono::steady_clock::time_point AttemptDue;
        std::chrono::seconds Backoff{1};
        std::mutex SubscriptionsMutex;
        std::unordered_map<std::string, Callback> Subscriptions;
        std::function<void()> ConnectedHandler;
        std::function<void(const std::string&)> ErrorHandler;
        std::unique_ptr<ConnectListener> Listener;
        mqtt::async_client Client;
    };
}

#endif //DIGITALTWIN_MQTTCLIENTSERVICE_H
