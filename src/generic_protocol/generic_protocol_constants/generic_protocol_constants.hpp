#ifndef _GENERIC_PROTOCOL_CONSTANTS_HPP
#define _GENERIC_PROTOCOL_CONSTANTS_HPP

#include <chrono>

namespace GenericProtocolConstants
{
    constexpr float packetLossProbability = 0;
    constexpr float packetCorruptionProbability = 0;
    constexpr int networkLatency = 1000;
    constexpr bool debugInformation = true;
    static constexpr auto resendTimeout = chrono::seconds(5);

}

#endif // _GENERIC_PROTOCOL_CONSTANTS_HPP
