#pragma once

#include "values/ValueEntity.h"
#include "../../cpp_physical_twin_communication_global.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace DigitalTwin::Communication {
	/**
	 * @class FunctionValueEntity
	 * @author Moritz Herzog
	 * @version 1.0
	 * Payload published on the topic <twin>/<package>/<component>/<variable>/<functionName>.
	 * It carries the input parameters of the function that computes the variable. The return value is not
	 * part of it, clients write the result to the variable topic itself.
	 * The payload holds one time_point and an ordered array of named parameters, each parameter carries
	 * the fields of its value entity (without the time_point).
	 */
	class CPPPHYSICALTWINCOMMUNICATION_EXPORT FunctionValueEntity : public ValueEntity
	{
	public:
		using NamedEntity = std::pair<std::string, std::shared_ptr<ValueEntity>>;

		/**
		 * Is deleted, because there is no need.
		 */
		FunctionValueEntity() = delete;

		/**
		 * Creates the entity from the named parameters. The order is preserved.
		 * @param parameters Name and value of every function parameter.
		 * The time_point of the payload is the one of this entity; the time_points of the parameters are not used.
		 * @throws std::invalid_argument if a parameter entity is a nullptr or a name appears twice.
		 */
		explicit FunctionValueEntity(std::vector<NamedEntity> parameters);

		/**
		 * Parses a payload. The JSON carries no types, so the signature defines the entity type of each parameter.
		 * @param jsonString The received payload.
		 * @param signature One entity per function parameter with the right dynamic type (built from the model).
		 * @throws std::invalid_argument if the payload does not match the signature or is malformed.
		 */
		FunctionValueEntity(const std::string& jsonString, const std::vector<NamedEntity>& signature);

		~FunctionValueEntity() override = default;

		/**
		 * @return The named parameters in their original order.
		 */
		const std::vector<NamedEntity>& getParameters() const;

		/**
		 * @param name Name of the parameter.
		 * @return The parameter or nullptr if there is no parameter with this name.
		 */
		std::shared_ptr<ValueEntity> getParameter(const std::string& name) const;

		/**
		 * @return The payload as JSON string.
		 */
		std::string getJson() const override;

		/**
		 * @return "FunctionValueEntity"
		 */
		std::string getType() const override;

	private:
		std::vector<NamedEntity> Parameters;
	};
}
