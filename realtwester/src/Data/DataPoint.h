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

//            void * fromJsonString(std::string jsonString);

            std::string toJsonString() override;

        protected:
            void * importDataFromJsonString(std::string jsonString) override;
        };

    } // REALTWESTER
} // DATA

#endif //DIGITWESTER_DATAPOINT_H
