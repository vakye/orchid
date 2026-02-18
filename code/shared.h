
#pragma once

// NOTE(vak): Variadic arguements

#include <stdarg.h>

// NOTE(vak): Architecture

#if defined(__x86_64__) || defined(_M_X64)
# define x86_64 (1)
#else
# error "Unknown architecture"
#endif

#if !defined(x86_64)
# define x86_64 (0)
#endif

// NOTE(vak): Keywords

#define local static
#define persist static

#define fallthrough

#define packed __attribute__((packed))
#define nonstring __attribute__((nonstring))

// NOTE(vak): Macros

#define CTAssert(Expression) _Static_assert(Expression, "Compile-time assertion failed")

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Minimum(A, B) ((A) < (B) ? (A) : (B))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

#define KB(Amount) ((ssize)(Amount) << 10)
#define MB(Amount) ((ssize)(Amount) << 20)
#define GB(Amount) ((ssize)(Amount) << 30)
#define TB(Amount) ((ssize)(Amount) << 40)

#define Align(Value, PowerOf2) (((Value) + (PowerOf2) - 1) & ~((PowerOf2) - 1))

// NOTE(vak): Types

typedef signed   char      s8;
typedef signed   short     s16;
typedef signed   int       s32;
typedef signed   long long s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef s64 ssize;
typedef u64 usize;

typedef float  f32;
typedef double f64;

typedef u8  b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

#define true  (1)
#define false (0)

#define USizeMax ((usize)(1) << (sizeof(usize)*8 - 1))

CTAssert(sizeof(s8 ) == 1);
CTAssert(sizeof(s16) == 2);
CTAssert(sizeof(s32) == 4);
CTAssert(sizeof(s64) == 8);

CTAssert(sizeof(u8 ) == 1);
CTAssert(sizeof(u16) == 2);
CTAssert(sizeof(u32) == 4);
CTAssert(sizeof(u64) == 8);

CTAssert(sizeof(ssize) == sizeof(void*));
CTAssert(sizeof(usize) == sizeof(void*));

CTAssert(sizeof(f32) == 4);
CTAssert(sizeof(f64) == 8);

// NOTE(vak): Memory

local void ZeroMemory(void* DestInit, usize Size);
local void FillMemory(void* DestInit, u8 Byte, usize Size);
local void CopyMemory(void* DestInit, void* SourceInit, usize Size);

#define ZeroType(Pointer)         ZeroMemory(Pointer, sizeof(*(Pointer)))
#define ZeroArray(Pointer, Count) ZeroMemory(Pointer, sizeof(*(Pointer)) * (Count))

// NOTE(vak): String

typedef struct
{
    char* Data;
    usize Size;
} string;

#define StrData(Data, Size)    (string){Data, Size}
#define Str(Literal)           (string){Literal, sizeof(Literal) - 1}

#define ImmStrData(Data, Size) {Data, Size}
#define ImmStr(Literal)        {Literal, sizeof(Literal) - 1}
