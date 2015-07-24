#ifndef __PBB_NET_H__
#define __PBB_NET_H__

#include <pbb/pbb.h>

// Define library import/export macros
#ifdef PBB_NET_EXPORT
    #ifdef __GNUC__
        #define PBB_NET_API __attribute__((visibility ("default") ))
    #else
        #define PBB_NET_API __declspec(dllexport)
    #endif
#else
    #ifdef __GNUC__
        #define PBB_NET_API
    #else
        #define PBB_NET_API __declspec(dllimport)
    #endif
#endif

#endif __PBB_NET_H__