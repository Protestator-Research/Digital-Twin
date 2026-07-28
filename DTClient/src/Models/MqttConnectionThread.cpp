//
// Created by Moritz Herzog on 13.01.25.
//

#include "MqttConnectionThread.h"
#include <chrono>
#include <Services/MqttClientService.h>
#include <QDebug>

namespace DigitalTwin::Client {

    MQTTConnectionThread::MQTTConnectionThread(std::string url, std::string port, [[maybe_unused]] std::string username, [[maybe_unused]] std::string password) :
    Client(mqtt::async_client(url + ":" + port, "digital-twin-client"))
    {


        Client.set_connected_handler([this](const std::string&) {
            connected();
        });

        IoC = new boost::asio::io_context();

        connect(this, SIGNAL(connected()), this, SLOT(onConnect()));

    }


    void MQTTConnectionThread::start() {
        if (Connected)
            return;

        auto connOpts = mqtt::connect_options_builder::v5()
            .keep_alive_interval(std::chrono::seconds(30))
            .clean_session(false)
            .automatic_reconnect()
            .finalize();

        mqttClientThread = std::thread([this, connOpts]()
        {
        	Client.start_consuming();
            std::cout << "Connecting to the MQTT server..." << std::endl;
            auto tok = Client.connect(connOpts);
            auto rsp = tok->get_connect_response();

            if (!rsp.is_session_present()) {
                std::cout << "  No session present on server. Subscribing..." << std::endl;
                //Client.subscribe(TOPIC, QOS)->wait();
            }
            std::cout << "OK" << std::endl;

        });
        

        //mqttClientThread = std::thread([this]() {
            //ClientService->start();
        //});

    	//ClientService->connectToHost();
    }

    void MQTTConnectionThread::addObserverForTopic(const std::string &,
                                                   std::function<void(std::string)> ) {
        //ClientService->subscribe(topic,[callbackFunction](const std::string&, const std::string& payload)->void {callbackFunction(payload);});
    }

    void MQTTConnectionThread::publish(std::string topic, std::string value) {
        std::cout << "Sending message..." << std::endl;
        std::cout << topic << std::endl << value;
        mqtt::message_ptr pubmsg = mqtt::make_message(topic, value);
        pubmsg->set_qos(QUALITY_OF_SERVICE);
        const auto resultPublish = Client.publish(pubmsg, nullptr, SendingListener);
        std::cout << "  ...OK" << std::endl;
    }

    void MQTTConnectionThread::onConnect() {
        qDebug()<<"MQTTConnectionThread::onConnect()";
        Connected = true;
    }

    void MQTTConnectionThread::onDisconnect() {
        qDebug()<<"MQTTConnectionThread::onDisconnect()";
    }

    void MQTTConnectionThread::onErrorChanged() {
        qDebug()<<"MQTTConnectionThread::onErrorChanged()";
    }
}
