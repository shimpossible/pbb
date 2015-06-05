#ifndef __PBB_H__
#define __PBB_H__

#include <stdint.h>
#include "pbb/config.h"     // get all defines about the platform

// To prevent GCC from complaining about static functions, prefix with PBB_UNUSED
#ifdef __GNUC__
    #define PBB_UNUSED __attribute__((unused))
#else
    #define PBB_UNUSED
#endif

#ifndef NULL
#define NULL (0)
#else
//#define NULL nullptr
#endif

// Define library import/export macros
#ifdef PBB_EXPORT
    #ifdef __GNUC__
        #define PBB_API __attribute__((visibility ("default") ))
    #else
        #define PBB_API __declspec(dllexport)
    #endif
#else
    #ifdef __GNUC__
        #define PBB_API
    #else
        #define PBB_API __declspec(dllimport)
    #endif
#endif

namespace pbb
{
    uint32_t PBB_API GetVersion();
};
#endif /* __PBB_H__ */
