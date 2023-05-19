//
// Created by moritzh on 12.05.23.
//

#ifndef DIGITWESTER_IDATABASE_H
#define DIGITWESTER_IDATABASE_H

#include <string>

namespace REALTWESTER {
    namespace DATA {
        class IDataBase {
        public:
            IDataBase()=default;
            virtual ~IDataBase()=default;

            virtual std::string toJsonString() = 0;

            /**
             * Gets an Json String and returns a pointer to an object of type IDataBase
             * @param jsonString The json string that is to be parsed
             * @return Pointer to an Object to IDataBase
             */
			static void* fromJsonString(std::string jsonString) {
                return nullptr;
			};

		protected:
			virtual void* importDataFromJsonString(std::string jsonString) = 0;
        };
    }
}

#endif //DIGITWESTER_IDATABASE_H
