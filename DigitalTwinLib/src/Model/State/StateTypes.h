// Plain data types that describe the state of a digital twin model. They carry no behaviour and depend on the
// standard library only, so that the model library (producer) and the store library (consumer) share them
// without linking against each other.
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace DigitalTwin::Model {
	/**
	 * Kind of an element of the model tree. The numbers are stored in the database (column element.kind).
	 */
	enum class ElementKind : int16_t {
		Package = 1,
		Component = 2,
		Port = 3,
		Measurable = 4,
		Controllable = 5,
		Attribute = 6
	};

	/**
	 * Value type of a variable. The numbers are stored in the database (column element.value_type).
	 * Natural and Positive variables are Integer, scalar quantities are Real.
	 */
	enum class ValueType : int16_t {
		Real = 1,
		Integer = 2,
		Boolean = 3,
		Rational = 4,
		Complex = 5,
		String = 6
	};

	/**
	 * A value in the encoding of the columns of the table measurement:
	 *  - Real, Integer, Boolean: `num` (Boolean as 0 / 1). An Integer whose magnitude exceeds 2^53 is exact only as
	 *    decimal digits, so it is stored in `text` instead.
	 *  - Rational ("n/d") and String: `text`.
	 *  - Complex: `json`, {"imaginaryValue": x, "realValue": y}.
	 * Exactly one of the three members is set.
	 */
	struct VariableValue {
		ValueType type = ValueType::Real;
		std::optional<double> num;
		std::optional<std::string> text;
		std::optional<std::string> json;
	};

	/**
	 * One element of the model tree. `path` is relative to the twin (levels separated by '/'); for a variable it
	 * equals the MQTT topic path of the variable without the function suffixes. `parentPath` is empty for a root.
	 */
	struct ModelElementInfo {
		std::string path;
		std::string parentPath;
		std::string name;
		ElementKind kind = ElementKind::Package;
		std::optional<ValueType> valueType;   // set for variables only
	};

	struct VariableState {
		std::string path;
		VariableValue value;
	};

	struct VariableChange {
		std::string path;
		VariableValue value;
		bool derived = false;   // false only for the variable the client addressed
	};

	/**
	 * Outcome of one update of the model: the variable the client addressed first, then every other variable
	 * whose value was set during the call.
	 */
	struct UpdateResult {
		std::vector<VariableChange> changes;
		std::optional<std::chrono::system_clock::time_point> timePoint;   // time_point of the payload
	};
}
