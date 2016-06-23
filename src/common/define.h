
#ifndef EQP_DEFINE_H
#define EQP_DEFINE_H

#ifdef _WIN32
# define EQP_WINDOWS
#else
# define EQP_LINUX
#endif

#include "netcode.h"

#ifdef EQP_WINDOWS
# include <windows.h>
#else
# include <errno.h>
# include <inttypes.h>
# include <unistd.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#ifdef __cplusplus
# ifdef EQP_WINDOWS
#  define EQP_API extern "C" __declspec(dllexport)
# else
#  define EQP_API extern "C"
# endif
#else
# ifdef EQP_WINDOWS
#  define EQP_API __declspec(dllexport)
# else
#  define EQP_API extern
# endif
#endif

typedef uint8_t byte;
typedef _Bool bool;

#define true 1
#define false 0

#define KILOBYTES(n) (1024 * (n))
#define MEGABYTES(n) (1024 * KILOBYTES(n))

#define TIMER_SECONDS(n)    ((n) * 1000)

#define STRUCT_DECLARE(name) typedef struct name name
#define STRUCT_DEFINE(name)     \
    typedef struct name name;   \
    struct name
    
#define ENUM_DEFINE(name)       \
    typedef enum name name;     \
    enum name
    
#define R(ptrType) ptrType restrict

#define sizeof_field(type, name) sizeof(((type*)0)->name)

#endif//EQP_DEFINE_H