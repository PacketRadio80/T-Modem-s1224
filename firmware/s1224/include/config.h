/**
 * Operator INI texts + sense tuning — defaults in FW, overlay from flash.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_CONFIG_H
#define S1224_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define S1224_TEXT_MAX 48

typedef struct {
    char starting[S1224_TEXT_MAX];
    char ready[S1224_TEXT_MAX];
    char fault[S1224_TEXT_MAX];
    char sending[S1224_TEXT_MAX];
    char signal[S1224_TEXT_MAX];
    char packet[S1224_TEXT_MAX];
    char power[S1224_TEXT_MAX];
    char idle[S1224_TEXT_MAX];
    /* Terminal-facing identity (CDC ID) */
    char callsign[S1224_TEXT_MAX];
    char device_id[S1224_TEXT_MAX];
    /* Extra CDC ASCII — quiet=1 default; never suppresses fixed boot banner */
    bool quiet;                /* true = no unsolicited STATUS/PROBE/CONFIG ASCII */
    /* Sense tuning */
    bool cdt_active_high;      /* false = active-low (default pull-up idle) */
    uint16_t packet_hold_ms;
    uint16_t packet_edge_min;
    uint16_t packet_window_ms;
} s1224_config_t;

void config_init(void);
const s1224_config_t *config_get(void);
s1224_config_t *config_mutable(void);
/** True when extra unsolicited CDC ASCII (STATUS/PROBE/CONFIG) is allowed. */
bool config_unsolicited_ok(void);

/** Apply defaults (RAM only). */
void config_load_defaults(void);
/** Load overlay from flash INI region if valid. */
bool config_load_flash(void);
/** Persist current config as INI text to flash. */
bool config_save_flash(void);
/** Erase flash overlay (next boot = defaults only until SAVE). */
bool config_erase_flash(void);

/** SET KEY=value (case-insensitive key). Returns false if unknown key. */
bool config_set_kv(const char *key, const char *value);
/** GET KEY → copy value into buf. */
bool config_get_kv(const char *key, char *buf, size_t buflen);

/** Dump INI-like text to USB (printf). */
void config_dump_ini(void);

#endif
