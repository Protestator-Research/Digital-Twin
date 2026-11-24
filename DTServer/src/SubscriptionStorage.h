#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace DIGITAL_TWIN_SERVER
{
	class Session;

	struct SubscriptionEntry {
		std::weak_ptr<Session> _Session;
		Session* RawSession = nullptr; // identity only, never dereferenced
		bool NoLocal = false;
	};

	enum class SubscriptionResult {
		Added,
		Updated,
		QuotaExceeded
	};

	class SubscriptionStorage
	{
	public:
		SubscriptionStorage() = default;
		~SubscriptionStorage() = default;

		/**
		 * Adds or updates a subscription. Updating an existing filter never counts against the quota.
		 * @param maxSubscriptions Maximum number of distinct filters of this session.
		 */
		SubscriptionResult add(std::shared_ptr<Session> const& session, std::string filter, bool no_local, std::size_t maxSubscriptions);
		void remove(Session* session, std::string_view filter);
		void removeAll(Session* session);
		bool matchFilter(std::string_view filter, std::string_view topic);
		static std::vector<std::string_view>  split(std::string_view s);

		/**
		 * Sends the message to every session with a matching filter. Sessions are collected under the lock,
		 * the sending happens outside of it. Expired entries are pruned.
		 */
		void broadcast(std::string topic, std::string payload, Session const* publisher = nullptr);
	private:
		void eraseFromReverseIndex(Session* session, std::string const& filter);

		std::mutex Mutex;
		std::unordered_map<std::string, std::vector<SubscriptionEntry>> Subscriptions;
		std::unordered_map<Session*, std::unordered_set<std::string>> FiltersBySession;
	};
}
