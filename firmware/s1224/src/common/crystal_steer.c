/*
 * Softmodem product — no crystal mux / no TCM3105 XTAL steering.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "crystal_steer.h"

static bool g_applied;

void crystal_steer_apply(void)
{
    g_applied = true;
}

bool crystal_steer_applied(void)
{
    return g_applied;
}
