//
// Created by Moritz Herzog on 14.12.23.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#pragma once
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <string>
#include <utility>
#include <nlohmann/json.hpp>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "IEntitie.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace BACKEND_COMMUNICATION::ENTITIES {
    /**
     *
     */
    class User : public IEntitie{
    public:
        /**
         *
         */
        User() = delete;
        /**
         *
         * @param username
         * @param password
         */
        User(std::string username, std::string password);
        /**
         *
         */
        ~User() = default;

        std::string generateRestCall() override;

    private:
        std::string Username;
        std::string Password;
    };

    User::User(std::string username, std::string password) : IEntitie() {
        Username = std::move(username);
        Password = std::move(password);
    }

    std::string User::generateRestCall() {
        nlohmann::json js;
        js["email"] = Username;
        js["password"] = Password;

        return js.dump();
    }
}
