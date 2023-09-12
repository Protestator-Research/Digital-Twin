#include "DriverSessionManager.h"
#include <MeasurePoint.h>
#include <chrono>
#include <limits>

REALTWESTER::RASPI::DriverSessionManager::DriverSessionManager() :
    serialPort(new mn::CppLinuxSerial::SerialPort("/dev/ttyACM1", mn::CppLinuxSerial::BaudRate::B_9600, mn::CppLinuxSerial::NumDataBits::EIGHT, mn::CppLinuxSerial::Parity::NONE, mn::CppLinuxSerial::NumStopBits::ONE))
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
    isNotStopped = true;

    bufferThread = new std::thread([this]{
        while (isNotStopped)
            readFromSerialPort();
    });

    sendingThread = new std::thread([this]{
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);

        while (isNotStopped)
            writeToMQTTServer();
    });


    bufferThread->join();
    sendingThread->join();
}

void REALTWESTER::RASPI::DriverSessionManager::stopGatheringData() {
    isNotStopped = false;

    delete bufferThread;
    delete sendingThread;

    bufferThread = nullptr;
    sendingThread = nullptr;
}

void REALTWESTER::RASPI::DriverSessionManager::readFromSerialPort() {
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
    mtx.lock();
    bufferString += buffer;
    mtx.unlock();
}

void REALTWESTER::RASPI::DriverSessionManager::writeToMQTTServer() {
    if(bufferString!=""){
        // std::cout << bufferString << std::endl;
        size_t first_occuence = bufferString.find("BOF{");
        mtx.lock();
        // std::cout << "First occurence: " << first_occuence <<std::endl;
        bufferString.erase(0, first_occuence);
        size_t first_eof_occuence = bufferString.find("}EOF");
        if(first_eof_occuence!=std::numeric_limits<size_t>::max()){
            std::string subString = bufferString.substr(0, first_eof_occuence + 4);
            std::cout<<"First EOF Occuence: "<<bufferString.find("}EOF")<<std::endl;
            bufferString.erase(0,first_eof_occuence);
            mtx.unlock();
            
            subString.erase(0,4);
            subString.erase(first_eof_occuence-4,first_eof_occuence+4);
            std::cout<<"Data Essential: "<<subString<<std::endl;
            
            if((subString.find('O')!=std::string::npos) || (subString.find('F')!=std::string::npos) || (subString.find('E')!=std::string::npos) || (subString.find('B')!=std::string::npos) || (subString.find('{')!=std::string::npos)|| (subString.find('}')!=std::string::npos))
                return;
            
            std::istringstream f(subString);
            int index = 0;
            std::string curentValueAsString;
            while (std::getline(f, curentValueAsString, ','))
            {
                std::cout<<topics[index]<<": "<<curentValueAsString<<std::endl;
                dataObserver->setNewMeasurePoint(std::make_shared<ENERGY_PROBE_DRIVER::MeasurePoint>(std::stof(curentValueAsString), topics[index] ));
                index++;

            }
            
            // dataObserver->setNewMeasurePoint(std::make_shared<ENERGY_PROBE_DRIVER::MeasurePoint>())
        }
        mtx.unlock();
        
    }
}