#include "MQTTConnectionManager.h"



REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::MQTTConnectionManager()
{
	Client = std::make_unique<mqtt::async_client>(SERVER, CLIENT_ID);

}
