#ifndef __PBB_ARCH_H__
#define __PBB_ARCH_H__
#include "pbb/pbb.h"

/** 
 architecture specfic function
 */
class Arch
{
public:
    /**
      Read current cycle counter.  This counts CPU cycles.
      Which means its monotonic, but may not always be the 
      same rate
     */
    static uint64_t cycle_count();
};
#endif /* __PBB_ARCH_H__ */