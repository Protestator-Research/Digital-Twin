//
// Created by herzog on 04.08.26.
//

#pragma once

#include <vector>
#include <any>

#include "IDigitalTwinElement.h"
#include "Variables/Variable.hpp"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model
{
    class CPPDIGITALTWINLIB_EXPORT Function : public IDigitalTwinElement
    {
    public:
        Function() = delete;

        explicit Function(const std::string& name);

        ~Function() override;

        std::vector<IVariable*> getParameters() const;
        IVariable* getReturnVariable() const;

    private:
        std::vector<IVariable*> Parameters;
        IVariable* ReturnValue;
    };
} // DigitalTwin::Model

