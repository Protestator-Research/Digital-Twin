//
// Created by herzog on 01.09.26.
//

#pragma once

#include "Variable.hpp"

namespace DigitalTwin::Model
{
    class PositiveVariable : public IVariable
    {
    public:
        PositiveVariable() = delete;
        PositiveVariable(std::string name);
        PositiveVariable(std::string name, unsigned int value);

        virtual ~PositiveVariable() noexcept override = default;
        
    	IVariable* copy() override;
        
        unsigned int getValue() const;
        void setValue(const unsigned int& value);
    	
    	std::string getType() override;

protected:
        void updateLinkedVariables() override;

        void setValueWithoutPropagation(const unsigned int& value);

    private:
        unsigned int Value;
    };
}

