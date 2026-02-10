#include "NetworkUtils.h"

void NetworkUtils::encodeLength(uint32_t len, uint8_t* bytes) {
    bytes[0] = (uint8_t)((len >> 24) & 0xFF);
    bytes[1] = (uint8_t)((len >> 16) & 0xFF);
    bytes[2] = (uint8_t)((len >> 8) & 0xFF);
    bytes[3] = (uint8_t)(len & 0xFF);
}

uint32_t NetworkUtils::decodeLength(const uint8_t* bytes) {
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}
