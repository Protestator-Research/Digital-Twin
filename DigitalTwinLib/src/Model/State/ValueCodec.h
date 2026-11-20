// Conversion between the values of the model and the column encoding of the history (see StateTypes.h).
#pragma once

#include <cstdint>
#include <string>

#include "StateTypes.h"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model::ValueCodec {
	/** Largest magnitude below which every integer is exact as a double: 2^53. */
	constexpr int64_t MAX_EXACT_INTEGER = int64_t{1} << 53;

	/** A non-finite value cannot be stored (JSON and the double column have no NaN / inf here): it becomes 0. */
	CPPDIGITALTWINLIB_EXPORT double finiteOrZero(double value);

	CPPDIGITALTWINLIB_EXPORT VariableValue encodeReal(double value);
	/** Integers beyond +-2^53 are written as exact decimal digits in `text`. */
	CPPDIGITALTWINLIB_EXPORT VariableValue encodeInteger(int64_t value);
	CPPDIGITALTWINLIB_EXPORT VariableValue encodeBoolean(bool value);
	CPPDIGITALTWINLIB_EXPORT VariableValue encodeRational(int64_t numerator, int64_t denominator);
	CPPDIGITALTWINLIB_EXPORT VariableValue encodeString(const std::string& value);
	CPPDIGITALTWINLIB_EXPORT VariableValue encodeComplex(double real, double imaginary);

	// The decode functions return false if the value has another type or is malformed; `out` is then unchanged.
	CPPDIGITALTWINLIB_EXPORT bool decodeReal(const VariableValue& value, double& out);
	CPPDIGITALTWINLIB_EXPORT bool decodeInteger(const VariableValue& value, int64_t& out);
	CPPDIGITALTWINLIB_EXPORT bool decodeBoolean(const VariableValue& value, bool& out);
	CPPDIGITALTWINLIB_EXPORT bool decodeRational(const VariableValue& value, int64_t& numerator, int64_t& denominator);
	CPPDIGITALTWINLIB_EXPORT bool decodeString(const VariableValue& value, std::string& out);
	CPPDIGITALTWINLIB_EXPORT bool decodeComplex(const VariableValue& value, double& real, double& imaginary);
}
