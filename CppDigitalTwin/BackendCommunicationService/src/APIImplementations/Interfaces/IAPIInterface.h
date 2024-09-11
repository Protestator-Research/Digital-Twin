//
// Created by Moritz Herzog on 05.09.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_IAPIINTERFACE_H
#define DIGITALTWIN_IAPIINTERFACE_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "../StatusCode.h"
#include "../../cpp_backend_communication_global.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------




namespace BACKEND_COMMUNICATION {

    class CPSBACKENDCOMMUNICATION_EXPORT IAPIInterface {
    public:
        IAPIInterface() = default;

        ~IAPIInterface() = default;

    protected:
        /**
         * Disconnects the server client connection and cleans up.
         */
        void disconnectAndCleanUp();

        /**
         * Sets up the CURL connection with everything that is needed.
         * @param urlAppendix The URL appendix (Everything after the first slash).
         * @param barrierString The barrier string that is needed for the login
         * @param postPayload The payload for the post command.
         * @return CURL object allowing for the connection to every REST endpoint.
         */
        virtual CURL* setUpServerConnection(const char* urlAppendix, const char* barrierString, const char* postPayload) = 0;

        /**
         * The Method, that allows the readout of the the CURL strings that are needed for the parsing. This is defined
         * by the CURL interface.
         * @see CURL
         */
        static size_t WriteBufferCallback(char *contents, size_t size, size_t nmemb, void* userp);

        /**
         * Trys to resolve the HTTP status code. Depended on the Error code the application runs or throws an exception.
         * @param httpErrorCode The status code received by CURL.
         * @param instance The CURL instance for further debugging and error resolution.
         * @return An internal statuscode allowing for the debugging and error resolution, if no exception is thrown.
         */
        virtual INTERNAL_STATUS_CODE tryToResolveHTTPError(long httpErrorCode, void* instance) = 0;
    };
}

#endif //DIGITALTWIN_IAPIINTERFACE_H
