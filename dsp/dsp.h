#pragma once

#include <stdint.h>
#include <stddef.h>

void dsp_init(float fs);
void dsp_set_volume(float volume);

void dsp_process(int16_t *buffer, size_t frames_count);
