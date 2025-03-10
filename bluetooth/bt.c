#include "bt.h"
#include "avrcp.h"
#include "sdp.h"
#include "a2dp.h"
#include <errno.h>
#include <hci.h>
#include <l2cap.h>
#include <btstack_event.h>
#include <btstack_run_loop.h>

typedef struct 
{
    btstack_packet_callback_registration_t hci_registration;
    bd_addr_t local_addr;
    const char *pin;
    btstack_timer_source_t periodic_timer;
    bt_periodic_callback_t periodic_callback;
    uint32_t periodic_interval_ms;
} bt_ctx_t;

static bt_ctx_t ctx;

static void bt_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    const uint8_t type = hci_event_packet_get_type(packet);
    bd_addr_t addr;

    switch (type) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                break;
            }
            gap_local_bd_addr(ctx.local_addr);
            break;

        case HCI_EVENT_PIN_CODE_REQUEST:
            hci_event_pin_code_request_get_bd_addr(packet, addr);
            gap_pin_code_response(addr, ctx.pin);
            break;

        default:
            break;
    }
}

static void bt_periodic_timer_callback(btstack_timer_source_t *ts)
{
    /* Run callback */
    if (ctx.periodic_callback != NULL) {
        ctx.periodic_callback();
    }

    /* Restart timer */
    btstack_run_loop_set_timer(ts, ctx.periodic_interval_ms);
    btstack_run_loop_add_timer(ts);
}

int bt_init(const char *name, const char *pin)
{
    if ((name == NULL) || (pin == NULL)) {
        return -EINVAL;
    }

    ctx.pin = pin;

    l2cap_init();

    bt_sdp_init();
    bt_a2dp_init();
    bt_avrcp_init();

    gap_set_local_name(name);
    gap_discoverable_control(1); // Allow to show up in Bluetooth inquiry
    gap_set_class_of_device(0x200414); // Service Class: Audio, Major Device Class: Audio, Minor: Loudspeaker
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE); // Allow for role switch in general and sniff mode
    gap_set_allow_role_switch(true); // Allow for role switch on outgoing connections

    ctx.hci_registration.callback = bt_packet_handler;
    hci_add_event_handler(&ctx.hci_registration);

    return 0;
}

void bt_set_stream_established_callback(bt_a2dp_stream_established_callback_t callback)
{
    bt_a2dp_set_stream_established_callback(callback);
}

void bt_set_periodic_callback(bt_periodic_callback_t callback, uint32_t interval_ms)
{
    /* Remove any previously set timer */
    btstack_run_loop_remove_timer(&ctx.periodic_timer);

    /* Sanity check */
    if ((callback == NULL) || (interval_ms == 0)) {
        return;
    }

    /* Add new timer to handle periodic callback */
    ctx.periodic_callback = callback;
    ctx.periodic_interval_ms = interval_ms;

    btstack_run_loop_set_timer_handler(&ctx.periodic_timer, bt_periodic_timer_callback);
    btstack_run_loop_set_timer(&ctx.periodic_timer, ctx.periodic_interval_ms);
    btstack_run_loop_add_timer(&ctx.periodic_timer);
}

void bt_run(void)
{
    hci_power_control(HCI_POWER_ON);
    btstack_run_loop_execute();
}
