//
// Tests of TLS for MqttClientService: the URI and scheme handling, the Paho SSL options, the check of the CA file and
// the reaction to a server that does not speak TLS (a small raw TCP server stands in for it).
//
// With DT_TLS_E2E_DIR set, the tests of MqttClientServiceTlsE2E connect to running DTServerRust brokers with TLS, see
// there.
//
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <Services/MqttClientService.h>
#include <Services/MqttTlsOptions.h>

using namespace std::chrono_literals;
using DigitalTwin::Communication::checkMqttCaFile;
using DigitalTwin::Communication::makeMqttEndpoint;
using DigitalTwin::Communication::makeMqttSslOptions;
using DigitalTwin::Communication::MqttClientService;
using DigitalTwin::Communication::MqttTlsOptions;

namespace {
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

    /** Collects the errors of a service. */
    class ErrorLog {
    public:
        void attach(MqttClientService& service) {
            service.setErrorHandler([this](const std::string& message) {
                std::lock_guard<std::mutex> lock(Mutex);
                Messages.push_back(message);
            });
        }

        std::vector<std::string> messages() {
            std::lock_guard<std::mutex> lock(Mutex);
            return Messages;
        }

        std::string all() {
            std::string text;
            for (const auto& message : messages())
                text += message + "\n";
            return text;
        }

    private:
        std::mutex Mutex;
        std::vector<std::string> Messages;
    };

    /**
     * Accepts TCP connections and answers like a plain MQTT broker (a CONNACK) without ever speaking TLS. A client
     * that speaks TLS gets bytes that are no TLS.
     */
    class PlainBroker {
    public:
        PlainBroker() : Acceptor(Context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 0)) {
            Acceptor.non_blocking(true);
            Worker = std::thread([this] {
                while (!Stop) {
                    boost::asio::ip::tcp::socket socket(Context);
                    boost::system::error_code error;
                    Acceptor.accept(socket, error);
                    if (error) {
                        std::this_thread::sleep_for(10ms);
                        continue;
                    }
                    ++Connections;
                    socket.non_blocking(false);
                    std::uint8_t buffer[512];
                    socket.read_some(boost::asio::buffer(buffer), error);
                    // Something that is no TLS record, longer than a record header.
                    const std::string answer = "This is no TLS, this is a plain server that does not know TLS.";
                    boost::asio::write(socket, boost::asio::buffer(answer), error);
                    std::this_thread::sleep_for(50ms);
                }
            });
        }

        ~PlainBroker() {
            Stop = true;
            Worker.join();
        }

        std::string port() const { return std::to_string(Acceptor.local_endpoint().port()); }
        int connections() const { return Connections; }

    private:
        boost::asio::io_context Context;
        boost::asio::ip::tcp::acceptor Acceptor;
        std::atomic<bool> Stop{false};
        std::atomic<int> Connections{0};
        std::thread Worker;
    };

    /** A throwaway CA that exists only for the tests (its key is not kept); a valid PEM certificate. */
    const char* const TEST_CA_PEM = R"PEM(-----BEGIN CERTIFICATE-----
