#pragma once
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

time_t getLocalPosixTimeSafe(time_t posix_time, TimeLocationName* name);
time_t getLocalPosixTimeUnsafe(time_t posix_time, TimeLocationName* name);

#ifdef __cplusplus
}
#endif