#include "battery.h"
#include <filter.h>
#include <utils.h>
#include <hardware/adc.h>
#include <pico/time.h>

#define BATTERY_ADC_CHANNEL_NUM (BATTERY_ADC_PIN - ADC_BASE_PIN)
#define BATTERY_ADC_VREF 3.3f
#define BATTERY_ADC_BITS 12
#define BATTERY_ADC_LEVELS (1 << BATTERY_ADC_BITS)
#define BATTERY_ADC_CONVERSION_FACTOR (BATTERY_ADC_VREF / BATTERY_VOLTAGE_DIVIDER_RATIO)

#define BATTERY_LUT_SIZE 6

typedef struct
{
    filter_t filter;
} battery_ctx_t;

typedef struct
{
    float voltage;
    uint8_t percent;
} battery_curve_point_t;

static battery_ctx_t ctx;

/* Data from some li-ion discharge curve found in Google */
static const battery_curve_point_t battery_lut[BATTERY_LUT_SIZE] = {
    {3.10f, 0},
    {3.55f, 20},
    {3.65f, 40},
    {3.80f, 60},
    {3.95f, 80},
    {4.15f, 100}
};

void battery_init(void)
{
    adc_init();
    adc_gpio_init(BATTERY_ADC_PIN);
    adc_select_input(BATTERY_ADC_CHANNEL_NUM);
    sleep_ms(100); // Wait for RC filter to stabilize after pin init

    const float voltage_raw = battery_get_voltage_raw();
    filter_init(&ctx.filter, voltage_raw, BATTERY_VOLTAGE_FILTER_ALPHA);
}

float battery_get_voltage_raw(void)
{
    const uint16_t adc_value = adc_read();
    const float voltage = ((float)adc_value / BATTERY_ADC_LEVELS) * BATTERY_ADC_CONVERSION_FACTOR;
    
    return voltage;
}

float battery_get_voltage(void)
{
    const float voltage_raw = battery_get_voltage_raw();
    const float voltage_filtered = filter_update(&ctx.filter, voltage_raw);

    return voltage_filtered;
}

uint8_t battery_get_percent(void)
{
    const float voltage = battery_get_voltage();
    
    /* Handle out-of-range cases */
    if (voltage <= battery_lut[0].voltage) {
        return PERCENT_MIN;
    }
    if (voltage >= battery_lut[BATTERY_LUT_SIZE - 1].voltage) {
        return PERCENT_MAX;
    }

    /* Find proper range and interpolate */
    for (size_t i = 1; i < BATTERY_LUT_SIZE; ++i) {
        if (voltage <= battery_lut[i].voltage) {
            return (uint8_t)INTERP1D(voltage, battery_lut[i - 1].voltage, battery_lut[i - 1].percent, battery_lut[i].voltage, battery_lut[i].percent);
        }
    }
    
    return PERCENT_MIN;
}
