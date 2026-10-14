#ifndef S1224_MODEM_PROBE_H
#define S1224_MODEM_PROBE_H

#include <stdbool.h>

typedef struct {
    bool softmodem_ok;
    bool af_tx_pwm;
    bool ready;
} modem_probe_result_t;

modem_probe_result_t modem_probe_run(void);

#endif
