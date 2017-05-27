#include "pbb/crc.h"

static inline uint32_t swap(uint32_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#else
    return (x >> 24) |
        ((x >> 8) & 0x0000FF00) |
        ((x << 8) & 0x00FF0000) |
        (x << 24);
#endif
}

/**
CRC using Slicing-by-4
*/
uint32_t crc_by4(const void* data, size_t length, uint32_t previous, const uint32_t table[4][256])
{
    uint32_t* current = (uint32_t*)data;
    uint32_t crc = ~previous;
    // process four bytes at once
    while (length >= 4)
    {
#if PBB_ARCH_ENDIAN == PBB_LITTLE_ENDIAN
        crc ^= *current++;
        crc = table[0][(crc >> 24)]
            ^ table[1][(crc >> 16) & 0xFF]
            ^ table[2][(crc >> 8) & 0xFF]
            ^ table[3][(crc >> 0) & 0xFF]
            ;
#else
        crc = *current++ ^ swap(crc);
        crc = table[0][crc & 0xFF] ^
            table[1][(crc >> 8) & 0xFF] ^
            table[2][(crc >> 16) & 0xFF] ^
            table[3][crc >> 24];
#endif
        length -= 4;
    }

    const unsigned char* currentChar = (unsigned char*)current;
    // remaining 1 to 3 bytes
    while (length--)
        crc = (crc >> 8) ^ table[0][(crc & 0xFF) ^ *currentChar++];
    return ~crc;
}