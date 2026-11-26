//
// The trusted root certificates for the MQTT connection over TLS, see MqttTlsSupport.h.
//

#include "MqttTlsSupport.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QSslConfiguration>
#include <QStandardPaths>

#include "DigitalTwinClientSettings.h"

#ifdef Q_OS_WIN
#include <QHash>
#include <QSet>
#include <windows.h>
#include <wincrypt.h>
#endif

namespace DigitalTwin::Client {
    using DigitalTwin::Communication::MqttTlsOptions;

    bool MqttTlsSupport::allowsServerAuthentication(bool restricted, const std::vector<std::string>& usageOids) {
        if (!restricted)
            return true;
        for (const std::string& oid : usageOids) {
            if (oid == "1.3.6.1.5.5.7.3.1" || oid == "2.5.29.37.0")
                return true;
        }
        return false;
    }

    bool MqttTlsSupport::isDistrustedByProperties(bool hasDisallowedFileTime, bool hasDisallowedUsage, bool disallowedUsageDecoded,
                                                  const std::vector<std::string>& disallowedUsageOids) {
        if (hasDisallowedFileTime)
            return true;
        if (!hasDisallowedUsage)
            return false;
        if (!disallowedUsageDecoded)
            return true;
        for (const std::string& oid : disallowedUsageOids) {
            if (oid == "1.3.6.1.5.5.7.3.1" || oid == "2.5.29.37.0")
                return true;
        }
        return false;
    }

#ifdef Q_OS_WIN
    namespace {
        // The views of the system store: CERT_SYSTEM_STORE_* location flags.
        const DWORD STORE_LOCATIONS[] = {CERT_SYSTEM_STORE_CURRENT_USER, CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                         CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY,
                                         CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY};

        /** An open certificate store, closed in the destructor. */
        class Store {
        public:
            Store(DWORD location, const wchar_t* name) :
                Handle(CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, 0,
                                     location | CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG, name)) {}
            ~Store() {
                if (Handle != nullptr)
                    CertCloseStore(Handle, 0);
            }
            Store(const Store&) = delete;
            Store& operator=(const Store&) = delete;
            HCERTSTORE handle() const { return Handle; }

        private:
            HCERTSTORE Handle;
        };

        /** The enumeration context of a store: freed in the destructor if the loop is left early (exception). */
        class Enumeration {
        public:
            explicit Enumeration(HCERTSTORE store) : StoreHandle(store) {}
            ~Enumeration() {
                if (Current != nullptr)
                    CertFreeCertificateContext(Current);
            }
            Enumeration(const Enumeration&) = delete;
            Enumeration& operator=(const Enumeration&) = delete;
            /** The next certificate (the previous one is released by the call), nullptr at the end. */
            PCCERT_CONTEXT next() {
                Current = CertEnumCertificatesInStore(StoreHandle, Current);
                return Current;
            }

        private:
            HCERTSTORE StoreHandle;
            PCCERT_CONTEXT Current = nullptr;
        };

        /**
         * Calls the function for every certificate of the named store in every view.
         * @return how many views could be opened
         */
        template<typename Function>
        int forEachCertificate(const wchar_t* storeName, Function function) {
            int opened = 0;
            for (const DWORD location : STORE_LOCATIONS) {
                Store store(location, storeName);
                if (store.handle() == nullptr)
                    continue; // this view does not exist (for example no group policy)
                ++opened;
                Enumeration enumeration(store.handle());
                while (PCCERT_CONTEXT context = enumeration.next())
                    function(context);
            }
            return opened;
        }

        /**
         * The usages of a certificate: extension and property together (dwFlags 0). cUsageIdentifier 0 with
         * CRYPT_E_NOT_FOUND is "valid for all uses", cUsageIdentifier 0 with no error is "no valid uses"
         * (documentation of CertGetEnhancedKeyUsage).
         */
        bool serverAuthenticationAllowed(PCCERT_CONTEXT context) {
            DWORD size = 0;
            SetLastError(0);
            if (!CertGetEnhancedKeyUsage(context, 0, nullptr, &size))
                return GetLastError() == CRYPT_E_NOT_FOUND; // not found: no restriction; any other error: not trusted
            std::vector<BYTE> buffer(size);
            auto* usage = reinterpret_cast<PCERT_ENHKEY_USAGE>(buffer.data());
            SetLastError(0);
            if (!CertGetEnhancedKeyUsage(context, 0, usage, &size))
                return GetLastError() == CRYPT_E_NOT_FOUND;
            if (usage->cUsageIdentifier == 0)
                return GetLastError() == CRYPT_E_NOT_FOUND; // not found: all uses; otherwise: no valid uses
            std::vector<std::string> oids;
            for (DWORD i = 0; i < usage->cUsageIdentifier; ++i)
                oids.emplace_back(usage->rgpszUsageIdentifier[i]);
            return MqttTlsSupport::allowsServerAuthentication(true, oids);
        }

