#include "pbb/crc.h"

void crc_precompute_by8(uint32_t table[8][256], uint32_t poly)
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
        // only Slicing-by-8
        table[4][i] = (table[3][i] >> 8) ^ table[0][table[3][i] & 0xFF];
        table[5][i] = (table[4][i] >> 8) ^ table[0][table[4][i] & 0xFF];
        table[6][i] = (table[5][i] >> 8) ^ table[0][table[5][i] & 0xFF];
        table[7][i] = (table[6][i] >> 8) ^ table[0][table[6][i] & 0xFF];
    }
}