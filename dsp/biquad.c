#include "biquad.h"
#include <utils.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>

typedef union
{
    struct {
        float a1;
        float a2;
        float b0;
        float b1;
        float b2;
    };
    float arr[5];
} biquad_coeffs_f32_t;

static void biquad_compute_coeffs(biquad_t *biquad, const biquad_coeffs_f32_t *coeffs)
{
    float max_coeff = 0.0f;

    /* Find the coefficient with maximum absolute value */
    for (size_t i = 0; i < UTILS_ARRAY_COUNT(coeffs->arr); ++i) {
        const float curr_coeff = fabsf(coeffs->arr[i]);
        if (max_coeff < curr_coeff) {
            max_coeff = curr_coeff;
        }
    }

    /* Find the post-shift factor - the exponent of 2 that gives value larger than max_coeff */
    if (max_coeff >= 1.0f) {
        biquad->post_shift = (uint32_t)ceilf(log2f(max_coeff));
    }
    else {
        biquad->post_shift = 0;
    }

    /* Scale to fit Q31 range and convert */
    biquad->coeffs.b0 = BIQUAD_FLOAT_TO_Q31(coeffs->b0 / (1U << biquad->post_shift));
    biquad->coeffs.b1 = BIQUAD_FLOAT_TO_Q31(coeffs->b1 / (1U << biquad->post_shift));
    biquad->coeffs.b2 = BIQUAD_FLOAT_TO_Q31(coeffs->b2 / (1U << biquad->post_shift));
    biquad->coeffs.a1 = BIQUAD_FLOAT_TO_Q31(coeffs->a1 / (1U << biquad->post_shift));
    biquad->coeffs.a2 = BIQUAD_FLOAT_TO_Q31(coeffs->a2 / (1U << biquad->post_shift));
}

static void biquad_lowpass_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float c, float alpha)
{
    const float a0 = 1.0f + alpha;

    coeffs->b0 = ((1.0f - c) / 2.0f) / a0;
    coeffs->b1 = (1.0f - c) / a0;
    coeffs->b2 = ((1.0f - c) / 2.0f) / a0;
    coeffs->a1 = (-2.0f * c) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}

static void biquad_highpass_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float c, float alpha)
{
    const float a0 = 1.0f + alpha;

    coeffs->b0 = ((1.0f + c) / 2.0f) / a0;
    coeffs->b1 = (-(1.0f + c)) / a0;
    coeffs->b2 = ((1.0f + c) / 2.0f) / a0;
    coeffs->a1 = (-2.0f * c) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}

static void biquad_bandpass_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float c, float alpha)
{
    const float a0 = 1.0f + alpha;

    coeffs->b0 = alpha / a0;
    coeffs->b1 = 0.0f;
    coeffs->b2 = -alpha / a0;
    coeffs->a1 = (-2.0f * c) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}

static void biquad_notch_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float c, float alpha)
{
    const float a0 = 1.0f + alpha;

    coeffs->b0 = 1.0f / a0;
    coeffs->b1 = (-2.0f * c) / a0;
    coeffs->b2 = 1.0f / a0;
    coeffs->a1 = (-2.0f * c) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}

static void biquad_allpass_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float c, float alpha)
{
    const float a0 = 1.0f + alpha;

    coeffs->b0 = (1.0f - alpha) / a0;
    coeffs->b1 = (-2.0f * c) / a0;
    coeffs->b2 = (1.0f + alpha) / a0;
    coeffs->a1 = (-2.0f * c) / a0;
    coeffs->a2 = (1.0f - alpha) / a0;
}

static void biquad_peak_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float c, float alpha, float gain)
{
    const float A = powf(10.0f, gain / 40.0f);
    const float a0 = 1.0f + (alpha / A);

    coeffs->b0 = (1.0f + (alpha * A)) / a0;
    coeffs->b1 = (-2.0f * c) / a0;
    coeffs->b2 = (1.0f - (alpha * A)) / a0;
    coeffs->a1 = (-2.0f * c) / a0;
    coeffs->a2 = (1.0f - (alpha / A)) / a0;
}

static void biquad_lowshelf_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float s, float c, float gain)
{
    const float A = powf(10.0f, gain / 40.0f);
    const float beta = sqrtf(2.0f * A); // "Handy intermediate variable for shelving EQ filters" for S = 1
    const float a0 = (A + 1.0f) + ((A - 1.0f) * c) + (s * beta);

    coeffs->b0 = (A * ((A + 1.0f) - ((A - 1.0f) * c) + (s * beta))) / a0;
    coeffs->b1 = (2.0f * A * ((A - 1.0f) - ((A + 1.0f) * c))) / a0;
    coeffs->b2 = (A * ((A + 1.0f) - ((A - 1.0f) * c) - (s * beta))) / a0;
    coeffs->a1 = (-2.0f * ((A - 1.0f) + ((A + 1.0f) * c))) / a0;
    coeffs->a2 = ((A + 1.0f) + ((A - 1.0f) * c) - (s * beta)) / a0;
}

