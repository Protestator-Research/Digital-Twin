//
// The trusted root certificates for the MQTT connection over TLS.
//

#ifndef DIGITALTWIN_MQTTTLSSUPPORT_H
#define DIGITALTWIN_MQTTTLSSUPPORT_H

#include <functional>
#include <string>
#include <vector>

#include <QList>
#include <QSslCertificate>
#include <QString>
#include <Services/MqttTlsOptions.h>

namespace DigitalTwin::Client {
    class DigitalTwinClientSettings;

    /** What the export of the system roots did (for tests and diagnostics). Counts of distinct roots. */
    struct RootExportStats {
        int Seen = 0;
        int Exported = 0;
        /** Dropped because the enhanced key usage does not allow server authentication. */
        int RemovedByUsage = 0;
        /** Dropped because it is in the Disallowed store. */
        int RemovedByDisallowedStore = 0;
        /** Dropped because of the property "disallowed after this date" (CERT_DISALLOWED_FILETIME_PROP_ID, 104). */
        int RemovedByDisallowedFileTime = 0;
        /** Dropped because of the property "disallowed usages" (CERT_DISALLOWED_ENHKEY_USAGE_PROP_ID, 122). */
        int RemovedByDisallowedUsage = 0;
        /** Roots that have property 122 at all (also those that stay). */
        int WithDisallowedUsageProperty = 0;
        /** How many of the views of the Disallowed store could be opened. */
        int DisallowedViewsOpened = 0;
    };

    /**
     * Prepares the TLS options of the MQTT connection from the settings.
     *
     * The OpenSSL inside the MQTT library does not use the certificate store of the operating system (on Windows not
     * at all), so a certificate of a public CA (Let's Encrypt) would not be trusted. Qt knows the system store, so the
     * client writes the system CA certificates (QSslConfiguration::systemCaCertificates()) as a PEM bundle into its
     * cache directory and hands the path of this file to the library as the trust store. The library stays free of Qt.
     * A CA file set in the settings replaces the system certificates (no bundle is written then).
     *
     * Nothing is written and nothing is checked if the connection does not use TLS.
     */
    class MqttTlsSupport {
    public:
        /**
         * @param settings The client settings (MQTT server, TLS switch, CA file).
         * @return The options for MqttClientService. If the trusted certificates cannot be provided, SetupError of the
         * options is set (system store empty or bundle not writable, CA file missing or unreadable): the client then
         * reports it and does not connect, instead of connecting without a way to verify the server.
         */
        static DigitalTwin::Communication::MqttTlsOptions prepare(DigitalTwinClientSettings& settings);

        /**
         * The same with the values given directly.
         * @param server MQTT server field (can contain a scheme)
         * @param port MQTT port
         * @param useTls the TLS switch
         * @param caFile the custom CA file; empty for the system certificates
         * @param bundleDirectory where the bundle of the system certificates goes (created if needed)
         * @param systemCertificates gives the certificates of the system (QSslConfiguration::systemCaCertificates());
         * only called if they are needed
         */
        static DigitalTwin::Communication::MqttTlsOptions prepare(const QString& server, const QString& port, bool useTls,
                                                                  const QString& caFile, const QString& bundleDirectory,
                                                                  const std::function<QList<QSslCertificate>()>& systemCertificates);

        /**
         * Writes the certificates as PEM to <directory>/<fileName>. The file is replaced atomically and only written if
         * its content changes (so an up to date file in a directory that is not writable is fine).
         * @param error set to a text for the user if it fails
         * @return the path of the file, empty on failure
         */
        static QString writeCaBundle(const QList<QSslCertificate>& certificates, const QString& directory, QString* error);

        /**
         * The root certificates that are trusted for TLS servers on this system.
         *
         * Windows: the roots of the ROOT store (current user, local machine and the group policy views), reduced to what
         * Windows itself trusts for server authentication: a root is dropped if its enhanced key usage (extension or
         * property) does not allow server authentication, and a root that is in the Disallowed store is dropped.
         * QSslConfiguration::systemCaCertificates() returns every root, also those that Windows only trusts for code
         * signing or time stamping, and ignores the Disallowed store. Other platforms: QSslConfiguration::systemCaCertificates().
         * If the Windows store cannot be read the list is empty (no fallback to the unfiltered list).
         */
        static QList<QSslCertificate> systemRootCertificates(RootExportStats* stats = nullptr);

        /**
         * Whether the properties that Microsoft puts on partly distrusted roots (Symantec/VeriSign, Entrust, ...) make a
         * root unusable for TLS servers. Such a root is trusted by Windows only for what it was issued before a date
         * or for other usages, which OpenSSL cannot express, so it is not exported.
         * @param hasDisallowedFileTime property CERT_DISALLOWED_FILETIME_PROP_ID (104) is present, whatever its value:
         * "certificates issued after this date are not trusted"
         * @param hasDisallowedUsage property CERT_DISALLOWED_ENHKEY_USAGE_PROP_ID (122) is present: usages that are not
         * allowed
         * @param disallowedUsageDecoded the value of property 122 could be decoded into disallowedUsageOids; if not
         * (unknown encoding) the root is dropped to be on the safe side
         * @param disallowedUsageOids the disallowed usages (dotted OIDs)
         */
        static bool isDistrustedByProperties(bool hasDisallowedFileTime, bool hasDisallowedUsage, bool disallowedUsageDecoded,
                                             const std::vector<std::string>& disallowedUsageOids);

        /**
         * The decision whether a root of the Windows store is trusted for TLS servers, from its enhanced key usage as
         * CertGetEnhancedKeyUsage (flags 0) reports it.
         * @param restricted false: the root has no usage restriction (cUsageIdentifier is 0 and GetLastError() is
         * CRYPT_E_NOT_FOUND): valid for all uses. true: the usages are in usageOids; an empty list then means "no valid
         * uses" (cUsageIdentifier 0 and GetLastError() 0) and the root is valid for nothing.
         * @param usageOids dotted OIDs
         * @return true if the root is valid for all uses, or the list has server authentication (1.3.6.1.5.5.7.3.1) or
         * anyExtendedKeyUsage (2.5.29.37.0, "all uses")
         */
        static bool allowsServerAuthentication(bool restricted, const std::vector<std::string>& usageOids);

        /** The directory for the bundle: the cache directory of the application. */
        static QString defaultBundleDirectory();

        /** The name of the bundle file. */
        static QString bundleFileName();
    };
}

#endif //DIGITALTWIN_MQTTTLSSUPPORT_H