        /** The value of a property of the certificate; false if there is none. */
        bool property(PCCERT_CONTEXT context, DWORD id, std::vector<BYTE>& value) {
            DWORD size = 0;
            if (!CertGetCertificateContextProperty(context, id, nullptr, &size))
                return false;
            value.assign(size, 0);
            if (size > 0 && !CertGetCertificateContextProperty(context, id, value.data(), &size))
                return false;
            value.resize(size);
            return true;
        }

        /**
         * Property CERT_DISALLOWED_ENHKEY_USAGE_PROP_ID (122): wincrypt.h does not say how the value is encoded; it is the
         * DER of a CERT_ENHKEY_USAGE (sequence of OIDs) like property 9, which is decoded with X509_ENHANCED_KEY_USAGE.
         * @return false if it cannot be decoded
         */
        bool decodeUsages(const std::vector<BYTE>& value, std::vector<std::string>& oids) {
            DWORD size = 0;
            PCERT_ENHKEY_USAGE usage = nullptr;
            // CRYPT_DECODE_ALLOC_FLAG: the structure is allocated by the function, no size query
            if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ENHANCED_KEY_USAGE, value.data(), static_cast<DWORD>(value.size()),
                                     CRYPT_DECODE_ALLOC_FLAG, nullptr, &usage, &size) || usage == nullptr)
                return false;
            struct Freed {
                void* Memory;
                ~Freed() { LocalFree(Memory); }
            } freed{usage};
            for (DWORD i = 0; i < usage->cUsageIdentifier; ++i)
                oids.emplace_back(usage->rgpszUsageIdentifier[i]);
            return true;
        }

        QByteArray derOf(PCCERT_CONTEXT context) {
            return QByteArray(reinterpret_cast<const char*>(context->pbCertEncoded), static_cast<qsizetype>(context->cbCertEncoded));
        }

        /** Why a root is not exported; the first reason counts. */
        enum class Verdict { Keep, Usage, DisallowedStore, DisallowedFileTime, DisallowedUsage };
    }

    QList<QSslCertificate> MqttTlsSupport::systemRootCertificates(RootExportStats* statsOut) {
        RootExportStats stats;
        QSet<QByteArray> disallowed;
        stats.DisallowedViewsOpened = forEachCertificate(L"Disallowed", [&disallowed](PCCERT_CONTEXT context) {
            disallowed.insert(QCryptographicHash::hash(derOf(context), QCryptographicHash::Sha256));
        });
        if (statsOut != nullptr)
            *statsOut = stats;
        // Without the Disallowed store the filter would be incomplete: no export (the caller reports a setup error).
        if (stats.DisallowedViewsOpened == 0)
            return {};

        // A root is only taken if every copy of it (user, machine, policy) is fine.
        QHash<QByteArray, Verdict> verdicts;
        QList<QByteArray> order;
        QSet<QByteArray> withUsageProperty;
        forEachCertificate(L"ROOT", [&](PCCERT_CONTEXT context) {
            const QByteArray der = derOf(context);
            Verdict verdict = Verdict::Keep;
            std::vector<BYTE> value;
            std::vector<std::string> oids;
            const bool hasFileTime = property(context, CERT_DISALLOWED_FILETIME_PROP_ID, value);
            const bool hasUsage = property(context, CERT_DISALLOWED_ENHKEY_USAGE_PROP_ID, value);
            const bool decoded = hasUsage && decodeUsages(value, oids);
            if (hasUsage)
                withUsageProperty.insert(der);
            if (disallowed.contains(QCryptographicHash::hash(der, QCryptographicHash::Sha256)))
                verdict = Verdict::DisallowedStore;
            else if (!serverAuthenticationAllowed(context))
                verdict = Verdict::Usage;
            else if (hasFileTime)
                verdict = Verdict::DisallowedFileTime;
            else if (isDistrustedByProperties(false, hasUsage, decoded, oids))
                verdict = Verdict::DisallowedUsage;
            const auto found = verdicts.find(der);
            if (found == verdicts.end()) {
                verdicts.insert(der, verdict);
                order.append(der);
            } else if (found.value() == Verdict::Keep) {
                found.value() = verdict;
            }
        });
        QList<QSslCertificate> result;
        for (const QByteArray& der : order) {
            ++stats.Seen;
            const Verdict verdict = verdicts.value(der);
            if (withUsageProperty.contains(der))
                ++stats.WithDisallowedUsageProperty;
            switch (verdict) {
            case Verdict::Usage: ++stats.RemovedByUsage; continue;
            case Verdict::DisallowedStore: ++stats.RemovedByDisallowedStore; continue;
            case Verdict::DisallowedFileTime: ++stats.RemovedByDisallowedFileTime; continue;
            case Verdict::DisallowedUsage: ++stats.RemovedByDisallowedUsage; continue;
            case Verdict::Keep: break;
            }
            const QSslCertificate certificate(der, QSsl::Der);
            if (!certificate.isNull()) {
                result.append(certificate);
                ++stats.Exported;
            }
        }
        if (statsOut != nullptr)
            *statsOut = stats;
        return result;
    }
