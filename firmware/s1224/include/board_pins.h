/**
 * T-Modem-s1224 pin map — softmodem service Pico (same PCB base as c1224).
 * No TCM3105: PWM AF out bypasses chip socket (prototype jumper to AF path).
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_BOARD_PINS_H
#define S1224_BOARD_PINS_H

#include <stdint.h>
#include <stdbool.h>

#define S1224_HAS_CHIP_MODEM  0

/* I2C LCD (shared-kit class; level-shifted on board) */
#define PIN_I2C_SDA           0
#define PIN_I2C_SCL           1
#define I2C_LCD_BAUDRATE      100000
#define I2C_LCD_ADDR_DEFAULT  0x27

/* GP6 · PCB net AF_TX · PWM AFSK out */
#define PIN_AF_TX             6

/* GP26 · PCB net AF_RX when routed · ADC channel 0 */
#define PIN_AF_RX_ADC         26

/* GP7/GP8 · NC on s1224 PCB · optional FW sense if wired */
#define PIN_GP7_SENSE         7
#define PIN_MODEM_RXD         PIN_GP7_SENSE  /* alias · was chip RXD name */
#define PIN_MODEM_CDT         8  /* unused · NC on board */

#define PIN_PTT_DRV           9  /* GP9 · PTT keying */

#define PIN_LED_PTT           10
#define PIN_LED_DCD           11
#define PIN_LED_TXRX          12

void board_pins_init(void);

#endif /* S1224_BOARD_PINS_H */
