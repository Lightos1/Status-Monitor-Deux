// Minimal stub replacing libnx's <switch.h> for host-side sanitizer builds.
// smd_parser.cpp includes <switch.h> for type aliases and crc32Calculate().
// This stub satisfies both without DevkitPro.
#pragma once
#include <stdint.h>
#include <stddef.h>

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

// libnx CRC-32 (IEEE 802.3 polynomial, same as zlib crc32).
// smd_parser.cpp calls this once per LoadFromFile/LoadFromMemory to hash the
// source text; the value is exposed via GetFileHash().
#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t crc32Calculate(const void* data, size_t size) {
    const uint8_t* p = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= p[i];
        for (int j = 0; j < 8; ++j)
            crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1u));
    }
    return ~crc;
}

#ifdef __cplusplus
}
#endif
