# T-Modem-s1224 — BOM (planning)

**Status:** planning BOM · shared PCB base with **c1224** · softmodem stuffing  
**Product:** **T-Modem-s1224** · slave `/home/akb/Code/10-PROJECTS/T-Modem-s1224/`  
**License:** GNU **GPLv3** · **non-profit** · **AS-IS**

## Delta vs c1224

| Item | Qty s1224 | Note |
|------|-----------|------|
| U_MODEM (TCM3105-class) | **0 DNI** | Softmodem replaces chip AFSK |
| SKT_MODEM (DIP-16 socket) | **0 DNI** | Omit |
| Y1 / Y2 crystals | **0 DNI** | Rate = FW profile (300/1200/2400) |
| Pico + power + display + AF jacks | as c1224 | Shared kit |

CSV: [`bom/BOM.csv`](bom/BOM.csv)
