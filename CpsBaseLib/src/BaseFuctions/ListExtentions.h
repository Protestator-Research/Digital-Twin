//
// Created by Moritz Herzog on 20.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_LISTEXTENTIONS_H
#define DIGITALTWIN_LISTEXTENTIONS_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <list>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

namespace CPSBASELIB::STD_EXTENTION::LIST {
    template <class T>
    bool areListsEqual(std::list<T> lhs, std::list<T> rhs){

        if(lhs.size()!=rhs.size())
            return false;

        return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
    }
}

#endif //DIGITALTWIN_LISTEXTENTIONS_H