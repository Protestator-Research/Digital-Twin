//
// Created by herzog on 04.08.26.
//

#pragma once

#include <vector>
#include <any>

#include "IDigitalTwinElement.h"
#include "Variable.hpp"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model
{
    class CPPDIGITALTWINLIB_EXPORT Function : public IDigitalTwinElement
    {
    public:
        Function() = delete;

        explicit Function(const std::string& name);

        ~Function() override;

        std::vector<Variable<std::any>*> getParameters() const;
        Variable<std::any>* getReturnVariable() const;

    private:
        std::vector<Variable<std::any>*> Parameters;
        Variable<std::any>* ReturnValue;
    };
} // DigitalTwin::Model

