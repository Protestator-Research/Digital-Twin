//
// Created by Moritz Herzog on 25.09.24.
//

#ifndef DIGITALTWIN_CONNECTION_H
#define DIGITALTWIN_CONNECTION_H

#include "IDigitalTwinElement.h"

namespace DigitalTwin::Model {
    class Connection : public IDigitalTwinElement{
    public:
        Connection() = delete;
        Connection(std::string name);

        ~Connection() = default;
    };
}

#endif //DIGITALTWIN_CONNECTION_H
