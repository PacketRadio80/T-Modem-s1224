# Docs — T-Modem-s1224

| Doc | Role |
|-----|------|
| [`USER-GUIDE.md`](USER-GUIDE.md) | Anyone — what it is · assemble · flash · terminals · SAVE |
| [`../README.md`](../README.md) | Product overview |
| [`../firmware/s1224/README.md`](../firmware/s1224/README.md) | Pico softmodem FW |
| [`POWER.md`](POWER.md) | 12 V barrel / no USB power |
| [`../hardware/kicad/t-modem-s1224/README.md`](../hardware/kicad/t-modem-s1224/README.md) | PCB · DNI modem/socket |
| [`../hardware/BOM.md`](../hardware/BOM.md) | Planning BOM (tracked) |
| [`figures/`](figures/) | Board overview + assembly steps 1–8 |

## Operate (standalone)

**No MAX25-Stack required.** Flash `s1224-*.uf2` → USB → `/dev/ttyACM*` → KISS (`0xC0`) or ASCII `HELP`. Fixed boot banner prints once. **All matching terminals work** insofar as applicable.

**INI `SAVE`:** last **4 KiB** of Pico flash as a config **blob** — not a filesystem.

## Figures

| File | Content |
|------|---------|
| [`figures/s1224-board-overview.png`](figures/s1224-board-overview.png) | Top-down board · major parts |
| [`figures/fig-01-pcb-standoffs.png`](figures/fig-01-pcb-standoffs.png) | Step 1 — PCB + standoffs |
| [`figures/fig-02-power-section.png`](figures/fig-02-power-section.png) | Step 2 — 12 V → buck → 5 V |
| [`figures/fig-03-modem-dni.png`](figures/fig-03-modem-dni.png) | Step 3 — modem/socket/XTAL **DNI** |
| [`figures/fig-04-pico.png`](figures/fig-04-pico.png) | Step 4 — Pico H |
| [`figures/fig-05-display.png`](figures/fig-05-display.png) | Step 5 — I²C display |
| [`figures/fig-06-leds.png`](figures/fig-06-leds.png) | Step 6 — indicators |
| [`figures/fig-07-connectors.png`](figures/fig-07-connectors.png) | Step 7 — AF/PTT · USB |
| [`figures/fig-08-final-check.png`](figures/fig-08-final-check.png) | Step 8 — final check |

**AS-IS** · GPLv3 · non-profit · standard reference only.
