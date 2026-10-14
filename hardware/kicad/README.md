# KiCad — T-Modem-s1224

Tracked project: **`t-modem-s1224/`** — started from **c1224** copper; softmodem line.

## State (2026-08-03)

| Metric | Value |
|--------|------:|
| Segments (tracks) | **0** (stripped for clean re-route) |
| Vias | **0** |
| Footprints on PCB | **31** |
| Schematic chip symbols | **DNP** (`U_MODEM`, `Y1`, `Y2`) |

**Backups:**

- `t-modem-s1224/t-modem-s1224.pre-strip-backup.kicad_pcb` (289 segments)
- `t-modem-s1224/t-modem-s1224.2026-08-03-pre-reroute-backup.kicad_pcb` (199 segments)
- MASTER vault: `hardware/open-designs/t-modem-s1224-pcb-backup/`

DRC: **unconnected** on all signal pads — expected until re-route.

Detail: [`t-modem-s1224/README.md`](t-modem-s1224/README.md).

## Keep for re-route

U1 Pico · power (J_DC / U_REG / D_OR1 / F1 / bulk+bypass) · J_AF · Q1 / R_PTT · J_LCD_I2C / U_LVL / RV2 · J_USB / J_USB_PICO · LEDs+R_LED* · SW1 / TP_BOOTSEL · H1–H4.

**Not stuffed / not on PCB:** RV1 · C_TX · C_RX · modem socket/chip.

## Softmodem AF (net names)

| GPIO | Net |
|------|-----|
| GP6 | **`AF_TX`** |
| GP26 | **`AF_RX`** |
| GP9 | **`PTT_DRV`** |
| GP7 / GP8 | **NC** |

U1.9 = **GP6** / **`AF_TX`** · U1.31 = **GP26** / **`AF_RX`** · U1.12 = **GP9** / **`PTT_DRV`**.
