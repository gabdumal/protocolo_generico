#ifndef _GENERIC_PROTOCOL_CONSTANTS_HPP
#define _GENERIC_PROTOCOL_CONSTANTS_HPP

namespace GenericProtocolConstants
{
    constexpr float packetLossProbability = 0.25;
    constexpr float packetCorruptionProbability = 0.35;
    constexpr int networkLatency = 1000;
    constexpr bool debugInformation = true;
}

#endif // _GENERIC_PROTOCOL_CONSTANTS_HPP