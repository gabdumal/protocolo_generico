#ifndef _GENERIC_PROTOCOL_CONSTANTS_HPP
#define _GENERIC_PROTOCOL_CONSTANTS_HPP

#include <chrono>

using namespace std;

namespace GenericProtocolConstants {
    constexpr bool debug_information = true;

    constexpr float packet_loss_probability = 0;
    constexpr float packet_corruption_probability = 0;
    constexpr int network_latency = 1000;

    constexpr int max_attempts_to_send_message = 10;
    static constexpr auto resend_timeout = chrono::seconds(5);
    constexpr int interval_to_check_unconfirmed_messages = 100;
}  // namespace GenericProtocolConstants

#endif  // _GENERIC_PROTOCOL_CONSTANTS_HPP
