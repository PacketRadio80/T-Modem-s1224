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
#define KISS_CMD_TXDELAY  0x01u
#define KISS_CMD_PERSIST  0x02u
#define KISS_CMD_SLOTTIME 0x03u
#define KISS_CMD_TXTAIL   0x04u
#define KISS_CMD_FULLDUP  0x05u
#define KISS_CMD_SETHW    0x06u
#define KISS_MAX_PAYLOAD 512

typedef struct {
    uint8_t buf[KISS_MAX_PAYLOAD + 4];
    size_t  len;
    bool    in_frame;
    bool    esc;
} kiss_rx_t;

typedef enum {
    KISS_RX_NONE = 0,
    KISS_RX_DATA,
    KISS_RX_PARAM,
} kiss_rx_kind_t;

void kiss_rx_init(kiss_rx_t *kx);
/** Feed one byte. Returns true when a DATA payload is ready in out. */
bool kiss_rx_byte(kiss_rx_t *kx, uint8_t b, uint8_t *out, size_t *out_len);
/** Feed one byte with full frame classification: DATA payload in out or
 *  PARAMETER frame (cmd 0x01..0x05 with one value byte). */
kiss_rx_kind_t kiss_rx_byte_ex(kiss_rx_t *kx, uint8_t b, uint8_t *out,
                               size_t *out_len, uint8_t *param_cmd,
                               uint8_t *param_val);

/** Write KISS DATA frame to dst; returns bytes written (0 on overflow). */
size_t kiss_encode_data(const uint8_t *payload, size_t len,
                        uint8_t *dst, size_t dst_cap);

#endif /* S1224_KISS_H */
