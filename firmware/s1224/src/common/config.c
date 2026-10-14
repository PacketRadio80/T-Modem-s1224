/*
 * INI defaults + flash overlay (last 4 KiB of Pico flash).
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "config.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define CFG_MAGIC0 'S'
#define CFG_MAGIC1 '1'
#define CFG_MAGIC2 '2'
#define CFG_MAGIC3 '4'
#define CFG_VERSION 1
#define CFG_SECTOR_SIZE 4096
/* Store config in the last flash sector */
#define CFG_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - CFG_SECTOR_SIZE)

typedef struct __attribute__((packed)) {
    uint8_t magic[4];
    uint8_t version;
    uint8_t reserved;
    uint16_t length;
    uint16_t crc;
    /* uint8_t data[length] follows */
} cfg_hdr_t;

static s1224_config_t g_cfg;

static uint16_t crc16_ccitt(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x8000) {
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static void copy_text(char *dst, const char *src)
{
    size_t n = strlen(src);
    if (n >= S1224_TEXT_MAX) {
        n = S1224_TEXT_MAX - 1;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

void config_load_defaults(void)
{
    copy_text(g_cfg.starting, "s1224 is starting");
    copy_text(g_cfg.ready, "s1224 is ready");
    copy_text(g_cfg.fault, "s1224 is fault");
    copy_text(g_cfg.sending, "s1224 is sending");
    copy_text(g_cfg.signal, "s1224 signal");
    copy_text(g_cfg.packet, "s1224 packet");
    copy_text(g_cfg.power, "s1224 power");
    copy_text(g_cfg.idle, "s1224 idle");
    copy_text(g_cfg.callsign, "");
    copy_text(g_cfg.device_id, "s1224");
    g_cfg.quiet = true;
    g_cfg.cdt_active_high = false;
    g_cfg.packet_hold_ms = 500;
    g_cfg.packet_edge_min = 4;
    g_cfg.packet_window_ms = 50;
}

static char *trim(char *s)
{
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) {
        *--e = '\0';
    }
    return s;
}

static bool parse_bool(const char *v, bool *out)
{
    if (!v || !*v) {
        return false;
    }
    if (v[0] == '1' || v[0] == 'y' || v[0] == 'Y' || v[0] == 't' || v[0] == 'T') {
        *out = true;
        return true;
    }
    if (v[0] == '0' || v[0] == 'n' || v[0] == 'N' || v[0] == 'f' || v[0] == 'F') {
        *out = false;
        return true;
    }
    return false;
}

bool config_set_kv(const char *key, const char *value)
{
    if (!key || !value) {
        return false;
    }
    char kbuf[32];
    size_t n = strlen(key);
    if (n >= sizeof(kbuf)) {
        return false;
    }
    for (size_t i = 0; i <= n; i++) {
        kbuf[i] = (char)tolower((unsigned char)key[i]);
    }

    if (strcmp(kbuf, "starting") == 0) {
        copy_text(g_cfg.starting, value);
        return true;
    }
    if (strcmp(kbuf, "ready") == 0) {
        copy_text(g_cfg.ready, value);
        return true;
    }
    if (strcmp(kbuf, "fault") == 0) {
        copy_text(g_cfg.fault, value);
        return true;
    }
    if (strcmp(kbuf, "sending") == 0) {
        copy_text(g_cfg.sending, value);
        return true;
    }
    if (strcmp(kbuf, "signal") == 0) {
        copy_text(g_cfg.signal, value);
        return true;
    }
    if (strcmp(kbuf, "packet") == 0) {
        copy_text(g_cfg.packet, value);
        return true;
    }
    if (strcmp(kbuf, "power") == 0) {
        copy_text(g_cfg.power, value);
        return true;
    }
    if (strcmp(kbuf, "idle") == 0) {
        copy_text(g_cfg.idle, value);
        return true;
    }
    if (strcmp(kbuf, "callsign") == 0) {
        copy_text(g_cfg.callsign, value);
        return true;
    }
    if (strcmp(kbuf, "device_id") == 0) {
        copy_text(g_cfg.device_id, value);
        return true;
    }
    if (strcmp(kbuf, "quiet") == 0) {
        bool b;
        if (!parse_bool(value, &b)) {
            return false;
        }
        g_cfg.quiet = b;
        return true;
    }
    if (strcmp(kbuf, "cdt_active_high") == 0) {
        bool b;
        if (!parse_bool(value, &b)) {
            return false;
        }
        g_cfg.cdt_active_high = b;
        return true;
    }
    if (strcmp(kbuf, "packet_hold_ms") == 0) {
        g_cfg.packet_hold_ms = (uint16_t)atoi(value);
        return true;
    }
    if (strcmp(kbuf, "packet_edge_min") == 0) {
        g_cfg.packet_edge_min = (uint16_t)atoi(value);
        return true;
    }
    if (strcmp(kbuf, "packet_window_ms") == 0) {
        g_cfg.packet_window_ms = (uint16_t)atoi(value);
        return true;
    }
    return false;
}

bool config_get_kv(const char *key, char *buf, size_t buflen)
{
    if (!key || !buf || buflen == 0) {
        return false;
    }
    char kbuf[32];
    size_t n = strlen(key);
    if (n >= sizeof(kbuf)) {
        return false;
    }
    for (size_t i = 0; i <= n; i++) {
        kbuf[i] = (char)tolower((unsigned char)key[i]);
    }

    const char *src = NULL;
    char num[16];
    if (strcmp(kbuf, "starting") == 0) {
        src = g_cfg.starting;
    } else if (strcmp(kbuf, "ready") == 0) {
        src = g_cfg.ready;
    } else if (strcmp(kbuf, "fault") == 0) {
        src = g_cfg.fault;
    } else if (strcmp(kbuf, "sending") == 0) {
        src = g_cfg.sending;
    } else if (strcmp(kbuf, "signal") == 0) {
        src = g_cfg.signal;
    } else if (strcmp(kbuf, "packet") == 0) {
        src = g_cfg.packet;
    } else if (strcmp(kbuf, "power") == 0) {
        src = g_cfg.power;
    } else if (strcmp(kbuf, "idle") == 0) {
        src = g_cfg.idle;
    } else if (strcmp(kbuf, "callsign") == 0) {
        src = g_cfg.callsign;
    } else if (strcmp(kbuf, "device_id") == 0) {
        src = g_cfg.device_id;
    } else if (strcmp(kbuf, "quiet") == 0) {
        snprintf(num, sizeof(num), "%d", (int)g_cfg.quiet);
        src = num;
    } else if (strcmp(kbuf, "cdt_active_high") == 0) {
        snprintf(num, sizeof(num), "%d", (int)g_cfg.cdt_active_high);
        src = num;
    } else if (strcmp(kbuf, "packet_hold_ms") == 0) {
        snprintf(num, sizeof(num), "%u", (unsigned)g_cfg.packet_hold_ms);
        src = num;
    } else if (strcmp(kbuf, "packet_edge_min") == 0) {
        snprintf(num, sizeof(num), "%u", (unsigned)g_cfg.packet_edge_min);
        src = num;
    } else if (strcmp(kbuf, "packet_window_ms") == 0) {
        snprintf(num, sizeof(num), "%u", (unsigned)g_cfg.packet_window_ms);
        src = num;
    } else {
        return false;
    }
    snprintf(buf, buflen, "%s", src);
    return true;
}

static void parse_ini_text(char *text)
{
    char *save = NULL;
    for (char *line = strtok_r(text, "\r\n", &save); line;
         line = strtok_r(NULL, "\r\n", &save)) {
        line = trim(line);
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            continue; /* section headers ignored — flat keys */
        }
        char *eq = strchr(line, '=');
        if (!eq) {
            continue;
        }
        *eq = '\0';
        char *k = trim(line);
        char *v = trim(eq + 1);
        (void)config_set_kv(k, v);
    }
}

bool config_load_flash(void)
{
    const uint8_t *base = (const uint8_t *)(XIP_BASE + CFG_FLASH_OFFSET);
    const cfg_hdr_t *hdr = (const cfg_hdr_t *)base;
    if (hdr->magic[0] != CFG_MAGIC0 || hdr->magic[1] != CFG_MAGIC1 ||
        hdr->magic[2] != CFG_MAGIC2 || hdr->magic[3] != CFG_MAGIC3) {
        return false;
    }
    if (hdr->version != CFG_VERSION) {
        return false;
    }
    if (hdr->length == 0 || hdr->length > (CFG_SECTOR_SIZE - sizeof(cfg_hdr_t) - 1)) {
        return false;
    }
    const uint8_t *data = base + sizeof(cfg_hdr_t);
    if (crc16_ccitt(data, hdr->length) != hdr->crc) {
        return false;
    }
    static char tmp[CFG_SECTOR_SIZE];
    if (hdr->length + 1 > sizeof(tmp)) {
        return false;
    }
    memcpy(tmp, data, hdr->length);
    tmp[hdr->length] = '\0';
    parse_ini_text(tmp);
    return true;
}

static size_t build_ini(char *out, size_t outsz)
{
    int n = snprintf(out, outsz,
                     "# s1224.ini — operator texts + terminal + sense\n"
                     "[texts]\n"
                     "starting=%s\n"
                     "ready=%s\n"
                     "fault=%s\n"
                     "sending=%s\n"
                     "signal=%s\n"
                     "packet=%s\n"
                     "power=%s\n"
                     "idle=%s\n"
                     "[terminal]\n"
                     "callsign=%s\n"
                     "device_id=%s\n"
                     "quiet=%d\n"
                     "[sense]\n"
                     "cdt_active_high=%d\n"
                     "packet_hold_ms=%u\n"
                     "packet_edge_min=%u\n"
                     "packet_window_ms=%u\n",
                     g_cfg.starting, g_cfg.ready, g_cfg.fault, g_cfg.sending,
                     g_cfg.signal, g_cfg.packet, g_cfg.power, g_cfg.idle,
                     g_cfg.callsign, g_cfg.device_id,
                     (int)g_cfg.quiet,
                     (int)g_cfg.cdt_active_high,
                     (unsigned)g_cfg.packet_hold_ms,
                     (unsigned)g_cfg.packet_edge_min,
                     (unsigned)g_cfg.packet_window_ms);
    if (n < 0) {
        return 0;
    }
    if ((size_t)n >= outsz) {
        return outsz - 1;
    }
    return (size_t)n;
}

bool config_save_flash(void)
{
    char body[CFG_SECTOR_SIZE - sizeof(cfg_hdr_t)];
    size_t len = build_ini(body, sizeof(body));
    if (len == 0) {
        return false;
    }
    uint8_t sector[CFG_SECTOR_SIZE];
    memset(sector, 0xFF, sizeof(sector));
    cfg_hdr_t *hdr = (cfg_hdr_t *)sector;
    hdr->magic[0] = CFG_MAGIC0;
    hdr->magic[1] = CFG_MAGIC1;
    hdr->magic[2] = CFG_MAGIC2;
    hdr->magic[3] = CFG_MAGIC3;
    hdr->version = CFG_VERSION;
    hdr->reserved = 0;
    hdr->length = (uint16_t)len;
    hdr->crc = crc16_ccitt((const uint8_t *)body, len);
    memcpy(sector + sizeof(cfg_hdr_t), body, len);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(CFG_FLASH_OFFSET, CFG_SECTOR_SIZE);
    flash_range_program(CFG_FLASH_OFFSET, sector, CFG_SECTOR_SIZE);
    restore_interrupts(ints);
    return true;
}

bool config_erase_flash(void)
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(CFG_FLASH_OFFSET, CFG_SECTOR_SIZE);
    restore_interrupts(ints);
    return true;
}

void config_dump_ini(void)
{
    char body[CFG_SECTOR_SIZE - sizeof(cfg_hdr_t)];
    size_t len = build_ini(body, sizeof(body));
    if (len == 0) {
        printf("(empty)\r\n");
        return;
    }
    /* print line by line for CDC */
    for (size_t i = 0; i < len; i++) {
        putchar(body[i]);
        if (body[i] == '\n') {
            putchar('\r');
        }
    }
    if (len == 0 || body[len - 1] != '\n') {
        printf("\r\n");
    }
}

void config_init(void)
{
    config_load_defaults();
    bool flash_ok = config_load_flash();
    /* Extra chatter only when quiet=0 (fixed boot banner is separate / always on). */
    if (config_unsolicited_ok()) {
        if (flash_ok) {
            printf("CONFIG: flash INI loaded\r\n");
        } else {
            printf("CONFIG: defaults (no flash overlay)\r\n");
        }
    }
}

const s1224_config_t *config_get(void)
{
    return &g_cfg;
}

s1224_config_t *config_mutable(void)
{
    return &g_cfg;
}

bool config_unsolicited_ok(void)
{
    return !g_cfg.quiet;
}
