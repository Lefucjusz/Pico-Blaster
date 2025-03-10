#pragma once

#include "a2dp.h"
#include <stdbool.h>
#include <stdint.h>

typedef void (*bt_periodic_callback_t)(void);

int bt_init(const char *name, const char *pin);
void bt_set_stream_established_callback(bt_a2dp_stream_established_callback_t callback);
void bt_set_periodic_callback(bt_periodic_callback_t callback, uint32_t interval_ms);

void bt_run(void);
