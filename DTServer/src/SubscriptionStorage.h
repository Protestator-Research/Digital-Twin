#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <string>
#include "Session.h"

namespace DIGITAL_TWIN_SERVER
{
	struct SubscriptionEntry {
		std::weak_ptr<Session> Session;
		std::string Filter;
		bool NoLocal = false;
	};

	class SubscriptionStorage
	{
	public:
		SubscriptionStorage() = default;
		~SubscriptionStorage() = default;

		void add(std::shared_ptr<Session> const& session, std::string filter, bool no_local);
		void removeAll(std::shared_ptr<Session> const& session);
		bool matchFilter(std::string_view filter, std::string_view topic);
		static std::vector<std::string_view> split(std::string_view s);

		template<class F>
		void forEachMatch(std::string_view topic, Session const* publisher, F&& value);
	private:
		std::mutex Mutex;
		std::vector<SubscriptionEntry> Subscriptions;
	};
}
