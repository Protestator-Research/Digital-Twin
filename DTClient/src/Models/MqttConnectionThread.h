//
// Created by Moritz Herzog on 13.01.25.
//

#ifndef DIGITALTWIN_MQTTCONNECTIONTHREAD_H
#define DIGITALTWIN_MQTTCONNECTIONTHREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <QObject>
#include <Services/IMqttClientService.h>
#include <Services/MqttTlsOptions.h>

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
		                     std::string username = "", std::string password = "",
		                     DigitalTwin::Communication::MqttTlsOptions tls = DigitalTwin::Communication::MqttTlsOptions());
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

		/**
		 * True if the last connect failed because of TLS (certificate not trusted, wrong host name, no TLS at the server,
		 * unusable CA file). The client keeps trying with growing pauses, except for an unusable CA file.
		 */
		bool hasTlsProblem() const;

		void addObserverForTopic(const std::string& topic, std::function<void(std::string)> callbackFunction);
		void publish(std::string topic, std::string value) override;
		void subscribe(std::string topic, std::function<void(std::string topic, std::string payload)> callback) override;

	signals:
		void connected();
		/**
		 * The DT server refused the user name or password of the connection.
		 */
		void loginRefused();
		/**
		 * A TLS problem with the DT server (text for the user). Sent once for this connection, the details of the
		 * following attempts only go to the log.
		 */
		void tlsProblem(QString message);

	private slots:
		void onConnect();

	private:
		bool Connected = false;
		std::atomic<bool> TlsProblemReported{false};
		std::unique_ptr<DigitalTwin::Communication::MqttClientService> Service;
	};
}


#endif //DIGITALTWIN_MQTTCONNECTIONTHREAD_H
