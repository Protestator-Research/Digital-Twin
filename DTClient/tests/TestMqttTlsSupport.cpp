// Tests of the trusted certificates for the MQTT connection over TLS: the bundle of the system certificates that is
// handed to the MQTT library, the custom CA file and the failure cases. The bundle goes into a temporary directory.

#include <gtest/gtest.h>

#include <iostream>

#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QTemporaryDir>
#include <QThread>

#include "../src/Models/MqttTlsSupport.h"

using DigitalTwin::Client::MqttTlsSupport;
using DigitalTwin::Client::RootExportStats;

namespace {
    /**
     * Qt loads its TLS backend as a plugin, which needs a QCoreApplication (the test runner has none: GTest::gtest_main).
     */
    class QtApplicationEnvironment : public ::testing::Environment {
    public:
        void SetUp() override {
            static int argc = 1;
            static char name[] = "DigitalTwinClientTests";
            static char* argv[] = {name, nullptr};
            if (QCoreApplication::instance() == nullptr)
                Application = std::make_unique<QCoreApplication>(argc, argv);
        }

        void TearDown() override {
            Application.reset();
        }

    private:
        std::unique_ptr<QCoreApplication> Application;
    };

    ::testing::Environment* const QT_APPLICATION = ::testing::AddGlobalTestEnvironment(new QtApplicationEnvironment);

    // Two throwaway CAs of the tests (valid for 100 years; their keys are not kept).
    const char* const FIRST_CA = R"PEM(-----BEGIN CERTIFICATE-----
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
    const char* const SECOND_CA = R"PEM(-----BEGIN CERTIFICATE-----
MIIDCjCCAfKgAwIBAgIULW//kuh2bfW7NN1FBA/83oOeZHEwDQYJKoZIhvcNAQEL
BQAwHDEaMBgGA1UEAwwRRFQgVW5pdCBUZXN0IENBIGIwIBcNMjYxMTI2MTIxMjMy
WhgPMjEyNjExMDIxMjEyMzJaMBwxGjAYBgNVBAMMEURUIFVuaXQgVGVzdCBDQSBi
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApl4VN4epLSDOK7pujEDU
4xtk+/qE5YhDFwcvVSY1GtvEJaV5i8SdkwAa/ETwKaNvPyrutDFKgvqDGlREjJU1
1Nbg9YsQWaDUy9q0c3Z4j3SxVW+mtuEqz1t9pQGCd9F3Hw4gsFEx0HAjqgd15+qc
cK37tSBpHK1kgm31VMRlva1adLGGEAs3J5eUl6EyC6Rt4HGg5E2SBfuBoI2VZeFD
qyy/+n5++wV849Y5ef2wvci0STlUk+EKz7V7cijPv6E6ncvtDl0RuAdrOAP1uy9L
uyUSkPOL6a2W9A62Cw5w+HJQL+oiqn28YDdp6PzKNUqogpyMerUSQ2ulcAgm/Po7
ZQIDAQABo0IwQDAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNV
HQ4EFgQU3mgpBr7A4JiD/+W+LXMF8ugliqkwDQYJKoZIhvcNAQELBQADggEBAG8V
VC8w3QK96bDzWReDDxHWebrJ7parsFdIZAP3j5Wg7iL0ZTB8j49thz7vSj6rz3jx
qXIH7fxeL89pYBZMCPz6wuhITZj7LjKFnk+0mmJfO/WH5Bvi8wrJwcVQEKfjofUa
t6vLiEn7a9gdq5g9oF0Cwp0R4JB035WR4u3NhlJK4F0m0sQl5YiDXlzmW5rzbG/W
gkQIrKxhI3NcBgtL8Gfbpq5Vws4uox5ZHwb5OxXfg1cm5tlPoQ9d9YhBYBYhLRVk
c/Ey9yTs9Kjfo8Hxk6unhvWTlL9zZ4JwuLV30i00yKjXkxXJ7j4iAFVE7Hn7TPke
1bElOXVAdHY9IMJijOE=
-----END CERTIFICATE-----
)PEM";

    QList<QSslCertificate> twoCertificates() {
        return QSslCertificate::fromData(QByteArray(FIRST_CA) + QByteArray(SECOND_CA), QSsl::Pem);
    }

    QString writeFile(const QString& path, const QByteArray& content) {
        QFile file(path);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(content);
        return path;
    }
}

class MqttTlsSupportTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(Directory.isValid());
        Bundles = Directory.filePath("cache/bundles");
    }

    QString bundleFile() const { return QDir(Bundles).filePath(MqttTlsSupport::bundleFileName()); }

    std::function<QList<QSslCertificate>()> certificates() {
        return [this]() {
            ++CertificateRequests;
            return twoCertificates();
        };
    }

    QTemporaryDir Directory;
    QString Bundles;
    int CertificateRequests = 0;
};

