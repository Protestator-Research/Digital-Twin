//
// Created by Moritz Herzog on 14.01.25.
//

#ifndef DIGITALTWIN_VALUEENTITY_H
#define DIGITALTWIN_VALUEENTITY_H

namespace PHYSICAL_TWIN_COMMUNICATION {
    class ValueEntity {
    private:
        ValueEntity() = default;
    public:
        ValueEntity(int value);
        ValueEntity(char element);


    };
}

#endif //DIGITALTWIN_VALUEENTITY_H
