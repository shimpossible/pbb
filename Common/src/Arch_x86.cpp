#include "pbb/arch.h"
#include <intrin.h>.

uint64_t Arch::cycle_count()
{
    return __rdtsc();
}