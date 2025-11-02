#pragma once

#include <stdint.h>

typedef int32_t biquad_q31_t;
typedef int64_t biquad_q63_t;

/* NOTE: these conversion are as simple as possible to speed up
 * computations. Normally they should also include rounding,
 * saturation, etc. */
#define BIQUAD_FLOAT_TO_Q31(x) ((biquad_q31_t)((x) * (1U << 31)))
#define BIQUAD_Q31_TO_FLOAT(x) ((float)(x) / (1U << 31))

#define BIQUAD_INT16_TO_Q31(x) ((biquad_q31_t)(x) << 15)
#define BIQUAD_Q31_TO_INT16(x) ((int16_t)(((x) + (1U << 14)) >> 15))

typedef enum
{
	BIQUAD_LOWPASS,
	BIQUAD_HIGHPASS,
	BIQUAD_BANDPASS,
	BIQUAD_NOTCH,
    BIQUAD_ALLPASS,
    BIQUAD_PEAK,
    BIQUAD_LOWSHELF,
    BIQUAD_HIGHSHELF,
	BIQUAD_TYPE_COUNT
} biquad_type_t;

typedef struct
{
	biquad_type_t type;
	float fc;
	float fs;
	float q;
	float gain;
	uint32_t post_shift;
	struct {
		biquad_q31_t a1;
		biquad_q31_t a2;
		biquad_q31_t b0;
		biquad_q31_t b1;
		biquad_q31_t b2;
	} coeffs;
	struct {
		biquad_q31_t xn_z1;
		biquad_q31_t xn_z2;
		biquad_q31_t yn_z1;
		biquad_q31_t yn_z2;
	} state;
} biquad_t;

int biquad_init(biquad_t *biquad, biquad_type_t type, float fc, float fs, float q, float gain);
int biquad_reset(biquad_t *biquad);

biquad_q31_t biquad_process(biquad_t *biquad, biquad_q31_t sample);
