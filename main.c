#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <bt.h>
#include <power_manager.h>

int main(void)
{
    stdio_init_all();
    power_manager_init();

    if (cyw43_arch_init()) {
        panic("Failed to init cyw43_arch!");
    }

    /* Initialize BT */
    bt_init("Pico Blaster", "0000");
    bt_set_stream_established_callback(power_manager_set_connected);
    bt_set_periodic_callback(power_manager_task, POWER_MANAGER_TASK_INTERVAL_MS);

    /* Run main BT loop */
    bt_run();
}
