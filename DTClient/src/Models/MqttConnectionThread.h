//
// Created by Moritz Herzog on 13.01.25.
//

#ifndef DIGITALTWIN_MQTTCONNECTIONTHREAD_H
#define DIGITALTWIN_MQTTCONNECTIONTHREAD_H

#include <string>
#include <functional>
#include <mqtt/iaction_listener.h>
//#include <QMqttClient>
#include <QThread>
#include <QDebug>
#include <QObject>
#include <boost/asio/io_context.hpp>
#include <mqtt/async_client.h>
#include <Services/IMqttClientService.h>

namespace PHYSICAL_TWIN_COMMUNICATION {
	class MqttClientService;
}

namespace DigitalTwin::Client {
	class MQTTActionListener : public mqtt::iaction_listener {
	public:
		// Geerbt über iaction_listener
		void on_failure([[maybe_unused]] const mqtt::token& asyncActionToken) override {
			qDebug() << "Message not successfully delivered";
		}

		void on_success([[maybe_unused]] const mqtt::token& asyncActionToken) override {
			qDebug() << "Message successully delivered";
		}
	};

	class MQTTConnectionThread : public QObject, public DigitalTwin::Communication::IMqttClientService {
		Q_OBJECT
	public:
		MQTTConnectionThread() = delete;
		MQTTConnectionThread(std::string url, std::string port, std::string username = "", std::string password = "");
		virtual ~MQTTConnectionThread() = default;

		void start();

		void addObserverForTopic(const std::string& topic, std::function<void(std::string)> callbackFunction);
		void publish(std::string topic, std::string value) override;
		
		signals:
			void connected();
			void messageReceived(const std::string& topic, const std::string& message);

	private slots:
		void onConnect();
		void onDisconnect();
		void onErrorChanged();

	private:
		const std::chrono::seconds TIMEOUT = std::chrono::seconds(10);
		const int QUALITY_OF_SERVICE = 0;
		MQTTActionListener SendingListener;
		bool Connected = false;
		boost::asio::io_context* IoC;
		std::thread mqttClientThread;
		mqtt::async_client Client;
	};
}


#endif //DIGITALTWIN_MQTTCONNECTIONTHREAD_H
