#ifndef _GENERIC_PROTOCOL_CONSTANTS_HPP
#define _GENERIC_PROTOCOL_CONSTANTS_HPP

#include <chrono>

using namespace std;

namespace GenericProtocolConstants {
    constexpr bool debug_information = true;

    constexpr float packet_loss_probability = 0;
    constexpr float packet_corruption_probability = 0;
    constexpr int network_latency = 1000;

    constexpr int max_attempts_to_send_message = 3;
    static constexpr auto resend_timeout = chrono::seconds(5);
}  // namespace GenericProtocolConstants

#endif  // _GENERIC_PROTOCOL_CONSTANTS_HPP
