//
// Created by Moritz Herzog on 05.01.24.
//

#include "ConnectionError.h"

namespace BACKEND_COMMUNICATION::EXCEPTIONS {
    ConnectionError::ConnectionError(CONNECTION_ERROR_TYPE errorType)
    : runtime_error("") {
        ErrorType = errorType;
    }

    const char *ConnectionError::what() const noexcept {
        switch (ErrorType) {
            case UNSUPPORTED_PROTOCOL:
                return "The used protocol is unsupported. \r\nError: " << ErrorType;
            case FAILED_INIT:
                return "Communication initialization Failed.";
            case URL_FORMAT:
                return "Wrong URL Format.";
            case UNKNOWN_PROTOCOL:
                return "Protocol unknown and thus unsupported.";
            case COULD_NOT_RESOLVE_PROXY:
                return "Given proxy could not be resolved.";
            case COULD_NOT_RESOLVE_HOST:
                return "Given host could not be resolved.";
            case COULD_NOT_CONNECT:
                return "Could not connect to host.";
            case WIRED_SERVER_REPLY:
                return "Server reply could not be parsed.";
            case REMOTE_ACCESS_DENIED:
                return "Remote access was denied.";
            case FTP_ACCESS_FAILED:
                return "Access to ftp server failed.";
            case FTP_WEIRED_PASS_REPLY:
                return "Ftp server password reply could not be parsed.";
            case FTP_ACCEPT_TIMEOUT:
                return "Accept response of the ftp server timed out.";
            case FTP_WEIRED_PASV_REPLY:
                return "Could not get sensible result back from the ftp server.";
            case FTP_WEIRED_227_FORMAT:
                return "";
            case FTP_CANT_GET_HOST:
                break;
            case HTTP2:
                break;
            case FTP_COULDNT_SET_TYPE:
                break;
            case PARTIAL_FILE:
                break;
            case FTP_COULDNT_RETR_FILE:
                break;
            case QUOTE_ERROR:
                break;
            case HTTP_RETURNED_ERROR:
                break;
            case WRITE_ERROR:
                break;
            case UPLOAD_FAILED:
                break;
            case READ_ERROR:
                break;
            case OUT_OF_MEMORY:
                break;
            case OPERATION_TIMEDOUT:
                break;
            case FTP_PORT_FAILED:
                break;
            case FTP_COULDNT_USE_REST:
                break;
            case RANGE_ERROR:
                break;
            case HTTP_POST_ERROR:
                break;
            case SSL_CONNECT_ERROR:
                break;
            case BAD_DOWNLOAD_RESUME:
                break;
            case FILE_COUNDNT_READ_FILE:
                break;
            case LDAP_CANNOT_BIND:
                break;
            case LDAP_SEARCH_FAILED:
                break;
            case FUNCTION_NOT_FOUND:
                break;
            case ABORTED_BY_CALLBACK:
                break;
            case BAD_FUNCTION_ARGUMENT:
                break;
            case INTERFACE_FAILED:
                break;
            case TOO_MANY_REDIRECTS:
                break;
            case UNKNOWN_OPTION:
                break;
            case GOT_NOTHING:
                break;
            case SLL_ENGINE_NOTFOUND:
                break;
            case SSL_ENGINE_SETFAILED:
                break;
            case SEND_ERROR:
                break;
            case RECV_ERROR:
                break;
            case SSL_CERTPROBLEM:
                break;
            case SSL_CIPHER:
                break;
            case PEER_FAILED_VERIFICATION:
                break;
            case BAD_CONTENT_ENCODING:
                break;
            case FILE_SIZE_EXCEEDED:
                break;
            case USE_SSL_FAILED:
                break;
            case SEND_FAIL_REWIND:
                break;
            case SSL_ENGINE_INITFAILED:
                break;
            case LOGIN_DENIED:
                break;
            case TFTP_NOTFOUND:
                break;
            case TFTP_PERM:
                break;
            case REMOTE_DISK_FULL:
                break;
            case TFP_ILLEGAL:
                break;
            case TFTP_UNKNOWNID:
                break;
            case REMOTE_FILE_EXITIS:
                break;
            case TFTP_NOSUCHUSER:
                break;
            case SSL_CACERT_BADFILE:
                break;
            case REMOTE_FILE_NOT_FOUND:
                break;
            case SSH:
                break;
            case SSL_SHUTDOWN_FAILED:
                break;
            case AGAIN:
                break;
            case SSL_CRL_BADFILE:
                break;
            case SSL_ISSUER_ERROR:
                break;
            case FTP_PRET_FAILED:
                break;
            case RTSP_CSEQ_ERROR:
                break;
            case RTSP_SESSION_ERROR:
                break;
            case FTP_BAS_DILE_LIST:
                break;
            case CHUNK_FAILED:
                break;
            case NO_CONNECTION_AVAILABLE:
                break;
            case SSK_PINNEDPUBKEYNOTMATCH:
                break;
            case SSL_INVALIDCERTSTATUS:
                break;
            case HTTP2_STREAM:
                break;
            case RECURSIVE_API_CALL:
                break;
            case AUTH_ERROR:
                break;
            case HTTP3:
                break;
            case QUIC_CONNECT_ERROR:
                break;
            case PROXY:
                break;
            case SSL_CLIENTCERT:
                break;
            case UNRECOVERABLE_POLL:
                break;
            case TOO_LARGE:
                break;
        }
    }


}