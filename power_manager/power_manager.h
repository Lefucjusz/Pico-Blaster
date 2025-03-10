#pragma once

#include <stdbool.h>

#define POWER_MANAGER_BUTTON_PIN 21
#define POWER_MANAGER_POWER_SWITCH_PIN 22
#define POWER_MANAGER_STATUS_LED_PIN 27

#define POWER_MANAGER_TASK_INTERVAL_MS 500

#define POWER_MANAGER_POWER_ON_DELAY_MS 2000
#define POWER_MANAGER_POWER_OFF_DELAY_MS 2000
#define POWER_MANAGER_AUTO_POWER_OFF_DELAY_MS (120 * 1000)

#define POWER_MANAGER_POWER_ON_LED_BLINK_TIME_MS 2000

void power_manager_init(void);
void power_manager_set_connected(bool connected);

void power_manager_task(void);
