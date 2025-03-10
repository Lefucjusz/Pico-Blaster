#pragma once

#include <stdbool.h>

typedef void (*bt_a2dp_stream_established_callback_t)(bool established);

void bt_a2dp_init(void);
void bt_a2dp_set_stream_established_callback(bt_a2dp_stream_established_callback_t callback);
