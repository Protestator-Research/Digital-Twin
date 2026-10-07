#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace DIGITAL_TWIN_SERVER
{
    /**
     * Configurable limits of the MQTT broker (see Markdownfiles/MQTT_Broker_Sicherheitskonzept.md, 4.2 and 4.3).
     * Every value has a default, so missing configuration keys are not an error.
     */
    struct BrokerLimits
    {
        std::uint32_t MaxPacketSize = 262144;
        std::size_t MaxConnections = 1000;
        std::size_t MaxConnectionsPerIp = 20;
        std::size_t MaxSubscriptionsPerSession = 100;
        std::size_t MaxPendingSendBytes = 4 * 1024 * 1024;
        std::size_t MaxTopicLength = 512;
        std::size_t MaxTopicLevels = 16;
        std::size_t MaxConsecutiveSendOverflows = 3;
        unsigned ConnectTimeoutSeconds = 10;
        std::uint16_t ServerKeepAliveSeconds = 60;
    };

    /**
     * Counts the open connections in total and per remote IP address. The key has to be normalized by the caller
     * (IPv4-mapped IPv6 addresses are converted to plain IPv4 before).
     */
    class ConnectionTracker
    {
    public:
        explicit ConnectionTracker(BrokerLimits limits) : Limits(limits) {}

        bool tryAcquire(std::string const& ip)
        {
            std::lock_guard lg(Mutex);
            if (Total >= Limits.MaxConnections)
                return false;
            auto& perIp = PerIp[ip];
            if (perIp >= Limits.MaxConnectionsPerIp) {
                if (perIp == 0)
                    PerIp.erase(ip);
                return false;
            }
            ++perIp;
            ++Total;
            return true;
        }

        void release(std::string const& ip)
        {
            std::lock_guard lg(Mutex);
            auto it = PerIp.find(ip);
            if (it == PerIp.end())
                return;
            if (--it->second == 0)
                PerIp.erase(it);
            if (Total > 0)
                --Total;
        }

        std::size_t total()
        {
            std::lock_guard lg(Mutex);
            return Total;
        }

    private:
        BrokerLimits Limits;
        std::mutex Mutex;
        std::size_t Total = 0;
        std::unordered_map<std::string, std::size_t> PerIp;
    };
}