MIIDCjCCAfKgAwIBAgIUWOmnWR9s/CkqPRzL5gm/BdiUt7IwDQYJKoZIhvcNAQEL
BQAwHDEaMBgGA1UEAwwRRFQgVW5pdCBUZXN0IENBIGEwIBcNMjYxMTI2MTIxMjMy
WhgPMjEyNjExMDIxMjEyMzJaMBwxGjAYBgNVBAMMEURUIFVuaXQgVGVzdCBDQSBh
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsE+aRidMn0yML05+iHwu
WiM7fJY2fNTDIHRVhPlH5TTDEptnsKEoAiBLJE2ySpYO/UN32zwksTaVYmwavkRO
97Lts4AA968qbxEpI6d+8fe1vv2IT0GRDxzOWQgAl0hsF7ebOVXSAYHLiAU4tBCU
YQTTdTQtdBHY/ZZ/qTkCsOVLpg7TPxmL9HWmmNK96nYZkBh2W6K0LpB9SCLefhEv
+HPUtznQlZevfTpVsVsUmeCyj04sxOl+Op0jFyXUc8vpqLRK1HB/plAsy6v4Gsje
GQcKGPmGOptyrAOWOvcAyOq6j5TzysE1MJB3v9YFWlgS88bwe10YZarxrTT5UCfb
/wIDAQABo0IwQDAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNV
HQ4EFgQUk1PwKWWmsfJUhs2xlJknyKfjxkwwDQYJKoZIhvcNAQELBQADggEBAJ8h
jBynl17mJWuLnSnUJvMfuZA2iFMpKEzG9QWGvsnq+4bnS2PB2eY5UH+qQWh85kZk
9I6JvAANix4bcPDsrP27KeH09AkZtVV0pXuJlQwLAGgPMCl0c5y0YZ3Z725LZ/wd
FW/B2Rcyalai+7QZxCwWJ0eJ/mINDtpWgsAd9H7OxS4X/M27wjY8ekCCL7dyrbUT
DL0u5sjLZvZgZNw/4QyqOdq5JybngD+pQqoHX745kM+Cs/C7520g3xxmZqDtqmsM
UDgXPLNk2shbOsBRa1bBxTgTZgv+IWc1sT2EJACOo1U07tmaPugETdRk9KSKFBNz
9X+Yy8iDt1RKBVuPpDw=
-----END CERTIFICATE-----
)PEM";

    /** A directory of its own in the temp directory (unique per instance), removed with the object. */
    class TempDir {
    public:
        TempDir() {
            static std::atomic<int> counter{0};
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            Path = std::filesystem::temp_directory_path() /
                   ("dt-tls-test-" + std::to_string(stamp) + "-" + std::to_string(counter++));
            std::filesystem::create_directories(Path);
        }

        ~TempDir() {
            std::error_code error;
            std::filesystem::remove_all(Path, error);
        }

        TempDir(const TempDir&) = delete;
        TempDir& operator=(const TempDir&) = delete;

        std::string dir() const { return Path.string(); }

        std::string write(const std::string& name, const std::string& content) const {
            const auto file = Path / name;
            std::ofstream(file, std::ios::binary) << content;
            return file.string();
        }

    private:
        std::filesystem::path Path;
    };
}

TEST(MqttEndpoint, WithoutTlsAndSchemeItIsPlainTcpAsBefore) {
    const auto endpoint = makeMqttEndpoint("localhost", "1883", false);
    EXPECT_EQ(endpoint.Uri, "tcp://localhost:1883");
    EXPECT_FALSE(endpoint.Tls);
    EXPECT_TRUE(endpoint.Notice.empty());
}

TEST(MqttEndpoint, PlainSchemesAreKeptWithoutTls) {
    for (const std::string scheme : {"tcp", "mqtt", "ws"}) {
        const auto endpoint = makeMqttEndpoint(scheme + "://broker.example.org", "1883", false);
        EXPECT_EQ(endpoint.Uri, scheme + "://broker.example.org:1883") << scheme;
        EXPECT_FALSE(endpoint.Tls) << scheme;
        EXPECT_TRUE(endpoint.Notice.empty()) << scheme;
    }
}

TEST(MqttEndpoint, EmptyPortAddsNoPort) {
    EXPECT_EQ(makeMqttEndpoint("tcp://localhost:1884", "", false).Uri, "tcp://localhost:1884");
    EXPECT_EQ(makeMqttEndpoint("localhost", "", true).Uri, "ssl://localhost");
}

TEST(MqttEndpoint, TlsSwitchedOnUsesSslScheme) {
    const auto endpoint = makeMqttEndpoint("mqtt.example.org", "8883", true);
    EXPECT_EQ(endpoint.Uri, "ssl://mqtt.example.org:8883");
    EXPECT_TRUE(endpoint.Tls);
    EXPECT_TRUE(endpoint.Notice.empty());
}

