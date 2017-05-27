#include "pbb/crc.h"

/**
Do CRC using a lookup table.  Much faster but uses more memory
*/
uint32_t crc_table(const void* data, size_t length, uint32_t previous, const uint32_t* table)
{
    uint32_t crc = ~previous;
    unsigned char* current = (unsigned char*)data;
    while (length--)
        crc = (crc >> 8) ^ table[(crc & 0xFF) ^ *current++];
    return ~crc;
};
