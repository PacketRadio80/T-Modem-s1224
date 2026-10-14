#ifndef S1224_CRYSTAL_STEER_H
#define S1224_CRYSTAL_STEER_H

#include <stdbool.h>

/** Softmodem — no-op (rate = compile-time S1224_BITRATE). */
void crystal_steer_apply(void);
bool crystal_steer_applied(void);

#endif
