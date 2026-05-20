// Minimal stub replacing libnx's <switch.h> for host-side sanitizer builds.
// smd_parser.cpp includes <switch.h> but only for its type aliases (none of
// which it actually uses — the file uses stdint types throughout).  This stub
// satisfies the include so the translation unit compiles without DevkitPro.
#pragma once
#include <stdint.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;
typedef float     f32;
typedef double    f64;

typedef u32 Result;
typedef u32 Handle;
