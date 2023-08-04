#pragma once

#include <mqtt/async_client.h>
#include <memory>
#include <time.h>

namespace REALTWESTER::CONNECTION::MQTT
{

	class MQTTConnectionManager
	{
	public:
		MQTTConnectionManager();
		virtual ~MQTTConnectionManager() = default;

	private:
		std::unique_ptr<mqtt::async_client> Client;

		const std::string CLIENT_ID = "realtwester";
		const std::string SERVER = "mqtt://cpsiot2.cs.uni-kl.de:1884";
		const std::chrono::duration<long> BUFFER_PERIOD = std::chrono::seconds(5);
		const int MAX_BUFFERED_MESSAGES = 120;
	};
}

