//
// Created by Moritz Herzog on 17.01.24.
//

#pragma once

#include <iostream>
#include <iomanip>
#include <set>

#include <mqtt_server_cpp.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace mi = boost::multi_index;

using con_t = MQTT_NS::server<>::endpoint_t;
using con_sp_t = std::shared_ptr<con_t>;

struct sub_con {
    sub_con(MQTT_NS::buffer topic, con_sp_t con, MQTT_NS::qos qos_value)
            :topic(std::move(topic)), con(std::move(con)), qos_value(qos_value) {}
    MQTT_NS::buffer topic;
    con_sp_t con;
    MQTT_NS::qos qos_value;
};

struct tag_topic {};
struct tag_con {};
struct tag_con_topic {};

using mi_sub_con = mi::multi_index_container<
        sub_con,
        mi::indexed_by<
                mi::ordered_unique<
                        mi::tag<tag_con_topic>,
                        mi::composite_key<
                                sub_con,
                                BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con),
                                BOOST_MULTI_INDEX_MEMBER(sub_con, MQTT_NS::buffer, topic)
                        >
                >,
                mi::ordered_non_unique<
                        mi::tag<tag_topic>,
                        BOOST_MULTI_INDEX_MEMBER(sub_con, MQTT_NS::buffer, topic)
                >,
                mi::ordered_non_unique<
                        mi::tag<tag_con>,
                        BOOST_MULTI_INDEX_MEMBER(sub_con, con_sp_t, con)
                >
        >
>;

namespace PHYSICAL_TWIN_COMMUNICATION {

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

    };
}

