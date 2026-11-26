//
// TLS for the MQTT client: the options, the server URI (scheme handling) and the Paho SSL options.
//

#ifndef DIGITALTWIN_MQTTTLSOPTIONS_H
#define DIGITALTWIN_MQTTTLSOPTIONS_H

#include <chrono>
#include <string>

#include "../cpp_physical_twin_communication_global.h"

// Not included here, so that users of the options (the Qt client) need no Paho headers; the user of
// makeMqttSslOptions() includes <mqtt/ssl_options.h>.
namespace mqtt {
    class ssl_options;
}

namespace DigitalTwin::Communication {
    /**
     * TLS settings of an MQTT connection. There is deliberately no switch to turn the verification of the server
     * certificate off: with TLS the certificate chain is always checked against the trusted roots and the host name
     * (or IP address) of the connection always has to be in the certificate.
     */
    struct MqttTlsOptions {
        /**
         * Use TLS (Paho scheme ssl://). TLS is also used, if the server string has a TLS scheme (ssl://, mqtts://,
         * tls://, wss://); see makeMqttEndpoint() for the case that the two disagree.
         */
        bool Enabled = false;
        /**
         * PEM file with the trusted CA certificates. It is required for TLS: Paho would fall back to the default verify
         * paths of OpenSSL if there were none, but the OpenSSL of the Conan packages does NOT use the certificate store of
         * the operating system (on Windows not at all), so that fallback trusts nothing. TLS without a CA file (and
         * without SetupError) is therefore reported as a configuration problem and does not connect. The file REPLACES
         * all other roots (they are not added), so a private CA is not mixed with the public ones.
         */
        std::string CaFile;
        /**
         * Set by an application that could not prepare the trusted roots (for example the system certificate store
         * is empty or the bundle cannot be written). The client then does not connect, reports this text as a TLS
         * configuration problem and does not retry. Empty: no problem.
         */
        std::string SetupError;
    };

    /**
     * The address a client connects to. Note: IPv6 literals ("[::1]") do not work with TLS in this Paho version (the
     * host name check of Paho cannot handle the brackets), use a host name or an IPv4 address.
     */
    struct MqttEndpoint {
        /** The URI for Paho: "tcp://host:port" or "ssl://host:port" (other schemes as typed). */
        std::string Uri;
        /** True if the connection uses TLS (the TLS options apply). */
        bool Tls = false;
        /**
         * Not empty if the settings contradicted each other or the scheme was rewritten; a text for the log. Never
         * contains credentials (only the scheme and the TLS flag are mentioned).
         */
        std::string Notice;
    };

    /**
     * Builds the URI from the server string, the port and the TLS option.
     *
     * Schemes (case-insensitive): ssl://, mqtts://, tls:// mean TLS and are written as ssl://; tcp://, mqtt:// mean
     * plain TCP and are written as tcp://; wss:// is TLS, ws:// is plain (both kept). Without a scheme tcp:// is used,
     * or ssl:// if tlsEnabled is set. Any other scheme is kept as typed and Paho reports it.
     *
     * If the scheme and tlsEnabled disagree, TLS wins: a plain scheme (tcp://, mqtt://, ws://) with tlsEnabled is
     * connected with TLS (ssl:// or wss://) and a TLS scheme without tlsEnabled is connected with TLS as well. The
     * connection is never silently downgraded to plain text. In the first case Notice says that the plain scheme was
     * replaced.
     */
    CPPPHYSICALTWINCOMMUNICATION_EXPORT MqttEndpoint makeMqttEndpoint(const std::string& server, const std::string& port,
                                                                      bool tlsEnabled);

    /**
     * The Paho SSL options for a verified connection: the server certificate is verified against the trust store
     * (enable_server_cert_auth, SSL_VERIFY_PEER) and, after the handshake, the host name or IP address of the URI is
     * checked against the certificate (verify, X509_check_host / X509_check_ip_asc). The trust store is the CA file,
     * if there is one.
     */
    CPPPHYSICALTWINCOMMUNICATION_EXPORT mqtt::ssl_options makeMqttSslOptions(const MqttTlsOptions& options);

    /**
     * Checks that a CA file can be used as the trust store: it exists, can be read, contains a PEM certificate and
     * OpenSSL can load it (SSL_CTX_load_verify_locations, as Paho does at connect).
     * @return An empty text if it is usable, otherwise what is wrong (with the file name).
     */
    CPPPHYSICALTWINCOMMUNICATION_EXPORT std::string checkMqttCaFile(const std::string& path);

    /** What a message of OpenSSL about a failed TLS handshake means. */
    struct MqttTlsErrorInfo {
        /** A sentence for the user (starts with "TLS"). */
        std::string Text;
    };

    /**
     * Turns the messages of OpenSSL that Paho hands over for a failed handshake ("error:0A000086:SSL routines::certificate
     * verify failed:ssl\statem\statem_clnt.c:2126:") into a text for the user. Paho and OpenSSL do not tell why the
     * verification failed (unknown issuer, expired, ...), only that it did. Messages that are not known are passed on
     * as they are (without the source file and line). Every such error is a failed connect attempt that is retried.
     */
    CPPPHYSICALTWINCOMMUNICATION_EXPORT MqttTlsErrorInfo describeTlsError(const std::string& opensslMessage);
}

#endif //DIGITALTWIN_MQTTTLSOPTIONS_H
