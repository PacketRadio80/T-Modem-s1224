#ifndef S1224_DISPLAY_H
#define S1224_DISPLAY_H

#include <stdbool.h>

/**
 * I2C HD44780 via PCF8574 backpack (0x27/0x3F). USB always logs.
 * Shows Power/ready, Signal, Packet from INI texts.
 */
void display_init(void);
bool display_present(void);
/** Refresh LCD from current status + sense (uses INI texts). */
void display_update(void);
/** Legacy single-line push (also used before full UI ready). */
void display_show(const char *line);

#endif
