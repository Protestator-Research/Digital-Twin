#pragma once

#include <async_mqtt/all.hpp>
#include <boost/asio/steady_timer.hpp>
#include "BrokerState.h"
#include "BrokerLimits.h"
#include "SubscriptionStorage.h"
#include "AuthenticationService.h"
#include <atomic>
#include <memory>
#include <string>

namespace DIGITAL_TWIN_SERVER
{
	class Session : public std::enable_shared_from_this<Session>
	{
	public:
		Session() = delete;
		Session(boost::asio::io_context* ioc, std::shared_ptr<SubscriptionStorage> subStore, AuthenticationService& authService,
			BrokerLimits const& limits);
		~Session();

		Session(Session const&) = delete;
		Session& operator=(Session const&) = delete;

		/**
		 * Registers the connection in the connection tracker. The slot is released when the session is destroyed.
		 */
		void registerConnection(std::shared_ptr<ConnectionTracker> tracker, std::string ipKey);

		void start();
		/** Idempotent. */
		void stop();
		void recv_connect();
		void recv_loop();
		void send_qos0_publish(std::string const& topic, std::string const& payload);
		boost::asio::ip::tcp::socket::lowest_layer_type& lowest_layer();
		bool operator==(const Session &) const;

		/**
		 * Topic name / filter limits (length in bytes and number of levels).
		 */
		static bool topicWithinLimits(std::string_view topic, BrokerLimits const& limits);

	private:
		using Endpoint = async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>;

		void startConnectTimer();

		Endpoint ServerEndpoint;
		boost::asio::steady_timer ConnectTimer;

		std::atomic<bool> Stopped{false};
		bool Connected = false;

		std::size_t PendingSendBytes = 0;
		std::size_t ConsecutiveSendOverflows = 0;

		std::shared_ptr<ConnectionTracker> Tracker;
		std::string TrackedIp;

		std::string ClientId;
		std::shared_ptr<SubscriptionStorage> _subscriptionStorage;
		BrokerLimits Limits;
	};
}
