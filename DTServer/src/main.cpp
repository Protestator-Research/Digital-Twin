// Created by Moritz Herzog on 13.12.23.
// Contians only the definition of the Main method for the Digital Twin Server.
//---------------------------------------------------------

#include "DigitalTwinServerInstanceManager.h"
#include <iostream>

/**
 * Main method for the digital twin server.
 * @return Success code for the running instance of the digital tiwn server.
 */
int main(int argc, char *argv[]) {

    auto instanceManager  = new DIGITAL_TWIN_SERVER::DigitalTwinServerInstanceManager(argc, argv);

    try {
        instanceManager->createInstance();
    }catch (...) {
        instanceManager->destroyOnError();
    }

    try {
        instanceManager->runInstance();
    }catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        instanceManager->destroyOnError();
    }

    const int ExitCode = instanceManager->getRunTimeCode();
    delete instanceManager;
    return ExitCode;
}