TEST(MqttEndpoint, TlsSchemesAreTlsAndWrittenAsSsl) {
    for (const std::string scheme : {"ssl", "mqtts", "tls", "SSL", "MQTTS", "Tls"}) {
        for (const bool enabled : {false, true}) {
            const auto endpoint = makeMqttEndpoint(scheme + "://mqtt.example.org", "8883", enabled);
            EXPECT_EQ(endpoint.Uri, "ssl://mqtt.example.org:8883") << scheme;
            EXPECT_TRUE(endpoint.Tls) << scheme;
            // only the case "scheme says TLS, switch is off" is worth a notice
            EXPECT_EQ(endpoint.Notice.empty(), enabled) << scheme;
        }
    }
    const auto wss = makeMqttEndpoint("wss://mqtt.example.org", "443", false);
    EXPECT_EQ(wss.Uri, "wss://mqtt.example.org:443");
    EXPECT_TRUE(wss.Tls);
}

TEST(MqttEndpoint, PlainSchemeWithTlsSwitchedOnIsNeverADowngrade) {
    for (const std::string scheme : {"tcp", "mqtt", "TCP"}) {
        const auto endpoint = makeMqttEndpoint(scheme + "://mqtt.example.org", "8883", true);
        EXPECT_EQ(endpoint.Uri, "ssl://mqtt.example.org:8883") << scheme;
        EXPECT_TRUE(endpoint.Tls) << scheme;
        EXPECT_FALSE(endpoint.Notice.empty()) << scheme;
    }
    const auto ws = makeMqttEndpoint("ws://mqtt.example.org", "80", true);
    EXPECT_EQ(ws.Uri, "wss://mqtt.example.org:80");
    EXPECT_TRUE(ws.Tls);
}

TEST(MqttEndpoint, UnknownSchemeIsLeftToPaho) {
    const auto plain = makeMqttEndpoint("foo://host", "1", false);
    EXPECT_EQ(plain.Uri, "foo://host:1");
    EXPECT_FALSE(plain.Tls);
    EXPECT_TRUE(makeMqttEndpoint("foo://host", "1", true).Tls);
}

TEST(MqttSslOptions, VerifyTheChainAndTheHostName) {
    const auto ssl = makeMqttSslOptions(MqttTlsOptions{true, ""});
    EXPECT_TRUE(ssl.get_enable_server_cert_auth());
    EXPECT_TRUE(ssl.get_verify()) << "Paho's default is no host name check";
    EXPECT_TRUE(ssl.get_trust_store().empty());
}

TEST(MqttSslOptions, CaFileIsTheTrustStore) {
    const auto ssl = makeMqttSslOptions(MqttTlsOptions{true, "C:/some/ca.pem"});
    EXPECT_EQ(ssl.get_trust_store(), "C:/some/ca.pem");
    EXPECT_TRUE(ssl.get_enable_server_cert_auth());
    EXPECT_TRUE(ssl.get_verify());
    // survives the copy into the connect options
    mqtt::connect_options options;
    options.set_ssl(ssl);
    EXPECT_EQ(options.get_ssl_options().get_trust_store(), "C:/some/ca.pem");
    EXPECT_TRUE(options.get_ssl_options().get_verify());
}

TEST(MqttCaFile, ProblemsAreNamed) {
    TempDir temp;
    EXPECT_NE(checkMqttCaFile("").find("No CA file"), std::string::npos);
    const auto missing = checkMqttCaFile("C:/does/not/exist/ca.pem");
    EXPECT_NE(missing.find("does not exist"), std::string::npos);
    EXPECT_NE(missing.find("C:/does/not/exist/ca.pem"), std::string::npos);
    EXPECT_NE(checkMqttCaFile(temp.dir()).find("not a file"), std::string::npos);
    EXPECT_NE(checkMqttCaFile(temp.write("garbage.pem", "this is no certificate\n")).find("no PEM certificate"), std::string::npos);
    EXPECT_NE(checkMqttCaFile(temp.write("empty.pem", "")).find("no PEM certificate"), std::string::npos);
    // a private key is no certificate
    EXPECT_NE(checkMqttCaFile(temp.write("key.pem", "-----BEGIN PRIVATE KEY-----\nAAAA\n-----END PRIVATE KEY-----\n"))
                  .find("no PEM certificate"), std::string::npos);
}