TEST_F(MqttTlsSupportTest, TheTestCertificatesAreValid) {
    ASSERT_EQ(twoCertificates().size(), 2);
}

TEST_F(MqttTlsSupportTest, BundleHoldsAllSystemCertificatesAsPem) {
    QString error;
    const QString path = MqttTlsSupport::writeCaBundle(twoCertificates(), Bundles, &error);
    ASSERT_FALSE(path.isEmpty()) << error.toStdString();
    EXPECT_TRUE(error.isEmpty());
    EXPECT_EQ(QFileInfo(path).absoluteFilePath(), QFileInfo(bundleFile()).absoluteFilePath());
    // what is in the file is read back as the same certificates
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    const auto read = QSslCertificate::fromData(file.readAll(), QSsl::Pem);
    EXPECT_EQ(read, twoCertificates());
}

TEST_F(MqttTlsSupportTest, UpToDateBundleIsNotWrittenAgain) {
    QString error;
    ASSERT_FALSE(MqttTlsSupport::writeCaBundle(twoCertificates(), Bundles, &error).isEmpty());
    const auto before = QFileInfo(bundleFile()).lastModified();
    QThread::msleep(50);
    ASSERT_FALSE(MqttTlsSupport::writeCaBundle(twoCertificates(), Bundles, &error).isEmpty());
    EXPECT_EQ(QFileInfo(bundleFile()).lastModified(), before);
    // changed certificates change the file
    ASSERT_FALSE(MqttTlsSupport::writeCaBundle(twoCertificates().mid(0, 1), Bundles, &error).isEmpty());
    QFile file(bundleFile());
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(QSslCertificate::fromData(file.readAll(), QSsl::Pem).size(), 1);
}

