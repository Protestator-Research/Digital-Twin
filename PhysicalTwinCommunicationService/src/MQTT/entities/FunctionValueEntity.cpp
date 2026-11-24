#include "FunctionValueEntity.h"
#include "JsonEntities.h"
#include "BaseFuctions/StringExtention.hpp"

#include "values/BooleanValueEntity.h"
#include "values/ComplexValueEntity.h"
#include "values/IntegerValueEntity.h"
#include "values/NaturalValueEntity.h"
#include "values/PositiveValueEntity.h"
#include "values/RationalValueEntity.h"
#include "values/RealValueEntity.h"
#include "values/StringValueEntity.h"

#include <nlohmann/json.hpp>

#include <map>
#include <set>
#include <stdexcept>

namespace {
	const char* const PARAMETERS_ENTITY = "parameters";
	const char* const NAME_ENTITY = "name";

	std::shared_ptr<DigitalTwin::Communication::ValueEntity> createEntity(const std::string& type, const std::string& json)
	{
		using namespace DigitalTwin::Communication;
		if (type == "RealValueEntity")
			return std::make_shared<RealValueEntity>(json);
		if (type == "IntegerValue")
			return std::make_shared<IntegerValueEntity>(json);
		if (type == "NaturalValueEntity")
			return std::make_shared<NaturalValueEntity>(json);
		if (type == "PositiveValueEntity")
			return std::make_shared<PositiveValueEntity>(json);
		if (type == "BooleanValue")
			return std::make_shared<BooleanValueEntity>(json);
		if (type == "StringValue")
			return std::make_shared<StringValueEntity>(StringValueEntity::fromJson(json));
		if (type == "RationalValueEntity")
			return std::make_shared<RationalValueEntity>(json);
		if (type == "ComplexValueEntity")
			return std::make_shared<ComplexValueEntity>(json);
		throw std::invalid_argument("FunctionValueEntity: unknown parameter entity type '" + type + "'.");
	}
}

namespace DigitalTwin::Communication {
	FunctionValueEntity::FunctionValueEntity(std::vector<NamedEntity> parameters)
		: Parameters(std::move(parameters))
	{
		std::set<std::string> names;
		for (const auto& parameter : Parameters) {
			if (parameter.second == nullptr)
				throw std::invalid_argument("FunctionValueEntity: parameter '" + parameter.first + "' is a nullptr.");
			if (!names.insert(parameter.first).second)
				throw std::invalid_argument("FunctionValueEntity: parameter '" + parameter.first + "' appears twice.");
		}
	}

	FunctionValueEntity::FunctionValueEntity(const std::string& jsonString, const std::vector<NamedEntity>& signature)
	{
		nlohmann::json json;
		try {
			json = nlohmann::json::parse(jsonString);
		} catch (const nlohmann::json::exception& e) {
			throw std::invalid_argument(std::string("FunctionValueEntity: invalid JSON: ") + e.what());
		}
		if (!json.is_object())
			throw std::invalid_argument("FunctionValueEntity: payload is not a JSON object.");
		if (!json.contains(JsonEntities::TIME_POINT_ENTITY) || !json[JsonEntities::TIME_POINT_ENTITY].is_string())
			throw std::invalid_argument("FunctionValueEntity: 'time_point' is missing or not a string.");
		if (!json.contains(PARAMETERS_ENTITY) || !json[PARAMETERS_ENTITY].is_array())
			throw std::invalid_argument("FunctionValueEntity: 'parameters' is missing or not an array.");

		const std::string timePointString = json[JsonEntities::TIME_POINT_ENTITY].get<std::string>();
		try {
			Timepoint = CPSBASELIB::STD_EXTENTION::StringExtention::timepointFromString(timePointString);
		} catch (const std::exception& e) {
			throw std::invalid_argument(std::string("FunctionValueEntity: invalid 'time_point': ") + e.what());
		}

		std::map<std::string, nlohmann::json> received;
		for (const auto& element : json[PARAMETERS_ENTITY]) {
			if (!element.is_object())
				throw std::invalid_argument("FunctionValueEntity: a parameter is not a JSON object.");
			if (!element.contains(NAME_ENTITY) || !element[NAME_ENTITY].is_string())
				throw std::invalid_argument("FunctionValueEntity: a parameter has no string 'name'.");
			const std::string name = element[NAME_ENTITY].get<std::string>();
			if (!received.emplace(name, element).second)
				throw std::invalid_argument("FunctionValueEntity: parameter '" + name + "' appears twice.");
		}

		std::set<std::string> expected;
		for (const auto& signatureEntry : signature) {
			if (signatureEntry.second == nullptr)
				throw std::invalid_argument("FunctionValueEntity: signature entry '" + signatureEntry.first + "' is a nullptr.");
			if (!expected.insert(signatureEntry.first).second)
				throw std::invalid_argument("FunctionValueEntity: signature parameter '" + signatureEntry.first + "' appears twice.");
		}
		for (const auto& entry : received) {
			if (expected.find(entry.first) == expected.end())
				throw std::invalid_argument("FunctionValueEntity: parameter '" + entry.first + "' is not part of the function signature.");
		}

		for (const auto& signatureEntry : signature) {
			auto found = received.find(signatureEntry.first);
			if (found == received.end())
				throw std::invalid_argument("FunctionValueEntity: parameter '" + signatureEntry.first + "' is missing in the payload.");
			nlohmann::json child = found->second;
			child.erase(NAME_ENTITY);
			child[JsonEntities::TIME_POINT_ENTITY] = timePointString;
			std::shared_ptr<ValueEntity> entity;
			try {
				entity = createEntity(signatureEntry.second->getType(), child.dump());
			} catch (const std::exception& e) {
				// The child entities throw nlohmann, boost and std exceptions on bad values; report them uniformly.
				throw std::invalid_argument("FunctionValueEntity: parameter '" + signatureEntry.first + "': " + e.what());
			}
			Parameters.emplace_back(signatureEntry.first, entity);
		}
	}

	const std::vector<FunctionValueEntity::NamedEntity>& FunctionValueEntity::getParameters() const
	{
		return Parameters;
	}

	std::shared_ptr<ValueEntity> FunctionValueEntity::getParameter(const std::string& name) const
	{
		for (const auto& parameter : Parameters) {
			if (parameter.first == name)
				return parameter.second;
		}
		return nullptr;
	}

	std::string FunctionValueEntity::getJson() const
	{
		nlohmann::json json;
		json[JsonEntities::TIME_POINT_ENTITY] = CPSBASELIB::STD_EXTENTION::StringExtention::timepointToString(Timepoint);
		json[PARAMETERS_ENTITY] = nlohmann::json::array();
		for (const auto& parameter : Parameters) {
			nlohmann::json child = nlohmann::json::parse(parameter.second->getJson());
			child.erase(JsonEntities::TIME_POINT_ENTITY);
			child[NAME_ENTITY] = parameter.first;
			json[PARAMETERS_ENTITY].push_back(child);
		}
		// Like StringValueEntity: invalid UTF-8 in a name must not make serialization throw.
		return json.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
	}

	std::string FunctionValueEntity::getType() const
	{
		return "FunctionValueEntity";
	}
}