TEST(MqttCaFile, ValidPemCertificateIsAccepted) {
    TempDir temp;
    EXPECT_EQ(checkMqttCaFile(temp.write("ca.pem", TEST_CA_PEM)), "");
    // text around the certificate does not matter, a second one is fine
    EXPECT_EQ(checkMqttCaFile(temp.write("two.pem", std::string("junk\n") + TEST_CA_PEM + TEST_CA_PEM)), "");
}

TEST(MqttCaFile, FileThatLooksLikePemButCannotBeLoadedIsRefused) {
    TempDir temp;
    const auto problem = checkMqttCaFile(temp.write("broken.pem", "-----BEGIN CERTIFICATE-----\nAAAA\n-----END CERTIFICATE-----\n"));
    EXPECT_NE(problem.find("could not be loaded as PEM certificates"), std::string::npos) << problem;
}

TEST(MqttClientServiceTls, MissingCaFileStopsTheClientWithAClearMessage) {
    PlainBroker broker;
    ErrorLog errors;
    MqttClientService service("127.0.0.1", broker.port(), "tls-test",
                              DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "alice", "s3cret",
                              MqttTlsOptions{true, "C:/does/not/exist/ca.pem"});
    EXPECT_TRUE(service.usesTls());
    errors.attach(service);
    service.start();

    ASSERT_TRUE(waitFor([&] { return service.isTlsConfigurationFailed(); }, 5s));
    EXPECT_TRUE(service.hasTlsProblem());
    std::this_thread::sleep_for(2500ms);
    EXPECT_EQ(errors.messages().size(), 1u) << errors.all();
    EXPECT_NE(errors.all().find("TLS"), std::string::npos);
    EXPECT_NE(errors.all().find("C:/does/not/exist/ca.pem"), std::string::npos);
    EXPECT_EQ(errors.all().find("s3cret"), std::string::npos);
    EXPECT_EQ(broker.connections(), 0) << "not even a TCP connection is made";
    EXPECT_FALSE(service.isConnected());
    service.stop();
}

TEST(MqttClientServiceTls, TlsWithoutAnyCaFileIsAConfigurationProblemNotAConnectWithoutTrust) {
    PlainBroker broker;
    ErrorLog errors;
    MqttClientService service("127.0.0.1", broker.port(), "tls-test",
                              DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "", "", MqttTlsOptions{true, ""});
    errors.attach(service);
    service.start();

    ASSERT_TRUE(waitFor([&] { return service.isTlsConfigurationFailed(); }, 5s));
    EXPECT_NE(errors.all().find("No trusted CA certificates are configured"), std::string::npos) << errors.all();
    std::this_thread::sleep_for(1500ms);
    EXPECT_EQ(broker.connections(), 0);
    service.stop();
}

TEST(MqttClientServiceTls, SetupErrorOfTheApplicationIsReportedAndStopsTheClient) {
    PlainBroker broker;
    ErrorLog errors;
    MqttTlsOptions options{true, "", "The certificate store of the system is empty."};
    MqttClientService service("127.0.0.1", broker.port(), "tls-test",
                              DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "", "", options);
    errors.attach(service);
    service.start();

    ASSERT_TRUE(waitFor([&] { return service.isTlsConfigurationFailed(); }, 5s));
    EXPECT_NE(errors.all().find("The certificate store of the system is empty."), std::string::npos) << errors.all();
    EXPECT_EQ(broker.connections(), 0);
    service.stop();
}

TEST(MqttClientServiceTls, CaFileThatCannotBeLoadedStopsTheClientBeforeAnyConnect) {
    // Looks like a PEM certificate, but OpenSSL cannot load it: found by the check up front, not by a handshake error.
    TempDir temp;
    PlainBroker broker;
    ErrorLog errors;
    const auto ca = temp.write("broken-ca.pem", "-----BEGIN CERTIFICATE-----\nAAAA\n-----END CERTIFICATE-----\n");
    MqttClientService service("127.0.0.1", broker.port(), "tls-test",
                              DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "", "", MqttTlsOptions{true, ca});
    errors.attach(service);
    service.start();

    ASSERT_TRUE(waitFor([&] { return service.isTlsConfigurationFailed(); }, 10s));
    EXPECT_NE(errors.all().find("could not be loaded as PEM certificates"), std::string::npos) << errors.all();
    std::this_thread::sleep_for(2500ms);
    EXPECT_EQ(broker.connections(), 0) << "no connect, no retry";
    service.stop();
}

