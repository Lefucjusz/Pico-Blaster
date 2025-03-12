#pragma once

#include <pico/time.h>

#define PERCENT_MIN 0
#define PERCENT_MAX 100

#define CLAMP(val, min, max) MIN(max, MAX(val, min))
#define INTERP1D(x, x1, y1, x2, y2) ((((y1) - (y2)) / ((x1) - (x2))) * ((x) - (x1)) + (y1))

inline static uint32_t get_ticks(void)
{
    return to_ms_since_boot(get_absolute_time());
}
