//
// Created by herzog on 04.08.26.
//

#pragma once

#include "IDigitalTwinElement.h"

namespace DigitalTwin::Model
{
    class Function : public IDigitalTwinElement
    {
    public:
        Function() = default;

        explicit Function(const std::string& name);

        ~Function() override;

    private:
    };
} // DigitalTwin::Model

