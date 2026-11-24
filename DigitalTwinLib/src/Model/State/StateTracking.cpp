#include "StateTracking.h"

#include <algorithm>

namespace DigitalTwin::Model {
	StateTracking::StateTracking(std::vector<ModelTreeEntry> entries) : Entries(std::move(entries))
	{
		for (auto& entry : Entries)
		{
			if (!entry.variable)
				continue;
			VariableOfPath[entry.info.path] = entry.variable;
			PathsOfVariable[entry.variable].push_back(entry.info.path);
			entry.variable->setObserver(this);
		}
	}

	StateTracking::~StateTracking()
	{
		for (const auto& entry : Entries)
			if (entry.variable && entry.variable->getObserver() == this)
				entry.variable->setObserver(nullptr);
	}

	const std::vector<ModelTreeEntry>& StateTracking::entries() const
	{
		return Entries;
	}

	StateTracking::Scope::Scope(StateTracking& tracking) : Tracking(tracking)
	{
		Tracking.Changed.clear();
		Tracking.Recording = true;
	}

	StateTracking::Scope::~Scope()
	{
		Tracking.Recording = false;
	}

	void StateTracking::variableChanged(IVariable& variable)
	{
		if (Recording && std::find(Changed.begin(), Changed.end(), &variable) == Changed.end())
			Changed.push_back(&variable);
	}

	std::vector<VariableChange> StateTracking::collectChanges(const IVariable* addressed, const std::string& addressedPath) const
	{
		std::vector<const IVariable*> order;
		if (std::find(Changed.begin(), Changed.end(), addressed) != Changed.end())
			order.push_back(addressed);
		for (const auto variable : Changed)
			if (variable != addressed)
				order.push_back(variable);

		std::vector<VariableChange> changes;
		for (const auto variable : order)
		{
			const auto paths = PathsOfVariable.find(variable);
			if (paths == PathsOfVariable.end())
				continue;   // not part of the tree (for example a variable of a component definition)
			const auto& list = paths->second;
			const bool isAddressed = variable == addressed;
			const auto& primary = std::find(list.begin(), list.end(), addressedPath) != list.end() ? addressedPath : list.front();
			const auto value = variable->getStateValue();
			if (isAddressed)
				changes.push_back(VariableChange{primary, value, false});
			for (const auto& path : list)
				if (!(isAddressed && path == primary))
					changes.push_back(VariableChange{path, value, true});
		}
		return changes;
	}

	std::vector<VariableState> StateTracking::snapshot() const
	{
		std::vector<VariableState> state;
		for (const auto& entry : Entries)
			if (entry.variable)
				state.push_back(VariableState{entry.info.path, entry.variable->getStateValue()});
		return state;
	}

	size_t StateTracking::restore(const std::vector<VariableState>& state)
	{
		size_t applied = 0;
		for (const auto& stored : state)
		{
			const auto found = VariableOfPath.find(stored.path);
			if (found == VariableOfPath.end())
				continue;
			if (found->second->getValueType() != stored.value.type)
				continue;
			if (found->second->setStateValue(stored.value))
				++applied;
		}
		return applied;
	}
}
