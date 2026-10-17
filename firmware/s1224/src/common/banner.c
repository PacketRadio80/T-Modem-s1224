/*
 * Fixed boot banner — edit source only; not INI / CDC.
 * Always printed once after USB CDC ready (main); quiet never suppresses it.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "banner.h"
#include "version.h"
#include "afsk_soft.h"
#include "hardware/watchdog.h"
#include <stdio.h>
#include <string.h>

/* Fixed CDC column width for centered serial look (40–48). */
#define BANNER_WIDTH 44

/** Display columns (UTF-8 code points ≈ 1 col; ASCII-safe). */
static size_t banner_cols(const char *s)
{
    size_t cols = 0;
    while (*s) {
        unsigned char c = (unsigned char)*s++;
        if ((c & 0x80u) == 0u) {
            cols++;
        } else if ((c & 0xE0u) == 0xC0u) {
            if (*s) {
                s++;
            }
            cols++;
        } else if ((c & 0xF0u) == 0xE0u) {
            if (*s) {
                s++;
            }
            if (*s) {
                s++;
            }
            cols++;
        } else if ((c & 0xF8u) == 0xF0u) {
            if (*s) {
                s++;
            }
            if (*s) {
                s++;
            }
            if (*s) {
                s++;
            }
            cols++;
        } else {
            cols++;
        }
    }
    return cols;
}

/* Boxed banner helpers. Pure ASCII ('+' '-' '|'), spaces only (no TAB),
 * fixed BANNER_WIDTH columns so CDC terminals and log parsers align. */
static void banner_rule(void)
{
    watchdog_update();
    putchar('+');
    for (size_t i = 0; i < BANNER_WIDTH - 2u; i++) {
        putchar('-');
    }
    putchar('+');
    fputs("\r\n", stdout);
}

static void banner_boxline(const char *text)
{
    size_t inner = BANNER_WIDTH - 2u;
    size_t cols = banner_cols(text);
    if (cols > inner) {
        cols = inner;
    }
    size_t pad = (inner - cols) / 2u;
    watchdog_update();
    putchar('|');
    for (size_t i = 0; i < pad; i++) {
        putchar(' ');
    }
    fputs(text, stdout);
    for (size_t i = cols + pad; i < inner; i++) {
        putchar(' ');
    }
    putchar('|');
    fputs("\r\n", stdout);
}

void banner_print(void)
{
    /* Fixed boot banner - edit source only. Identical across T-Modem
     * firmwares (c1224 chip modem + s1224 softmodem); only the runtime
     * AFSK rate differs per build. */
    char line[BANNER_WIDTH + 8];

    banner_rule();

    snprintf(line, sizeof(line), "T-Modem Firmware v%s", S1224_FW_VERSION);
    banner_boxline(line);

    snprintf(line, sizeof(line), "USB KISS Half-TNC - AFSK %u",
             (unsigned)afsk_bitrate());
    banner_boxline(line);

    banner_boxline("Packet Radio | GNU GPLv3 | AS-IS");
    banner_boxline("github.com/PacketRadio80");

    banner_rule();
}
