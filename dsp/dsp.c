#include "dsp.h"
#include <biquad.h>
#include <math.h>
#include <utils.h>

typedef struct
{
    biquad_t subbass_cut;
    biquad_t bass_boost;
    biquad_t mid_cut;
    biquad_t high_boost;
    float volume;
} dsp_ctx_t;

static dsp_ctx_t ctx;

void dsp_init(float fs)
{
    biquad_init(&ctx.subbass_cut, BIQUAD_HIGHPASS, 72.0f, fs, M_SQRT1_2, 1.0f); // Removes subbass not reproducible by the speaker
    biquad_init(&ctx.bass_boost, BIQUAD_PEAK, 145.0f, fs, 1.4f, 4.5f);          // Boosts the bass
    biquad_init(&ctx.mid_cut, BIQUAD_PEAK, 400.0f, fs, 1.6f, -4.5f);            // Cuts resonant "boxiness" of the speaker
    biquad_init(&ctx.high_boost, BIQUAD_HIGHSHELF, 7000.0f, fs, 1.0f, 2.0f);    // Adds "air" by boosting the highest frequencies
}

void dsp_set_volume(float volume)
{
    if (volume == 0.0f) {
        ctx.volume = 0.0f;
        return;
    }

    const float a = 3.1623e-3f;
    const float b = 5.757f;
    ctx.volume = a * expf(b * volume);
	ctx.volume = UTILS_CLAMP(ctx.volume, 0.0f, 1.0f);
}

void dsp_process(int16_t *buffer, size_t frames_count)
{
    for (size_t i = 0; i < frames_count; ++i) {
        biquad_q31_t sample;

        /* Convert stereo to mono - the speaker is mono anyway,
         * so no need to process each channel separately. */
        sample = (buffer[2 * i] + buffer[2 * i + 1]) / 2;

        /* Scale volume */
        sample *= ctx.volume;

        /* Convert to Q31 */
        sample = BIQUAD_INT16_TO_Q31(sample);

        /* Apply EQ */
        sample = biquad_process(&ctx.subbass_cut, sample);
        sample = biquad_process(&ctx.bass_boost, sample);
        sample = biquad_process(&ctx.mid_cut, sample);
        sample = biquad_process(&ctx.high_boost, sample);

        /* Put processed sample back to the buffer */
        buffer[2 * i] = buffer[2 * i + 1] = BIQUAD_Q31_TO_INT16(sample);
    }
}