TEST_F(MqttTlsSupportTest, EmptySystemStoreIsAnError) {
    QString error;
    EXPECT_TRUE(MqttTlsSupport::writeCaBundle({}, Bundles, &error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
    EXPECT_TRUE(error.contains("no trusted root certificates")) << error.toStdString();
    EXPECT_FALSE(QFile::exists(bundleFile()));
}

TEST_F(MqttTlsSupportTest, BundleDirectoryThatCannotBeCreatedIsAnError) {
    // a file where the directory should be
    const QString blocker = writeFile(Directory.filePath("blocker"), "x");
    QString error;
    EXPECT_TRUE(MqttTlsSupport::writeCaBundle(twoCertificates(), blocker + "/sub", &error).isEmpty());
    EXPECT_TRUE(error.contains("cannot be")) << error.toStdString();
    EXPECT_TRUE(error.contains("CA file in the settings")) << "the way out is named: " << error.toStdString();
}

TEST_F(MqttTlsSupportTest, BundleThatCannotBeWrittenIsAnError) {
    // a directory where the file should be
    ASSERT_TRUE(QDir().mkpath(bundleFile()));
    QString error;
    EXPECT_TRUE(MqttTlsSupport::writeCaBundle(twoCertificates(), Bundles, &error).isEmpty());
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(MqttTlsSupportTest, WithoutTlsNothingIsPreparedOrWritten) {
    const auto options = MqttTlsSupport::prepare("localhost", "1883", false, "", Bundles, certificates());
    EXPECT_FALSE(options.Enabled);
    EXPECT_TRUE(options.CaFile.empty());
    EXPECT_TRUE(options.SetupError.empty());
    EXPECT_EQ(CertificateRequests, 0) << "the system store is not even read";
    EXPECT_FALSE(QDir(Bundles).exists());
    // also not with a plain scheme and a CA file that does not exist
    const auto plain = MqttTlsSupport::prepare("tcp://localhost", "1883", false, "C:/no/such/file.pem", Bundles, certificates());
    EXPECT_TRUE(plain.SetupError.empty());
    EXPECT_TRUE(plain.CaFile.empty());
}

TEST_F(MqttTlsSupportTest, TlsUsesTheBundleOfTheSystemCertificates) {
    const auto options = MqttTlsSupport::prepare("mqtt.example.org", "8883", true, "", Bundles, certificates());
    EXPECT_TRUE(options.Enabled);
    EXPECT_TRUE(options.SetupError.empty()) << options.SetupError;
    EXPECT_EQ(QFileInfo(QString::fromStdString(options.CaFile)).absoluteFilePath(), QFileInfo(bundleFile()).absoluteFilePath());
    EXPECT_TRUE(QFile::exists(bundleFile()));
}

TEST_F(MqttTlsSupportTest, TlsSchemeInTheServerFieldNeedsTheBundleToo) {
    const auto options = MqttTlsSupport::prepare("ssl://mqtt.example.org", "8883", false, "", Bundles, certificates());
    EXPECT_TRUE(options.SetupError.empty());
    EXPECT_FALSE(options.CaFile.empty());
    EXPECT_TRUE(QFile::exists(bundleFile()));
}

TEST_F(MqttTlsSupportTest, CustomCaFileReplacesTheSystemCertificates) {
    const QString ca = writeFile(Directory.filePath("private-ca.pem"), FIRST_CA);
    const auto options = MqttTlsSupport::prepare("mqtt.example.org", "8883", true, ca, Bundles, certificates());
    EXPECT_TRUE(options.SetupError.empty()) << options.SetupError;
    EXPECT_EQ(QString::fromStdString(options.CaFile), ca);
    EXPECT_EQ(CertificateRequests, 0) << "the system certificates are not added";
    EXPECT_FALSE(QFile::exists(bundleFile())) << "and no bundle is written";
}

TEST_F(MqttTlsSupportTest, MissingOrUnreadableCustomCaFileIsASetupError) {
    const QString missing = Directory.filePath("missing.pem");
    auto options = MqttTlsSupport::prepare("mqtt.example.org", "8883", true, missing, Bundles, certificates());
    EXPECT_FALSE(options.SetupError.empty());
    EXPECT_NE(options.SetupError.find("does not exist"), std::string::npos) << options.SetupError;
    EXPECT_EQ(CertificateRequests, 0) << "no silent fallback to the system certificates";

    const QString garbage = writeFile(Directory.filePath("garbage.pem"), "no certificate here");
    options = MqttTlsSupport::prepare("mqtt.example.org", "8883", true, garbage, Bundles, certificates());
    EXPECT_NE(options.SetupError.find("no PEM certificate"), std::string::npos) << options.SetupError;
}

TEST_F(MqttTlsSupportTest, EmptySystemStoreWithTlsIsASetupErrorNotAConnectionWithoutVerification) {
    const auto options = MqttTlsSupport::prepare("mqtt.example.org", "8883", true, "", Bundles,
                                                 []() { return QList<QSslCertificate>(); });
    EXPECT_FALSE(options.SetupError.empty());
    EXPECT_TRUE(options.CaFile.empty());
    EXPECT_TRUE(options.Enabled);
}

TEST_F(MqttTlsSupportTest, TheRealSystemStoreCanBeExported) {
    const auto system = MqttTlsSupport::systemRootCertificates();
    if (system.isEmpty())
        GTEST_SKIP() << "this machine has no system certificates for Qt";
    QString error;
    const QString path = MqttTlsSupport::writeCaBundle(system, Bundles, &error);
    ASSERT_FALSE(path.isEmpty()) << error.toStdString();
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(QSslCertificate::fromData(file.readAll(), QSsl::Pem).size(), system.size());
    // To look at the bundle (for example with "openssl s_client -CAfile"): DT_TEST_KEEP_SYSTEM_BUNDLE is a file name.
    if (const QByteArray keep = qgetenv("DT_TEST_KEEP_SYSTEM_BUNDLE"); !keep.isEmpty()) {
        QFile::remove(QString::fromLocal8Bit(keep));
        QFile::copy(path, QString::fromLocal8Bit(keep));
    }
}

TEST(MqttTlsRootUsage, NoRestrictionAllowsEverything) {
    EXPECT_TRUE(MqttTlsSupport::allowsServerAuthentication(false, {}));
}

TEST(MqttTlsRootUsage, RestrictedWithoutAnyUsageIsValidForNothing) {
    // cUsageIdentifier 0 and no CRYPT_E_NOT_FOUND: "no valid uses"
    EXPECT_FALSE(MqttTlsSupport::allowsServerAuthentication(true, {}));
}

TEST(MqttTlsRootUsage, ServerAuthenticationOrAnyUsageIsNeeded) {
    EXPECT_TRUE(MqttTlsSupport::allowsServerAuthentication(true, {"1.3.6.1.5.5.7.3.1"}));
    EXPECT_TRUE(MqttTlsSupport::allowsServerAuthentication(true, {"1.3.6.1.5.5.7.3.3", "1.3.6.1.5.5.7.3.1"}));
    EXPECT_TRUE(MqttTlsSupport::allowsServerAuthentication(true, {"2.5.29.37.0"}));
    // code signing, time stamping, client authentication, e-mail protection
    EXPECT_FALSE(MqttTlsSupport::allowsServerAuthentication(true, {"1.3.6.1.5.5.7.3.3"}));
    EXPECT_FALSE(MqttTlsSupport::allowsServerAuthentication(true, {"1.3.6.1.5.5.7.3.8", "1.3.6.1.5.5.7.3.2", "1.3.6.1.5.5.7.3.4"}));
    // not a prefix match
    EXPECT_FALSE(MqttTlsSupport::allowsServerAuthentication(true, {"1.3.6.1.5.5.7.3.10"}));
}

TEST(MqttTlsRootProperties, DisallowedFileTimeAlwaysDistrusts) {
    // CERT_DISALLOWED_FILETIME_PROP_ID (104): whatever the value is
    EXPECT_TRUE(MqttTlsSupport::isDistrustedByProperties(true, false, false, {}));
    EXPECT_TRUE(MqttTlsSupport::isDistrustedByProperties(true, true, true, {"1.3.6.1.5.5.7.3.3"}));
}

TEST(MqttTlsRootProperties, NoPropertiesNoDistrust) {
    EXPECT_FALSE(MqttTlsSupport::isDistrustedByProperties(false, false, false, {}));
}

TEST(MqttTlsRootProperties, DisallowedUsagesDecideByServerAuthentication) {
    // CERT_DISALLOWED_ENHKEY_USAGE_PROP_ID (122)
    EXPECT_TRUE(MqttTlsSupport::isDistrustedByProperties(false, true, true, {"1.3.6.1.5.5.7.3.1"}));
    EXPECT_TRUE(MqttTlsSupport::isDistrustedByProperties(false, true, true, {"1.3.6.1.5.5.7.3.3", "1.3.6.1.5.5.7.3.1"}));
    EXPECT_TRUE(MqttTlsSupport::isDistrustedByProperties(false, true, true, {"2.5.29.37.0"}));
    // other usages disallowed (code signing, e-mail): server authentication stays
    EXPECT_FALSE(MqttTlsSupport::isDistrustedByProperties(false, true, true, {"1.3.6.1.5.5.7.3.3"}));
    EXPECT_FALSE(MqttTlsSupport::isDistrustedByProperties(false, true, true, {"1.3.6.1.5.5.7.3.4", "1.3.6.1.5.5.7.3.8"}));
    EXPECT_FALSE(MqttTlsSupport::isDistrustedByProperties(false, true, true, {}));
}

TEST(MqttTlsRootProperties, UndecodableDisallowedUsageDistrustsToBeSafe) {
    EXPECT_TRUE(MqttTlsSupport::isDistrustedByProperties(false, true, false, {}));
}

TEST_F(MqttTlsSupportTest, TheRealRootExportIsNotEmptyAndEveryCertificateParses) {
    RootExportStats stats;
    const auto roots = MqttTlsSupport::systemRootCertificates(&stats);
    const auto all = QSslConfiguration::systemCaCertificates();
    std::cerr << "roots: Qt systemCaCertificates() " << all.size() << ", seen " << stats.Seen << ", exported " << roots.size()
              << " (removed: usage " << stats.RemovedByUsage << ", Disallowed store " << stats.RemovedByDisallowedStore
              << ", property 104 " << stats.RemovedByDisallowedFileTime << ", property 122 " << stats.RemovedByDisallowedUsage
              << "; roots with property 122: " << stats.WithDisallowedUsageProperty << "; Disallowed views opened "
              << stats.DisallowedViewsOpened << ")" << std::endl;
    if (all.isEmpty())
        GTEST_SKIP() << "this machine has no system certificates for Qt";
#ifdef Q_OS_WIN
    ASSERT_FALSE(roots.isEmpty());
    EXPECT_EQ(stats.Exported, roots.size());
#else
    if (roots.isEmpty())
        GTEST_SKIP() << "no system certificates";
#endif
    if (const QByteArray names = qgetenv("DT_TEST_LIST_ROOTS"); !names.isEmpty()) {
        for (const QSslCertificate& certificate : roots)
            std::cerr << "root: " << certificate.subjectDisplayName().toStdString() << std::endl;
    }
    for (const QSslCertificate& certificate : roots) {
        EXPECT_FALSE(certificate.isNull());
        EXPECT_FALSE(certificate.subjectDisplayName().isEmpty() && certificate.subjectInfo(QSslCertificate::CommonName).isEmpty() &&
                     certificate.subjectInfo(QSslCertificate::Organization).isEmpty());
    }
    // the PEM round trip of the bundle gives the same number
    QString error;
    const QString path = MqttTlsSupport::writeCaBundle(roots, Bundles, &error);
    ASSERT_FALSE(path.isEmpty()) << error.toStdString();
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    EXPECT_EQ(QSslCertificate::fromData(file.readAll(), QSsl::Pem).size(), roots.size());
}
