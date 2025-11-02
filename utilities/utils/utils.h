#pragma once

#include <pico/time.h>

#define UTILS_PERCENT_MIN 0
#define UTILS_PERCENT_MAX 100

#define UTILS_CLAMP(val, min, max) MIN(max, MAX(val, min))
#define UTILS_INTERP1D(x, x1, y1, x2, y2) ((((y1) - (y2)) / ((x1) - (x2))) * ((x) - (x1)) + (y1))

#define UTILS_ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

inline static uint32_t get_ticks(void)
{
    return to_ms_since_boot(get_absolute_time());
}