TEST(MqttClientServiceTls, TlsToAServerWithoutTlsIsReportedAsTlsProblemAndRetriedWithBackoff) {
    TempDir temp;
    PlainBroker broker;
    ErrorLog errors;
    const auto ca = temp.write("ca.pem", TEST_CA_PEM);
    MqttClientService service("127.0.0.1", broker.port(), "tls-test",
                              DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "alice", "s3cret",
                              MqttTlsOptions{true, ca});
    errors.attach(service);
    service.start();

    ASSERT_TRUE(waitFor([&] { return !errors.messages().empty(); }, 10s));
    EXPECT_FALSE(service.isConnected());
    EXPECT_TRUE(service.hasTlsProblem());
    EXPECT_FALSE(service.isTlsConfigurationFailed());
    EXPECT_FALSE(service.isAuthenticationFailed());
    EXPECT_NE(errors.messages().front().find("TLS problem: the server does not speak TLS"), std::string::npos)
        << errors.messages().front();
    // Retried with growing pauses (1 s, 2 s, 4 s ...), not in a tight loop: in 6 s there are three attempts (at about
    // 0 s, 1 s and 3 s; the next one is due at 7 s), each one reported once.
    std::this_thread::sleep_for(6s);
    // The flag stays set while the retries go on (it is cleared only by a successful connect), so that the application
    // knows that a new connection with changed settings makes sense.
    EXPECT_TRUE(service.hasTlsProblem());
    service.stop();
    EXPECT_GE(errors.messages().size(), 2u) << "it is retried";
    EXPECT_LE(errors.messages().size(), 4u) << "with growing pauses";
    EXPECT_EQ(errors.all().find("s3cret"), std::string::npos);
    EXPECT_GE(broker.connections(), 2);
    EXPECT_LE(broker.connections(), 4) << "one TCP connection per attempt, nothing else connects";
}

TEST(MqttTlsError, OpenSslMessagesAreTranslated) {
    using DigitalTwin::Communication::describeTlsError;
    const auto verify = describeTlsError("0CF10000:error:0A000086:SSL routines:tls_post_process_server_certificate:certificate verify failed:ssl/statem/statem_clnt.c:2126:\n");
    EXPECT_NE(verify.Text.find("TLS certificate problem"), std::string::npos);
    EXPECT_NE(verify.Text.find("missing from the certificate store of the system"), std::string::npos);
    EXPECT_NE(verify.Text.find("CA file"), std::string::npos);
    const auto version = describeTlsError("error:0A00010B:SSL routines::wrong version number:ssl/record/rec_layer_s3.c:913:");
    EXPECT_NE(version.Text.find("does not speak TLS"), std::string::npos);
    const auto eof = describeTlsError("58CB0000:error:0A000126:SSL routines::unexpected eof while reading:ssl/record/rec_layer_s3.c:703:");
    EXPECT_NE(eof.Text.find("closed the connection during the TLS handshake"), std::string::npos);
    // a malformed certificate of the server is a failed attempt like any other, not a problem of the CA file
    const auto asn1 = describeTlsError("CC3A0000:error:0688010A:asn1 encoding \nroutines:asn1_item_embed_d2i:nested asn1 error:crypto/asn1/tasn_dec.c:375:Type=X509\n");
    EXPECT_EQ(asn1.Text, "TLS problem: nested asn1 error");
    const auto unknown = describeTlsError("error:1234:SSL routines:func:something new:file.c:1:");
    EXPECT_EQ(unknown.Text, "TLS problem: something new");
}

TEST(MqttClientServiceTls, PlainClientIsUnchanged) {
    MqttClientService service("localhost", "1", "tls-test");
    EXPECT_FALSE(service.usesTls());
    EXPECT_EQ(service.serverUri(), "tcp://localhost:1");
    EXPECT_FALSE(service.hasTlsProblem());
}

namespace {
    std::string e2eDir() {
        const char* dir = std::getenv("DT_TLS_E2E_DIR");
        return dir ? dir : "";
    }

