//
// Created by Moritz Herzog on 06.08.24.
//

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/asio/recycling_allocator.hpp>
#include <memory>
#include <async_mqtt/endpoint.hpp>

#include "../broker_includes/broker.hpp"
#include "../broker_includes/endpoint_variant.hpp"
#include "MqttBrokerService.h"
#include "../broker_includes/fixed_core_map.hpp"

namespace PHYSICAL_TWIN_COMMUNICATION {

    MQTTBrokerService::MQTTBrokerService() {
        try {
            boost::asio::io_context timer_ioc;

            using epv_type = async_mqtt::basic_endpoint_variant<
                    async_mqtt::role::server,
                    2,
                    async_mqtt::protocol::mqtt
#if defined(ASYNC_MQTT_USE_WS)
                    ,
            async_mqttprotocol::ws
#endif // defined(ASYNC_MQTT_USE_WS)
#if defined(ASYNC_MQTT_USE_TLS)
                    ,
            async_mqttprotocol::mqtts
#if defined(ASYNC_MQTT_USE_WS)
            ,
            async_mqttprotocol::wss
#endif // defined(ASYNC_MQTT_USE_WS)
#endif // defined(ASYNC_MQTT_USE_TLS)
            >;

            async_mqtt::broker<
            epv_type
            > brk{timer_ioc };

            size_t num_of_iocs = 1;

            size_t threads_per_ioc = 1;

            if (threads_per_ioc == 0) {
                threads_per_ioc = std::min(std::size_t(std::thread::hardware_concurrency()), std::size_t(4));
                ASYNC_MQTT_LOG("mqtt_broker", info)
                        << "threads_per_ioc set to auto decide (0). Automatically set to "
                        << threads_per_ioc;
            }

            ASYNC_MQTT_LOG("mqtt_broker", info)
                    << "iocs:" << num_of_iocs
                    << " threads_per_ioc:" << threads_per_ioc
                    << " total threads:" << num_of_iocs * threads_per_ioc;

//            auto set_auth =
//                    [&] {
//                        if (vm.count("auth_file")) {
//                            std::string auth_file = vm["auth_file"].as<std::string>();
//                            if (!auth_file.empty()) {
//                                ASYNC_MQTT_LOG("mqtt_broker", info)
//                                        << "auth_file:" << auth_file;
//
//                                std::ifstream input(auth_file);
//
//                                if (input) {
//                                    async_mqttsecurity security;
//                                    security.load_json(input);
//                                    brk.set_security(async_mqttforce_move(security));
//                                }
//                                else {
//                                    ASYNC_MQTT_LOG("mqtt_broker", warning)
//                                            << "Authorization file '"
//                                            << auth_file
//                                            << "' not found,  broker doesn't use authorization file.";
//                                }
//                            }
//                        }
//                    };
//            set_auth();
            boost::asio::io_context accept_ioc;

            int concurrency_hint = boost::numeric_cast<int>(threads_per_ioc);
            if (concurrency_hint == 1) {
                concurrency_hint = BOOST_ASIO_CONCURRENCY_HINT_UNSAFE_IO;
            }
            std::mutex mtx_con_iocs;
            std::vector<std::shared_ptr<boost::asio::io_context>> con_iocs;
            con_iocs.reserve(num_of_iocs);
            for (std::size_t i = 0; i != num_of_iocs; ++i) {
                con_iocs.emplace_back(std::make_shared<boost::asio::io_context>(concurrency_hint));
            }
            BOOST_ASSERT(!con_iocs.empty());

            std::vector<
                    boost::asio::executor_work_guard<
                    boost::asio::io_context::executor_type
            >
            > guard_con_iocs;
            guard_con_iocs.reserve(con_iocs.size());
            for (auto& con_ioc : con_iocs) {
                guard_con_iocs.emplace_back(con_ioc->get_executor());
            }

            auto con_iocs_it = con_iocs.begin();

            auto con_ioc_getter =
                    [&mtx_con_iocs, &con_iocs, &con_iocs_it]() -> boost::asio::io_context& {
                        std::lock_guard<std::mutex> g{mtx_con_iocs};
                        auto& ret = **con_iocs_it;
                        ++con_iocs_it;
                        if (con_iocs_it == con_iocs.end()) con_iocs_it = con_iocs.begin();
                        return ret;
                    };

            // mqtt (MQTT on TCP)
            std::optional<boost::asio::ip::tcp::endpoint> mqtt_endpoint;
            std::optional<boost::asio::ip::tcp::acceptor> mqtt_ac;
            std::function<void()> mqtt_async_accept;
            auto apply_socket_opts =
                    [&](auto& lowest_layer) {
                        lowest_layer.set_option(boost::asio::ip::tcp::no_delay(true));
                        lowest_layer.set_option(boost::asio::socket_base::receive_buffer_size(2048));
                        lowest_layer.set_option(boost::asio::socket_base::send_buffer_size(2048));
            };

            mqtt_endpoint.emplace(boost::asio::ip::tcp::v4(), 1883);
            mqtt_ac.emplace(accept_ioc, *mqtt_endpoint);
            mqtt_async_accept = [&] {
                auto epsp = async_mqtt::basic_endpoint<async_mqtt::role::server,2,async_mqtt::protocol::mqtt>::create(async_mqtt::protocol_version::undetermined,boost::asio::make_strand(con_ioc_getter().get_executor()));
                epsp->set_bulk_write(true);
                epsp->set_bulk_read_buffer_size(1024);
                auto& lowest_layer = epsp->lowest_layer();
                mqtt_ac->async_accept(lowest_layer,[&mqtt_async_accept, &apply_socket_opts, &lowest_layer, &brk, epsp](boost::system::error_code const& ec) mutable {
                    if (ec) {
                        ASYNC_MQTT_LOG("mqtt_broker", error)<< "TCP accept error:" << ec.message();
                    }
                    else {
                        apply_socket_opts(lowest_layer);
                        brk.handle_accept(epv_type{force_move(epsp)});
                    }
                    mqtt_async_accept();
                });
            };

                mqtt_async_accept();

#if defined(ASYNC_MQTT_USE_WS)
            // ws (MQTT on WebSocket)
        std::optional<boost::asio::ip::tcp::endpoint> ws_endpoint;
        std::optional<boost::asio::ip::tcp::acceptor> ws_ac;
        std::function<void()> ws_async_accept;
        if (vm.count("ws.port")) {
            ws_endpoint.emplace(boost::asio::ip::tcp::v4(), vm["ws.port"].as<std::uint16_t>());
            ws_ac.emplace(accept_ioc, *ws_endpoint);
            ws_async_accept =
                [&] {
                    auto epsp =
                        std::make_shared<
                            async_mqtt::basic_endpoint<
                                async_mqtt::role::server,
                                2,
                               async_mqtt::protocol::ws
                            >
                        >(
                            async_mqtt::protocol_version::undetermined,
                            boost::asio::make_strand(con_ioc_getter().get_executor())
                        );
                    epsp->set_bulk_write(vm["bulk_write"].as<bool>());
                    epsp->set_bulk_read_buffer_size(vm["bulk_read_buf_size"].as<std::size_t>());
                    auto& lowest_layer = epsp->lowest_layer();
                    ws_ac->async_accept(
                        lowest_layer,
                        [&ws_async_accept, &apply_socket_opts, &lowest_layer, &brk, epsp]
                        (boost::system::error_code const& ec) mutable {
                            if (ec) {
                                ASYNC_MQTT_LOG("mqtt_broker", error)
                                    << "TCP accept error:" << ec.message();
                            }
                            else {
                                apply_socket_opts(lowest_layer);
                                auto& ws_layer = epsp->next_layer();
                                ws_layer.async_accept(
                                    [&brk, epsp]
                                    (boost::system::error_code const& ec) mutable {
                                        if (ec) {
                                            ASYNC_MQTT_LOG("mqtt_broker", error)
                                                << "WS accept error:" << ec.message();
                                        }
                                        else {
                                            brk.handle_accept(epv_type{force_move(epsp)});
                                        }
                                    }
                                );
                            }
                            ws_async_accept();
                        }
                    );
                };

            ws_async_accept();
        }

#endif // defined(ASYNC_MQTT_USE_WS)

#if defined(ASYNC_MQTT_USE_TLS)
            // mqtts (MQTT on TLS TCP)
        std::optional<boost::asio::ip::tcp::endpoint> mqtts_endpoint;
        std::optional<boost::asio::ip::tcp::acceptor> mqtts_ac;
        std::function<void()> mqtts_async_accept;
        std::optional<boost::asio::steady_timer> mqtts_timer;
        mqtts_timer.emplace(accept_ioc);
        auto mqtts_verify_field_obj =
            std::unique_ptr<ASN1_OBJECT, decltype(&ASN1_OBJECT_free)>(
                OBJ_txt2obj(vm["verify_field"].as<std::string>().c_str(), 0),
                &ASN1_OBJECT_free
            );
        if (!mqtts_verify_field_obj) {
            throw std::runtime_error(
                "An invalid verify field was specified: " +
                vm["verify_field"].as<std::string>()
            );
        }
        if (vm.count("tls.port")) {
            mqtts_endpoint.emplace(boost::asio::ip::tcp::v4(), vm["tls.port"].as<std::uint16_t>());
            mqtts_ac.emplace(accept_ioc, *mqtts_endpoint);
            mqtts_async_accept =
                [&] {
                    std::optional<std::string> verify_file;
                    if (vm.count("verify_file")) {
                        verify_file = vm["verify_file"].as<std::string>();
                    }
                    auto mqtts_ctx = init_ctx(
                        vm["certificate"].as<std::string>(),
                        vm["private_key"].as<std::string>(),
                        verify_file
                    );
                    // shared_ptr for username
                    auto username = std::make_shared<std::optional<std::string>>();
                    mqtts_ctx->set_verify_mode(boost::asio::ssl::verify_peer);
                    mqtts_ctx->set_verify_callback(
                        [username, &vm] // copy capture socket shared_ptr
                        (bool preverified, boost::asio::ssl::verify_context& ctx) {
                            // user can set username in the callback
                            return
                                verify_certificate(
                                    vm["verify_field"].as<std::string>(),
                                    preverified,
                                    ctx,
                                    username
                                );
                        }
                    );
                    auto epsp =
                        std::make_shared<
                            async_mqtt::basic_endpoint<
                                async_mqtt::role::server,
                                2,
                               async_mqtt::protocol::mqtts
                            >
                        >(
                            async_mqtt::protocol_version::undetermined,
                            boost::asio::make_strand(con_ioc_getter().get_executor()),
                            *mqtts_ctx
                        );
                    epsp->set_bulk_write(vm["bulk_write"].as<bool>());
                    epsp->set_bulk_read_buffer_size(vm["bulk_read_buf_size"].as<std::size_t>());
                    auto& lowest_layer = epsp->lowest_layer();
                    mqtts_ac->async_accept(
                        lowest_layer,
                        [&mqtts_async_accept, &apply_socket_opts, &lowest_layer, &brk, epsp, username, mqtts_ctx]
                        (boost::system::error_code const& ec) mutable {
                            if (ec) {
                                ASYNC_MQTT_LOG("mqtt_broker", error)
                                    << "TCP accept error:" << ec.message();
                            }
                            else {
                                // TBD insert underlying timeout here
                                apply_socket_opts(lowest_layer);
                                epsp->next_layer().async_handshake(
                                    boost::asio::ssl::stream_base::server,
                                    [&brk, epsp, username, mqtts_ctx]
                                    (boost::system::error_code const& ec) mutable {
                                        if (ec) {
                                            ASYNC_MQTT_LOG("mqtt_broker", error)
                                                << "TLS handshake error:" << ec.message();
                                        }
                                        else {
                                            brk.handle_accept(epv_type{force_move(epsp)}, *username);
                                        }
                                    }
                                );
                            }
                            mqtts_async_accept();
                        }
                    );
                };

            mqtts_async_accept();
        }

#if defined(ASYNC_MQTT_USE_WS)
        // wss (MQTT on WebScoket TLS TCP)
        std::optional<boost::asio::ip::tcp::endpoint> wss_endpoint;
        std::optional<boost::asio::ip::tcp::acceptor> wss_ac;
        std::function<void()> wss_async_accept;
        std::optional<boost::asio::steady_timer> wss_timer;
        wss_timer.emplace(accept_ioc);
        auto wss_verify_field_obj =
            std::unique_ptr<ASN1_OBJECT, decltype(&ASN1_OBJECT_free)>(
                OBJ_txt2obj(vm["verify_field"].as<std::string>().c_str(), 0),
                &ASN1_OBJECT_free
            );
        if (!wss_verify_field_obj) {
            throw std::runtime_error(
                "An invalid verify field was specified: " +
                vm["verify_field"].as<std::string>()
            );
        }
        if (vm.count("wss.port")) {
            wss_endpoint.emplace(boost::asio::ip::tcp::v4(), vm["wss.port"].as<std::uint16_t>());
            wss_ac.emplace(accept_ioc, *wss_endpoint);
            wss_async_accept =
                [&] {
                    std::optional<std::string> verify_file;
                    if (vm.count("verify_file")) {
                        verify_file = vm["verify_file"].as<std::string>();
                    }
                    auto wss_ctx = init_ctx(
                        vm["certificate"].as<std::string>(),
                        vm["private_key"].as<std::string>(),
                        verify_file
                    );
                    // shared_ptr for username
                    auto username = std::make_shared<std::optional<std::string>>();
                    wss_ctx->set_verify_mode(boost::asio::ssl::verify_peer);
                    wss_ctx->set_verify_callback(
                        [username, &vm]
                        (bool preverified, boost::asio::ssl::verify_context& ctx) {
                            // user can set username in the callback
                            return
                                verify_certificate(
                                    vm["verify_field"].as<std::string>(),
                                    preverified,
                                    ctx,
                                    username
                                );
                        }
                    );
                    auto epsp =
                        std::make_shared<
                            async_mqtt::basic_endpoint<
                                async_mqtt::role::server,
                                2,
                               async_mqtt::protocol::wss
                            >
                        >(
                            async_mqtt::protocol_version::undetermined,
                            boost::asio::make_strand(con_ioc_getter().get_executor()),
                            *wss_ctx
                        );
                    epsp->set_bulk_write(vm["bulk_write"].as<bool>());
                    epsp->set_bulk_read_buffer_size(vm["bulk_read_buf_size"].as<std::size_t>());
                    auto& lowest_layer = epsp->lowest_layer();
                    wss_ac->async_accept(
                        lowest_layer,
                        [&wss_async_accept, &apply_socket_opts, &lowest_layer, &brk, epsp, username, wss_ctx]
                        (boost::system::error_code const& ec) mutable {
                            if (ec) {
                                ASYNC_MQTT_LOG("mqtt_broker", error)
                                    << "TCP accept error:" << ec.message();
                            }
                            else {
                                // TBD insert underlying timeout here
                                apply_socket_opts(lowest_layer);
                                epsp->next_layer().next_layer().async_handshake(
                                    boost::asio::ssl::stream_base::server,
                                    [&brk, epsp, username, wss_ctx]
                                    (boost::system::error_code const& ec) mutable {
                                        if (ec) {
                                            ASYNC_MQTT_LOG("mqtt_broker", error)
                                                << "TLS handshake error:" << ec.message();
                                        }
                                        else {
                                            auto& ws_layer = epsp->next_layer();
                                            ws_layer.binary(true);
                                            ws_layer.async_accept(
                                                [&brk, epsp, username]
                                                (boost::system::error_code const& ec) mutable {
                                                    if (ec) {
                                                        ASYNC_MQTT_LOG("mqtt_broker", error)
                                                            << "WS accept error:" << ec.message();
                                                    }
                                                    else {
                                                        brk.handle_accept(epv_type{force_move(epsp)}, *username);
                                                    }
                                                }
                                            );
                                        }
                                    }
                                );
                            }
                            wss_async_accept();
                        }
                    );
                };

            wss_async_accept();
        }

#endif // defined(ASYNC_MQTT_USE_WS)
#endif // defined(ASYNC_MQTT_USE_TLS)

            std::thread th_accept {
                    [&accept_ioc] {
                        try {
                            accept_ioc.run();
                        }
                        catch (std::exception const& e) {
                            ASYNC_MQTT_LOG("mqtt_broker", error)
                                    << "th_accept exception:" << e.what();
                        }
                        ASYNC_MQTT_LOG("mqtt_broker", trace) << "accept_ioc.run() finished";
                    }
            };

            boost::asio::executor_work_guard<
            boost::asio::io_context::executor_type
            > guard_timer_ioc(timer_ioc.get_executor());

            std::thread th_timer {
                    [&timer_ioc] {
                        try {
                            timer_ioc.run();
                        }
                        catch (std::exception const& e) {
                            ASYNC_MQTT_LOG("mqtt_broker", error)
                                    << "th_timer exception:" << e.what();
                        }
                        ASYNC_MQTT_LOG("mqtt_broker", trace) << "timer_ioc.run() finished";
                    }
            };
            std::vector<std::thread> ts;
            ts.reserve(num_of_iocs * threads_per_ioc);
            auto fixed_core_map = false;
            std::size_t ioc_index = 0;
            for (auto& con_ioc : con_iocs) {
                for (std::size_t i = 0; i != threads_per_ioc; ++i) {
                    ts.emplace_back(
                            [&con_ioc, ioc_index, fixed_core_map] {
                                try {
                                    if (fixed_core_map) {
                                        async_mqtt::map_core_to_this_thread(ioc_index);
                                    }
                                    con_ioc->run();
                                }
                                catch (std::exception const& e) {
                                    ASYNC_MQTT_LOG("mqtt_broker", error)
                                            << "th con exception:" << e.what();
                                }
                                ASYNC_MQTT_LOG("mqtt_broker", trace) << "con_ioc->run() finished";
                            }
                    );
                }
                ++ioc_index;
            }

            boost::asio::io_context ioc_signal;
            boost::asio::signal_set signals{
                    ioc_signal,
                    SIGINT,
                    SIGTERM
#if !defined(_WIN32)
                    ,
                    SIGUSR1
#endif // !defined(_WIN32)
            };
            std::function<void(boost::system::error_code const&, int num)> handle_signal
                    = [/*&set_auth,*/ &signals, &handle_signal] (
                            boost::system::error_code const& ec,
                            int num
                    ) {
                        if (!ec) {
                            if (num == SIGINT || num == SIGTERM) {
                                ASYNC_MQTT_LOG("mqtt_broker", trace)
                                        << "Signal " << num << " received. exit program";
                                exit(-1);
                            }
#if !defined(_WIN32)
                            else if (num == SIGUSR1) {
                                ASYNC_MQTT_LOG("mqtt_broker", trace)
                                        << "Signal " << num << " received. Update auth information";
                                //set_auth();
                                signals.async_wait(handle_signal);
                            }
#endif // !defined(_WIN32)
                        }
                    };
            signals.async_wait(handle_signal);
            std::thread th_signal {
                    [&] {
                        try {
                            ioc_signal.run();
                        }
                        catch (std::exception const& e) {
                            ASYNC_MQTT_LOG("mqtt_broker", error)
                                    << "th_signal exception:" << e.what();
                        }
                    }
            };

            th_accept.join();
            ASYNC_MQTT_LOG("mqtt_broker", trace) << "th_accept joined";

            for (auto& g : guard_con_iocs) g.reset();
            for (auto& t : ts) t.join();
            ASYNC_MQTT_LOG("mqtt_broker", trace) << "ts joined";

            guard_timer_ioc.reset();
            th_timer.join();
            ASYNC_MQTT_LOG("mqtt_broker", trace) << "th_timer joined";

            signals.cancel();
            th_signal.join();
            ASYNC_MQTT_LOG("mqtt_broker", trace) << "th_signal joined";
        }
        catch (std::exception const& e) {
            ASYNC_MQTT_LOG("mqtt_broker", error) << e.what();
        }
    }

//    void MQTTBrokerService::listen() {
//        //Server.listen();
//    }

//    MQTT_NS::broker::broker_t &MQTTBrokerService::getBroker() const {
//        return Broker;
//    }

//    void MQTTBrokerService::close() {
////        Server.close();
//    }

    void MQTTBrokerService::runBroker(uint16_t) {
        MQTTBrokerService();
    }

}