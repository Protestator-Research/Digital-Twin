#include "ValueCodec.h"

#include <charconv>
#include <cmath>
#include <limits>
#include <nlohmann/json.hpp>

namespace DigitalTwin::Model::ValueCodec {
	namespace {
		VariableValue withNum(ValueType type, double number)
		{
			VariableValue value;
			value.type = type;
			value.num = number;
			return value;
		}

		VariableValue withText(ValueType type, std::string text)
		{
			VariableValue value;
			value.type = type;
			value.text = std::move(text);
			return value;
		}

		bool parseInt64(const std::string& text, size_t begin, size_t end, int64_t& out)
		{
			if (begin >= end)
				return false;
			const char* first = text.data() + begin;
			const char* last = text.data() + end;
			if (*first == '+')
				++first;   // from_chars does not accept a leading plus
			int64_t parsed = 0;
			const auto [ptr, ec] = std::from_chars(first, last, parsed);
			if (ec != std::errc() || ptr != last)
				return false;
			out = parsed;
			return true;
		}
	}

	double finiteOrZero(double value)
	{
		return std::isfinite(value) ? value : 0.0;
	}

	VariableValue encodeReal(double value)
	{
		return withNum(ValueType::Real, finiteOrZero(value));
	}

	VariableValue encodeInteger(int64_t value)
	{
		// -MAX_EXACT_INTEGER is exact as well, the guard on the magnitude is on the safe side for INT64_MIN.
		if (value > MAX_EXACT_INTEGER || value < -MAX_EXACT_INTEGER)
			return withText(ValueType::Integer, std::to_string(value));
		return withNum(ValueType::Integer, static_cast<double>(value));
	}

	VariableValue encodeBoolean(bool value)
	{
		return withNum(ValueType::Boolean, value ? 1.0 : 0.0);
	}

	VariableValue encodeRational(int64_t numerator, int64_t denominator)
	{
		return withText(ValueType::Rational, std::to_string(numerator) + "/" + std::to_string(denominator));
	}

	VariableValue encodeString(const std::string& value)
	{
		return withText(ValueType::String, value);
	}

	VariableValue encodeComplex(double real, double imaginary)
	{
		nlohmann::json json;
		json["imaginaryValue"] = finiteOrZero(imaginary);
		json["realValue"] = finiteOrZero(real);
		VariableValue value;
		value.type = ValueType::Complex;
		value.json = json.dump();
		return value;
	}

	bool decodeReal(const VariableValue& value, double& out)
	{
		if (value.type != ValueType::Real || !value.num.has_value() || !std::isfinite(*value.num))
			return false;
		out = *value.num;
		return true;
	}

	bool decodeInteger(const VariableValue& value, int64_t& out)
	{
		if (value.type != ValueType::Integer)
			return false;
		if (value.num.has_value())
		{
			const double number = *value.num;
			if (!std::isfinite(number) || number != std::floor(number) ||
				number > static_cast<double>(MAX_EXACT_INTEGER) || number < -static_cast<double>(MAX_EXACT_INTEGER))
				return false;
			out = static_cast<int64_t>(number);
			return true;
		}
		if (value.text.has_value())
			return parseInt64(*value.text, 0, value.text->size(), out);
		return false;
	}

	bool decodeBoolean(const VariableValue& value, bool& out)
	{
		if (value.type != ValueType::Boolean || !value.num.has_value())
			return false;
		out = *value.num != 0.0;
		return true;
	}

	bool decodeRational(const VariableValue& value, int64_t& numerator, int64_t& denominator)
	{
		if (value.type != ValueType::Rational || !value.text.has_value())
			return false;
		const auto& text = *value.text;
		const auto slash = text.find('/');
		if (slash == std::string::npos)
			return false;
		int64_t n = 0;
		int64_t d = 0;
		if (!parseInt64(text, 0, slash, n) || !parseInt64(text, slash + 1, text.size(), d) || d == 0)
			return false;
		numerator = n;
		denominator = d;
		return true;
	}

	bool decodeString(const VariableValue& value, std::string& out)
	{
		if (value.type != ValueType::String || !value.text.has_value())
			return false;
		out = *value.text;
		return true;
	}

	bool decodeComplex(const VariableValue& value, double& real, double& imaginary)
	{
		if (value.type != ValueType::Complex || !value.json.has_value())
			return false;
		const auto json = nlohmann::json::parse(*value.json, nullptr, false);
		if (!json.is_object() || !json.contains("realValue") || !json.contains("imaginaryValue") ||
			!json["realValue"].is_number() || !json["imaginaryValue"].is_number())
			return false;
		real = json["realValue"].get<double>();
		imaginary = json["imaginaryValue"].get<double>();
		return true;
	}
}
