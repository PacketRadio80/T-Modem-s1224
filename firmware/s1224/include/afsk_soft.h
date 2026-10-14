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

typedef void (*afsk_rx_frame_fn)(const uint8_t *info, size_t len);
void afsk_rx_set_callback(afsk_rx_frame_fn fn);
void afsk_rx_poll(void);

bool afsk_tx_busy(void);
unsigned afsk_bitrate(void);

/** Carrier/energy sense for UI (softmodem — not chip CDT). */
bool afsk_rx_carrier_active(void);

#endif /* S1224_AFSK_SOFT_H */
