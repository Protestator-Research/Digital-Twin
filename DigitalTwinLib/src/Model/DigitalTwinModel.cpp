//
// Created by Moritz Herzog on 23.05.24.
//

#include <sysmlv2/rest/entities/DataIdentity.h>
#include <kerml/root/elements/Element.h>
#include <BaseFuctions/StringExtention.hpp>
#include <boost/uuid/uuid.hpp>
#include <iostream>
#include <kerml/root/annotations/TextualRepresentation.h>
#include <sysmlv2/rest/entities/Commit.h>
#include <sysmlv2/rest/entities/IEntity.h>
#include <sysmlv2/rest/entities/Project.h>
#include <sysmlv2/Parser.h>


#include "DigitalTwinModel.h"
#include "Entities/IDigitalTwinElement.h"
#include "Entities/Component.h"
#include "Entities/Port.h"
#include "Exceptions/DigitalTwinAddressException.h"
#include "../DigitalTwinManager.h"
#include "entities/DigitalTwin.h"
#include "../Parser/SysMLv2BaseListener.h"
#include "../Parser/SysMLv2Lexer.h"
#include "../Parser/SysMLv2Parser.h"

#include "Entities/Port.h"

namespace DigitalTwin::Model {
    DigitalTwinModel::DigitalTwinModel(std::shared_ptr<SysMLv2::REST::DigitalTwin> digitalTwin, DigitalTwinManager *manager) :
            DigitalTwin(digitalTwin),
            Manager(manager),
            UpdateModelFunction([]{})
    {
        generateDigitalTwinBackend();
    }

    DigitalTwinModel::~DigitalTwinModel() {

    }

    void DigitalTwinModel::generateDigitalTwinBackend() {
        DigitalTwinModelElements = Manager->downloadDigitalTwinModel(DigitalTwin->owningProject()->getId(), DigitalTwin->referencedCommit()->getId());

        std::string completeModel;

        for(const auto &elem : DigitalTwinModelElements)
            if ((std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(elem)->language()!="Markdown")&& (std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(elem)->language() != "YaML"))
                completeModel+=std::dynamic_pointer_cast<KerML::Entities::TextualRepresentation>(elem)->body();

        antlr4::ANTLRInputStream input(completeModel);
        auto listenerImplementation = new SysMLv2BaseListener();
        SysMLv2Lexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        SysMLv2Parser parser(&tokens);
        parser.addParseListener(listenerImplementation);
        parser.start();

        auto digitalTwinElements = listenerImplementation->getElements();

        for(auto dtElement : digitalTwinElements) {
            auto component = dynamic_cast<Component*>(dtElement);
            if (component != nullptr)
                ComponentMap.insert(std::make_pair(component->getName(), component));

            auto port = dynamic_cast<Port*>(dtElement);
            if (port != nullptr)
                PortMap.insert(std::make_pair(port->getName(), port));
        }
    }

    std::string DigitalTwinModel::digitalTwinName() {
        return DigitalTwin->getName();
    }

    void DigitalTwinModel::setUpdateModelFunction(std::function<void()> updateModel) {
        UpdateModelFunction = updateModel;
    }

    std::vector<IDigitalTwinElement *> DigitalTwinModel::getAllComponents() const {
        std::vector<IDigitalTwinElement*> returnValue=std::vector<IDigitalTwinElement*>();

        for(auto element : ComponentMap)
            returnValue.push_back(element.second);

        return returnValue;
    }

    std::vector<std::string> DigitalTwinModel::getElementStrings() {
        std::vector<std::string> elements;

        for(const auto& element : ComponentMap)
            for(const auto& string : dynamic_cast<Component*>(element.second)->getAllMQTTTopics())
                elements.push_back(element.first + "/" + string);

        return elements;
    }

    Component *DigitalTwinModel::getComponentWithAddress(std::string address) {
        const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');

        if(splittedAdress.size()<1)
            throw DigitalTwinAddressException();

        if(splittedAdress.size()==1)
            return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]]);

        std::string addressWithHigherIndex="";
        for(size_t i = 1; i<splittedAdress.size(); i++){
            addressWithHigherIndex+=splittedAdress[i];
            if(i<(splittedAdress.size()-1))
                addressWithHigherIndex+="/";
        }

        return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]])->getComponent(addressWithHigherIndex);
    }

    Variable<std::any>* DigitalTwinModel::getVariableWithAddress(std::string address) {
        const auto splittedAdress = CPSBASELIB::STD_EXTENTION::StringExtention::splitString(address, '/');

        if(splittedAdress.size()<2)
            throw DigitalTwinAddressException();

        std::string addressWithHigherIndex="";
        for(size_t i = 1; i<splittedAdress.size(); i++){
            addressWithHigherIndex+=splittedAdress[i];
            if(i<(splittedAdress.size()-1))
                addressWithHigherIndex+="/";
        }

        return dynamic_cast<Component*>(ComponentMap[splittedAdress[0]])->resolveVariable(addressWithHigherIndex);
    }
}