    int e2ePort(int offset) {
        const char* base = std::getenv("DT_TLS_E2E_PORT_BASE");
        return (base ? std::atoi(base) : 18883) + offset;
    }
}

/**
 * Opt-in: DT_TLS_E2E_DIR is a directory with ca.pem (the CA of the server certificate) and other_ca.pem (a different
 * CA). Brokers (DTServerRust, authentication "none") have to run on localhost with TLS on the ports
 * DT_TLS_E2E_PORT_BASE (default 18883) with a certificate of ca.pem for "localhost", +1 with an expired one of ca.pem, +2
 * with one of other_ca.pem, +3 without TLS.
 */
class MqttClientServiceTlsE2E : public ::testing::Test {
protected:
    void SetUp() override {
        if (e2eDir().empty())
            GTEST_SKIP() << "DT_TLS_E2E_DIR is not set";
    }

    struct Outcome {
        bool Connected = false;
        bool TlsProblem = false;
        std::string Errors;
    };

    Outcome attempt(const std::string& host, int port, const MqttTlsOptions& tls, std::chrono::milliseconds limit = 8s) {
        ErrorLog errors;
        std::atomic<bool> connected{false};
        MqttClientService service(host, std::to_string(port), "tls-e2e",
                                  DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "alice", "s3cret", tls);
        service.setConnectedHandler([&connected] { connected = true; });
        errors.attach(service);
        service.start();
        waitFor([&] { return connected.load() || !errors.messages().empty(); }, limit);
        Outcome outcome;
        outcome.Connected = connected;
        outcome.TlsProblem = service.hasTlsProblem();
        outcome.Errors = errors.all();
        service.stop();
        return outcome;
    }
};

TEST_F(MqttClientServiceTlsE2E, ConnectsWithTheRightCa) {
    const auto outcome = attempt("localhost", e2ePort(0), MqttTlsOptions{true, e2eDir() + "/ca.pem"});
    EXPECT_TRUE(outcome.Connected) << outcome.Errors;
    EXPECT_FALSE(outcome.TlsProblem);
}

TEST_F(MqttClientServiceTlsE2E, SchemeInTheServerFieldWorksWithoutTheSwitch) {
    const auto outcome = attempt("ssl://localhost", e2ePort(0), MqttTlsOptions{false, e2eDir() + "/ca.pem"});
    EXPECT_TRUE(outcome.Connected) << outcome.Errors;
}

TEST_F(MqttClientServiceTlsE2E, WithoutAnyCaFileItDoesNotEvenConnect) {
    const auto outcome = attempt("localhost", e2ePort(0), MqttTlsOptions{true, ""});
    EXPECT_FALSE(outcome.Connected);
    EXPECT_TRUE(outcome.TlsProblem);
    EXPECT_NE(outcome.Errors.find("No trusted CA certificates are configured"), std::string::npos) << outcome.Errors;
    std::cerr << "E2E message (no CA): " << outcome.Errors;
}

TEST_F(MqttClientServiceTlsE2E, RefusedWithAnotherCa) {
    const auto outcome = attempt("localhost", e2ePort(0), MqttTlsOptions{true, e2eDir() + "/other_ca.pem"});
    std::cerr << "E2E message (RefusedWithAnotherCa): " << outcome.Errors << std::endl;
    EXPECT_FALSE(outcome.Connected);
    EXPECT_TRUE(outcome.TlsProblem);
    EXPECT_NE(outcome.Errors.find("TLS"), std::string::npos) << outcome.Errors;
    EXPECT_EQ(outcome.Errors.find("s3cret"), std::string::npos);
}

TEST_F(MqttClientServiceTlsE2E, RefusedForAnAddressThatIsNotInTheCertificate) {
    const auto outcome = attempt("127.0.0.1", e2ePort(0), MqttTlsOptions{true, e2eDir() + "/ca.pem"});
    std::cerr << "E2E message (RefusedForAnAddressThatIsNotInTheCertificate): " << outcome.Errors << std::endl;
    EXPECT_FALSE(outcome.Connected);
    EXPECT_TRUE(outcome.TlsProblem);
    EXPECT_NE(outcome.Errors.find("TLS"), std::string::npos) << outcome.Errors;
    EXPECT_EQ(outcome.Errors.find("s3cret"), std::string::npos);
}

