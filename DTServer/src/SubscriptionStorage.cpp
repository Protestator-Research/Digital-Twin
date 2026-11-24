#include "SubscriptionStorage.h"
#include <algorithm>

#include "Session.h"

namespace DIGITAL_TWIN_SERVER
{
	SubscriptionResult SubscriptionStorage::add(std::shared_ptr<Session> const& session, std::string filter, bool no_local, std::size_t maxSubscriptions)
	{
		std::lock_guard lg(Mutex);
		Session* raw = session.get();
		auto& entries = Subscriptions[filter];
		for (auto& existing : entries)
		{
			if (existing.RawSession == raw)
			{
				existing.NoLocal = no_local;
				return SubscriptionResult::Updated;
			}
		}

		auto& owned = FiltersBySession[raw];
		if (owned.size() >= maxSubscriptions)
		{
			if (owned.empty())
				FiltersBySession.erase(raw);
			if (entries.empty())
				Subscriptions.erase(filter);
			return SubscriptionResult::QuotaExceeded;
		}

		entries.push_back(SubscriptionEntry{ session, raw, no_local });
		owned.insert(std::move(filter));
		return SubscriptionResult::Added;
	}

	void SubscriptionStorage::remove(Session* session, std::string_view filter)
	{
		std::lock_guard lg(Mutex);
		auto it = Subscriptions.find(std::string(filter));
		if (it == Subscriptions.end())
			return;
		auto& entries = it->second;
		entries.erase(std::remove_if(entries.begin(), entries.end(), [&](SubscriptionEntry const& elem)
		{
			return elem.RawSession == session;
		}), entries.end());
		if (entries.empty())
			Subscriptions.erase(it);
		eraseFromReverseIndex(session, std::string(filter));
	}

	void SubscriptionStorage::removeAll(Session* session)
	{
		std::lock_guard lg(Mutex);
		auto rev = FiltersBySession.find(session);
		if (rev == FiltersBySession.end())
			return;
		for (auto const& filter : rev->second)
		{
			auto it = Subscriptions.find(filter);
			if (it == Subscriptions.end())
				continue;
			auto& entries = it->second;
			entries.erase(std::remove_if(entries.begin(), entries.end(), [&](SubscriptionEntry const& elem)
			{
				return elem.RawSession == session;
			}), entries.end());
			if (entries.empty())
				Subscriptions.erase(it);
		}
		FiltersBySession.erase(rev);
	}

	void SubscriptionStorage::eraseFromReverseIndex(Session* session, std::string const& filter)
	{
		auto rev = FiltersBySession.find(session);
		if (rev == FiltersBySession.end())
			return;
		rev->second.erase(filter);
		if (rev->second.empty())
			FiltersBySession.erase(rev);
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

	void SubscriptionStorage::broadcast(std::string topic, std::string payload, Session const* publisher) {
		std::vector<std::shared_ptr<Session>> targets;
		{
			std::lock_guard lg(Mutex);
			std::unordered_set<Session const*> seen;
			for (auto it = Subscriptions.begin(); it != Subscriptions.end();) {
				auto& entries = it->second;
				// Prune expired sessions.
				entries.erase(std::remove_if(entries.begin(), entries.end(), [&](SubscriptionEntry const& elem)
				{
					if (!elem._Session.expired())
						return false;
					eraseFromReverseIndex(elem.RawSession, it->first);
					return true;
				}), entries.end());

				if (entries.empty()) {
					it = Subscriptions.erase(it);
					continue;
				}

				if (matchFilter(it->first, topic)) {
					for (auto const& entry : entries) {
						if (entry.NoLocal && entry.RawSession == publisher)
							continue;
						if (seen.count(entry.RawSession))
							continue;
						if (auto locked = entry._Session.lock()) {
							seen.insert(entry.RawSession);
							targets.push_back(std::move(locked));
						}
					}
				}
				++it;
			}
		}
		for (auto const& session : targets) {
			session->send_qos0_publish(topic, payload);
		}
	}
}
