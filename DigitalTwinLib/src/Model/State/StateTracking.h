// Index of a model tree (path <-> variable) and recorder of the variables that change while an update runs.
// Separate from DigitalTwinModel so that it can be tested on hand-built trees.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "ModelTree.h"
#include "StateTypes.h"
#include "../Entities/Variables/Variable.hpp"
#include "../../cpp_digital_twin_lib_global.h"

namespace DigitalTwin::Model {
	/**
	 * Takes the entries of walkModelTree, installs itself as the observer of every variable of them and removes
	 * itself again when it is destroyed. The variables must outlive it.
	 */
	class CPPDIGITALTWINLIB_EXPORT StateTracking : public IVariableObserver {
	public:
		explicit StateTracking(std::vector<ModelTreeEntry> entries);
		~StateTracking() override;
		StateTracking(const StateTracking&) = delete;
		StateTracking& operator=(const StateTracking&) = delete;

		const std::vector<ModelTreeEntry>& entries() const;

		/** Records the changes of variables while it lives, in the order of their first notification. */
		class CPPDIGITALTWINLIB_EXPORT Scope {
		public:
			explicit Scope(StateTracking& tracking);
			~Scope();
			Scope(const Scope&) = delete;
			Scope& operator=(const Scope&) = delete;
		private:
			StateTracking& Tracking;
		};

		/**
		 * The change set of the recorded changes: the addressed variable first (if it was changed), then the others.
		 * A variable that is reachable under several paths is listed under each of them. Of the paths of the
		 * addressed variable, `addressedPath` (or the first one if it is not among them) is not derived, the others
		 * are. Variables that are not part of the tree are left out.
		 */
		std::vector<VariableChange> collectChanges(const IVariable* addressed, const std::string& addressedPath) const;

		/** The current value of every variable of the tree. */
		std::vector<VariableState> snapshot() const;

		/** See DigitalTwinModel::restoreState. The changes are not recorded. */
		size_t restore(const std::vector<VariableState>& state);

		void variableChanged(IVariable& variable) override;

	private:
		std::vector<ModelTreeEntry> Entries;
		std::map<std::string, IVariable*> VariableOfPath;
		std::map<const IVariable*, std::vector<std::string>> PathsOfVariable;   // in tree order
		bool Recording = false;
		std::vector<const IVariable*> Changed;
	};
}
