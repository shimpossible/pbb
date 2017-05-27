#include "pbb/crc.h"

/**
Do CRC as bitwise math.  smallest code size
*/
uint32_t crc_bitwise(const void* data, size_t length, uint32_t previous, uint32_t poly)
{

    uint32_t crc = ~previous; // same as previousCrc32 ^ 0xFFFFFFFF
    unsigned char* current = (unsigned char*)data;
    while (length--)
    {
        crc ^= *current++;
        for (unsigned int j = 0; j < 8; j++)
            if (crc & 1)
                crc = (crc >> 1) ^ poly;
            else
                crc = crc >> 1;
    }
    return ~crc; // same as crc ^ 0xFFFFFFFF
};