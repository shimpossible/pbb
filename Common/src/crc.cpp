#include "pbb/crc.h"

void crc_precompute(uint32_t* table, uint32_t poly)
{
    for (unsigned int i = 0; i <= 0xFF; i++)
    {
        uint32_t crc = i;
        for (unsigned int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (-int(crc & 1) & poly);
        table[i] = crc;
    }
}