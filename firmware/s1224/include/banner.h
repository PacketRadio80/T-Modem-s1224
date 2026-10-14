/**
 * Fixed CDC boot banner — source text only (not INI / CDC keys).
 * Always printed once after USB ready; quiet does not suppress it.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef S1224_BANNER_H
#define S1224_BANNER_H

/** Print fixed boot banner (call once after CDC ready). */
void banner_print(void);

#endif /* S1224_BANNER_H */
