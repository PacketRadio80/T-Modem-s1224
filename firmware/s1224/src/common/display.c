/*
 * HD44780 LCD via PCF8574 I2C backpack + USB mirror.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "display.h"
#include "board_pins.h"
#include "config.h"
#include "status.h"
#include "sense.h"
#include "ptt.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* PCF8574 bit map (common backpack): P0=RS P1=RW P2=E P3=BL P4-7=D4-D7 */
#define LCD_RS  0x01
#define LCD_RW  0x02
#define LCD_EN  0x04
#define LCD_BL  0x08

#define I2C_WRITE_TIMEOUT_US  5000u
#define I2C_FAIL_LIMIT        3u

static bool g_lcd;
static uint8_t g_addr = I2C_LCD_ADDR_DEFAULT;
static char g_last_l0[17];
static char g_last_l1[17];
static unsigned g_i2c_fail;

static void i2c_bus_recover(void)
{
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_SIO);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_I2C_SDA, GPIO_IN);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_put(PIN_I2C_SCL, 0);
    gpio_set_dir(PIN_I2C_SCL, GPIO_OUT);
    for (unsigned i = 0; i < 9 && !gpio_get(PIN_I2C_SDA); i++) {
        gpio_set_dir(PIN_I2C_SCL, GPIO_IN);
        sleep_us(5);
        gpio_set_dir(PIN_I2C_SCL, GPIO_OUT);
        sleep_us(5);
    }
    gpio_set_dir(PIN_I2C_SDA, GPIO_OUT);
    gpio_put(PIN_I2C_SDA, 0);
    sleep_us(5);
    gpio_set_dir(PIN_I2C_SDA, GPIO_IN);
    sleep_us(5);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}

static bool i2c_write_u8(uint8_t addr, uint8_t v)
{
    if (i2c_write_timeout_us(i2c0, addr, &v, 1, false, I2C_WRITE_TIMEOUT_US) == 1) {
        g_i2c_fail = 0;
        return true;
    }
    g_i2c_fail++;
    i2c_bus_recover();
    return false;
}

static bool i2c_probe(uint8_t addr)
{
    uint8_t b = 0;
    return i2c_write_timeout_us(i2c0, addr, &b, 1, false, I2C_WRITE_TIMEOUT_US) == 1;
}

static bool lcd_write4(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (uint8_t)((nibble & 0xF0) | LCD_BL | mode);
    bool ok = i2c_write_u8(g_addr, (uint8_t)(data | LCD_EN));
    sleep_us(1);
    ok = i2c_write_u8(g_addr, (uint8_t)(data & (uint8_t)~LCD_EN)) && ok;
    sleep_us(50);
    return ok;
}

static bool lcd_send(uint8_t value, uint8_t mode)
{
    bool ok = lcd_write4((uint8_t)(value & 0xF0), mode);
    return lcd_write4((uint8_t)((value << 4) & 0xF0), mode) && ok;
}

static bool lcd_cmd(uint8_t c)
{
    return lcd_send(c, 0);
}

static bool lcd_data(uint8_t c)
{
    return lcd_send(c, LCD_RS);
}

static bool lcd_set_cursor(uint8_t col, uint8_t row)
{
    static const uint8_t row_addr[] = {0x00, 0x40, 0x14, 0x54};
    if (row > 1) {
        row = 1;
    }
    return lcd_cmd((uint8_t)(0x80 | (row_addr[row] + col)));
}

static bool lcd_print_line(uint8_t row, const char *s)
{
    char buf[17];
    size_t n = strlen(s);
    if (n > 16) {
        n = 16;
    }
    memset(buf, ' ', 16);
    memcpy(buf, s, n);
    buf[16] = '\0';
    bool ok = lcd_set_cursor(0, row);
    for (int i = 0; i < 16; i++) {
        ok = lcd_data((uint8_t)buf[i]) && ok;
    }
    return ok;
}

static bool lcd_init_seq(uint8_t addr)
{
    bool ok = true;
    g_addr = addr;
    sleep_ms(50);
    ok = lcd_write4(0x30, 0) && ok;
    sleep_ms(5);
    ok = lcd_write4(0x30, 0) && ok;
    sleep_us(150);
    ok = lcd_write4(0x30, 0) && ok;
    sleep_us(150);
    ok = lcd_write4(0x20, 0) && ok; /* 4-bit */
    ok = lcd_cmd(0x28) && ok;       /* 2 line, 5x8 */
    ok = lcd_cmd(0x0C) && ok;       /* display on, cursor off */
    ok = lcd_cmd(0x06) && ok;       /* entry mode */
    ok = lcd_cmd(0x01) && ok;       /* clear */
    sleep_ms(2);
    return ok;
}

void display_init(void)
{
    g_lcd = false;
    g_last_l0[0] = '\0';
    g_last_l1[0] = '\0';

    uint8_t addrs[] = {0x27, 0x3F};
    for (unsigned i = 0; i < sizeof(addrs); i++) {
        g_i2c_fail = 0;
        if (i2c_probe(addrs[i]) && lcd_init_seq(addrs[i]) && g_i2c_fail == 0) {
            g_lcd = true;
            if (config_unsolicited_ok()) {
                printf("LCD: HD44780 @ 0x%02X\r\n", addrs[i]);
            }
            return;
        }
    }
    if (config_unsolicited_ok()) {
        printf("LCD: none — USB status only\r\n");
    }
}

bool display_present(void)
{
    return g_lcd;
}

void display_show(const char *line)
{
    if (!line) {
        return;
    }
    if (g_lcd) {
        if (!lcd_print_line(0, line) && g_i2c_fail >= I2C_FAIL_LIMIT) {
            g_lcd = false;
        }
    }
}

void display_update(void)
{
    const s1224_config_t *cfg = config_get();
    char l0[17];
    char l1[17];

    s1224_state_t st = status_get();
    if (st == S1224_STATE_FAULT) {
        snprintf(l0, sizeof(l0), "%.16s", cfg->fault);
    } else if (ptt_get()) {
        snprintf(l0, sizeof(l0), "%.16s", cfg->sending);
    } else if (st == S1224_STATE_STARTING) {
        snprintf(l0, sizeof(l0), "%.16s", cfg->starting);
    } else {
        /* Power/ready on line 0 */
        snprintf(l0, sizeof(l0), "%.16s", cfg->ready);
    }

    /* Line 1: Signal + Packet from sense */
    bool sig = sense_signal_active();
    bool pkt = sense_packet_active();
    if (sig && pkt) {
        snprintf(l1, sizeof(l1), "%.7s %.7s", cfg->signal, cfg->packet);
    } else if (sig) {
        snprintf(l1, sizeof(l1), "%.16s", cfg->signal);
    } else if (pkt) {
        snprintf(l1, sizeof(l1), "%.16s", cfg->packet);
    } else {
        snprintf(l1, sizeof(l1), "%.16s", cfg->power);
    }

    if (strcmp(l0, g_last_l0) != 0 || strcmp(l1, g_last_l1) != 0) {
        strncpy(g_last_l0, l0, sizeof(g_last_l0));
        strncpy(g_last_l1, l1, sizeof(g_last_l1));
        g_last_l0[16] = '\0';
        g_last_l1[16] = '\0';
        if (g_lcd) {
            bool ok = lcd_print_line(0, l0);
            ok = lcd_print_line(1, l1) && ok;
            if (!ok && g_i2c_fail >= I2C_FAIL_LIMIT) {
                g_lcd = false;
            }
        }
    }
}
