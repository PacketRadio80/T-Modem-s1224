/**
 * Dual sense: CDT→Signal/RX, RXD→Packet (HDLC 0x7E / edge activity).
 * Status only — not a Host↔Chip data bridge.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_SENSE_H
#define S1224_SENSE_H

#include <stdbool.h>

void sense_init(void);
/** Poll GPIO / heuristics; call from main loop (~10 ms). */
void sense_poll(void);

bool sense_signal_active(void);  /* CDT carrier/tone */
bool sense_packet_active(void);  /* RXD activity or flag seen (held) */
bool sense_packet_flag(void);    /* last window saw 0x7E */

#endif
