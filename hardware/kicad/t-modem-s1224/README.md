# KiCad — T-Modem-s1224

Same PCB base as **T-Modem-c1224**, softmodem stuffing / metadata variant.

| File | Role |
|------|------|
| `t-modem-s1224.kicad_sch` | Schematic (chip symbols **DNP**) |
| `t-modem-s1224.kicad_pcb` | PCB **180×180 mm** · **0** tracks · **0** vias |
| `t-modem-s1224.kicad_pro` | Project · `PROJECT=T-Modem-s1224` · `VERSION=1.0-upcoming1` |
| `t-modem-s1224.kicad_sym` | Symbols |
| `t-modem-s1224.rules` | Custom DRC rules |
| `t-modem-s1224.pre-strip-backup.kicad_pcb` | Pre first strip (289 segments) |
| `t-modem-s1224.2026-08-03-pre-reroute-backup.kicad_pcb` | Pre second strip / re-route reset (199 segments) |

## Inventory — PCB footprints (**31**)

**Keep:** U1 Pico · J_DC / U_REG / D_OR1 / F1 / C_BULK / C_BYP* / C_IN · J_AF · Q1 / R_PTT · J_LCD_I2C / U_LVL / RV2 · J_USB / J_USB_PICO · LED_* / R_LED* · SW1 / TP_BOOTSEL · H1–H4.

**Removed (softmodem — not on PCB):**

| Ref | Role |
|-----|------|
| **U_MODEM** / **SKT_MODEM** | TCM3105 + socket (sch **DNP** only) |
| **Y1** / **Y2** / **C_Y*** | Pierce (sch **DNP**) |
| **RV1** / **C_TX** / **C_RX** | Chip-era AF trim / coupling — **DNP** |
| **RV_CDL** / **R_TXR*** / **R_TRS** | Chip straps |

## Schematic DNP (not stuffed)

| Ref | Value | Flags |
|-----|-------|-------|
| **U_MODEM** | TCM3105 | `dnp` · `in_bom no` · `on_board no` |
| **Y1** | 4.4336 MHz | same |
| **Y2** | 6.5536 MHz | same |
| **D_OR2** | DNI-no-VBUS-OR | same (USB VBUS OR unused) |

Tracks (`segment`): **0** — stripped 2026-08-03 for clean re-route.

## U1 Pico AF nets (s1224 — no chip-era `MODEM_*` names)

| Pad | GPIO | Net | Role |
|-----|------|-----|------|
| **9** | GP6 | **`AF_TX`** | PWM AFSK TX → radio AF inject |
| **10** | GP7 | *(NC)* | Unconnected on board |
| **11** | GP8 | *(NC)* | Unconnected on board |
| **31** | GP26 | **`AF_RX`** | ADC RX ← radio AF |
| **12** | GP9 | **`PTT_DRV`** | PTT keying |

## Connector / power nets (pad names)

| Ref | Nets |
|-----|------|
| **J_AF** | `AF_TX` · `AF_RX` · `PTT_OC` · `GND` |
| **J_DC** | `VIN_12` · `GND` ×2 |
| **F1** | `+5V_REG` → `+5V` |
| **D_OR1** | `+5V_OR` ← `+5V` |
| **Q1** / **R_PTT** | `PTT_DRV` → `PTT_B` → `PTT_OC` |

No **`MODEM_*`** · **`TXA`** · **`RXA`** · **`TX_LVL`** on PCB.

## Prototype AF wiring

- **GP6** PWM AFSK out → `AF_TX` → radio inject
- **GP26** ADC0 ← `AF_RX` ← radio RX AF
- **GP9** PTT → `PTT_DRV`

**AS-IS** · GPLv3 · non-profit.
