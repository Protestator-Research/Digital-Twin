#include "SubscriptionStorage.h"


namespace DIGITAL_TWIN_SERVER
{
	void SubscriptionStorage::add(std::shared_ptr<Session> const& session, std::string filter, bool no_local)
	{
		std::lock_guard lg(Mutex);
		Subscriptions.push_back(SubscriptionEntry{ session,std::move(filter), no_local });
	}

	void SubscriptionStorage::removeAll(std::shared_ptr<Session> const& session)
	{
		std::lock_guard lg(Mutex);
		Subscriptions.erase(std::remove_if(Subscriptions.begin(), Subscriptions.end(), [&](const& auto elem)
		{
			auto session_lock = elem.Session.lock();
			return !session_lock || session_lock.get() == session.get();
		}), Subscriptions.end());
	}

	bool SubscriptionStorage::matchFilter(std::string_view filter, std::string_view topic)
	{
		auto f = split(filter);
		auto t = split(topic);

		for (size_t i = 0; i < f.size(); ++i) {
			if (f[i] == "#") return (i + 1 == f.size()); // '#' nur am Ende
			if (i >= t.size()) return false;
			if (f[i] == "+") continue;
			if (f[i] != t[i]) return false;
		}
		return f.size() == t.size();
	}

	std::vector<std::string_view> SubscriptionStorage::split(std::string_view s)
	{
		std::vector<std::string_view> out;
		size_t i = 0;
		for (;;) {
			auto j = s.find('/', i);
			if (j == std::string_view::npos) { out.push_back(s.substr(i)); break; }
			out.push_back(s.substr(i, j - i));
			i = j + 1;
		}
		return out;
	}

	template <class F>
	void SubscriptionStorage::forEachMatch(std::string_view topic, Session const* publisher, F&& function)
	{
		std::lock_guard lg(Mutex);
		for (auto it = Subscriptions.begin(); it != Subscriptions.end();)
		{
			auto session_lock = it->Session.lock();
			if (!session_lock)
			{
				it = Subscriptions.erase(it);
				continue;
			}
			if (it->NoLocal && session_lock.get() == publisher)
			{
				++it;
				continue;
			}
			if (matchFilter(it->Filter, topic))
				function(session_lock);
		}
	}
}
