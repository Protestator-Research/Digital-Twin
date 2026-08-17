//
// Created by herzog on 04.08.26.
//

#pragma once

#include "ICollectionType.h"

namespace DigitalTwin::Model
{
    class Individual : public ICollectionType
    {
    public:
        Individual() = default;

        explicit Individual(const std::string& name);

        void appendComponent(Component* compoonent) override;
        void appendPort(Port* port) override;
        void appendAttribute(Variable<std::any>* variable) override;
        void appendControllable(Variable<std::any>* variable) override;
        void appendMeasurable(Variable<std::any>* variable) override;
        Variable<std::any>* resolveVariable(std::string name) override;

    };
} // DigitalTwin::Model

