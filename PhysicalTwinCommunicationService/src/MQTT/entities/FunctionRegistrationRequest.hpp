//
// Created by herzog on 05.10.26.
//

#ifndef DIGITALTWIN_REGISTRATIONREQUEST_H
#define DIGITALTWIN_REGISTRATIONREQUEST_H

#include <string>
#include <vector>
#include <utility>
#include <nlohmann/json.hpp>

namespace DigitalTwin::Communication
{
    class FunctionRegistrationRequest
    {
    public:
        FunctionRegistrationRequest() = delete;

        FunctionRegistrationRequest(const std::string& qualifiedName, const std::vector<std::pair<std::string, std::string>>& linkedVariablesWithQualifiedName, const std::string& linkedReturnQualifiedName)
        {
            QualifiedName = qualifiedName;
            LinkedVariablesWithQualifiedName = linkedVariablesWithQualifiedName;
            LinkedReturnQualifiedName = linkedReturnQualifiedName;
        }

        FunctionRegistrationRequest(std::string jsonString)
        {
            nlohmann::json json = nlohmann::json::parse(jsonString);
            QualifiedName = json["qualified_name"].get<std::string>();
            LinkedVariablesWithQualifiedName = json["linked_variables"];
            LinkedReturnQualifiedName = json["linked_variable_return"];
        }

        std::string toJsonString() const
        {
            nlohmann::json json;
            json["qualified_name"] = QualifiedName;
            json["linked_variables"] = LinkedVariablesWithQualifiedName;
            json["linked_variable_return"] = LinkedReturnQualifiedName;
            return json.dump();
        }

        std::vector<std::pair<std::string, std::string>> getLinkedVariablesWithQualifiedName() const
        {
            return LinkedVariablesWithQualifiedName;
        }

        std::string getQualifiedName() const
        {
            return QualifiedName;
        }

        std::string getLinkedReturnQualifiedName() const
        {
            return LinkedReturnQualifiedName;
        }

    private:
        std::string QualifiedName;
        std::vector<std::pair<std::string, std::string>> LinkedVariablesWithQualifiedName;
        std::string LinkedReturnQualifiedName;
    };
}

#endif //DIGITALTWIN_REGISTRATIONREQUEST_H
