//
// Created by Moritz Herzog on 17.01.24.
//

#include "PTCommunicationService.h"


namespace PHYSICAL_TWIN_COMMUNICATION {

    using mi_subscription_connnection = boost::multi_index::multi_index_container<
            subscription_connection,
            boost::multi_index::indexed_by<
                    boost::multi_index::ordered_unique<
                            boost::multi_index::tag<tag_connection_topic>,
                            boost::multi_index::composite_key<
                                    subscription_connection,
                                    BOOST_MULTI_INDEX_MEMBER(subscription_connection, std::shared_ptr<MQTT_NS::server<>::endpoint_t>, con),
                                    BOOST_MULTI_INDEX_MEMBER(subscription_connection, MQTT_NS::buffer, topic)
                                    >
                            >,
                    boost::multi_index::ordered_non_unique<
                            boost::multi_index::tag<tag_topic>,
                            BOOST_MULTI_INDEX_MEMBER(subscription_connection, MQTT_NS::buffer, topic)
                            >,
                    boost::multi_index::ordered_non_unique<
                            boost::multi_index::tag<tag_connection>,
                            BOOST_MULTI_INDEX_MEMBER(subscription_connection, std::shared_ptr<MQTT_NS::server<>::endpoint_t>, con)
                            >
                    >
            >;

    CommunicationService::CommunicationService(std::string mqttPort) {
        std::cout<<"Initialize Communication Service"<<std::endl;

        MQTT_NS::setup_log();

        std::uint16_t port = std::stoi(mqttPort);

        boost::asio::io_context iocs;
        auto s = MQTT_NS::server<>(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port),iocs);
        std::set<std::shared_ptr<MQTT_NS::server<>::endpoint_t>> connections;

        mi_subscription_connnection subs;
        std::thread th(
                [&] {
                    server_process(s, connections, subs);
                    iocs.run();
                }
        );

