#pragma once

#include <string>
#include <mqtt/iclient_persistence.h>

namespace REALTWESTER::CONNECTION::MQTT
{
	class MQTTPersistence : virtual public mqtt::iclient_persistence
	{
	public:
		MQTTPersistence(const std::string& encodeKey);

		void open(const std::string& clientId, const std::string& serverURI) override;
		void close() override;
		void clear() override;
		bool contains_key(const mqtt::string& key) override;
		mqtt::string_collection keys() const override;
		void put(const mqtt::string& key, const std::vector<mqtt::string_view>& bufs) override;
		std::string get(const mqtt::string& key) const override;
		void remove(const mqtt::string& key) override;

	private:
		std::string name;
		std::string encodeKey;

		void encode(std::string& s) const;
		std::string path_name(const std::string& key) const;

		std::vector<std::string> splitString(const std::string& s, char delimiter) const;
	};
}
