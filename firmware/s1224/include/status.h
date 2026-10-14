#ifndef S1224_STATUS_H
#define S1224_STATUS_H

#include <stdbool.h>

typedef enum {
    S1224_STATE_STARTING = 0,
    S1224_STATE_READY,
    S1224_STATE_FAULT,
    S1224_STATE_SENDING,
    S1224_STATE_SIGNAL,   /* CDT active (Signal/RX) */
    S1224_STATE_PACKET    /* RXD packet sense (may coexist with signal) */
} s1224_state_t;

/** Operator-facing text for state (from INI). */
const char *s1224_state_string(s1224_state_t st);
void status_set(s1224_state_t st);
s1224_state_t status_get(void);
/** Recompute composite state from PTT + sense; publish if changed. */
void status_tick(void);
void status_publish(void);

#endif
