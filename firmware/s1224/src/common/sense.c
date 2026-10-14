/*
 * Sense for softmodem: carrier from demod stub + optional GP7 heuristic.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "sense.h"
#include "board_pins.h"
#include "config.h"
#include "afsk_soft.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdint.h>

static bool g_signal;
static bool g_packet;
static bool g_flag;
static absolute_time_t g_packet_until;
static absolute_time_t g_window_start;
static uint16_t g_edges;
static bool g_last_rxd;
static bool g_hold_valid;

void sense_init(void)
{
    g_signal = false;
    g_packet = false;
    g_flag = false;
    g_hold_valid = false;
    g_window_start = get_absolute_time();
    g_edges = 0;
    g_last_rxd = gpio_get(PIN_MODEM_RXD);
}

void sense_poll(void)
{
    const s1224_config_t *cfg = config_get();

    g_signal = afsk_rx_carrier_active();

    bool rxd = gpio_get(PIN_MODEM_RXD);
    if (rxd != g_last_rxd) {
        g_edges++;
        g_last_rxd = rxd;
    }

    uint32_t win_ms = cfg->packet_window_ms ? cfg->packet_window_ms : 50;
    if (absolute_time_diff_us(g_window_start, get_absolute_time()) >=
        (int64_t)win_ms * 1000) {
        if (g_edges >= cfg->packet_edge_min) {
            g_packet_until = make_timeout_time_ms(
                cfg->packet_hold_ms ? cfg->packet_hold_ms : 500);
            g_hold_valid = true;
        }
        g_edges = 0;
        g_window_start = get_absolute_time();
    }

    if (g_hold_valid) {
        if (time_reached(g_packet_until)) {
            g_hold_valid = false;
            g_packet = false;
            g_flag = false;
        } else {
            g_packet = true;
        }
    } else {
        g_packet = false;
    }
}

bool sense_signal_active(void) { return g_signal; }
bool sense_packet_active(void) { return g_packet; }
bool sense_packet_flag(void) { return g_flag; }
