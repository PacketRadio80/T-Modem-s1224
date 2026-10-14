/*
 * Board GPIO setup — T-Modem-s1224 softmodem Pico.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "board_pins.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

void board_pins_init(void)
{
    gpio_init(PIN_AF_TX);
    gpio_set_dir(PIN_AF_TX, GPIO_OUT);
    gpio_put(PIN_AF_TX, 0);

#if PIN_AF_RX_ADC >= 0
    gpio_init(PIN_AF_RX_ADC);
    gpio_set_dir(PIN_AF_RX_ADC, GPIO_IN);
#endif

    gpio_init(PIN_MODEM_RXD);
    gpio_set_dir(PIN_MODEM_RXD, GPIO_IN);
    gpio_pull_up(PIN_MODEM_RXD);

    gpio_init(PIN_MODEM_CDT);
    gpio_set_dir(PIN_MODEM_CDT, GPIO_IN);
    gpio_pull_up(PIN_MODEM_CDT);

    gpio_init(PIN_PTT_DRV);
    gpio_set_dir(PIN_PTT_DRV, GPIO_OUT);
    gpio_put(PIN_PTT_DRV, 0);

    gpio_init(PIN_LED_PTT);
    gpio_set_dir(PIN_LED_PTT, GPIO_OUT);
    gpio_put(PIN_LED_PTT, 0);
    gpio_init(PIN_LED_DCD);
    gpio_set_dir(PIN_LED_DCD, GPIO_OUT);
    gpio_put(PIN_LED_DCD, 0);
    gpio_init(PIN_LED_TXRX);
    gpio_set_dir(PIN_LED_TXRX, GPIO_OUT);
    gpio_put(PIN_LED_TXRX, 0);

    i2c_init(i2c0, I2C_LCD_BAUDRATE);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}
