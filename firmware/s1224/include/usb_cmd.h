#ifndef S1224_USB_CMD_H
#define S1224_USB_CMD_H

#include <stddef.h>
#include <stdint.h>

void usb_cmd_init(void);
void usb_cmd_poll(void);
void usb_cmd_on_rx_frame(const uint8_t *info, size_t len);

#endif
