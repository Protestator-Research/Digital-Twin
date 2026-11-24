#pragma once

#include "cpp_digital_twin_store_global.h"

namespace DigitalTwin::Store {
	/**
	 * The text of sql/0001_schema.sql, embedded at build time.
	 */
	CPPDIGITALTWINSTORE_EXPORT const char* embeddedSchemaSql();
}
