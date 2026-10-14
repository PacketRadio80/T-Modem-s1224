/*
 * Continuous-phase AFSK TX via PWM frequency (mark/space) + NRZI/HDLC.
 * RX: per-bit Goertzel mark/space on ADC (GP26) when wired.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "afsk_soft.h"
#include "afsk_profile.h"
#include "hdlc.h"
#include "board_pins.h"
#include "ptt.h"
#include "status.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include <string.h>
#include <math.h>

#define BIT_US           (1000000u / (unsigned)S1224_BITRATE)
#define TX_BITS_MAX      ((HDLC_MAX_INFO + 4) * 10 + 64)
#define PWM_CLKDIV       4.0f
#define RX_SAMPLES_BIT   8
#define RX_FS_HZ         ((unsigned)S1224_BITRATE * RX_SAMPLES_BIT)
#define CARRIER_THRESH   50.0f

static uint8_t g_tx_bits[TX_BITS_MAX];
static volatile bool g_tx_busy;
static bool g_nrzi_mark;
static hdlc_rx_t g_rx;
static afsk_rx_frame_fn g_rx_cb;
static uint8_t g_rx_out[HDLC_MAX_INFO];
static uint g_pwm_slice;
static uint g_pwm_chan;
static afsk_profile_t g_prof;
static bool g_carrier;

#if PIN_AF_RX_ADC >= 0
#include "hardware/adc.h"
#define RX_GOERTZEL_ENABLE 1
static float g_rx_samp[RX_SAMPLES_BIT];
static unsigned g_rx_n;
static absolute_time_t g_next_adc;
static bool g_rx_nrzi_mark;
static float g_coeff_mark;
static float g_coeff_space;
#else
#define RX_GOERTZEL_ENABLE 0
#endif

static void pwm_tone_hz(uint32_t freq_hz)
{
    if (freq_hz < 100) {
        pwm_set_enabled(g_pwm_slice, false);
        return;
    }
    uint32_t sys = clock_get_hz(clk_sys);
    uint32_t wrap = (uint32_t)((float)sys / (freq_hz * PWM_CLKDIV)) - 1u;
    if (wrap < 10) {
        wrap = 10;
    }
    pwm_set_clkdiv(g_pwm_slice, PWM_CLKDIV);
    pwm_set_wrap(g_pwm_slice, wrap);
    pwm_set_chan_level(g_pwm_slice, g_pwm_chan, wrap / 2u);
    pwm_set_enabled(g_pwm_slice, true);
}

static void nrzi_play_bit(bool data_bit)
{
    if (!data_bit) {
        g_nrzi_mark = !g_nrzi_mark;
    }
    pwm_tone_hz(g_nrzi_mark ? g_prof.mark_hz : g_prof.space_hz);
    busy_wait_us_32(BIT_US);
}

#if RX_GOERTZEL_ENABLE
static float goertzel_mag(const float *x, unsigned n, float coeff)
{
    float s0 = 0.0f, s1 = 0.0f, s2 = 0.0f;
    for (unsigned i = 0; i < n; i++) {
        s0 = x[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    float power = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    return power < 0.0f ? 0.0f : power;
}

static float goertzel_coeff(uint32_t tone_hz)
{
    float w = 2.0f * (float)M_PI * ((float)tone_hz / (float)RX_FS_HZ);
    return 2.0f * cosf(w);
}
#endif

void afsk_soft_init(void)
{
    g_prof = afsk_profile_get();
    gpio_set_function(PIN_AF_TX, GPIO_FUNC_PWM);
    g_pwm_slice = pwm_gpio_to_slice_num(PIN_AF_TX);
    g_pwm_chan = pwm_gpio_to_channel(PIN_AF_TX);
    pwm_tone_hz(0);
    g_tx_busy = false;
    g_nrzi_mark = true;
    hdlc_rx_init(&g_rx);
    g_rx_cb = NULL;
    g_carrier = false;

#if RX_GOERTZEL_ENABLE
    adc_init();
    adc_gpio_init(PIN_AF_RX_ADC);
    adc_select_input((uint)(PIN_AF_RX_ADC - 26));
    g_rx_n = 0;
    g_rx_nrzi_mark = true;
    g_coeff_mark = goertzel_coeff(g_prof.mark_hz);
    g_coeff_space = goertzel_coeff(g_prof.space_hz);
    g_next_adc = get_absolute_time();
#endif
}

unsigned afsk_bitrate(void)
{
    return (unsigned)S1224_BITRATE;
}

bool afsk_tx_busy(void)
{
    return g_tx_busy;
}

bool afsk_rx_carrier_active(void)
{
    return g_carrier;
}

void afsk_rx_set_callback(afsk_rx_frame_fn fn)
{
    g_rx_cb = fn;
}

bool afsk_tx_frame(const uint8_t *info, size_t info_len)
{
    if (g_tx_busy || !info || info_len == 0 || info_len > HDLC_MAX_INFO) {
        return false;
    }
    size_t nbits = hdlc_encode_bits(info, info_len, g_tx_bits, TX_BITS_MAX, 2);
    if (nbits == 0) {
        return false;
    }

    g_tx_busy = true;
    status_set(S1224_STATE_SENDING);
    status_publish();
    ptt_set(true);
    sleep_ms(40);

    g_nrzi_mark = true;
    pwm_tone_hz(g_prof.mark_hz);

    for (size_t i = 0; i < nbits; i++) {
        nrzi_play_bit(g_tx_bits[i] != 0);
    }

    pwm_tone_hz(0);
    sleep_ms(20);
    ptt_set(false);
    status_set(S1224_STATE_READY);
    status_publish();
    g_tx_busy = false;
    return true;
}

#if RX_GOERTZEL_ENABLE
static void rx_process_bit_window(void)
{
    float m = goertzel_mag(g_rx_samp, RX_SAMPLES_BIT, g_coeff_mark);
    float s = goertzel_mag(g_rx_samp, RX_SAMPLES_BIT, g_coeff_space);
    float peak = m > s ? m : s;
    g_carrier = peak > CARRIER_THRESH;

    bool tone_mark = (m >= s);
    bool data_bit = (tone_mark == g_rx_nrzi_mark); /* NRZI: 1 = no transition */
    if (tone_mark != g_rx_nrzi_mark) {
        g_rx_nrzi_mark = tone_mark;
    }

    size_t out_len = 0;
    if (hdlc_rx_bit(&g_rx, data_bit, g_rx_out, &out_len) && g_rx_cb && out_len > 0) {
        g_rx_cb(g_rx_out, out_len);
    }
}

static void rx_goertzel_sample(void)
{
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(g_next_adc, now) < 0) {
        return;
    }
    g_next_adc = delayed_by_us(now, 1000000u / RX_FS_HZ);

    uint16_t raw = adc_read();
    /* Center around mid-scale (12-bit ADC). */
    g_rx_samp[g_rx_n++] = ((float)raw - 2048.0f) / 2048.0f;

    if (g_rx_n >= RX_SAMPLES_BIT) {
        g_rx_n = 0;
        rx_process_bit_window();
    }
}
#else
static void rx_goertzel_sample(void)
{
    /* TX-only boards: leave PIN_AF_RX_ADC as -1. */
    g_carrier = false;
}
#endif

void afsk_rx_poll(void)
{
    if (g_tx_busy) {
        return;
    }
    rx_goertzel_sample();
}
