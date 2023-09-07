#include "MQTTConnectionManager.h"

#include <mqtt/topic.h>
#include <thread>

#include "MQTTPersistence.h"
#include "../../Logging/LoggingService.h"

REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::MQTTConnectionManager()
{
	LOGGING::LoggingService::log("Initializing MQTT Client");

//#ifndef WIN32
	MQTTPersistence persist("persitentMemory");
//#endif

	Client = std::make_unique<mqtt::async_client>(SERVER, CLIENT_ID);


	ConnectionOptions = mqtt::connect_options_builder()
		.keep_alive_interval(MAX_BUFFERED_MESSAGES * BUFFER_PERIOD)
		.clean_session(true)
		.automatic_reconnect(true)
		.finalize();

	LOGGING::LoggingService::log("Starting MQTT Client");

	try
	{
		Client->connect(ConnectionOptions)->wait();
	}catch (const mqtt::exception& ex)
	{
		LOGGING::LoggingService::error(ex.get_error_str());
	}

}

void REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::publishToTopic(std::string topic, int value)
{
	if(!TopicMap.contains(topic))
		TopicMap.insert_or_assign(topic, generateTopic(topic));
	
	std::string generatedPayload = "{" + std::to_string(value) + "}";
	try {
		TopicMap[topic]->publish(std::move(generatedPayload));
	} catch (const mqtt::exception& ex)
	{
		LOGGING::LoggingService::error(ex.get_error_str());
	}
}

void REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::publishToTopic(std::string topic, size_t value)
{
	if (!TopicMap.contains(topic))
		TopicMap.insert_or_assign(topic, generateTopic(topic));

	std::string generatedPayload = "{" + std::to_string(value) + "}";
	try {
		TopicMap[topic]->publish(std::move(generatedPayload));
	}
	catch (const mqtt::exception& ex)
	{
		LOGGING::LoggingService::error(ex.get_error_str());
	}
}

void REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::publishToTopic(std::string topic, std::string value)
{

    auto thread = std::thread([this, topic, value] {
        if (!TopicMap.contains(topic))
            TopicMap.insert_or_assign(topic, generateTopic(topic));

        std::string generatedPayload = "{" + value + "}";
        try {
            TopicMap[topic]->publish(std::move(generatedPayload));
        }
        catch (const mqtt::exception &ex) {
            LOGGING::LoggingService::error(ex.get_error_str());
        }
    });
}

void REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::publishToTopic(std::string topic, double value)
{
    auto thread = std::thread([this, topic, value] {
        if (!TopicMap.contains(topic))
            TopicMap.insert_or_assign(topic, generateTopic(topic));

        std::string generatedPayload = "{" + std::to_string(value) + "}";
        try {
            TopicMap[topic]->publish(std::move(generatedPayload));
        }
        catch (const mqtt::exception &ex) {
            LOGGING::LoggingService::error(ex.get_error_str());
        }
    });
}

void REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::publishToTopic(std::string topic, float value)
{
    // auto thread = std::thread([this, topic, value]{
        if (!TopicMap.contains(topic))
            TopicMap.insert_or_assign(topic, generateTopic(topic));

        std::string generatedPayload = "{" + std::to_string(value) + "}";
        try {
            TopicMap[topic]->publish(std::move(generatedPayload));
        }
        catch (const mqtt::exception& ex)
        {
            LOGGING::LoggingService::error(ex.get_error_str());
        }catch (const std::exception& ex) {
			LOGGING::LoggingService::error(ex.what());
		}
    // });
}

mqtt::topic* REALTWESTER::CONNECTION::MQTT::MQTTConnectionManager::generateTopic(std::string topic)
{
	return new mqtt::topic(*Client, topic, QOS,true);
}
