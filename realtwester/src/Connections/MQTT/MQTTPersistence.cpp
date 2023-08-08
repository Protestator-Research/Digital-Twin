#include "MQTTPersistence.h"

#include <mqtt/exception.h>

#include <filesystem>

//#if !defined(_WIN32)
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <algorithm>
//#endif

REALTWESTER::CONNECTION::MQTT::MQTTPersistence::MQTTPersistence(const std::string& encodeKey) :
mqtt::iclient_persistence(),
encodeKey(encodeKey)
{ }

void REALTWESTER::CONNECTION::MQTT::MQTTPersistence::open(const std::string& clientId, const std::string& serverURI)
{
	if (clientId.empty() || serverURI.empty())
		throw mqtt::persistence_exception();

	name = serverURI + "-" + clientId;
	std::replace(name.begin(), name.end(), ':', '-');

	std::filesystem::create_directory(name.c_str());
}

void REALTWESTER::CONNECTION::MQTT::MQTTPersistence::close()
{
	std::filesystem::remove(name.c_str());
}

void REALTWESTER::CONNECTION::MQTT::MQTTPersistence::clear()
{
	for (const auto& entry : std::filesystem::directory_iterator(name))
		if((entry.path()!=".")&&(entry.path() != ".."))
			std::filesystem::remove(entry.path());
}

bool REALTWESTER::CONNECTION::MQTT::MQTTPersistence::contains_key(const mqtt::string& key)
{
	for (const auto& entry : std::filesystem::directory_iterator(name)) {
		if(entry.path().generic_string().find(key))
			return true;
	}

	return false;

}

mqtt::string_collection REALTWESTER::CONNECTION::MQTT::MQTTPersistence::keys() const
{
	mqtt::string_collection ks;

	for (const auto& entry : std::filesystem::directory_iterator(name)) {
		if ((entry.path() != ".") && (entry.path() != ".."))
		{
			std::string value = splitString(entry.path().generic_string(), '/').back();
			ks.push_back(value);
		}
	}

	return ks;
}

void REALTWESTER::CONNECTION::MQTT::MQTTPersistence::put(const mqtt::string& key,
	const std::vector<mqtt::string_view>& bufs)
{
	auto path = path_name(key);

	std::ofstream os(path, std::ios_base::binary);
	if (!os)
		throw mqtt::persistence_exception();

	std::string s;
	for (const auto& b : bufs)
		s.append(b.data(), b.size());

	encode(s);
	os.write(s.data(), s.size());
}

std::string REALTWESTER::CONNECTION::MQTT::MQTTPersistence::get(const mqtt::string& key) const
{
	auto path = path_name(key);

	std::ifstream is(path, std::ios_base::ate | std::ios_base::binary);
	if (!is)
		throw mqtt::persistence_exception();

	// Read the whole file into a string
	std::streamsize sz = is.tellg();
	if (sz == 0) return std::string();

	is.seekg(0);
	std::string s(sz, '\0');
	is.read(&s[0], sz);
	if (is.gcount() < sz)
		s.resize(is.gcount());

	encode(s);
	return s;
}

void REALTWESTER::CONNECTION::MQTT::MQTTPersistence::remove(const mqtt::string& key)
{
	auto path = path_name(key);
	::remove(path.c_str());
}

void REALTWESTER::CONNECTION::MQTT::MQTTPersistence::encode(std::string& s) const
{
	size_t n = encodeKey.size();
	if (n == 0 || s.empty())
		return;

	for (size_t i = 0; i < s.size(); ++i)
		s[i] ^= encodeKey[i % n];
}

std::string REALTWESTER::CONNECTION::MQTT::MQTTPersistence::path_name(const std::string& key) const
{
	return name + "/" + key;
}

std::vector<std::string> REALTWESTER::CONNECTION::MQTT::MQTTPersistence::splitString(const std::string& s,
	char delimiter) const
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

