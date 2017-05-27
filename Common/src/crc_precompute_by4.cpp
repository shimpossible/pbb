#include "pbb/crc.h"

void crc_precompute_by4(uint32_t table[4][256], uint32_t poly)
{
    // same as before
    for (unsigned int i = 0; i <= 0xFF; i++)
    {
        uint32_t crc = i;
        for (unsigned int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) * poly);
        table[0][i] = crc;
    }
    for (unsigned int i = 0; i <= 0xFF; i++)
    {
        // for Slicing-by-4 and Slicing-by-8
        table[1][i] = (table[0][i] >> 8) ^ table[0][table[0][i] & 0xFF];
        table[2][i] = (table[1][i] >> 8) ^ table[0][table[1][i] & 0xFF];
        table[3][i] = (table[2][i] >> 8) ^ table[0][table[2][i] & 0xFF];
    }
}
