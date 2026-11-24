//
// Created by Moritz Herzog on 17.01.24.
//

#include "DigitalTwinManager.h"
#include "Model/DigitalTwinModel.h"

#include <BECommunicationService.h>
#include <entities/DigitalTwin.h>
#include <kerml/root/elements/Element.h>
#include <../../PhysicalTwinCommunicationService/src/MQTT/entities/DigitalTwinEntity.h>
#include <sysmlv2/service/online/HttpException.h>
#include <boost/uuid.hpp>
#include <MQTT/Topics.h>
#include <iostream>
#include <sysmlv2/rest/entities/Project.h>

#include "MQTT/entities/FunctionRegistrationRequest.hpp"


namespace DigitalTwin {

    DigitalTwinManager::DigitalTwinManager(BACKEND_COMMUNICATION::CommunicationService *communicationService, DigitalTwin::Communication::IMqttClientService* clientService, bool isClient) {
        BackendCommunicationService = communicationService;
        ClientService = clientService;
        IsClient = isClient;
    }

    DigitalTwinManager::~DigitalTwinManager() {

    }

    void DigitalTwinManager::downloadDigitalTwin(boost::uuids::uuid projectId, boost::uuids::uuid digitalTwinId) {
        try {
            auto digitalTwins = BackendCommunicationService->getAllDigitalTwinsForProjectWithId(projectId);

            for (auto digitalTwin: digitalTwins)
                if (digitalTwin->getId() == digitalTwinId) {
                    auto returnValue = new Model::DigitalTwinModel(digitalTwin, this);
                    {
                        std::lock_guard lg(DigitalTwinModelMapMutex);
                        DigitalTwinModelMap.insert(std::make_pair(digitalTwin->getId(), returnValue));
                    }
                    generateMQTTInterface(returnValue);
                }
        } catch(SysMLv2::API::EXCEPTIONS::HTTPException& ex) {
            std::cout << ex.what() <<std::endl;
        }
    }

    std::vector < std::shared_ptr < KerML::Entities::Element >> DigitalTwinManager::downloadDigitalTwinModel(boost::uuids::uuid projectId, boost::uuids::uuid commitId) {
        return BackendCommunicationService->getAllElementsOfCommit(projectId,commitId);
    }

    Model::DigitalTwinModel* DigitalTwinManager::addDigitalTwinAndCreateModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin) {
        Model::DigitalTwinModel* returnValue = new Model::DigitalTwinModel(digitalTwin,this);
        DigitalTwin::Communication::DigitalTwinEntity entity(digitalTwin->getId(), digitalTwin->owningProject()->getId());
        const std::string announcement = entity.serialize();
        {
            std::lock_guard lg(DigitalTwinModelMapMutex);
            DigitalTwinModelMap.insert(std::make_pair(digitalTwin->getId(),returnValue));
            TwinAnnouncements[digitalTwin->getId()] = announcement;
        }
        // Dropped if the client is not connected yet; announceOpenTwins sends it again after the connect.
        ClientService->publish(DigitalTwin::Communication::CONNECT_TO_TWIN, announcement);
        return returnValue;
    }

    void DigitalTwinManager::setClientService(DigitalTwin::Communication::IMqttClientService* clientService) {
        // Only the client replaces its connection; it never subscribed anything through generateMQTTInterface, so
        // there are no subscriptions to move. The announcements are sent by announceOpenTwins on the new connect.
        ClientService = clientService;
    }

    void DigitalTwinManager::announceOpenTwins() {
        std::vector<std::string> announcements;
        {
            std::lock_guard lg(DigitalTwinModelMapMutex);
            for (const auto& entry : TwinAnnouncements)
                announcements.push_back(entry.second);
        }
        for (const auto& announcement : announcements)
            ClientService->publish(DigitalTwin::Communication::CONNECT_TO_TWIN, announcement);
    }

    void DigitalTwinManager::generateMQTTInterface(const std::string& baseName, const std::pair<std::string, std::shared_ptr<DigitalTwin::Communication::ValueEntity>>& element) {
        if(element.second == nullptr)
            return;
        std::string topic = baseName + "/" + element.first;
        ClientService->publish(topic, element.second->getJson());
        const auto updateVariableFunction = [this](std::string topic, std::string payload)
            {
                std::cout << "Variable with Address: " << topic << " is updated with Value:" << std::endl << payload << std::endl;
            };
        ClientService->subscribe(topic, updateVariableFunction);
    }

    void DigitalTwinManager::setCallbackDispatcher(std::function<void(std::function<void()>)> dispatcher) {
        CallbackDispatcher = std::move(dispatcher);
    }

    void DigitalTwinManager::runOnOwningThread(std::function<void()> work) {
        if (CallbackDispatcher)
            CallbackDispatcher(std::move(work));
        else
            work();
    }

    void DigitalTwinManager::generateMQTTInterface(Model::DigitalTwinModel* digitalTwin) {
        std::string baseName = digitalTwin->digitalTwinName();
        for(const auto &element : digitalTwin->getElementValueEntities()) {
            generateMQTTInterface(baseName, element);
        }
        const auto DTFunctionRegistrationReq = DigitalTwin::Communication::FunctionRegistrationRequest("",{},"");
        ClientService->publish(baseName + "/" + DigitalTwin::Communication::REGISTER_FUNCTION, DTFunctionRegistrationReq.toJsonString());
        // Runs on a thread of the MQTT library. The server and every client publish an empty request here, which is
        // not a registration. The model is changed through the dispatcher (the GUI thread in the client).
        const auto registerFunctionFunction = [digitalTwin, this]([[maybe_unused]] std::string topic, std::string payload)->void {
            std::unique_ptr<DigitalTwin::Communication::FunctionRegistrationRequest> request;
            try {
                request = std::make_unique<DigitalTwin::Communication::FunctionRegistrationRequest>(payload);
            } catch (const std::exception& e) {
                std::cerr << "Ignoring malformed function registration: " << e.what() << std::endl;
                return;
            }
            if (request->getQualifiedName().empty())
                return;
            auto shared = std::shared_ptr<DigitalTwin::Communication::FunctionRegistrationRequest>(std::move(request));
            runOnOwningThread([digitalTwin, shared]() { digitalTwin->registerFunction(shared.get()); });
        };
        ClientService->subscribe(baseName + "/" + DigitalTwin::Communication::REGISTER_FUNCTION, registerFunctionFunction);

    }
}
