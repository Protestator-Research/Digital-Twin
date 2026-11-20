//
// Created by Moritz Herzog on 13.01.25.
//

#ifndef DIGITALTWIN_MQTTCONNECTIONTHREAD_H
#define DIGITALTWIN_MQTTCONNECTIONTHREAD_H

#include <functional>
#include <memory>
#include <string>
#include <QObject>
#include <Services/IMqttClientService.h>

namespace DigitalTwin::Communication {
	class MqttClientService;
}

namespace DigitalTwin::Client {
	/**
	 * Qt wrapper around the MqttClientService (Paho). The connection runs on the threads of the Paho library; this
	 * class only turns the "connected" event into a Qt signal of the thread this object lives in.
	 */
	class MQTTConnectionThread : public QObject, public DigitalTwin::Communication::IMqttClientService {
		Q_OBJECT
	public:
		MQTTConnectionThread() = delete;
		MQTTConnectionThread(std::string url, std::string port,
		                     DigitalTwin::Communication::MqttProtocolVersion version = DigitalTwin::Communication::MqttProtocolVersion::V3_1_1,
		                     std::string username = "", std::string password = "");
		~MQTTConnectionThread() override;

		/**
		 * Connects asynchronously, returns immediately.
		 */
		void start();

		/**
		 * True if the DT server refused the login (wrong SysML user name or password). The client does not try
		 * again then.
		 */
		bool isLoginRefused() const;

		void addObserverForTopic(const std::string& topic, std::function<void(std::string)> callbackFunction);
		void publish(std::string topic, std::string value) override;
		void subscribe(std::string topic, std::function<void(std::string topic, std::string payload)> callback) override;

	signals:
		void connected();
		/**
		 * The DT server refused the user name or password of the connection.
		 */
		void loginRefused();

	private slots:
		void onConnect();

	private:
		bool Connected = false;
		std::unique_ptr<DigitalTwin::Communication::MqttClientService> Service;
	};
}


#endif //DIGITALTWIN_MQTTCONNECTIONTHREAD_H
