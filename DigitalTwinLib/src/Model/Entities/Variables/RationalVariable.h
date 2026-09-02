//
// Created by herzog on 01.09.26.
//

#pragma once

#include "Variable.hpp"

#include <boost/rational.hpp>

namespace DigitalTwin::Model
{
    class RationalVariable : public IVariable
    {
    public:
        RationalVariable() = delete;
        explicit RationalVariable(std::string name);
        explicit RationalVariable(std::string name, const long& numerator, const long& denumerator);
        explicit RationalVariable(std::string name, const boost::rational<long>& value);
    	virtual ~RationalVariable() = default;
    	
    	IVariable* copy() override;
        std::string getType() override;

        void setValue(const long& numerator, const long& denumerator);
        void setValue(const boost::rational<long>& value);
        boost::rational<long> getValue();

	protected:
        void updateLinkedVariables() override;

        void setValueWithoutPropagation(const boost::rational<long>& value);
    private:
        boost::rational<long> Value;

    };
}

