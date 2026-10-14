#ifndef S1224_BAUD_CONFIG_H
#define S1224_BAUD_CONFIG_H

typedef struct {
    const char *label;
} baud_profile_config_t;

const baud_profile_config_t *baud_config_get(void);

#endif
