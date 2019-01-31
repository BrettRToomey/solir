#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @TODO
 *
 *   Mach-O:
 *      - Relocations
 *      - External relocations
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
	#ifndef SOL_SYSTEM_WINDOWS
	#define SOL_SYSTEM_WINDOWS 1
	#endif
#elif defined(__APPLE__) && defined(__MACH__)
    #ifndef SOL_SYSTEM_POSIX
    #define SOL_SYSTEM_POSIX 1
    #endif

    #ifndef SOL_SYSTEM_OSX
	#define SOL_SYSTEM_OSX 1
	#endif
#elif defined(__unix__)
	#ifndef SOL_SYSTEM_POSIX
    #define SOL_SYSTEM_POSIX 1
    #endif

    #ifndef SOL_SYSTEM_UNIX
	#define SOL_SYSTEM_UNIX 1
	#endif

	#if defined(__linux__)
		#ifndef SOL_SYSTEM_LINUX
		#define SOL_SYSTEM_LINUX 1
		#endif
	#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
		#ifndef SOL_SYSTEM_FREEBSD
		#define SOL_SYSTEM_FREEBSD 1
		#endif
	#else
		#error This UNIX operating system is not supported
	#endif
#else
	#error This operating system is not supported
#endif

#if SOL_SYSTEM_POSIX
    #define SOL_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#elif SYSTEM_WINDOWS
    #include <crtdbg.h>

    #define SOL_STATIC_ASSERT(cond, msg) _STATIC_ASSERT(cond)
#endif

#ifndef SOLIR_NO_TYPES
#include <stdint.h>

typedef int16_t Sol_I16;
typedef int32_t Sol_I32;
typedef int64_t Sol_I64;

typedef uint8_t  Sol_U8;
typedef uint32_t Sol_U32;
typedef uint64_t Sol_U64;

typedef Sol_I32 Sol_B32;
#endif

#define SOL_SYMBOL_FLAG_EXTERNAL (0x1)
#define SOL_SYMBOL_FLAG_DEFINED  (0x2)
#define SOL_SYMBOL_FLAG_BRANCH   (0x4)

struct SolSymbol {
    const char *name;
    int offset;
    Sol_U8 flags;
};

#define SOL_EXTERNAL(x) (x.flags & SOL_SYMBOL_FLAG_EXTERNAL)
#define SOL_DEFINED(x)  (x.flags & SOL_SYMBOL_FLAG_DEFINED)
#define SOL_BRANCH(x)   (x.flags & SOL_SYMBOL_FLAG_BRANCH)

#ifndef SOL_NO_MACH
    #include "mach.c"
#endif

#ifndef SOL_NO_ELF
    #include "elf.c"
#endif

#include "ir.c"

#ifdef __cplusplus
}
#endif
