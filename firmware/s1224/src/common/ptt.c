/*
 * PTT via Pico GPIO — host asserts over USB CDC (Type-A Path B), not softmodem.
 * PIN_PTT_DRV = keying (required). PIN_LED_PTT = optional/spare TX LED mirror.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ptt.h"
#include "board_pins.h"
#include "hardware/gpio.h"

static bool g_ptt;

void ptt_init(void)
{
    gpio_init(PIN_PTT_DRV);
    gpio_set_dir(PIN_PTT_DRV, GPIO_OUT);
    gpio_put(PIN_PTT_DRV, 0);
    g_ptt = false;
    gpio_put(PIN_LED_PTT, 0);
}

void ptt_set(bool on)
{
    g_ptt = on;
    gpio_put(PIN_PTT_DRV, on ? 1 : 0);
    gpio_put(PIN_LED_PTT, on ? 1 : 0);
}

bool ptt_get(void)
{
    return g_ptt;
}