static void biquad_highshelf_get_f32_coeffs(biquad_coeffs_f32_t *coeffs, float s, float c, float gain)
{
    const float A = powf(10.0f, gain / 40.0f);
    const float beta = sqrtf(2.0f * A); // "Handy intermediate variable for shelving EQ filters" for S = 1
    const float a0 = (A + 1.0f) - ((A - 1.0f) * c) + (s * beta);

    coeffs->b0 = (A * ((A + 1.0f) + ((A - 1.0f) * c) + (s * beta))) / a0;
    coeffs->b1 = (-2.0f * A * ((A - 1.0f) + ((A + 1.0f) * c))) / a0;
    coeffs->b2 = (A * ((A + 1.0f) + ((A - 1.0f) * c) - (s * beta))) / a0;
    coeffs->a1 = (2.0f * ((A - 1.0f) - ((A + 1.0f) * c))) / a0;
    coeffs->a2 = ((A + 1.0f) - ((A - 1.0f) * c) - (s * beta)) / a0;
}

int biquad_init(biquad_t *biquad, biquad_type_t type, float fc, float fs, float q, float gain)
{
    if (biquad == NULL) {
        return -EINVAL;
    }

    if ((type < 0) || (type >= BIQUAD_TYPE_COUNT)) {
        return -EINVAL;
    }

    if ((fc <= 0) || (fs <= 0) || (q <= 0)) {
        return -EINVAL;
    }

    biquad->type = type;
    biquad->fc = fc;
    biquad->fs = fs;
    biquad->q = q;
    biquad->gain = gain;

    const float w0 = 2.0f * (float)M_PI * biquad->fc / biquad->fs;
    const float s = sinf(w0);
    const float c = cosf(w0);
    const float alpha = s / (2.0f *  biquad->q);

    biquad_coeffs_f32_t f32_coeffs;

    switch (biquad->type) {
        case BIQUAD_LOWPASS:
            biquad_lowpass_get_f32_coeffs(&f32_coeffs, c, alpha);
            break;
        case BIQUAD_HIGHPASS:
            biquad_highpass_get_f32_coeffs(&f32_coeffs, c, alpha);
            break;
        case BIQUAD_BANDPASS:
            biquad_bandpass_get_f32_coeffs(&f32_coeffs, c, alpha);
            break;
        case BIQUAD_NOTCH:
            biquad_notch_get_f32_coeffs(&f32_coeffs, c, alpha);
            break;
        case BIQUAD_ALLPASS:
            biquad_allpass_get_f32_coeffs(&f32_coeffs, c, alpha);
            break;
        case BIQUAD_PEAK:
            biquad_peak_get_f32_coeffs(&f32_coeffs, c, alpha, biquad->gain);
            break;
        case BIQUAD_LOWSHELF:
            biquad_lowshelf_get_f32_coeffs(&f32_coeffs, s, c, biquad->gain);
            break;
        case BIQUAD_HIGHSHELF:
            biquad_highshelf_get_f32_coeffs(&f32_coeffs, s, c, biquad->gain);
            break;
        default:
            break;
    }

    biquad_compute_coeffs(biquad, &f32_coeffs);
    biquad_reset(biquad);

    return 0;
}

int biquad_reset(biquad_t *biquad)
{
    if (biquad == NULL) {
        return -EINVAL;
    }

    biquad->state.xn_z1 = 0;
    biquad->state.xn_z2 = 0;
    biquad->state.yn_z1 = 0;
    biquad->state.yn_z2 = 0;

    return 0;
}

biquad_q31_t biquad_process(biquad_t *biquad, biquad_q31_t sample)
{
    const biquad_q31_t xn = sample;
    biquad_q63_t accum;

    accum = (biquad_q63_t)biquad->coeffs.b0 * xn;
    accum += (biquad_q63_t)biquad->coeffs.b1 * biquad->state.xn_z1;
    accum += (biquad_q63_t)biquad->coeffs.b2 * biquad->state.xn_z2;
    accum -= (biquad_q63_t)biquad->coeffs.a1 * biquad->state.yn_z1;
    accum -= (biquad_q63_t)biquad->coeffs.a2 * biquad->state.yn_z2;

    /* Convert the result from Q62 back to Q31, adding post-shift to compensate for coefficients scaling */
    const biquad_q31_t yn = accum >> (31 - biquad->post_shift);

    biquad->state.xn_z2 = biquad->state.xn_z1;
    biquad->state.xn_z1 = xn;

    biquad->state.yn_z2 = biquad->state.yn_z1;
    biquad->state.yn_z1 = yn;

    return yn;
}
