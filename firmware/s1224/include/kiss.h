/**
 * KISS frame encode/decode (FEND 0xC0).
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_KISS_H
#define S1224_KISS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define KISS_FEND   0xC0u
#define KISS_FESC   0xDBu
#define KISS_TFEND  0xDCu
#define KISS_TFESC  0xDDu
#define KISS_CMD_DATA 0x00u
#define KISS_MAX_PAYLOAD 512

typedef struct {
    uint8_t buf[KISS_MAX_PAYLOAD + 4];
    size_t  len;
    bool    in_frame;
    bool    esc;
} kiss_rx_t;

void kiss_rx_init(kiss_rx_t *kx);
/** Feed one byte. Returns true when a DATA payload is ready in out. */
bool kiss_rx_byte(kiss_rx_t *kx, uint8_t b, uint8_t *out, size_t *out_len);

/** Write KISS DATA frame to dst; returns bytes written (0 on overflow). */
size_t kiss_encode_data(const uint8_t *payload, size_t len,
                        uint8_t *dst, size_t dst_cap);

#endif /* S1224_KISS_H */
