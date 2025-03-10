#pragma once

#include <pico/time.h>

inline static uint32_t get_ticks(void)
{
    return to_ms_since_boot(get_absolute_time());
}