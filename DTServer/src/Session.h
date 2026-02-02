#pragma once

#include <async_mqtt/all.hpp>
#include "BrokerState.h"
#include "SubscriptionStorage.h"
#include "AuthenticationService.h"
#include <memory>

namespace DIGITAL_TWIN_SERVER
{
	class Session : std::enable_shared_from_this<Session>
	{
	public:
		Session() = delete;
		explicit Session(boost::asio::any_io_executor executor, SubscriptionStorage& subStore, AuthenticationService& authService, Principal principal);
		~Session() = default;



	private:
		std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>> ServerEndpoint;
		boost::asio::strand<boost::asio::any_io_executor> Strand;

		BrokerState& _BrokerState;
		bool Authenticated;
		std::string ClientId;
	};
}
