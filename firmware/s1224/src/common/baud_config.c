#include "baud_config.h"
#include "afsk_soft.h"

static const baud_profile_config_t cfg = {
#if S1224_BITRATE == 300
    .label = "s1224-300",
#elif S1224_BITRATE == 1200
    .label = "s1224-1200",
#elif S1224_BITRATE == 2400
    .label = "s1224-2400",
#else
    .label = "s1224",
#endif
};

const baud_profile_config_t *baud_config_get(void)
{
    return &cfg;
}
