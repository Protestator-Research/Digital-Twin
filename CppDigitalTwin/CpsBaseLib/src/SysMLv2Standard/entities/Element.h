//
// Created by Moritz Herzog on 27.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_ELEMENT_H
#define DIGITALTWIN_ELEMENT_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <vector>
#include <string>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Data.h"
//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    class Identification;
}

namespace SysMLv2::Entities {
    /**
     * This class is not an exact, but a compatible Representation of the SysMLv2 Element class. This implementation
     * stems from the AGILA Backend implementation of the Element class.
     * @version 1.0
     * @author Moritz Herzog <herzogm@rptu.de>
     * @class Element
     */
    class Element : public Data{
    public:
        /**
         * C-Tor
         */
        Element();

        /**
         * C-Tor for the parsing.
         * @param jsonString JSON string to
         */
        Element(std::string jsonString);

        /**
         * D-Tor
         */
        ~Element();

        std::string serializeToJson() override;
    protected:

    private:
        std::vector<std::string> AliasIDs;
        std::string Name;
        std::string ShortName;
        std::string DeclaredName;

        std::vector<Identification*> OwnedElements;
        Identification* Owner;
        Identification* OwningMembership;
        Identification* OwningNamespace;
        Identification* OwningRelationship;

        std::string Direction;
        std::string ImportedMemberName;
        std::string ImportedNamespace;

        /**
         * Value Specs only there for the Compatibility to the AGILA Backend Version 2
         */
        std::vector<std::string> ValueSpecs;
        /**
         * Unit Spec only there for the Compatibility to the AGILA Backend Version 2
         */
        std::string UnitSpec;

        std::string Language;
        std::string Body;

        bool IsImplied;
        bool IsImpliedIncluded;
        bool IsStandard;
        bool IsLibraryElement;

        Identification* Documentation;
        std::vector<Identification*> TextualRepresentation;

        std::vector<Identification*> Source;
        std::vector<Identification*> Target;
    };
} // SysMLv2

#endif //DIGITALTWIN_ELEMENT_H