#else
    QList<QSslCertificate> MqttTlsSupport::systemRootCertificates(RootExportStats* stats) {
        const QList<QSslCertificate> certificates = QSslConfiguration::systemCaCertificates();
        if (stats != nullptr) {
            *stats = RootExportStats();
            stats->Seen = stats->Exported = static_cast<int>(certificates.size());
        }
        return certificates;
    }
#endif

    QString MqttTlsSupport::bundleFileName() {
        return "mqtt-trusted-roots.pem";
    }

    QString MqttTlsSupport::defaultBundleDirectory() {
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }

    QString MqttTlsSupport::writeCaBundle(const QList<QSslCertificate>& certificates, const QString& directory, QString* error) {
        auto fail = [error](const QString& text) {
            if (error != nullptr)
                *error = text;
            return QString();
        };

        QByteArray bundle;
        for (const QSslCertificate& certificate : certificates) {
            if (!certificate.isNull())
                bundle += certificate.toPem();
        }
        if (bundle.isEmpty())
            return fail(QObject::tr("The certificate store of the system contains no trusted root certificates, so the "
                                    "certificate of the server cannot be verified. Set a CA file in the settings."));

        const QString path = QDir(directory).filePath(bundleFileName());
        // Nothing to write, if the file is up to date (it may be in a directory that cannot be written now).
        {
            QFile existing(path);
            if (existing.open(QIODevice::ReadOnly) && existing.readAll() == bundle)
                return path;
        }
        if (directory.isEmpty() || !QDir().mkpath(directory))
            return fail(QObject::tr("The trusted root certificates cannot be saved: the directory '%1' cannot be created. "
                                    "Set a CA file in the settings.").arg(QDir::toNativeSeparators(directory)));
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(bundle) != bundle.size() || !file.commit())
            return fail(QObject::tr("The trusted root certificates cannot be saved to '%1': %2. Set a CA file in the "
                                    "settings.").arg(QDir::toNativeSeparators(path), file.errorString()));
        return path;
    }

    MqttTlsOptions MqttTlsSupport::prepare(DigitalTwinClientSettings& settings) {
        return prepare(settings.getMQTTServer(), settings.getMQTTPort(), settings.getMQTTUseTLS(), settings.getMQTTCaFile(),
                       defaultBundleDirectory(),
                       []() { return systemRootCertificates(); });
    }

    MqttTlsOptions MqttTlsSupport::prepare(const QString& server, const QString& port, bool useTls, const QString& caFile,
                                           const QString& bundleDirectory,
                                           const std::function<QList<QSslCertificate>()>& systemCertificates) {
        MqttTlsOptions options;
        options.Enabled = useTls;
        // Not TLS (no switch, no TLS scheme): nothing to prepare.
        if (!DigitalTwin::Communication::makeMqttEndpoint(server.toStdString(), port.toStdString(), useTls).Tls)
            return options;

        const QString customFile = caFile.trimmed();
        if (!customFile.isEmpty()) {
            // Replaces the system certificates. The library checks the file for every connect attempt; the problem is
            // named here already, so that the text is the same for all causes.
            options.CaFile = customFile.toUtf8().toStdString();
            const std::string problem = DigitalTwin::Communication::checkMqttCaFile(options.CaFile);
            if (!problem.empty())
                options.SetupError = problem;
            return options;
        }

        QString error;
        const QString bundle = writeCaBundle(systemCertificates(), bundleDirectory, &error);
        if (bundle.isEmpty())
            options.SetupError = error.toStdString();
        else
            options.CaFile = bundle.toUtf8().toStdString();
        return options;
    }
}
