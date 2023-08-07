#pragma once

#include <mqtt/async_client.h>
#include <memory>
#include <string>
#include <map>
#include <time.h>

namespace mqtt{
	class topic;
}

namespace REALTWESTER::CONNECTION::MQTT
{

	class MQTTConnectionManager
	{
	public:
		MQTTConnectionManager();
		virtual ~MQTTConnectionManager() = default;

		void publishToTopic(std::string topic, int value);
		void publishToTopic(std::string topic, size_t value);
		void publishToTopic(std::string topic, std::string value);
		void publishToTopic(std::string topic, double value);
		void publishToTopic(std::string topic, float value);


	private:
		mqtt::topic* generateTopic(std::string topic);

		std::unique_ptr<mqtt::async_client> Client;
		mqtt::connect_options ConnectionOptions;

		std::map<std::string, mqtt::topic*> TopicMap;

		const std::string CLIENT_ID = "realtwester";
		const std::string SERVER = "mqtt://cpsiot2.cs.uni-kl.de:1884";
		const std::chrono::duration<long> BUFFER_PERIOD = std::chrono::seconds(5);
		const int MAX_BUFFERED_MESSAGES = 120;
		const int QOS = 1;
		
	};
}

