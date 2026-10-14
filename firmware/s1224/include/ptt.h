#ifndef S1224_PTT_H
#define S1224_PTT_H

#include <stdbool.h>

void ptt_init(void);
void ptt_set(bool on);
bool ptt_get(void);

#endif