TEST_F(MqttClientServiceTlsE2E, RefusedExpiredCertificate) {
    const auto outcome = attempt("localhost", e2ePort(1), MqttTlsOptions{true, e2eDir() + "/ca.pem"});
    std::cerr << "E2E message (RefusedExpiredCertificate): " << outcome.Errors << std::endl;
    EXPECT_FALSE(outcome.Connected);
    EXPECT_TRUE(outcome.TlsProblem);
    EXPECT_NE(outcome.Errors.find("TLS"), std::string::npos) << outcome.Errors;
    EXPECT_EQ(outcome.Errors.find("s3cret"), std::string::npos);
}

TEST_F(MqttClientServiceTlsE2E, RefusedCertificateOfAnotherCa) {
    const auto outcome = attempt("localhost", e2ePort(2), MqttTlsOptions{true, e2eDir() + "/ca.pem"});
    std::cerr << "E2E message (RefusedCertificateOfAnotherCa): " << outcome.Errors << std::endl;
    EXPECT_FALSE(outcome.Connected);
    EXPECT_TRUE(outcome.TlsProblem);
    EXPECT_NE(outcome.Errors.find("TLS"), std::string::npos) << outcome.Errors;
    EXPECT_EQ(outcome.Errors.find("s3cret"), std::string::npos);
}

TEST_F(MqttClientServiceTlsE2E, TlsToAPlainBrokerIsRefused) {
    const auto outcome = attempt("localhost", e2ePort(3), MqttTlsOptions{true, e2eDir() + "/ca.pem"});
    std::cerr << "E2E message (TlsToAPlainBrokerIsRefused): " << outcome.Errors << std::endl;
    EXPECT_FALSE(outcome.Connected);
    EXPECT_TRUE(outcome.TlsProblem);
    EXPECT_NE(outcome.Errors.find("TLS"), std::string::npos) << outcome.Errors;
    EXPECT_EQ(outcome.Errors.find("s3cret"), std::string::npos);
}

TEST_F(MqttClientServiceTlsE2E, PlainClientToATlsBrokerIsRefused) {
    const auto outcome = attempt("localhost", e2ePort(0), MqttTlsOptions{});
    std::cerr << "E2E message (PlainClientToATlsBrokerIsRefused): " << outcome.Errors << std::endl;
    EXPECT_FALSE(outcome.Connected);
}

TEST_F(MqttClientServiceTlsE2E, PlainClientToAPlainBrokerStillConnects) {
    const auto outcome = attempt("localhost", e2ePort(3), MqttTlsOptions{});
    EXPECT_TRUE(outcome.Connected) << outcome.Errors;
}

TEST_F(MqttClientServiceTlsE2E, CaFileInADirectoryWithNonAsciiName) {
    // The user name in C:\Users\<name>\AppData is often not ASCII; file names are UTF-8 in the library.
    const auto directory = std::filesystem::temp_directory_path() / std::filesystem::path(u8"dt-tls-M\u00fcller-\u4e2d-" + std::u8string(reinterpret_cast<const char8_t*>(std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()).c_str())));
    std::filesystem::create_directories(directory);
    const auto copy = directory / "ca.pem";
    std::filesystem::copy_file(std::filesystem::path(e2eDir()) / "ca.pem", copy, std::filesystem::copy_options::overwrite_existing);
    const auto utf8 = copy.u8string();
    const auto outcome = attempt("localhost", e2ePort(0), MqttTlsOptions{true, std::string(utf8.begin(), utf8.end())});
    std::filesystem::remove_all(directory);
    EXPECT_TRUE(outcome.Connected) << outcome.Errors;
}

