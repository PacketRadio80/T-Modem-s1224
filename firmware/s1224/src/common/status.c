/*
 * Status from PTT + dual sense; texts from INI.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "status.h"
#include "config.h"
#include "display.h"
#include "ptt.h"
#include "sense.h"
#include <stdio.h>

static s1224_state_t g_state = S1224_STATE_STARTING;

const char *s1224_state_string(s1224_state_t st)
{
    const s1224_config_t *cfg = config_get();
    switch (st) {
    case S1224_STATE_STARTING:
        return cfg->starting;
    case S1224_STATE_READY:
        return cfg->ready;
    case S1224_STATE_FAULT:
        return cfg->fault;
    case S1224_STATE_SENDING:
        return cfg->sending;
    case S1224_STATE_SIGNAL:
        return cfg->signal;
    case S1224_STATE_PACKET:
        return cfg->packet;
    default:
        return cfg->fault;
    }
}

void status_set(s1224_state_t st)
{
    g_state = st;
}

s1224_state_t status_get(void)
{
    return g_state;
}

void status_publish(void)
{
    const char *s = s1224_state_string(g_state);
    if (config_unsolicited_ok()) {
        printf("STATUS %s\r\n", s);
    }
    display_update();
}

void status_tick(void)
{
    if (g_state == S1224_STATE_FAULT || g_state == S1224_STATE_STARTING) {
        display_update();
        return;
    }

    s1224_state_t next;
    if (ptt_get()) {
        next = S1224_STATE_SENDING;
    } else if (sense_packet_active()) {
        next = S1224_STATE_PACKET;
    } else if (sense_signal_active()) {
        next = S1224_STATE_SIGNAL;
    } else {
        next = S1224_STATE_READY;
    }

    if (next != g_state) {
        g_state = next;
        status_publish();
    } else {
        display_update();
    }
}
