//
// Created by herzog on 01.09.26.
//

#pragma once

#include "Variable.hpp"
namespace DigitalTwin::Model
{
    class IntegerVariable : public IVariable
    {
    public:
        IntegerVariable() = delete;
        IntegerVariable(std::string name);
        IntegerVariable(std::string name, const long& value);
        virtual ~IntegerVariable() noexcept override = default;
        
        void setValue(const long& value);
        long getValue() const;

        IVariable* copy() override;
        std::string getType() override;

	protected:
        void updateLinkedVariables() override;

        void setValueWithoutPropagation(long value);

    private:
        long Value;
    };
}

