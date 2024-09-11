//
// Created by Moritz Herzog on 06.08.24.
//

#include "MqttBrokerService.h"

#include <boost/asio.hpp>
#include <boost/optional.hpp>

typedef boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::io_context::executor_type> socket_type;

namespace PHYSICAL_TWIN_COMMUNICATION {

    MQTTBrokerService::MQTTBrokerService(as::io_context &ioc_accept, std::function<as::io_context &()> ioc_con_getter,
                                         mqtt::broker::broker_t &broker, uint16_t port)
                                         : Server(as::ip::tcp::endpoint(as::ip::tcp::v4(), port),
                                                  ioc_accept,
                                                  MQTT_NS::force_move(ioc_con_getter),
                                                  [](auto& acceptor){
                                             acceptor.set_option(as::ip::tcp::acceptor::reuse_address(true));
                                         }), Broker(broker){
        Server.set_error_handler([](MQTT_NS::error_code){});
        Server.set_accept_handler([&](con_sp_t spep){Broker.handle_accept(MQTT_NS::force_move(spep));});
    }

    void MQTTBrokerService::listen() {
        Server.listen();
    }

    MQTT_NS::broker::broker_t &MQTTBrokerService::getBroker() const {
        return Broker;
    }

    void MQTTBrokerService::close() {
        Server.close();
    }

    std::thread* MQTTBrokerService::runBroker(uint16_t port, bool&) {
        try {
            as::io_context timer_ioc;
            MQTT_NS::broker::broker_t b(timer_ioc);

            auto num_of_iocs =
                    [&] () -> std::size_t {
                        return 1;
                    } ();
            if (num_of_iocs == 0) {
                num_of_iocs = std::thread::hardware_concurrency();
                std::cout << "[mqtt_broker]" << "iocs set to auto decide (0). Automatically set to " << num_of_iocs << std::endl;
            }

            auto threads_per_ioc =
                    [&] () -> std::size_t {
                        return 1;
                    } ();
            if (threads_per_ioc == 0) {
                threads_per_ioc = std::min(std::size_t(std::thread::hardware_concurrency()), std::size_t(4));
                std::cout << "[mqtt_broker]" << "threads_per_ioc set to auto decide (0). Automatically set to "
                        << threads_per_ioc << std::endl;
            }

            std::cout << "[mqtt_broker]" << "iocs:" << num_of_iocs
                    << " threads_per_ioc:" << threads_per_ioc
                    << " total threads:" << num_of_iocs * threads_per_ioc << std::endl;


            as::io_context accept_ioc;

            std::mutex mtx_con_iocs;
            std::vector<as::io_context> con_iocs(num_of_iocs);
            BOOST_ASSERT(!con_iocs.empty());

            std::vector<
                    as::executor_work_guard<
                            as::io_context::executor_type
                    >
            > guard_con_iocs;
            guard_con_iocs.reserve(con_iocs.size());
            for (auto& con_ioc : con_iocs) {
                guard_con_iocs.emplace_back(con_ioc.get_executor());
            }

            auto con_iocs_it = con_iocs.begin();

            auto con_ioc_getter =
                    [&mtx_con_iocs, &con_iocs, &con_iocs_it]() -> as::io_context& {
                        std::lock_guard<std::mutex> g{mtx_con_iocs};
                        auto& ret = *con_iocs_it++;
                        if (con_iocs_it == con_iocs.end()) con_iocs_it = con_iocs.begin();
                        return ret;
                    };

            MQTT_NS::optional<MQTTBrokerService> s;
            s.emplace(
                accept_ioc,
                con_ioc_getter,
                b,
                port);
            s->listen();



            std::thread th_accept {
                    [&accept_ioc] {
                        accept_ioc.run();
                        std::cout << "[mqtt_broker]" << "accept_ioc.run() finished" << std::endl;
                    }
            };

            as::executor_work_guard<
                    as::io_context::executor_type
            > guard_timer_ioc(timer_ioc.get_executor());

            std::thread th_timer {
                    [&timer_ioc] {
                        timer_ioc.run();
                        std::cout << "[mqtt_broker]" << "timer_ioc.run() finished" << std::endl;
                    }
            };
            std::vector<std::thread> ts;
            ts.reserve(num_of_iocs * threads_per_ioc);
            for (auto& con_ioc : con_iocs) {
                for (std::size_t i = 0; i != threads_per_ioc; ++i) {
                    ts.emplace_back(
                            [&con_ioc] {
                                con_ioc.run();
                                std::cout << "[mqtt_broker]" << "con_ioc.run() finished" << std::endl;
                            }
                    );
                }
            }

            as::io_context ioc_signal;
            as::signal_set signals{ioc_signal, SIGINT, SIGTERM};
            signals.async_wait(
                    [] (
                            boost::system::error_code const& ec,
                            int num
                    ) {
                        if (!ec) {
                            std::cout << "[mqtt_broker]" << "Signal " << num << " received. exit program" << std::endl;
                            exit(-1);
                        }
                    }
            );
            std::thread th_signal  {
                    [&] {
                        ioc_signal.run();
                    }
            };

            th_accept.join();
            std::cout << "[mqtt_broker]" << "th_accept joined" << std::endl;

            for (auto& g : guard_con_iocs) g.reset();
            for (auto& t : ts) t.join();
            std::cout << "[mqtt_broker]" << "ts joined" << std::endl;

            guard_timer_ioc.reset();
            th_timer.join();
            std::cout << "[mqtt_broker]" << "th_timer joined" << std::endl;

            signals.cancel();
            th_signal.join();
            std::cout << "[mqtt_broker]" << "th_signal joined" << std::endl;

        } catch(std::exception &e) {
            std::cout << "[mqtt_broker]" << e.what() << std::endl;
        }

        return nullptr;
    }

}