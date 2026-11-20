//
// Tests of the login of MqttClientService: the credentials are sent in the CONNECT, and a refused login does not
// lead to endless reconnects. A small fake MQTT server (just enough of CONNECT/CONNACK) stands in for the broker.
//
// With DT_SMOKE_MQTT_PORT set, SmokeAgainstRunningBroker connects to a running DT server instead (see there).
//
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <Services/MqttClientService.h>

using namespace std::chrono_literals;
using DigitalTwin::Communication::MqttClientService;
using DigitalTwin::Communication::MqttProtocolVersion;

namespace {
    /**
     * Accepts TCP connections, reads one CONNECT per connection, remembers its credentials and answers with the
     * given CONNACK code (0 = accepted) in the format of the protocol level of the CONNECT. Refused connections are
     * closed after the CONNACK.
     */
    class FakeBroker {
    public:
        /** @param port 0 for any free port. */
        explicit FakeBroker(std::uint8_t connackCode, unsigned short port = 0) :
            Code(connackCode),
            Acceptor(Context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port)) {
            Acceptor.non_blocking(true);
            Worker = std::thread([this] { run(); });
        }

        ~FakeBroker() {
            Stop = true;
            Worker.join();
        }

        std::string port() const {
            return std::to_string(Acceptor.local_endpoint().port());
        }

        int connections() const { return Connections; }

        std::string user() {
            std::lock_guard<std::mutex> lock(Mutex);
            return User;
        }

        std::string password() {
            std::lock_guard<std::mutex> lock(Mutex);
            return Password;
        }

        int level() const { return Level; }

        /** The CONNACK code for connections from now on. */
        void setCode(std::uint8_t code) { Code = code; }

        /** Closes the connections that were accepted so far. */
        void dropConnections() { ++Generation; }

    private:
        static std::string readString(const std::vector<std::uint8_t>& body, std::size_t& pos) {
            if (pos + 2 > body.size())
                return {};
            const std::size_t length = (body[pos] << 8) | body[pos + 1];
            pos += 2;
            if (pos + length > body.size())
                return {};
            std::string text(body.begin() + pos, body.begin() + pos + length);
            pos += length;
            return text;
        }

        void run() {
            std::vector<std::thread> clients;
            while (!Stop) {
                boost::asio::ip::tcp::socket socket(Context);
                boost::system::error_code error;
                Acceptor.accept(socket, error);
                if (error) {
                    std::this_thread::sleep_for(10ms);
                    continue;
                }
                ++Connections;
                clients.emplace_back([this, s = std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket))] {
                    serve(*s);
                });
            }
            for (auto& client : clients)
                client.join();
        }

        void serve(boost::asio::ip::tcp::socket& socket) {
            socket.non_blocking(false);
            boost::system::error_code error;
            std::uint8_t first = 0;
            boost::asio::read(socket, boost::asio::buffer(&first, 1), error);
            std::size_t length = 0;
            for (int shift = 0; !error; shift += 7) {
                std::uint8_t b = 0;
                boost::asio::read(socket, boost::asio::buffer(&b, 1), error);
                length |= static_cast<std::size_t>(b & 0x7F) << shift;
                if ((b & 0x80) == 0)
                    break;
            }
            std::vector<std::uint8_t> body(length);
            if (!error)
                boost::asio::read(socket, boost::asio::buffer(body), error);
            if (error || (first >> 4) != 1)
                return;

            std::size_t pos = 0;
            readString(body, pos); // protocol name
            const int level = body[pos++];
            const std::uint8_t flags = body[pos++];
            pos += 2; // keep alive
            if (level == 5) {
                std::size_t properties = 0;
                for (int shift = 0;; shift += 7) {
                    const std::uint8_t b = body[pos++];
                    properties |= static_cast<std::size_t>(b & 0x7F) << shift;
                    if ((b & 0x80) == 0)
                        break;
                }
                pos += properties;
            }
            readString(body, pos); // client id
            std::string user;
            std::string password;
            if (flags & 0x80)
                user = readString(body, pos);
            if (flags & 0x40)
                password = readString(body, pos);
            {
                std::lock_guard<std::mutex> lock(Mutex);
                User = user;
                Password = password;
            }
            Level = level;

            const std::uint8_t code = Code;
            const int generation = Generation;
            std::vector<std::uint8_t> connack = level == 5
                ? std::vector<std::uint8_t>{0x20, 3, 0, code, 0}
                : std::vector<std::uint8_t>{0x20, 2, 0, code};
            boost::asio::write(socket, boost::asio::buffer(connack), error);
            if (code != 0)
                return; // refused: the connection is closed
            // accepted: keep the connection open until the client goes away
            std::uint8_t sink[256];
            while (!Stop && !error && generation == Generation) {
                socket.non_blocking(true);
                socket.read_some(boost::asio::buffer(sink), error);
                if (error == boost::asio::error::would_block) {
                    error.clear();
                    std::this_thread::sleep_for(10ms);
                }
            }
        }

        std::atomic<std::uint8_t> Code;
        std::atomic<int> Generation{0};
        boost::asio::io_context Context;
        boost::asio::ip::tcp::acceptor Acceptor;
        std::atomic<bool> Stop{false};
        std::atomic<int> Connections{0};
        std::atomic<int> Level{0};
        std::mutex Mutex;
        std::string User;
        std::string Password;
        std::thread Worker;
    };

    template<typename Condition>
    bool waitFor(Condition condition, std::chrono::milliseconds limit) {
        const auto end = std::chrono::steady_clock::now() + limit;
        while (std::chrono::steady_clock::now() < end) {
            if (condition())
                return true;
            std::this_thread::sleep_for(20ms);
        }
        return condition();
    }
}

