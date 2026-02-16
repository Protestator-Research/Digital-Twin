#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>

namespace DIGITAL_TWIN_SERVER
{
	class Session;

	class BrokerState
	{
	public:
		BrokerState() = default;
		~BrokerState() = default;
		void addSubscription(std::string filter, const std::shared_ptr<Session>& sess);

		void removeSuscription(const std::shared_ptr<Session>& sess);

	private:
		std::unordered_map<std::string, std::unordered_set<std::shared_ptr<Session>>> Subscriptions;
	};
}