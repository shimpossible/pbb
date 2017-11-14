#ifndef __PBB_CRC_H__
#define __PBB_CRC_H__
#include "pbb/pbb.h"

void PBB_API crc_precompute(uint32_t* table, uint32_t poly);
void PBB_API crc_precompute_by4(uint32_t table[4][256], uint32_t poly);
void PBB_API crc_precompute_by8(uint32_t table[8][256], uint32_t poly);

/**
  Do CRC as bitwise math.  smallest code size
 */
uint32_t PBB_API crc_bitwise(const void* data, size_t length, uint32_t previous, uint32_t poly = 0xEDB88320);

/**
  Do CRC using a lookup table.  Much faster but uses more memory
 */
uint32_t PBB_API crc_table(const void* data, size_t length, uint32_t previous, const uint32_t* table);



uint32_t PBB_API crc_by4(const void* data, size_t length, uint32_t previous, const uint32_t table[4][256]);

/**
  CRC using Slicing-by-8
*/
uint32_t PBB_API crc_by8(const void* data, size_t length, uint32_t previous, const uint32_t** table);

#endif /* __PBB_CRC_H__ */