//
// Created by Moritz Herzog on 27.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_DATA_H
#define DIGITALTWIN_DATA_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <boost/uuid/uuid.hpp>
#include <string>
//---------------------------------------------------------
// Internal Classes
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
    class Data  : public IEntity{
    public:
        Data();
        Data(boost::uuids::uuid id);
        Data(std::string jsonString);

        virtual ~Data() = default;

        boost::uuids::uuid getId();

        std::string getType();

        std::string serializeToJson() override;
    protected:
        std::string Type;
    private:
        boost::uuids::uuid Id;
    };
}

#endif //DIGITALTWIN_DATA_H
