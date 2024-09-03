#ifndef _GENERIC_PROTOCOL_CONSTANTS_HPP
#define _GENERIC_PROTOCOL_CONSTANTS_HPP

#include <chrono>

using namespace std;

namespace GenericProtocolConstants {
    constexpr bool debug_information = true;

    constexpr float packet_loss_probability = 0;
    constexpr float packet_corruption_probability = 0;
    constexpr int network_latency = 1000;

    constexpr unsigned int connection_buffer_size = 10;
    constexpr int max_attempts_to_send_package = 100;
    static constexpr auto resend_timeout = chrono::seconds(1);
    constexpr int interval_to_check_unconfirmed_packages = 100;

    constexpr auto interval_to_check_connections = chrono::seconds(1);
    constexpr unsigned int max_attempts_to_connect = 100;
}  // namespace GenericProtocolConstants

#endif  // _GENERIC_PROTOCOL_CONSTANTS_HPP
