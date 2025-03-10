#include "power_manager.h"
#include <utilities.h>
#include <hardware/gpio.h>

typedef struct
{
    bool connected;
    uint32_t last_disconnect_tick;
    uint32_t last_press_tick;
} power_manager_ctx_t;

static power_manager_ctx_t ctx;

static void power_manager_enable_power(bool enable)
{
    gpio_put(POWER_MANAGER_POWER_SWITCH_PIN, enable);
}

static void power_manager_enable_status_led(bool enable)
{
    gpio_put(POWER_MANAGER_STATUS_LED_PIN, enable);
}

static bool power_manager_is_button_pressed(void)
{
    return !gpio_get(POWER_MANAGER_BUTTON_PIN); // Active low
}

static void power_manager_gpio_irq_callback(uint gpio, uint32_t events)
{
    if (gpio == POWER_MANAGER_BUTTON_PIN) {
        ctx.last_press_tick = get_ticks();
    }
}

void power_manager_init(void)
{
    /* Wait to filter out accidental button clicks */
    sleep_ms(POWER_MANAGER_POWER_ON_DELAY_MS);

    /* Configure GPIOs */
    gpio_set_function(POWER_MANAGER_BUTTON_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(POWER_MANAGER_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(POWER_MANAGER_BUTTON_PIN);
    gpio_set_irq_enabled(POWER_MANAGER_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true);

    gpio_set_function(POWER_MANAGER_POWER_SWITCH_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(POWER_MANAGER_POWER_SWITCH_PIN, GPIO_OUT);

    gpio_set_function(POWER_MANAGER_STATUS_LED_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(POWER_MANAGER_STATUS_LED_PIN, GPIO_OUT);

    /* Configure GPIO IRQ */
    gpio_set_irq_callback(power_manager_gpio_irq_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);

    /* Enable power switch */
    power_manager_enable_power(true);

    /* Blink status LED to indicate the device is on */
    power_manager_enable_status_led(true);
    sleep_ms(POWER_MANAGER_POWER_ON_LED_BLINK_TIME_MS);
    power_manager_enable_status_led(false);
}

void power_manager_set_connected(bool connected)
{
    if (!connected) {
        ctx.last_disconnect_tick = get_ticks();
    }
    ctx.connected = connected;
}

void power_manager_task(void)
{
    const uint32_t current_tick = get_ticks();

    /* Check if turn off by button */
    if (power_manager_is_button_pressed() && ((current_tick - ctx.last_press_tick) >= POWER_MANAGER_POWER_OFF_DELAY_MS)) {
        // TODO disconnect? Mute audio?
        power_manager_enable_status_led(true); // Turn on status LED
        power_manager_enable_power(false); // Disable power switch
        return;
    }

    /* Check if turn off due to no connected device */
    if (!ctx.connected && ((current_tick - ctx.last_disconnect_tick) >= POWER_MANAGER_AUTO_POWER_OFF_DELAY_MS)) {
        power_manager_enable_power(false); // Disable power switch
        return;
    }

    /* TODO add blinking when battery low */
    /* TODO add power off when battery low */
    
}
