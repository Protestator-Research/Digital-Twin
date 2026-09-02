//
// Created by herzog on 01.09.26.
//

#pragma once

#include "Variable.hpp"

namespace DigitalTwin::Model
{
    class BooleanVariable : public IVariable
    {
    public:
	    BooleanVariable() = delete;
        explicit BooleanVariable(const std::string& name);
        explicit BooleanVariable(const std::string& name, bool value);
        ~BooleanVariable() noexcept override = default;

        bool getValue() const;
        void setValue(const bool &value);

	    
	    IVariable* copy() override;
	    std::string getType() override;

    protected:
	    void updateLinkedVariables() override;

        void setValueWithoutPropagation(bool value);

    private:
        bool Value;
    };
} // DigitalTwin::Model
