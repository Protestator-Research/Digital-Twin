//
// Created by herzog on 01.09.26.
//

#pragma once

#include "Variable.hpp"

namespace DigitalTwin::Model
{
    class StringVariable : public IVariable
    {
    public:
	    StringVariable() = delete;

        explicit StringVariable(const std::string& name);
        explicit StringVariable(const std::string& name, const std::string& value);

        virtual ~StringVariable() = default;

	    IVariable* copy() override;
	    
        std::string getValue() const;
        void setValue(const std::string& value);
    	
    	std::string getType() override;

    protected:
	    void updateLinkedVariables() override;

        void setValueWithoutPropagation(std::string value);
    private:
        std::string Value;
    };
}

