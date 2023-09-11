#include "DriverSessionManager.h"
#include <thread>
//("/dev/ttyUSB0", BaudRate::B_57600, NumDataBits::EIGHT, Parity::NONE, NumStopBits::ONE);

REALTWESTER::RASPI::DriverSessionManager::DriverSessionManager() :
    serialPort(new mn::CppLinuxSerial::SerialPort("/dev/ttyACM0", mn::CppLinuxSerial::BaudRate::B_9600, mn::CppLinuxSerial::NumDataBits::EIGHT, mn::CppLinuxSerial::Parity::NONE, mn::CppLinuxSerial::NumStopBits::ONE))
{
    serialPort->SetTimeout(15);
    serialPort->Open();
}

REALTWESTER::RASPI::DriverSessionManager::~DriverSessionManager() {
    serialPort->Close();
}

void REALTWESTER::RASPI::DriverSessionManager::registerObserver(std::shared_ptr<ENERGY_PROBE_DRIVER::DataPointObserver> observer) {
    dataObserver=observer;
}

void REALTWESTER::RASPI::DriverSessionManager::startGatheringData() {
    std::thread thread([this]{
        while (true)
        {   
            std::string buffer = "";
            for (int i = 0; i<2; i++){
                std::string readData = "";
                while (readData.find("\nBOF{") == std::string::npos)
                {
                    serialPort->Read(readData);
                }
                while (readData.find("}EOF\r\n") == std::string::npos)
                {
                    std::string data;
                    serialPort->Read(data);

                    if(data != "")
                        readData += data;
                }
                buffer += readData;
            }

            std::cout << buffer;
        }
    });
    thread.join();
    
    // dataObserver->setNewMeasurePoint()
}