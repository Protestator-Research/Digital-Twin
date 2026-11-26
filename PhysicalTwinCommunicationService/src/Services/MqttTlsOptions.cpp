//
// TLS for the MQTT client, see MqttTlsOptions.h.
//

#include "MqttTlsOptions.h"

#include <mqtt/ssl_options.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace DigitalTwin::Communication {
    namespace {
        std::string toLower(std::string text) {
            std::transform(text.begin(), text.end(), text.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return text;
        }

        bool contains(const std::string& text, const char* part) {
            return text.find(part) != std::string::npos;
        }

        /** File names are UTF-8 in this library (Qt: QString::toUtf8()), also on Windows. */
        std::filesystem::path pathFromUtf8(const std::string& path) {
            return std::filesystem::path(std::u8string(path.begin(), path.end()));
        }
    }

    namespace {
        /** "CC3A0000:error:0A000086:SSL routines::certificate verify failed:ssl\statem\statem_clnt.c:2126:" -> "certificate verify failed" */
        std::string openSslReason(const std::string& message) {
            // thread:error:code:library:function:reason:file:line:
            std::vector<std::string> fields;
            std::string field;
            for (const char c : message) {
                if (c == ':') {
                    fields.push_back(field);
                    field.clear();
                } else {
                    field += c;
                }
            }
            if (!field.empty())
                fields.push_back(field);
            // the thread id in front is not always there
            for (std::size_t i = 0; i < fields.size() && i < 2; ++i) {
                if (fields[i] == "error" && fields.size() > i + 4)
                    return fields[i + 4];
            }
            return message;
        }

        /** "SSL_CTX_load_verify_locations" failed: the last error of OpenSSL as text. */
        std::string openSslLastError() {
            const unsigned long code = ERR_peek_last_error();
            if (code == 0)
                return {};
            char buffer[256];
            ERR_error_string_n(code, buffer, sizeof(buffer));
            return buffer;
        }

        std::string collapseWhitespace(const std::string& text) {
            std::string result;
            bool space = false;
            for (const char c : text) {
                if (std::isspace(static_cast<unsigned char>(c))) {
                    space = !result.empty();
                } else {
                    if (space)
                        result += ' ';
                    space = false;
                    result += c;
                }
            }
            return result;
        }
    }

    MqttEndpoint makeMqttEndpoint(const std::string& server, const std::string& port, bool tlsEnabled) {
        MqttEndpoint endpoint;
        std::string scheme;
        std::string rest = server;
        const auto separator = server.find("://");
        if (separator != std::string::npos) {
            scheme = server.substr(0, separator);
            rest = server.substr(separator + 3);
        }
        const std::string lowerScheme = toLower(scheme);

        std::string outScheme;
        if (scheme.empty()) {
            outScheme = tlsEnabled ? "ssl" : "tcp";
            endpoint.Tls = tlsEnabled;
        } else if (lowerScheme == "ssl" || lowerScheme == "mqtts" || lowerScheme == "tls") {
            outScheme = "ssl";
            endpoint.Tls = true;
            if (!tlsEnabled)
                endpoint.Notice = "The server address has the TLS scheme '" + scheme +
                                  "://', so TLS is used although it is not switched on in the settings.";
        } else if (lowerScheme == "wss") {
            outScheme = "wss";
            endpoint.Tls = true;
            if (!tlsEnabled)
                endpoint.Notice = "The server address has the TLS scheme 'wss://', so TLS is used although it is "
                                  "not switched on in the settings.";
        } else if (lowerScheme == "tcp" || lowerScheme == "mqtt" || lowerScheme == "ws") {
            if (tlsEnabled) {
                // TLS wins, the connection is never downgraded to plain text.
                outScheme = lowerScheme == "ws" ? "wss" : "ssl";
                endpoint.Tls = true;
                endpoint.Notice = "TLS is switched on in the settings, so the scheme '" + scheme + "://' of the server "
                                  "address is replaced by '" + outScheme + "://'.";
            } else {
                outScheme = lowerScheme;
            }
        } else {
            // Not known: Paho reports it. TLS is still applied if it is switched on.
            outScheme = scheme;
            endpoint.Tls = tlsEnabled;
        }

        endpoint.Uri = outScheme + "://" + rest;
        if (!port.empty())
            endpoint.Uri += ":" + port;
        return endpoint;
    }

    mqtt::ssl_options makeMqttSslOptions(const MqttTlsOptions& options) {
        mqtt::ssl_options ssl;
        // Chain verification: SSL_CTX_set_verify(SSL_VERIFY_PEER). Paho's default is on, but it is set here, so that it
        // does not depend on a default.
        ssl.set_enable_server_cert_auth(true);
        // Host name check after the handshake: Paho's default is OFF (struct field "verify" is 0 in
        // MQTTAsync_SSLOptions_initializer), a certificate for any other name would be accepted without it.
        ssl.set_verify(true);
        // With a trust store Paho loads only this file (SSL_CTX_load_verify_locations). Without one it uses the default
        // verify paths of OpenSSL; the C++ wrapper of this Paho version has no way to switch that off.
        if (!options.CaFile.empty())
            ssl.set_trust_store(options.CaFile);
        return ssl;
    }

    std::string checkMqttCaFile(const std::string& path) {
        if (path.empty())
            return "No CA file given.";
        std::error_code error;
        const auto file = pathFromUtf8(path);
        if (!std::filesystem::exists(file, error))
            return "The CA file '" + path + "' does not exist.";
        if (!std::filesystem::is_regular_file(file, error))
            return "The CA file '" + path + "' is not a file.";
        std::ifstream stream(file, std::ios::binary);
        if (!stream)
            return "The CA file '" + path + "' cannot be read.";
        const std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        if (!contains(content, "-----BEGIN CERTIFICATE-----") && !contains(content, "-----BEGIN TRUSTED CERTIFICATE-----"))
            return "The CA file '" + path + "' contains no PEM certificate.";
        // Load it like Paho does at connect, so that a file that looks right but cannot be parsed is found here.
        SSL_CTX* context = SSL_CTX_new(TLS_client_method());
        if (context == nullptr)
            return "The CA file '" + path + "' could not be checked: OpenSSL is not available.";
        ERR_clear_error();
        const int loaded = SSL_CTX_load_verify_locations(context, path.c_str(), nullptr);
        const std::string reason = loaded == 1 ? std::string() : openSslLastError();
        SSL_CTX_free(context);
        ERR_clear_error();
        if (loaded != 1)
            return "The CA file '" + path + "' could not be loaded as PEM certificates" +
                   (reason.empty() ? std::string() : " (" + openSslReason(reason) + ")") + ".";
        return {};
    }

    MqttTlsErrorInfo describeTlsError(const std::string& opensslMessage) {
        MqttTlsErrorInfo info;
        const std::string text = toLower(collapseWhitespace(opensslMessage));
        if (contains(text, "certificate verify failed")) {
            info.Text = "TLS certificate problem: the certificate of the server could not be verified (it is not issued by a "
                        "trusted CA, it is expired or not yet valid, or its chain is incomplete). The root of the server's "
                        "certificate may also be missing from the certificate store of the system (the system only has the "
                        "roots it has already fetched); a CA file with the root can be set in the settings. Also check the "
                        "certificate of the server.";
        } else if (contains(text, "wrong version number") || contains(text, "packet length too long") ||
                   contains(text, "http request") || contains(text, "unsupported protocol") ||
                   contains(text, "record layer failure") || contains(text, "bad record") ||
                   contains(text, "wrong ssl version")) {
            info.Text = "TLS problem: the server does not speak TLS on this port (is it a port for plain MQTT?).";
        } else if (contains(text, "unexpected eof") || contains(text, "connection reset") || contains(text, "eof")) {
            info.Text = "TLS problem: the server closed the connection during the TLS handshake (does it speak TLS on this "
                        "port?).";
        } else if (contains(text, "alert")) {
            info.Text = "TLS problem: the server rejected the TLS handshake (" + openSslReason(collapseWhitespace(opensslMessage)) + ").";
        } else {
            info.Text = "TLS problem: " + openSslReason(collapseWhitespace(opensslMessage));
        }
        return info;
    }
}
