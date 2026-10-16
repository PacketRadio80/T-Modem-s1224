/*
 * KISS encode/decode.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "kiss.h"

void kiss_rx_init(kiss_rx_t *kx)
{
    kx->len = 0;
    kx->in_frame = false;
    kx->esc = false;
}

kiss_rx_kind_t kiss_rx_byte_ex(kiss_rx_t *kx, uint8_t b, uint8_t *out,
                               size_t *out_len, uint8_t *param_cmd,
                               uint8_t *param_val)
{
    if (b == KISS_FEND) {
        if (kx->in_frame && kx->len >= 1) {
            uint8_t cmd = kx->buf[0];
            uint8_t type = cmd & 0x0Fu;
            if (type == KISS_CMD_DATA && kx->len > 1) {
                size_t n = kx->len - 1;
                for (size_t i = 0; i < n; i++) {
                    out[i] = kx->buf[i + 1];
                }
                *out_len = n;
                kx->len = 0;
                kx->esc = false;
                kx->in_frame = false;
                return KISS_RX_DATA;
            }
            if (type >= KISS_CMD_TXDELAY && type <= KISS_CMD_FULLDUP &&
                kx->len == 2 && param_cmd != NULL && param_val != NULL) {
                *param_cmd = type;
                *param_val = kx->buf[1];
                kx->len = 0;
                kx->esc = false;
                kx->in_frame = false;
                return KISS_RX_PARAM;
            }
        }
        kx->len = 0;
        kx->esc = false;
        kx->in_frame = true;
        return KISS_RX_NONE;
    }
    if (!kx->in_frame) {
        return KISS_RX_NONE;
    }
    if (kx->esc) {
        if (b == KISS_TFEND) {
            b = KISS_FEND;
        } else if (b == KISS_TFESC) {
            b = KISS_FESC;
        }
        kx->esc = false;
    } else if (b == KISS_FESC) {
        kx->esc = true;
        return KISS_RX_NONE;
    }
    if (kx->len < sizeof(kx->buf)) {
        kx->buf[kx->len++] = b;
    } else {
        /* Buffer overflow: hard reset to a state equivalent to idle.
         * Without this, a stale kx->esc survives and the next byte
         * gets decoded against a half-open escape. */
        kx->in_frame = false;
        kx->esc = false;
        kx->len = 0;
    }
    return KISS_RX_NONE;
}

bool kiss_rx_byte(kiss_rx_t *kx, uint8_t b, uint8_t *out, size_t *out_len)
{
    return kiss_rx_byte_ex(kx, b, out, out_len, NULL, NULL) == KISS_RX_DATA;
}

static bool push(uint8_t *dst, size_t cap, size_t *n, uint8_t b)
{
    if (*n >= cap) {
        return false;
    }
    dst[(*n)++] = b;
    return true;
}

static bool push_esc(uint8_t *dst, size_t cap, size_t *n, uint8_t b)
{
    if (b == KISS_FEND) {
        return push(dst, cap, n, KISS_FESC) && push(dst, cap, n, KISS_TFEND);
    }
    if (b == KISS_FESC) {
        return push(dst, cap, n, KISS_FESC) && push(dst, cap, n, KISS_TFESC);
    }
    return push(dst, cap, n, b);
}

size_t kiss_encode_data(const uint8_t *payload, size_t len,
                        uint8_t *dst, size_t dst_cap)
{
    size_t n = 0;
    if (!push(dst, dst_cap, &n, KISS_FEND)) {
        return 0;
    }
    if (!push_esc(dst, dst_cap, &n, KISS_CMD_DATA)) {
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (!push_esc(dst, dst_cap, &n, payload[i])) {
            return 0;
        }
    }
    if (!push(dst, dst_cap, &n, KISS_FEND)) {
        return 0;
    }
    return n;
}
