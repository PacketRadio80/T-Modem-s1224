# T-Modem-s1224 — user guide

**Audience:** anyone · **Version:** v1.0-prototype · **AS-IS** · GPLv3 · non-profit

Softmodem sibling of **c1224**: Pico does AFSK (PWM/ADC) + USB **KISS** half-TNC. **No** TCM3105 / socket. Power **12 V**; USB = flash + KISS + ASCII service only.

## What you need

- Assembled board with modem IC / socket / crystals **DNI**
- **12 V** centre-positive supply
- USB host + **any** serial/KISS terminal that speaks the CDC interface below
- AF wiring: **GP6** → radio TX AF · RX AF → **GP26** · **GP9** PTT

## Assemble · flash (overview)

Figures: [`figures/`](figures/) — steps 1–8 (do not edit PNGs for text-only doc updates).

1. Power path on board · **no** USB VBUS power.  
2. Leave modem/socket/XTAL area **unstuffed**.  
3. Flash **`s1224-300.uf2`**, **`s1224-1200.uf2`**, or **`s1224-2400.uf2`**.  
4. Apply **12 V** · plug USB for CDC.

| UF2 | Mark / Space (Hz) |
|-----|-------------------|
| 300 | 1600 / 1800 |
| 1200 | 1200 / 2200 (Bell 202) |
| 2400 | ~1775 / 3250 (Y2-class · not Dire Wolf 2400) |

Build: [`../firmware/s1224/README.md`](../firmware/s1224/README.md).

## Terminals (all that match)

USB CDC ACM · raw **8N1**:

| Path | Use |
|------|-----|
| **KISS** `0xC0` … `0xC0` | Packet (half-TNC + softmodem) |
| ASCII lines when not mid-frame | `HELP` · `STATUS` · `PTT` · `ID` · INI |

**All terminals should work insofar as applicable** — serial apps for service ASCII; KISS tools for packet.

Open `/dev/ttyACM*` → fixed boot banner once → KISS or `HELP`.

## CDC commands

`HELP` · `STATUS` · `ID` · `PTT ON`/`OFF` · `QUIET`/`VERBOSE` · `INI`/`GET`/`SET`/`SAVE`/`RELOAD`/`DEFAULTS`

## INI · SAVE

`SET key=value` then **`SAVE`**. Persist storage = **last 4 KiB of Pico flash** as a **config blob** — **not** a filesystem. Example: [`../share/s1224.ini.example`](../share/s1224.ini.example).

## Optional stacks

Standalone is enough. MAX25 / HyBBX are **not required** and are not claimed as shipped integrations here.

## Power

See [`POWER.md`](POWER.md).

**AS-IS** · no warranty · no support · standard reference only · sibling of **c1224**.