class MqttClientServiceLogin : public ::testing::TestWithParam<MqttProtocolVersion> {};

TEST_P(MqttClientServiceLogin, SendsUserNameAndPassword) {
    FakeBroker broker(0);
    std::atomic<bool> connected{false};
    MqttClientService service("127.0.0.1", broker.port(), "login-test", GetParam(), "alice", "s3cret");
    service.setConnectedHandler([&connected] { connected = true; });
    service.start();

    ASSERT_TRUE(waitFor([&] { return connected.load(); }, 10s));
    EXPECT_EQ(broker.user(), "alice");
    EXPECT_EQ(broker.password(), "s3cret");
    EXPECT_EQ(broker.level(), GetParam() == MqttProtocolVersion::V5 ? 5 : 4);
    EXPECT_FALSE(service.isAuthenticationFailed());
    service.stop();
}

TEST_P(MqttClientServiceLogin, SendsNoCredentialsWithoutUserName) {
    FakeBroker broker(0);
    std::atomic<bool> connected{false};
    MqttClientService service("127.0.0.1", broker.port(), "login-test", GetParam());
    service.setConnectedHandler([&connected] { connected = true; });
    service.start();

    ASSERT_TRUE(waitFor([&] { return connected.load(); }, 10s));
    EXPECT_EQ(broker.user(), "");
    EXPECT_EQ(broker.password(), "");
    service.stop();
}

TEST_P(MqttClientServiceLogin, RefusedLoginStopsTheConnectAttempts) {
    // 3.1.1: return code 4 (bad user name or password), MQTT 5: reason code 0x86
    const std::uint8_t code = GetParam() == MqttProtocolVersion::V5 ? 0x86 : 0x04;
    FakeBroker broker(code);
    std::mutex mutex;
    std::vector<std::string> errors;
    MqttClientService service("127.0.0.1", broker.port(), "login-test", GetParam(), "alice", "s3cret");
    service.setErrorHandler([&](const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        errors.push_back(message);
    });
    service.start();

    ASSERT_TRUE(waitFor([&] { return service.isAuthenticationFailed(); }, 10s));
    // Paho would retry after one second at the earliest and then with growing pauses
    std::this_thread::sleep_for(5s);
    EXPECT_EQ(broker.connections(), 1);
    EXPECT_FALSE(service.isConnected());
    std::lock_guard<std::mutex> lock(mutex);
    ASSERT_FALSE(errors.empty());
    for (const auto& message : errors) {
        EXPECT_EQ(message.find("s3cret"), std::string::npos) << "the password is in: " << message;
    }
}

TEST_P(MqttClientServiceLogin, RefusalOnALaterAttemptStopsToo) {
    // The server is not there when the client starts; when it comes up it refuses the login.
    const std::uint8_t code = GetParam() == MqttProtocolVersion::V5 ? 0x86 : 0x04;
    std::string port;
    {
        FakeBroker probe(0);
        port = probe.port();
    }
    std::mutex mutex;
    int refusals = 0;
    MqttClientService service("127.0.0.1", port, "login-test", GetParam(), "alice", "wrong");
    service.setErrorHandler([&](const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        if (message.find("login at the DT server failed") != std::string::npos)
            ++refusals;
    });
    service.start();
    std::this_thread::sleep_for(500ms); // the first attempt failed: nobody listens
    EXPECT_FALSE(service.isAuthenticationFailed());

    FakeBroker broker(code, static_cast<unsigned short>(std::stoi(port)));
    ASSERT_TRUE(waitFor([&] { return service.isAuthenticationFailed(); }, 15s));
    std::this_thread::sleep_for(5s);
    EXPECT_EQ(broker.connections(), 1);
    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_EQ(refusals, 1);
}

