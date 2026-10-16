/**
 * Softmodem AFSK — PWM/DDS TX + Goertzel RX (ADC when wired).
 * Profile tones selected at compile time via S1224_BITRATE=300|1200|2400.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_AFSK_SOFT_H
#define S1224_AFSK_SOFT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef S1224_BITRATE
#define S1224_BITRATE 1200
#endif

void afsk_soft_init(void);

/** Transmit HDLC info payload (AX.25 frame without flags/FCS). Auto-PTT. */
bool afsk_tx_frame(const uint8_t *info, size_t info_len);

/** Emit a pure single-frequency tone for `duration_ms` (clamped to 60000).
 *  No HDLC, no NRZI, no TXDELAY/TXTAIL. Auto-PTT. Returns false if a
 *  packet TX is already running. Use to A/B test the analog path
 *  (e.g. distinguish a cable hum from a modem artefact). */
bool afsk_tx_tone(uint32_t freq_hz, uint32_t duration_ms);

typedef void (*afsk_rx_frame_fn)(const uint8_t *info, size_t len);
void afsk_rx_set_callback(afsk_rx_frame_fn fn);
void afsk_rx_poll(void);

bool afsk_tx_busy(void);
unsigned afsk_bitrate(void);

/** Carrier/energy sense for UI (softmodem — not chip CDT). */
bool afsk_rx_carrier_active(void);

/** Last per-bit Goertzel powers (mark/space) for tuning/debug. */
void afsk_rx_debug_mags(float *mark_p, float *space_p);

#endif /* S1224_AFSK_SOFT_H */
