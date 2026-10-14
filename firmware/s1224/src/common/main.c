/*
 * T-Modem-s1224 — USB KISS half-TNC + softmodem AFSK (PWM TX / Goertzel RX).
 * Three UF2 builds (300 / 1200 / 2400). No TCM3105 chip path.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "pico/stdlib.h"
#include "board_pins.h"
#include "status.h"
#include "ptt.h"
#include "modem_probe.h"
#include "usb_cmd.h"
#include "display.h"
#include "crystal_steer.h"
#include "baud_config.h"
#include "config.h"
#include "sense.h"
#include "afsk_soft.h"
#include "banner.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include <stdio.h>

int main(void)
{
    stdio_init_all();
    sleep_ms(800);
    watchdog_enable(8000, true);

    board_pins_init();
    ptt_init();
    config_init();
    display_init();
    sense_init();
    afsk_soft_init();
    usb_cmd_init();
    afsk_rx_set_callback(usb_cmd_on_rx_frame);

    status_set(S1224_STATE_STARTING);
    status_publish();
    /* Fixed boot banner — always once after CDC ready; source-only (banner.c). */
    banner_print();

    modem_probe_result_t probe = modem_probe_run();
    if (probe.ready) {
        status_set(S1224_STATE_READY);
    } else {
        status_set(S1224_STATE_FAULT);
    }
    status_publish();

    while (true) {
        watchdog_update();
        usb_cmd_poll();
        afsk_rx_poll();
        sense_poll();

        gpio_put(PIN_LED_DCD, sense_signal_active() ? 1 : 0);
        gpio_put(PIN_LED_PTT, ptt_get() ? 1 : 0);
        gpio_put(PIN_LED_TXRX, sense_packet_active() ? 1 : 0);

        status_tick();
        sleep_us(50);
    }
}
