//
// Created by Moritz Herzog on 23.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_COMMIT_H
#define DIGITALTWIN_COMMIT_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <list>

//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------
#include "Record.h"

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------
namespace SysMLv2::Entities{
    class Project;
    class DataVersion;
}


namespace SysMLv2::Entities {

    /**
     * Represents an abstract baseclass that is used for the SysMLv2 API
     * @class Commit
     * @author Moritz Herzog <herzogm@rptu.de>
     * @version 1.0
     */
    class Commit : public Record {
    public:
        /**
         * Generalized Constructor is deleted, according to the SysMLv2 standard.
         */
        Commit() = delete;

        /**
         *
         * @param id
         * @param name
         * @param description
         * @param owningProject
         * @param previusCommits
         */
        Commit(boost::uuids::uuid id, std::string name, std::string description, Project* owningProject, Commit** previusCommits = nullptr);

        /**
         *
         * @param name
         * @param description
         * @param owningProject
         * @param previusCommits
         */
        Commit(std::string name, std::string description, Project* owningProject, Commit** previusCommits = nullptr);

        /**
         * Constructor for Parsing an Commit from the REST API, according to the SysMLv2 Standard.
         * @param jsonString The JSON String that is received by the REST Client
         */
        explicit Commit(std::string jsonString);

        /**
         * Destructor
         */
        virtual ~Commit();

        void setChange(DataVersion* dataVersion);
        DataVersion* getDataVersion();

        [[nodiscard]] Commit** getPreviusCommits() const;

        [[nodiscard]] Project* getOwningProject() const;

        std::string serializeToJSON();

    private:
        Commit** PreviusCommits;
        Project* OwningProject;
        DataVersion* Change;
    };
}

#endif //DIGITALTWIN_COMMIT_H
