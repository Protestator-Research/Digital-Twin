#include "BrokerState.h"

namespace DIGITAL_TWIN_SERVER
{
	void BrokerState::addSubscription(std::string filter, const std::shared_ptr<Session>& sess)
	{
		Subscriptions[std::move(filter)].insert(sess);
	}

	void BrokerState::removeSuscription(const std::shared_ptr<Session>& sess)
	{
		for (auto iterator = Subscriptions.begin(); iterator != Subscriptions.end();)
		{
			iterator->second.erase(sess);

			if (iterator->second.empty())
				iterator = Subscriptions.erase(iterator);
			else
				++iterator;
		}
	}
}

