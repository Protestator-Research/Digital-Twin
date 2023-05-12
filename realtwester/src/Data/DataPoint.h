//
// Created by moritzh on 12.05.23.
//

#ifndef DIGITWESTER_DATAPOINT_H
#define DIGITWESTER_DATAPOINT_H

#include "IDataBase.h"

namespace REALTWESTER {
    namespace DATA {

        class DataPoint : public IDataBase {
        public:
            DataPoint()=default;
            ~DataPoint() override =default;

            void * fromJsonString(std::string jsonString) override;

            std::string toJsonString() override;
        };

    } // REALTWESTER
} // DATA

#endif //DIGITWESTER_DATAPOINT_H
