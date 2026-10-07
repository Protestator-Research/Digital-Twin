//
// Created by herzog on 01.10.26.
//

#ifndef DIGITALTWIN_SCALARQUANTITIES_H
#define DIGITALTWIN_SCALARQUANTITIES_H

#include "Variable.hpp"

namespace DigitalTwin::Model
{
    class ScalarQuantities : public IVariable
    {
    public:
        ScalarQuantities() = delete;

        explicit ScalarQuantities(std::string name);
        //explicit ScalarQuantities(const std::string & name, );

        IVariable* copy() override;
        std::string getType() override;

    protected:
        void updateLinkedVariables() override;
    };
}

#endif //DIGITALTWIN_SCALARQUANTITIES_H
