#include "Session.h"

#include <iostream>
#include <algorithm>
#include <async_mqtt/all.hpp>

namespace DIGITAL_TWIN_SERVER
{
    Session::Session(boost::asio::io_context* ioc, std::shared_ptr<SubscriptionStorage> subStore,
                     [[maybe_unused]] AuthenticationService &authService, BrokerLimits const& limits) :
        ServerEndpoint(async_mqtt::protocol_version::v5, ioc->get_executor()),
        ConnectTimer(ioc->get_executor()),
        _subscriptionStorage(std::move(subStore)),
        Limits(limits) {
    }

    Session::~Session() {
        // Safety net: stop() normally already removed the subscriptions, this is idempotent.
        if (_subscriptionStorage)
            _subscriptionStorage->removeAll(this);
        if (Tracker)
            Tracker->release(TrackedIp);
    }

    void Session::registerConnection(std::shared_ptr<ConnectionTracker> tracker, std::string ipKey) {
        Tracker = std::move(tracker);
        TrackedIp = std::move(ipKey);
    }

    void Session::start() {
        startConnectTimer();
        recv_connect();
    }

    void Session::startConnectTimer() {
        ConnectTimer.expires_after(std::chrono::seconds(Limits.ConnectTimeoutSeconds));
        ConnectTimer.async_wait([self = shared_from_this()](boost::system::error_code const& ec) {
            if (ec) return; // cancelled
            if (!self->Connected) {
                std::cout << "CONNECT timeout, closing connection" << std::endl;
                self->stop();
            }
        });
    }

    void Session::stop() {
        if (Stopped.exchange(true))
            return;
        _subscriptionStorage->removeAll(this);
        boost::system::error_code ec;
        ConnectTimer.cancel();
        ServerEndpoint.lowest_layer().close(ec);
    }

    bool Session::topicWithinLimits(std::string_view topic, BrokerLimits const& limits) {
        if (topic.size() > limits.MaxTopicLength)
            return false;
        const auto levels = static_cast<std::size_t>(std::count(topic.begin(), topic.end(), '/')) + 1;
        return levels <= limits.MaxTopicLevels;
    }

