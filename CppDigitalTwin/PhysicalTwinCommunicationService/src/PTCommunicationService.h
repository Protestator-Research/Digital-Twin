//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once

#include <string>
#include <iostream>
#include <mqtt_server_cpp.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace PHYSICAL_TWIN_COMMUNICATION {

    struct subscription_connection {
        subscription_connection(MQTT_NS::buffer topic, std::shared_ptr<MQTT_NS::server<>::endpoint_t> con, MQTT_NS::qos qos_value)
                :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value) {}
        MQTT_NS::buffer topic;
        std::shared_ptr<MQTT_NS::server<>::endpoint_t> con;
        MQTT_NS::qos qos_value;
    };

    struct tag_topic {};
    struct tag_connection {};
    struct tag_connection_topic {};

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


    /**
     * @author Moritz Herzog
     * Creates the MQTT Communication Service for the Server or the Client.
     */
    class CommunicationService {
    public:
        /**
         * Is Deleted
         */
        CommunicationService() = delete;
        /**
         * Generates a Server instance with the given port as the listening port.
         * @param mqttPort The listening Port of the
         */
        explicit CommunicationService(std::string mqttPort);

        /**
         * Generates a client instance of the MQTT Interface.
         * @param address Address of the server.
         * @param mqttPort Port of the Server
         */
        CommunicationService(std::string address, std::string mqttPort);


        /**
         * Destructor
         */
        virtual  ~CommunicationService() = default;

    private:
        void close_process(std::set<std::shared_ptr<MQTT_NS::server<>::endpoint_t>>& cons, mi_subscription_connnection& subs, std::shared_ptr<MQTT_NS::server<>::endpoint_t> const& con);

        template<typename Server>
        void server_process(Server& s, std::set<std::shared_ptr<MQTT_NS::server<>::endpoint_t>>& connections, mi_subscription_connnection& subs);
    };
}

