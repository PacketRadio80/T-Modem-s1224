/*
 * HDLC encode/decode — AX.25 bit order (LSB first), FCS-CCITT.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "hdlc.h"

uint16_t hdlc_fcs_update(uint16_t fcs, uint8_t byte)
{
    fcs ^= (uint16_t)byte;
    for (int i = 0; i < 8; i++) {
        if (fcs & 1u) {
            fcs = (uint16_t)((fcs >> 1) ^ 0x8408u);
        } else {
            fcs >>= 1;
        }
    }
    return fcs;
}

uint16_t hdlc_fcs_final(uint16_t fcs)
{
    return (uint16_t)~fcs;
}

static bool push_bit(uint8_t *bits, size_t bits_cap, size_t *n, bool v)
{
    if (*n >= bits_cap) {
        return false;
    }
    bits[*n] = v ? 1u : 0u;
    (*n)++;
    return true;
}

static bool stuff_bit(uint8_t *bits, size_t bits_cap, size_t *n,
                      unsigned *ones, bool bit)
{
    if (!push_bit(bits, bits_cap, n, bit)) {
        return false;
    }
    if (bit) {
        (*ones)++;
        if (*ones == 5u) {
            if (!push_bit(bits, bits_cap, n, false)) {
                return false;
            }
            *ones = 0;
        }
    } else {
        *ones = 0;
    }
    return true;
}

static bool emit_raw_byte(uint8_t *bits, size_t bits_cap, size_t *n, uint8_t b)
{
    for (int i = 0; i < 8; i++) {
        if (!push_bit(bits, bits_cap, n, (b >> i) & 1u)) {
            return false;
        }
    }
    return true;
}

static bool emit_stuffed_byte(uint8_t *bits, size_t bits_cap, size_t *n,
                              unsigned *ones, uint8_t b)
{
    for (int i = 0; i < 8; i++) {
        if (!stuff_bit(bits, bits_cap, n, ones, (b >> i) & 1u)) {
            return false;
        }
    }
    return true;
}

size_t hdlc_encode_bits(const uint8_t *info, size_t info_len,
                        uint8_t *bits, size_t bits_cap,
                        unsigned trailing_flags)
{
    if (!info || !bits || info_len > HDLC_MAX_INFO) {
        return 0;
    }
    size_t n = 0;
    unsigned ones = 0;

    if (!emit_raw_byte(bits, bits_cap, &n, HDLC_FLAG)) {
        return 0;
    }

    uint16_t fcs = HDLC_FCS_INIT;
    for (size_t i = 0; i < info_len; i++) {
        fcs = hdlc_fcs_update(fcs, info[i]);
        if (!emit_stuffed_byte(bits, bits_cap, &n, &ones, info[i])) {
            return 0;
        }
    }
    fcs = hdlc_fcs_final(fcs);
    if (!emit_stuffed_byte(bits, bits_cap, &n, &ones, (uint8_t)(fcs & 0xFFu))) {
        return 0;
    }
    if (!emit_stuffed_byte(bits, bits_cap, &n, &ones, (uint8_t)((fcs >> 8) & 0xFFu))) {
        return 0;
    }

    ones = 0;
    unsigned flags = trailing_flags ? trailing_flags : 1u;
    for (unsigned f = 0; f < flags; f++) {
        if (!emit_raw_byte(bits, bits_cap, &n, HDLC_FLAG)) {
            return 0;
        }
    }
    return n;
}

void hdlc_rx_init(hdlc_rx_t *rx)
{
    rx->len = 0;
    rx->ones = 0;
    rx->in_frame = false;
    rx->stuffing = false;
    rx->byte_acc = 0;
    rx->bit_count = 0;
}

bool hdlc_rx_bit(hdlc_rx_t *rx, bool bit, uint8_t *out, size_t *out_len)
{
    if (bit) {
        rx->ones++;
    } else {
        if (rx->ones == 5u) {
            /* stuffed 0 — discard */
            rx->ones = 0;
            return false;
        }
        if (rx->ones == 6u) {
            /* flag */
            if (rx->in_frame && rx->len >= 3) {
                uint16_t fcs = HDLC_FCS_INIT;
                for (size_t i = 0; i < rx->len; i++) {
                    fcs = hdlc_fcs_update(fcs, rx->buf[i]);
                }
                if (fcs == HDLC_FCS_GOOD) {
                    size_t info_len = rx->len - 2;
                    for (size_t i = 0; i < info_len; i++) {
                        out[i] = rx->buf[i];
                    }
                    *out_len = info_len;
                    rx->len = 0;
                    rx->bit_count = 0;
                    rx->byte_acc = 0;
                    rx->ones = 0;
                    rx->in_frame = true; /* abutting frames */
                    return true;
                }
            }
            rx->len = 0;
            rx->bit_count = 0;
            rx->byte_acc = 0;
            rx->ones = 0;
            rx->in_frame = true;
            return false;
        }
        if (rx->ones > 6u) {
            /* abort */
            rx->in_frame = false;
            rx->len = 0;
            rx->bit_count = 0;
            rx->byte_acc = 0;
            rx->ones = 0;
            return false;
        }
        rx->ones = 0;
    }

    if (!rx->in_frame) {
        return false;
    }

    rx->byte_acc |= (uint8_t)((bit ? 1u : 0u) << rx->bit_count);
    rx->bit_count++;
    if (rx->bit_count >= 8) {
        if (rx->len < sizeof(rx->buf)) {
            rx->buf[rx->len++] = rx->byte_acc;
        } else {
            rx->in_frame = false;
            rx->len = 0;
        }
        rx->byte_acc = 0;
        rx->bit_count = 0;
    }
    return false;
}
