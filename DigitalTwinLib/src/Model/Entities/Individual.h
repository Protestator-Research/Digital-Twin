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
        void appendAttribute(IVariable* variable) override;
        void appendControllable(IVariable* variable) override;
        void appendMeasurable(IVariable* variable) override;
        IVariable* resolveVariable(std::string name) override;

    };
} // DigitalTwin::Model