TEST(MqttClientServiceTls, UnreachableServerBeforeTheFirstConnectIsATlsProblem) {
    // Nothing was ever trusted: the user has to look at the TLS settings (and the host name) as well.
    TempDir temp;
    std::string port;
    {
        PlainBroker probe;
        port = probe.port();
    }
    ErrorLog errors;
    const auto ca = temp.write("ca.pem", TEST_CA_PEM);
    MqttClientService service("127.0.0.1", port, "tls-test", DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "", "",
                              MqttTlsOptions{true, ca});
    errors.attach(service);
    service.start();

    ASSERT_TRUE(waitFor([&] { return !errors.messages().empty(); }, 15s));
    EXPECT_TRUE(service.hasTlsProblem());
    EXPECT_FALSE(service.isTlsConfigurationFailed());
    EXPECT_NE(errors.messages().front().find("TLS connect to"), std::string::npos) << errors.messages().front();
    service.stop();
}

/**
 * Opt-in, needs a driver (see DT_TLS_E2E_CONTROL_DIR): the client connects over TLS, the broker is stopped and started
 * again from outside, and the test checks that an outage after a successful connect is no TLS problem and that a CA file
 * that is gone for a while does not end the connection for good. The driver and the test talk through flag files in the
 * directory: connected.flag (test) -> stopped.flag (driver) -> outage-checked.flag (test) -> restarted.flag (driver) ->
 * ca-restored.flag, reconnected.flag (test).
 */
TEST_F(MqttClientServiceTlsE2E, OutageAfterAConnectIsNoTlsProblemAndAMissingCaFileIsRetried) {
    const char* control = std::getenv("DT_TLS_E2E_CONTROL_DIR");
    if (control == nullptr)
        GTEST_SKIP() << "DT_TLS_E2E_CONTROL_DIR is not set";
    const std::filesystem::path flags(control);
    auto flag = [&flags](const std::string& name) { return std::filesystem::exists(flags / name); };
    auto raise = [&flags](const std::string& name) { std::ofstream(flags / name) << "x"; };

    TempDir temp;
    const auto ca = temp.write("ca.pem", TEST_CA_PEM);
    std::filesystem::copy_file(std::filesystem::path(e2eDir()) / "ca.pem", ca, std::filesystem::copy_options::overwrite_existing);
    ErrorLog errors;
    std::atomic<int> connects{0};
    MqttClientService service("localhost", std::to_string(e2ePort(0)), "tls-e2e",
                              DigitalTwin::Communication::MqttProtocolVersion::V3_1_1, "", "", MqttTlsOptions{true, ca});
    service.setConnectedHandler([&connects] { ++connects; });
    errors.attach(service);
    service.start();
    ASSERT_TRUE(waitFor([&] { return connects.load() == 1; }, 15s));
    raise("connected.flag");

    // the driver stops the broker
    ASSERT_TRUE(waitFor([&] { return flag("stopped.flag"); }, 120s));
    ASSERT_TRUE(waitFor([&] { return !service.isConnected() && !errors.messages().empty(); }, 30s));
    std::this_thread::sleep_for(6s);
    EXPECT_FALSE(service.hasTlsProblem()) << "an outage after a successful connect is no TLS problem: " << errors.all();
    EXPECT_FALSE(service.isTlsConfigurationFailed());
    EXPECT_EQ(errors.all().find("TLS connect to"), std::string::npos) << errors.all();
    EXPECT_EQ(errors.all().find("TLS problem"), std::string::npos) << errors.all();
    std::cerr << "E2E outage messages: " << errors.all();

    // the CA file is gone while the broker comes back
    std::filesystem::remove(ca);
    raise("outage-checked.flag");
    ASSERT_TRUE(waitFor([&] { return flag("restarted.flag"); }, 120s));
    ASSERT_TRUE(waitFor([&] { return errors.all().find("cannot be used at the moment") != std::string::npos; }, 90s)) << errors.all();
    EXPECT_FALSE(service.isTlsConfigurationFailed()) << "not stopped for good";
    EXPECT_FALSE(service.isConnected());
    std::cerr << "E2E CA file missing messages: " << errors.all();

    std::filesystem::copy_file(std::filesystem::path(e2eDir()) / "ca.pem", ca);
    raise("ca-restored.flag");
    EXPECT_TRUE(waitFor([&] { return connects.load() == 2; }, 120s)) << errors.all();
    EXPECT_FALSE(service.hasTlsProblem());
    raise("reconnected.flag");
    service.stop();
}
