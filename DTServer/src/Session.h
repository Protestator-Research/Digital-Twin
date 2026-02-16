#pragma once

#include <async_mqtt/all.hpp>
#include "BrokerState.h"
#include "SubscriptionStorage.h"
#include "AuthenticationService.h"
#include <memory>

namespace DIGITAL_TWIN_SERVER
{
	class Session
	{
	public:
		Session() = delete;
		Session(boost::asio::io_context* ioc, SubscriptionStorage& subStore, AuthenticationService& authService);
		~Session() = default;

		void start();
		void stop();
		void recv_connect();
		void recv_loop();
		void send_qos0_(std::string const& topic, std::string const& payload);
		boost::asio::ip::tcp::socket::lowest_layer_type& lowest_layer();
		bool operator==(const Session &) const;

	private:
		async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>* ServerEndpoint;
		//boost::asio::strand<boost::asio::any_io_executor> Strand;

		//BrokerState& _BrokerState;
		// bool Authenticated;
		std::string ClientId;
		SubscriptionStorage& _subscriptionStorage;
	};
}
