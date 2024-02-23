//
// Created by Moritz Herzog on 19.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_RECORD_H
#define DIGITALTWIN_RECORD_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <boost/uuid/uuid.hpp>
#include <string>
#include <list>

//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include "IEntity.h"

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    /**
     * Represents an abstract baseclass that is used for the SysMLv2 API
     * @class Record
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class Record : public IEntity {
    public:
        /**
         * Generalized Constructor
         */
        Record();

        /**
         * Init the Record Object with all the nesecary objects.
         * @param id Id of the Object
         * @param alias Alias of the Object
         * @param name Name of the Object
         * @param description Description of the Object
         */
        Record(boost::uuids::uuid id, std::list<std::string> alias, std::string name, std::string description);

        /**
         * Converts a JSON String into a valid Record
         * @param jsonString The JSON String that is converted to a Record.
         */
        explicit Record(std::string jsonString);

        /**
         * Destructor
         */
        virtual ~Record() = default;

        /**
         * Checks the equality of the individual Records.
         * @param other The other Record object to check
         * @return True if the objects are identical.
         */
        bool operator==(Record const &other);

        /**
         *
         * @return
         */
        [[nodiscard]] std::string getName() const;

        /**
         *
         * @param name
         */
        void setName(std::string& name);

        /**
         *
         * @return
         */
        [[nodiscard]] std::list<std::string> getAlias() const;

        /**
         *
         * @param alias
         */
        void appendAlias(std::string& alias);

        /**
         *
         * @return
         */
        [[nodiscard]] std::string getDescription() const;

        /**
         *
         * @param description
         */
        void setDescription(std::string& description);

        /**
         *
         * @return
         */
        [[nodiscard]] boost::uuids::uuid getId() const;

        /**
         *
         * @return
         */
        [[nodiscard]] std::string getType() const;

        std::string serializeToJson() override;
    protected:
        std::string Type;
        std::string Name;
        std::list<std::string> Alias;
        std::string Description;
        boost::uuids::uuid Id;
    };
}

#endif //DIGITALTWIN_RECORD_H
