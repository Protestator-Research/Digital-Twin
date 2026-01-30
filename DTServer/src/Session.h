#pragma once

#include <async_mqtt/all.hpp>
#include "BrokerState.h"
#include <memory>

namespace DIGITAL_TWIN_SERVER
{
	class Session : std::enable_shared_from_this<Session>
	{
	public:
		explicit Session(boost::asio::any_io_executor executor, ) = default;
		~Session() = default;



	private:
		std::shared_ptr<async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>> ServerEndpoint;
		boost::asio::strand<boost::asio::any_io_executor> Strand;

		BrokerState& _BrokerState;
		bool Authenticated;
		std::string ClientId;
	};
}
