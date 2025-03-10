#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <bt.h>
#include <power_manager.h>

/* TODO
* - Add turn off after 2 minutes without connection
* - Add voltage readout and led blinking when low */

static void stream_established_callback(bool established) // TODO this can be removed
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, established); // TODO this is only for debug
    power_manager_set_connected(established);
}

int main(void)
{
    stdio_init_all();
    power_manager_init();

    if (cyw43_arch_init()) {
        panic("Failed to init cyw43_arch!");
    }

    /* Initialize BT */
    bt_init("Lefucjusz's Pico Speaker", "0000");
    bt_set_stream_established_callback(stream_established_callback);
    bt_set_periodic_callback(power_manager_task, POWER_MANAGER_TASK_INTERVAL_MS);

    /* Run main BT loop */
    bt_run();
}
