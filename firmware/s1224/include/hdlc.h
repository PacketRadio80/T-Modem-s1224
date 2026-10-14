/**
 * HDLC frame helpers — flags, bit-stuff, FCS-CCITT (AX.25).
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_HDLC_H
#define S1224_HDLC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HDLC_FLAG       0x7Eu
#define HDLC_FCS_INIT   0xFFFFu
#define HDLC_FCS_GOOD   0xF0B8u
#define HDLC_MAX_INFO   512

uint16_t hdlc_fcs_update(uint16_t fcs, uint8_t byte);
uint16_t hdlc_fcs_final(uint16_t fcs);

/**
 * Build on-wire bit stream (NRZI already applied by caller after this):
 * FLAG + stuffed(info + FCS_lo + FCS_hi) + FLAG(+optional trailing flags).
 * Returns bit count written to bits[] (MSB-first within each stuffed byte stream
 * is not used — output is a raw bit array, LSB-first per AX.25 bit order).
 *
 * bits_cap is capacity in bits. Returns 0 on overflow / bad args.
 */
size_t hdlc_encode_bits(const uint8_t *info, size_t info_len,
                        uint8_t *bits, size_t bits_cap,
                        unsigned trailing_flags);

/**
 * Feed one NRZ bit (after NRZI decode). Returns true when a complete good
 * frame is available in out[] (info only, FCS stripped).
 */
typedef struct {
    uint8_t  buf[HDLC_MAX_INFO + 2];
    size_t   len;
    uint8_t  ones;
    bool     in_frame;
    bool     stuffing;
    uint8_t  byte_acc;
    uint8_t  bit_count;
} hdlc_rx_t;

void hdlc_rx_init(hdlc_rx_t *rx);
bool hdlc_rx_bit(hdlc_rx_t *rx, bool bit, uint8_t *out, size_t *out_len);

#endif /* S1224_HDLC_H */
