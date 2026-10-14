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

static void banner_line(const char *text)
{
    size_t cols = banner_cols(text);
    size_t pad = 0;
    if (cols < BANNER_WIDTH) {
        pad = (BANNER_WIDTH - cols) / 2u;
    }
    for (size_t i = 0; i < pad; i++) {
        watchdog_update();
        putchar(' ');
    }
    fputs(text, stdout);
    fputs("\r\n", stdout);
}

void banner_print(void)
{
    /* Fixed boot banner — edit source only */
    char line[BANNER_WIDTH + 8];

    snprintf(line, sizeof(line), "T-Modem-s1224 %s", S1224_FW_VERSION);
    banner_line(line);

    snprintf(line, sizeof(line), "USB KISS Half-TNC · AFSK %u",
             (unsigned)afsk_bitrate());
    banner_line(line);

    banner_line("Non-Profit · GNU GPLv3 · AS-IS");
    banner_line("https://github.com/ngteq");
}
