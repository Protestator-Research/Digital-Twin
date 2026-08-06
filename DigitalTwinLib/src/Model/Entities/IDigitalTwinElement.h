//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_IDIGITALTWINELEMENT_H
#define DIGITALTWIN_IDIGITALTWINELEMENT_H

#include <string>
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
    /**
     * @class IDigitalTwinElement
     * @author Moritz Herzog
     * @version 1.0
     * This Class represents the BaseClass of the Elements of the Digital Twin.
     * If there are parts that are needed across all DT Elements, this holds it.
     */
    class CPPDIGITALTWINLIB_EXPORT IDigitalTwinElement {
    public:
        /**
         * Is deleted, because there is no need.
         */
        IDigitalTwinElement() = delete;

        /**
         * Constructor that initializes the name of the Element. The name can never be changed,
         * because it is a core component in the
         * @param name Name of a DT Element
         */
        explicit IDigitalTwinElement(std::string name){
            Name = name;
        }

        virtual ~IDigitalTwinElement() = default;

        /**
         * Gives access to the Digital Twin.
         * @return Name of the Element.
         */
        std::string getName() {
            return Name;
        }

        /**
         * Allows to reset the name of the corresponding Object.
         * @param name The name of the corresponding Object.
         */
        void setName(std::string name) {
            Name = name;
        }

    protected:
        std::string Name;
    };
}

#endif //DIGITALTWIN_IDIGITALTWINELEMENT_H
