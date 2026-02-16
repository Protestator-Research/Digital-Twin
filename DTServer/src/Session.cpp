#include "Session.h"

#include <iostream>
#include <async_mqtt/all.hpp>

namespace DIGITAL_TWIN_SERVER
{
    Session::Session(boost::asio::io_context* ioc, SubscriptionStorage &subStore,
                     [[maybe_unused]] AuthenticationService &authService) :
        _subscriptionStorage(subStore){
        ServerEndpoint = new async_mqtt::endpoint<async_mqtt::role::server, async_mqtt::protocol::mqtt>(async_mqtt::protocol_version::v5,ioc->get_executor());
    }

    void Session::start() {
        _subscriptionStorage.add(this,"",false);
        recv_connect();
    }

    void Session::stop() {
        _subscriptionStorage.removeAll(this);
        boost::system::error_code ec;
        ServerEndpoint->lowest_layer().close(ec);
    }

    void Session::recv_connect() {
        auto self = this;
        ServerEndpoint->async_recv([self](async_mqtt::error_code const& ec, std::optional<async_mqtt::packet_variant> pv_opt) {
            if (ec || !pv_opt) return self->stop();

            pv_opt->visit(async_mqtt::overload{
                [&](async_mqtt::v5::connect_packet const& cp) {
                    std::cout << "CONNECT client_id=" << cp.client_id() << "\n";

                    async_mqtt::v5::connack_packet ack{
                        /*session_present*/false,
                        async_mqtt::connect_reason_code::success
                    };
                    self->ServerEndpoint->async_send(ack, [self](async_mqtt::error_code const& ec2) {
                        if (ec2) return self->stop();
                        self->recv_loop();
                    });
                },
                [&](auto const&) {
                    // first packet wasn't CONNECT
                    self->stop();
                }
            });
        });
    }

    void Session::recv_loop() {
        auto self = this;
        ServerEndpoint->async_recv([self](async_mqtt::error_code const& ec, std::optional<async_mqtt::packet_variant> pv_opt) {
            if (ec || !pv_opt) return self->stop();

            pv_opt->visit(async_mqtt::overload{
                [&](async_mqtt::v5::pingreq_packet const&) {
                    async_mqtt::v5::pingresp_packet resp;
                    self->ServerEndpoint->async_send(resp, [](async_mqtt::error_code const&) {});
                },
                [&](async_mqtt::v5::subscribe_packet const& sp) {
                    std::vector<async_mqtt::suback_reason_code> reasons(
                        sp.entries().size(),
                        async_mqtt::suback_reason_code::granted_qos_0
                    );
                    async_mqtt::v5::suback_packet ack{sp.packet_id(), reasons};
                    self->ServerEndpoint->async_send(ack, [](async_mqtt::error_code const&) {});
                },
                [&](async_mqtt::v5::publish_packet const& pp) {
                    // QoS0-only
                    if (pp.opts().get_qos() != async_mqtt::qos::at_most_once) {
                        async_mqtt::v5::disconnect_packet dp{
                            async_mqtt::disconnect_reason_code::protocol_error
                        };
                        self->ServerEndpoint->async_send(dp, [self](async_mqtt::error_code const&) { self->stop(); });
                        return;
                    }

                    std::string topic = std::string(pp.topic());
                    std::string payload {pp.payload().data(),pp.payload().size()};

                    std::cout << "PUBLISH topic=" << topic
                              << " payload_bytes=" << payload.size() << "\n";

                    self->_subscriptionStorage.broadcast(topic, payload);
                },
                [&](async_mqtt::v5::disconnect_packet const&) {
                    self->stop();
                },
                [&](auto const&) {
                    // ignore other packets for minimal broker
                }
            });

            self->recv_loop(); // next receive (nicht parallelisieren)
        });
    }

    void Session::send_qos0_publish(std::string const& topic, std::string const& payload) {
        async_mqtt::v5::publish_packet out{
            topic,
            payload,
            async_mqtt::qos::at_most_once
        };
        ServerEndpoint->async_send(out, [](async_mqtt::error_code const&) {});
    }

    boost::asio::ip::tcp::socket::lowest_layer_type & Session::lowest_layer() {
        return ServerEndpoint->lowest_layer();
    }

    bool Session::operator==(const Session &other) const {
        return ClientId==other.ClientId;
    }
}