TEST_P(MqttClientServiceLogin, RefusalAfterAConnectionLossStops) {
    // Connected, then the connection is lost and the server no longer accepts the login (password changed).
    FakeBroker broker(0);
    std::atomic<bool> connected{false};
    std::mutex mutex;
    int refusals = 0;
    MqttClientService service("127.0.0.1", broker.port(), "login-test", GetParam(), "alice", "s3cret");
    service.setConnectedHandler([&connected] { connected = true; });
    service.setErrorHandler([&](const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        if (message.find("login at the DT server failed") != std::string::npos)
            ++refusals;
    });
    service.start();
    ASSERT_TRUE(waitFor([&] { return connected.load(); }, 10s));

    broker.setCode(GetParam() == MqttProtocolVersion::V5 ? 0x86 : 0x04);
    broker.dropConnections();
    ASSERT_TRUE(waitFor([&] { return service.isAuthenticationFailed(); }, 15s));
    std::this_thread::sleep_for(4s);
    EXPECT_EQ(broker.connections(), 2); // the first connection and the refused attempt
    EXPECT_FALSE(service.isConnected());
    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_EQ(refusals, 1);
}

TEST_P(MqttClientServiceLogin, StopCancelsPendingAttempts) {
    // Nobody listens: the client keeps trying with pauses. After stop() there is no further attempt, also not
    // from a server that appears later.
    std::string port;
    {
        FakeBroker probe(0);
        port = probe.port();
    }
    MqttClientService service("127.0.0.1", port, "login-test", GetParam(), "alice", "pw");
    std::atomic<int> failures{0};
    service.setErrorHandler([&failures](const std::string&) { ++failures; });
    service.start();
    // the first attempt has failed (on Windows a refused connect can take a while), the next one is pending
    ASSERT_TRUE(waitFor([&] { return failures.load() >= 1; }, 10s));
    service.stop();
    FakeBroker broker(0, static_cast<unsigned short>(std::stoi(port)));
    std::this_thread::sleep_for(2500ms);
    EXPECT_EQ(broker.connections(), 0);
}

TEST_P(MqttClientServiceLogin, UnavailableServerIsNoRefusedLogin) {
    // 3.1.1: return code 3 (server unavailable), MQTT 5: reason code 0x88
    FakeBroker broker(GetParam() == MqttProtocolVersion::V5 ? 0x88 : 0x03);
    MqttClientService service("127.0.0.1", broker.port(), "login-test", GetParam(), "alice", "s3cret");
    service.start();

    // it tries again with pauses
    ASSERT_TRUE(waitFor([&] { return broker.connections() >= 2; }, 10s));
    EXPECT_FALSE(service.isAuthenticationFailed());
    service.stop();
}

INSTANTIATE_TEST_SUITE_P(Versions, MqttClientServiceLogin,
                         ::testing::Values(MqttProtocolVersion::V3_1_1, MqttProtocolVersion::V5));

/**
 * Manual smoke test against a running DT server, skipped unless DT_SMOKE_MQTT_PORT is set. Further variables:
 * DT_SMOKE_MQTT_VERSION ("3.1.1" or "5"), DT_SMOKE_MQTT_USER, DT_SMOKE_MQTT_PASSWORD and DT_SMOKE_EXPECT ("accepted"
 * or "refused"). Waits for the result, then, for a refused login, for 6 seconds to see that there is no reconnect.
 */
TEST(MqttClientServiceSmoke, SmokeAgainstRunningBroker) {
    const char* port = std::getenv("DT_SMOKE_MQTT_PORT");
    if (port == nullptr)
        GTEST_SKIP() << "DT_SMOKE_MQTT_PORT is not set";
    auto env = [](const char* name) {
        const char* value = std::getenv(name);
        return std::string(value == nullptr ? "" : value);
    };
    const auto version = env("DT_SMOKE_MQTT_VERSION") == "5" ? MqttProtocolVersion::V5 : MqttProtocolVersion::V3_1_1;
    const bool expectAccepted = env("DT_SMOKE_EXPECT") != "refused";

    std::atomic<bool> connected{false};
    std::atomic<int> errors{0};
    MqttClientService service("127.0.0.1", port, "smoke-test", version, env("DT_SMOKE_MQTT_USER"),
                              env("DT_SMOKE_MQTT_PASSWORD"));
    service.setConnectedHandler([&connected] { connected = true; });
    service.setErrorHandler([&errors](const std::string& message) {
        // connect failures before the server is up do not count
        if (message.find("login at the DT server failed") != std::string::npos)
            ++errors;
    });
    service.start();

    if (expectAccepted) {
        EXPECT_TRUE(waitFor([&] { return connected.load(); }, 15s));
        EXPECT_FALSE(service.isAuthenticationFailed());
    } else {
        EXPECT_TRUE(waitFor([&] { return service.isAuthenticationFailed(); }, 15s));
        std::this_thread::sleep_for(6s);
        EXPECT_FALSE(connected.load());
        // the refusal is reported once; a reconnect loop would report again and again
        // (the server may have been started after the client: connect failures before that are not counted)
        EXPECT_EQ(errors.load(), 1);
    }
    service.stop();
}
