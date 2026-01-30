#include "SubscriptionStorage.h"


namespace DIGITAL_TWIN_SERVER
{
	void SubscriptionStorage::add(std::shared_ptr<Session> const& session, std::string filter,
		bool no_local)
	{
	}

	void SubscriptionStorage::removeAll(std::shared_ptr<Session> const& session)
	{
	}

	template <class F>
	void SubscriptionStorage::forEachMatch(std::string_view topic, Session const* publisher, F&& value)
	{
	}
}
