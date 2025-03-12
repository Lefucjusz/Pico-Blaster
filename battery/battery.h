#pragma once

#include <stdint.h>

#define BATTERY_ADC_PIN 26                  // Battery input ADC pin
#define BATTERY_VOLTAGE_FILTER_ALPHA 0.25f  // Alpha coefficient of EMA filter
#define BATTERY_VOLTAGE_DIVIDER_RATIO 0.5f  // R1 = R2 = 100k

void battery_init(void);

float battery_get_voltage_raw(void);
float battery_get_voltage(void);
uint8_t battery_get_percent(void);
