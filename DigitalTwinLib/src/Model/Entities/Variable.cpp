// //
// // Created by Moritz Herzog on 11.09.24.
// //
//
// #include "Variable.h"
//
// #include <exception>
// #include <iostream>
//
// namespace DigitalTwin::Model {
//     template <typename T>
//     Variable<T>::Variable(std::string name) : IDigitalTwinElement(name)
//     {    }
//
//     template <typename T>
//     Variable<T>::Variable(std::string name, T value) : IDigitalTwinElement(name)
//     {
//         Value = value;
//     }
//
//     template <typename T>
//     void Variable<T>::updateLinkedVariables() {
//         for(auto variable : LinkedVariables)
//             variable->setVariableValueWithoutPropagation(Value);
//     }
//
//     template <typename T>
//     void Variable<T>::setVariableValueWithoutPropagation(T value) {
//         Value = value;
//     }
//
//     template <typename T>
//     void Variable<T>::addLinkToVariable(Variable<T> *variable) {
//         LinkedVariables.push_back(variable);
//     }
//
//     template <typename T>
//     int Variable<T>::getValueAsInt() {
//         if(std::same_as<T,int>) {
//             return Value;
//         }
//         else
//             throw std::exception();
//     }
//
//     template<typename T>
//     char Variable<T>::getValueAsChar() {
//         if(std::same_as<T,char>) {
//             return Value;
//         }
//         else
//             throw std::exception();
//     }
//
//     template<typename T>
//     double Variable<T>::getValueAsDouble() {
//         if(std::same_as<T,double>) {
//             return Value;
//         }
//         else
//             throw std::exception();
//     }
//
//     template<typename T>
//     void Variable<T>::setNewValue(T value) {
//         Value=value;
//         updateLinkedVariables();
//     }
//
//     template <typename T>
//     T Variable<T>::getValue()
//     {
//         return Value;
//     }
//
//     template<typename T>
//     bool Variable<T>::getValueAsBoolean() {
//         if(std::same_as<T,bool>) {
//             return Value;
//         }
//         else
//             throw std::exception();
//     }
//
//     template<typename T>
//     Variable<T>* Variable<T>::copy()
//     {
//         return new Variable<T>(Name);
//
//     }
// }
