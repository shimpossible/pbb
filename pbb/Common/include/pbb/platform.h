#ifndef __PBB_PLATFORM_H__
#define __PBB_PLATFORM_H__
/*
    Define platform specific settings here
 */

#define PBB_OS_WINDOWS_NT  (1)
#define PBB_OS_WINDOWS_CE  (2)
#define PBB_OS_LINUX       (3)
#define PBB_OS_MACOS       (4)

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#elif defined(_AIX) || defined(__TOS_AIX__)
#elif defined(hpux) || defined(_hpux) || defined(__hpux)
#elif defined(__digital__) || defined(__osf__)
#elif defined(__NACL__)
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__) || defined(EMSCRIPTEN)
#elif defined(__APPLE__) || define(__TOS_MACOS__)
    #define PBB_OS PBB_OS_MACOS
#elif defined(__NetBSD__)
#elif defined(__OpenBSD__)
#elif defined(sgi) || defined(__sgi)
#elif defined(sun) || defined(__sun)
#elif defined(__QNX__)
#elif defined(__CYGWIN__)
#elif defined(__VXWORKS__)
#elif defined (unix) || defined(__unix) || defined(__unix__)
#elif defined(_WIN32_WCE)
    #define PBB_OS PBB_OS_WINDOWS_CE
#elif defined(_WIN32) || defined(_WIN64)
    #define PBB_OS PBB_OS_WINDOWS_NT
#elif defined(__VMS)
#endif

#define PBB_ARCH_ALPHA   1
#define PBB_ARCH_IA32    2
#define PBB_ARCH_IA64    3
#define PBB_ARCH_MIPS    4
#define PBB_ARCH_HPPA    5
#define PBB_ARCH_PPC     6
#define PBB_ARCH_POWER   7
#define PBB_ARCH_SPARC   8
#define PBB_ARCH_AMD64   9
#define PBB_ARCH_ARM     10
#define PBB_ARCH_M68K    11
#define PBB_ARCH_S390    12
#define PBB_ARCH_SH      13
#define PBB_ARCH_NIOS2   14
#define PBB_ARCH_AARCH64 15

#define PBB_BIG_ENDIAN      1
#define PBB_LITTLE_ENDIAN   2

#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(EMSCRIPTEN)
    #define PBB_ARCH        PBB_ARCH_IA32
    #define PBB_ARCH_ENDIAN PBB_LITTLE_ENDIAN
#elif defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
    #define PBB_ARCH        PBB_ARCH_IA64
    #if defined(hpux) || defined(_hpux)
        #define PBB_ARCH_ENDIAN PBB_BIG_ENDIAN
    #else
        #define PBB_ARCH_ENDIAN PBB_LITTLE_ENDIAN
    #endif
#elif defined(__x86_64__) || defined(_M_X64)
    #define PBB_ARCH PBB_ARCH_AMD64
    #define PBB_ARCH_ENDIAN PBB_LITTLE_ENDIAN
#elif defined(__PPC) || defined(__POWERPC__) || defined(__powerpc) || defined(__PPC__) || \
      defined(__powerpc__) || defined(__ppc__) || defined(__ppc) || defined(_ARCH_PPC) || defined(_M_PPC)
    #define PBB_ARCH        PBB_ARCH_PPC
    #define PBB_ARCH_ENDIAN PBB_BIG_ENDIAN
#elif defined(__arm64__) || defined(__arm64) 
    #define PBB_ARCH PBB_ARCH_ARM64
    #if defined(__ARMEB__)
        #define PBB_ARCH_ENDIAN PBB_BIG_ENDIAN
    #elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define PBB_ARCH_ENDIAN PBB_BIG_ENDIAN
    #else
        #define PBB_ARCH_ENDIAN PBB_LITTLE_ENDIAN
    #endif
#endif

#if !define(PBB_ARCH)
    #error "Unsupported/Unknown Architecture"
#endif
#endif /* __PBB_PLATFORM_H__ */