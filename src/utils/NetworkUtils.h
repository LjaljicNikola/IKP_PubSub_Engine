#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <cstdint>

class NetworkUtils {
public:
    static void encodeLength(uint32_t len, uint8_t* bytes);
    static uint32_t decodeLength(const uint8_t* bytes);
};

#endif
