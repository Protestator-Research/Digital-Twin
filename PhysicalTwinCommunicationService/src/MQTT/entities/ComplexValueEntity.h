//
// Created by herzog on 18.08.26.
//

#pragma once

#include "ValueEntity.h"
#include <complex>


namespace DigitalTwin::Communication
{
    class ComplexValueEntity : public ValueEntity
    {
    public:
        ComplexValueEntity() = delete;
        ComplexValueEntity(double real, double imaginary);
        ComplexValueEntity(std::complex<double> value);
        ComplexValueEntity(std::string jsonString);
        virtual ~ComplexValueEntity() = default;

        std::complex<double> getValue() const;

        std::string getJson() const override;
        std::string getType() const override;

    private:
        std::complex<double> Value;
    };
}