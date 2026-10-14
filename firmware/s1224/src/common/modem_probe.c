/*
 * Softmodem health — no chip/crystal checks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "modem_probe.h"
#include "config.h"
#include "afsk_soft.h"
#include <stdio.h>

modem_probe_result_t modem_probe_run(void)
{
    modem_probe_result_t r = {0};
    r.softmodem_ok = true;
    r.af_tx_pwm = true;
    r.ready = r.softmodem_ok && (afsk_bitrate() == 300u || afsk_bitrate() == 1200u || afsk_bitrate() == 2400u);

    if (config_unsolicited_ok()) {
        printf("PROBE softmodem=%d bitrate=%u -> %s\r\n",
               (int)r.softmodem_ok, afsk_bitrate(), r.ready ? "READY" : "FAULT");
    }
    return r;
}