    void Session::recv_connect() {
        ServerEndpoint.async_recv([self = shared_from_this()](async_mqtt::error_code const& ec, std::optional<async_mqtt::packet_variant> pv_opt) {
            if (self->Stopped) return;
            if (ec || !pv_opt) return self->stop();

            pv_opt->visit(async_mqtt::overload{
                [&](async_mqtt::v5::connect_packet const& cp) {
                    std::cout << "CONNECT client_id=" << cp.client_id() << "\n";
                    self->ClientId = std::string(cp.client_id());
                    self->Connected = true;
                    self->ConnectTimer.cancel();

                    std::vector<async_mqtt::property_variant> props;
                    props.emplace_back(async_mqtt::property::maximum_packet_size{self->Limits.MaxPacketSize});
                    props.emplace_back(async_mqtt::property::server_keep_alive{self->Limits.ServerKeepAliveSeconds});

                    async_mqtt::v5::connack_packet ack{
                        /*session_present*/false,
                        async_mqtt::connect_reason_code::success,
                        async_mqtt::force_move(props)
                    };
                    self->ServerEndpoint.async_send(ack, [self](async_mqtt::error_code const& ec2) {
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
        if (Stopped) return;
        ServerEndpoint.async_recv([self = shared_from_this()](async_mqtt::error_code const& ec, std::optional<async_mqtt::packet_variant> pv_opt) {
            if (self->Stopped) return;
            if (ec || !pv_opt) return self->stop();

            pv_opt->visit(async_mqtt::overload{
                [&](async_mqtt::v5::pingreq_packet const&) {
                    async_mqtt::v5::pingresp_packet resp;
                    self->ServerEndpoint.async_send(resp, [self](async_mqtt::error_code const&) {});
                },
                [&](async_mqtt::v5::subscribe_packet const& sp) {
                    std::vector<async_mqtt::suback_reason_code> reasons;
                    reasons.reserve(sp.entries().size());
                    for (auto const& entry : sp.entries()) {
                        if (!topicWithinLimits(entry.topic(), self->Limits)) {
                            reasons.push_back(async_mqtt::suback_reason_code::topic_filter_invalid);
                            continue;
                        }
                        bool no_local = entry.opts().get_nl() == async_mqtt::sub::nl::yes;
                        auto result = self->_subscriptionStorage->add(self, std::string(entry.topic()), no_local,
                                                                      self->Limits.MaxSubscriptionsPerSession);
                        if (result == SubscriptionResult::QuotaExceeded)
                            reasons.push_back(async_mqtt::suback_reason_code::quota_exceeded);
                        else
                            reasons.push_back(async_mqtt::suback_reason_code::granted_qos_0);
                    }
                    async_mqtt::v5::suback_packet ack{sp.packet_id(), reasons};
                    self->ServerEndpoint.async_send(ack, [self](async_mqtt::error_code const&) {});
                },
                [&](async_mqtt::v5::unsubscribe_packet const& up) {
                    for (auto const& entry : up.entries()) {
                        self->_subscriptionStorage->remove(self.get(), entry.topic());
                    }
                    std::vector<async_mqtt::unsuback_reason_code> reasons(
                        up.entries().size(),
                        async_mqtt::unsuback_reason_code::success
                    );
                    async_mqtt::v5::unsuback_packet ack{up.packet_id(), reasons};
                    self->ServerEndpoint.async_send(ack, [self](async_mqtt::error_code const&) {});
                },
                [&](async_mqtt::v5::publish_packet const& pp) {
                    // QoS0-only
                    if (pp.opts().get_qos() != async_mqtt::qos::at_most_once) {
                        async_mqtt::v5::disconnect_packet dp{
                            async_mqtt::disconnect_reason_code::protocol_error
                        };
                        self->ServerEndpoint.async_send(dp, [self](async_mqtt::error_code const&) { self->stop(); });
                        return;
                    }

                    if (!topicWithinLimits(pp.topic(), self->Limits)) {
                        std::cout << "PUBLISH dropped: topic exceeds limits" << std::endl;
                        return;
                    }

                    std::string topic = std::string(pp.topic());
                    std::string payload {pp.payload().data(),pp.payload().size()};

                    std::cout << "PUBLISH topic=" << topic
                              << " payload_bytes=" << payload.size() << "\n";

                    self->_subscriptionStorage->broadcast(std::move(topic), std::move(payload), self.get());
                },
                [&](async_mqtt::v5::disconnect_packet const&) {
                    self->stop();
                },
                [&](auto const&) {
                    // ignore other packets for minimal broker
                }
            });

            self->recv_loop(); // next receive (nicht parallelisieren), does nothing after stop()
        });
    }

    void Session::send_qos0_publish(std::string const& topic, std::string const& payload) {
        if (Stopped)
            return;

        const std::size_t size = topic.size() + payload.size();
        if (PendingSendBytes + size > Limits.MaxPendingSendBytes) {
            ++ConsecutiveSendOverflows;
            std::cout << "Send queue overflow for client '" << ClientId << "', message dropped ("
                      << ConsecutiveSendOverflows << ")" << std::endl;
            if (ConsecutiveSendOverflows >= Limits.MaxConsecutiveSendOverflows) {
                std::cout << "Disconnecting slow client '" << ClientId << "'" << std::endl;
                stop();
            }
            return;
        }
        ConsecutiveSendOverflows = 0;
        PendingSendBytes += size;

        async_mqtt::v5::publish_packet out{
            topic,
            payload,
            async_mqtt::qos::at_most_once
        };
        ServerEndpoint.async_send(out, [self = shared_from_this(), size](async_mqtt::error_code const& code) {
            self->PendingSendBytes -= std::min(size, self->PendingSendBytes);
            if (code)
                std::cout << "MQTT Error: " << code << std::endl;
        });
    }

    boost::asio::ip::tcp::socket::lowest_layer_type & Session::lowest_layer() {
        return ServerEndpoint.lowest_layer();
    }

    bool Session::operator==(const Session &other) const {
        return ClientId==other.ClientId;
    }
}
