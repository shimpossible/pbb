#ifndef __PBB_H__
#define __PBB_H__

#include <stdint.h>
#include "pbb/config.h"     // get all defines about the platform
#include <pbb/platform.h>

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

#if defined(_MSC_VER)
/////////////////////////////////////////////////////////////
// MSVC specific code
//
    #pragma warning(disable:4251) // dll-interface errors
#else
    // no stdcall outside of windows
    #define __stdcall
#endif
#endif /* __PBB_H__ */