        th.join();
    }

    CommunicationService::CommunicationService(std::string, std::string) {

    }

    void CommunicationService::close_process(std::set<std::shared_ptr<mqtt::server<>::endpoint_t>> &cons,
                                             mi_subscription_connnection &subs,
                                             const std::shared_ptr<mqtt::server<>::endpoint_t> &con) {
        cons.erase(con);

        auto& idx = subs.get<tag_connection>();
        auto r = idx.equal_range(con);
        idx.erase(r.first, r.second);

    }

    template<typename Server>
    void
    CommunicationService::server_process(Server &s, std::set<std::shared_ptr<mqtt::server<>::endpoint_t>> &connections,
                                         mi_subscription_connnection &subs) {
        s.set_error_handler(
                [](MQTT_NS::error_code ec) {
                    std::cout << "[server] error: " << ec.message() << std::endl;
                }
        );
        s.set_accept_handler(
                [&s, &connections, &subs, this](std::shared_ptr<MQTT_NS::server<>::endpoint_t> spep) {
                    auto& ep = *spep;
                    std::weak_ptr<MQTT_NS::server<>::endpoint_t> wp(spep);

                    using packet_id_t = typename std::remove_reference_t<decltype(ep)>::packet_id_t;
                    std::cout << "[server] accept" << std::endl;
                    // For server close if ep is closed.
                    auto g = MQTT_NS::shared_scope_guard(
                            [&s] {
                                std::cout << "[server] session end" << std::endl;
                                s.close();
                            }
                    );

                    ep.start_session(std::make_tuple(std::move(spep), std::move(g)));

                    ep.set_close_handler(
                             [&connections, &subs, wp, this]
                                    (){
                                std::cout << "[server] closed." << std::endl;
                                auto sp = wp.lock();
                                BOOST_ASSERT(sp);
                                close_process(connections, subs, sp);
                            });
                    ep.set_error_handler(
                            [&connections, &subs, wp, this]
                                    (MQTT_NS::error_code ec){
                                std::cout << "[server] error: " << ec.message() << std::endl;
                                auto sp = wp.lock();
                                BOOST_ASSERT(sp);
                                close_process(connections, subs, sp);
                            });

                    // set MQTT level handlers
                    ep.set_connect_handler(
                            [&connections, wp]
                                    (MQTT_NS::buffer client_id,
                                     MQTT_NS::optional<MQTT_NS::buffer> username,
                                     MQTT_NS::optional<MQTT_NS::buffer> password,
                                     MQTT_NS::optional<MQTT_NS::will>,
                                     bool clean_session,
                                     std::uint16_t keep_alive) {
                                using namespace MQTT_NS::literals;
                                std::cout << "[server] client_id    : " << client_id << std::endl;
                                std::cout << "[server] username     : " << (username ? username.value() : "none"_mb) << std::endl;
                                std::cout << "[server] password     : " << (password ? password.value() : "none"_mb) << std::endl;
                                std::cout << "[server] clean_session: " << std::boolalpha << clean_session << std::endl;
                                std::cout << "[server] keep_alive   : " << keep_alive << std::endl;
                                auto sp = wp.lock();
                                BOOST_ASSERT(sp);
                                connections.insert(sp);
                                sp->connack(false, MQTT_NS::connect_return_code::accepted);
                                return true;
                            }
                    );
                    ep.set_disconnect_handler(
                            [&connections, &subs, wp, this]
                                    (){
                                std::cout << "[server] disconnect received." << std::endl;
                                auto sp = wp.lock();
                                BOOST_ASSERT(sp);
                                close_process(connections, subs, sp);
                            });
                    ep.set_puback_handler(
                            []
                                    (packet_id_t packet_id){
                                std::cout << "[server] puback received. packet_id: " << packet_id << std::endl;
                                return true;
                            });
                    ep.set_pubrec_handler(
                            []
                                    (packet_id_t packet_id){
                                std::cout << "[server] pubrec received. packet_id: " << packet_id << std::endl;
                                return true;
                            });
                    ep.set_pubrel_handler(
                            []
                                    (packet_id_t packet_id){
                                std::cout << "[server] pubrel received. packet_id: " << packet_id << std::endl;
                                return true;
                            });
                    ep.set_pubcomp_handler(
                            []
                                    (packet_id_t packet_id){
                                std::cout << "[server] pubcomp received. packet_id: " << packet_id << std::endl;
                                return true;
                            });
                    ep.set_publish_handler(
                            [&subs]
                                    (MQTT_NS::optional<packet_id_t> packet_id,
                                     MQTT_NS::publish_options pubopts,
                                     MQTT_NS::buffer topic_name,
                                     MQTT_NS::buffer contents){
                                std::cout << "[server] publish received."
                                              << " dup: "    << pubopts.get_dup()
                                              << " qos: "    << pubopts.get_qos()
                                              << " retain: " << pubopts.get_retain() << std::endl;
                                if (packet_id)
                                    std::cout << "[server] packet_id: " << *packet_id << std::endl;
                                std::cout << "[server] topic_name: " << topic_name << std::endl;
                                std::cout << "[server] contents: " << contents << std::endl;
                                auto const& idx = subs.get<tag_topic>();
                                auto r = idx.equal_range(topic_name);
                                for (; r.first != r.second; ++r.first) {
                                    r.first->con->publish(
                                            topic_name,
                                            contents,
                                            std::min(r.first->qos_value, pubopts.get_qos())
                                    );
                                }
                                return true;
                            });
                    ep.set_subscribe_handler(
                            [&subs, wp]
                                    (packet_id_t packet_id,
                                     std::vector<MQTT_NS::subscribe_entry> entries) {
                                std::cout << "[server] subscribe received. packet_id: " << packet_id << std::endl;
                                std::vector<MQTT_NS::suback_return_code> res;
                                res.reserve(entries.size());
                                auto sp = wp.lock();
                                BOOST_ASSERT(sp);
                                for (auto const& e : entries) {
                                    std::cout << "[server] topic_filter: " << e.topic_filter  << " qos: " << e.subopts.get_qos() << std::endl;
                                    res.emplace_back(MQTT_NS::qos_to_suback_return_code(e.subopts.get_qos()));
                                    subs.emplace(std::move(e.topic_filter), sp, e.subopts.get_qos());
                                }
                                sp->suback(packet_id, res);
                                return true;
                            }
                    );
                    ep.set_unsubscribe_handler(
                            [&subs, wp]
                                    (packet_id_t packet_id,
                                     std::vector<MQTT_NS::unsubscribe_entry> entries) {
                                std::cout << "[server] unsubscribe received. packet_id: " << packet_id << std::endl;
                                auto sp = wp.lock();
                                for (auto const& e : entries) {
                                    auto it = subs.find(std::make_tuple(sp, e.topic_filter));
                                    if (it != subs.end()) {
                                        subs.erase(it);
                                    }
                                }
                                BOOST_ASSERT(sp);
                                sp->unsuback(packet_id);
                                return true;
                            }
                    );
                }
        );

        s.listen();
    }
}