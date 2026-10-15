/*
 * USB CDC multiplexer: KISS binary (0xC0 frames) + ASCII service commands.
 * Transparent Host↔Chip bit bridge stays abandoned — KISS half-TNC is the exception.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "usb_cmd.h"
#include "pico/stdlib.h"
#include "ptt.h"
#include "status.h"
#include "config.h"
#include "sense.h"
#include "board_pins.h"
#include "afsk_soft.h"
#include "kiss.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "tusb.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LINE_MAX 96

static char g_line[LINE_MAX];
static unsigned g_len;
static kiss_rx_t g_kiss;
static uint8_t g_kiss_payload[KISS_MAX_PAYLOAD];
static bool g_baud1200;

static void trim_copy_upper_cmd(char *dst, size_t dstsz, const char *src)
{
    while (*src && isspace((unsigned char)*src)) {
        src++;
    }
    size_t i = 0;
    while (src[i] && i + 1 < dstsz) {
        dst[i] = (char)toupper((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
    while (i > 0 && isspace((unsigned char)dst[i - 1])) {
        dst[--i] = '\0';
    }
}

static void handle_line(void)
{
    char cmd[LINE_MAX];
    trim_copy_upper_cmd(cmd, sizeof(cmd), g_line);
    if (cmd[0] == '\0') {
        return;
    }

    if (strcmp(cmd, "PTT ON") == 0 || strcmp(cmd, "PTT 1") == 0) {
        ptt_set(true);
        status_tick();
        printf("OK PTT ON\r\n");
        return;
    }
    if (strcmp(cmd, "PTT OFF") == 0 || strcmp(cmd, "PTT 0") == 0) {
        ptt_set(false);
        status_tick();
        printf("OK PTT OFF\r\n");
        return;
    }
    if (strcmp(cmd, "STATUS") == 0) {
        printf("STATUS state=%s ptt=%d signal=%d packet=%d flag=%d "
               "carrier=%d bitrate=%u softmodem=1 kiss_half_tnc=1\r\n",
               s1224_state_string(status_get()),
               (int)ptt_get(),
               (int)sense_signal_active(),
               (int)sense_packet_active(),
               (int)sense_packet_flag(),
               (int)afsk_rx_carrier_active(),
               afsk_bitrate());
        return;
    }
    if (strcmp(cmd, "HELP") == 0) {
        printf("Commands:\r\n");
        printf("  PTT ON | PTT OFF\r\n");
        printf("  STATUS | HELP | ID\r\n");
        printf("  QUIET ON|OFF | VERBOSE ON|OFF\r\n");
        printf("  INI | GET <key> | SET <key>=<value>\r\n");
        printf("  SAVE | RELOAD | DEFAULTS\r\n");
        printf("  BOOTSEL | REBOOT\r\n");
        printf("Dual protocol: byte 0xC0 → KISS; line ASCII → service.\r\n");
        printf("Packet: use KISS-only (any host). Service cmds optional.\r\n");
        printf("Boot banner always on (source). quiet=1 damps extra ASCII.\r\n");
        printf("PTT = GP9 (auto during KISS TX; manual PTT ON/OFF kept).\r\n");
        printf("Softmodem AFSK · S1224_BITRATE=%u (build: 300/1200/2400).\r\n",
               afsk_bitrate());
        printf("PWM AF_TX GP6 · ADC AF_RX GP26 · no chip modem · no baud switch.\r\n");
        printf("INI: texts + callsign device_id quiet + sense keys\r\n");
        return;
    }
    if (strcmp(cmd, "ID") == 0) {
        const s1224_config_t *cfg = config_get();
        printf("ID device_id=%s callsign=%s bitrate=%u quiet=%d\r\n",
               cfg->device_id[0] ? cfg->device_id : "s1224",
               cfg->callsign,
               afsk_bitrate(),
               (int)cfg->quiet);
        return;
    }
    if (strcmp(cmd, "QUIET ON") == 0 || strcmp(cmd, "QUIET 1") == 0) {
        config_mutable()->quiet = true;
        printf("OK QUIET ON\r\n");
        return;
    }
    if (strcmp(cmd, "QUIET OFF") == 0 || strcmp(cmd, "QUIET 0") == 0) {
        config_mutable()->quiet = false;
        printf("OK QUIET OFF\r\n");
        return;
    }
    if (strcmp(cmd, "VERBOSE ON") == 0 || strcmp(cmd, "VERBOSE 1") == 0) {
        config_mutable()->quiet = false;
        printf("OK VERBOSE ON\r\n");
        return;
    }
    if (strcmp(cmd, "VERBOSE OFF") == 0 || strcmp(cmd, "VERBOSE 0") == 0) {
        config_mutable()->quiet = true;
        printf("OK VERBOSE OFF\r\n");
        return;
    }
    if (strcmp(cmd, "INI") == 0) {
        config_dump_ini();
        return;
    }
    if (strcmp(cmd, "SAVE") == 0) {
        printf(config_save_flash() ? "OK SAVE\r\n" : "ERR SAVE\r\n");
        return;
    }
    if (strcmp(cmd, "RELOAD") == 0) {
        config_load_defaults();
        if (config_load_flash()) {
            printf("OK RELOAD flash\r\n");
        } else {
            printf("OK RELOAD defaults (no flash)\r\n");
        }
        status_publish();
        return;
    }
    if (strcmp(cmd, "DEFAULTS") == 0) {
        config_load_defaults();
        printf("OK DEFAULTS (RAM; SAVE to persist)\r\n");
        status_publish();
        return;
    }
    if (strcmp(cmd, "BOOTSEL") == 0 || strcmp(cmd, "DFU") == 0) {
        printf("OK BOOTSEL\r\n");
        stdio_flush();
        sleep_ms(100);
        watchdog_disable();
        reset_usb_boot(0, 0);
    }
    if (strcmp(cmd, "REBOOT") == 0) {
        printf("OK REBOOT\r\n");
        stdio_flush();
        sleep_ms(100);
        watchdog_reboot(0, 0, 10);
        while (1) {
            tight_loop_contents();
        }
    }
    if (strncmp(cmd, "GET ", 4) == 0) {
        char key[32];
        const char *p = g_line;
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (strncasecmp(p, "GET", 3) == 0) {
            p += 3;
        }
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        size_t i = 0;
        while (*p && !isspace((unsigned char)*p) && i + 1 < sizeof(key)) {
            key[i++] = *p++;
        }
        key[i] = '\0';
        char val[S1224_TEXT_MAX];
        if (config_get_kv(key, val, sizeof(val))) {
            printf("OK %s=%s\r\n", key, val);
        } else {
            printf("ERR unknown key\r\n");
        }
        return;
    }
    if (strncmp(cmd, "SET ", 4) == 0) {
        const char *p = g_line;
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (strncasecmp(p, "SET", 3) == 0) {
            p += 3;
        }
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        char tmp[LINE_MAX];
        snprintf(tmp, sizeof(tmp), "%s", p);
        char *eq = strchr(tmp, '=');
        if (!eq) {
            printf("ERR SET key=value\r\n");
            return;
        }
        *eq = '\0';
        char *k = tmp;
        char *v = eq + 1;
        while (*k && isspace((unsigned char)*k)) {
            k++;
        }
        char *ke = k + strlen(k);
        while (ke > k && isspace((unsigned char)ke[-1])) {
            *--ke = '\0';
        }
        while (*v && isspace((unsigned char)*v)) {
            v++;
        }
        char *ve = v + strlen(v);
        while (ve > v && isspace((unsigned char)ve[-1])) {
            *--ve = '\0';
        }
        if (config_set_kv(k, v)) {
            printf("OK SET %s=%s (SAVE to persist)\r\n", k, v);
            status_publish();
        } else {
            printf("ERR unknown key or bad value\r\n");
        }
        return;
    }

    printf("ERR unknown (try HELP)\r\n");
}

void usb_cmd_on_rx_frame(const uint8_t *info, size_t len)
{
    if (!sense_signal_active()) {
        return;
    }
    uint8_t frame[KISS_MAX_PAYLOAD + 16];
    size_t n = kiss_encode_data(info, len, frame, sizeof(frame));
    for (size_t i = 0; i < n; i++) {
        watchdog_update();
        putchar_raw((int)frame[i]);
    }
}

void usb_cmd_poll(void)
{
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        uint8_t b = (uint8_t)c;

        /* KISS path: FEND or mid-frame */
        if (b == KISS_FEND || g_kiss.in_frame) {
            size_t plen = 0;
            if (kiss_rx_byte(&g_kiss, b, g_kiss_payload, &plen)) {
                if (plen > 0 && !afsk_tx_busy()) {
                    afsk_tx_frame(g_kiss_payload, plen);
                }
            }
            continue;
        }

        if (c == '\r' || c == '\n') {
            if (g_len > 0) {
                g_line[g_len] = '\0';
                handle_line();
                g_len = 0;
            }
            continue;
        }
        if (c == 0x08 || c == 0x7f) {
            if (g_len > 0) {
                g_len--;
            }
            continue;
        }
        if (g_len + 1 < LINE_MAX && c >= 32 && c < 127) {
            g_line[g_len++] = (char)c;
        }
    }
}

void usb_cmd_init(void)
{
    g_len = 0;
    kiss_rx_init(&g_kiss);
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    (void)itf;
    g_baud1200 = (p_line_coding->bit_rate == 1200);
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void)itf;
    (void)rts;
    if (g_baud1200 && !dtr) {
        watchdog_disable();
        reset_usb_boot(0, 0);
    }
}
