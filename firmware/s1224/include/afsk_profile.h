/**
 * Mark/space tone pairs per build (Hz) — see product README.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_AFSK_PROFILE_H
#define S1224_AFSK_PROFILE_H

#include <stdint.h>

#ifndef S1224_BITRATE
#define S1224_BITRATE 1200
#endif

typedef struct {
    uint32_t baud;
    uint32_t mark_hz;
    uint32_t space_hz;
    const char *profile_name;
} afsk_profile_t;

static inline afsk_profile_t afsk_profile_get(void)
{
#if S1224_BITRATE == 300
    return (afsk_profile_t){300, 1600, 1800, "HF-300 (1600/1800)"};
#elif S1224_BITRATE == 600
    return (afsk_profile_t){600, 1200, 1800, "HF-600 (1200/1800)"};
#elif S1224_BITRATE == 1200
    return (afsk_profile_t){1200, 1200, 2200, "Bell202-1200"};
#elif S1224_BITRATE == 2400
    return (afsk_profile_t){2400, 1775, 3250, "Y2-class-2400"};
#elif S1224_BITRATE == 3600
    return (afsk_profile_t){3600, 1800, 2700, "HF-3600 (1800/2700)"};
#elif S1224_BITRATE == 4800
    return (afsk_profile_t){4800, 2100, 3300, "HF-4800 (2100/3300)"};
#else
#error "S1224_BITRATE must be 300, 600, 1200, 2400, 3600, or 4800"
#endif
}

#endif /* S1224_AFSK_PROFILE_H */
