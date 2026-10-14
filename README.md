# T-Modem-s1224

**Prototype Notice:** This repository contains hardware prototypes that have **passed all automated software tests** (unit, integration, stress). **Practical field testing with real radio equipment is still pending.** Use at your own risk. GPLv3 — non-profit.

---

USB KISS half-TNC + softmodem AFSK for Raspberry Pi Pico. No modem chip — all signal processing done in software (PWM TX / ADC Goertzel RX). Six baud rates available as separate firmware images.

**Firmware language:** C · **License:** GPLv3 · **Platform:** RP2040 (Raspberry Pi Pico)

## Firmware Images

Pre-built UF2 files in `firmware/s1224/bin/`:

| File | Baud | Mark (Hz) | Space (Hz) | Profile |
|------|------|-----------|------------|---------|
| `s1224-300.uf2` | 300 | 1600 | 1800 | HF narrow AFSK |
| `s1224-600.uf2` | 600 | 1200 | 1800 | HF standard |
| `s1224-1200.uf2` | 1200 | 1200 | 2200 | Bell-202 |
| `s1224-2400.uf2` | 2400 | 1775 | 3250 | Y2-class |
| `s1224-3600.uf2` | 3600 | 1800 | 2700 | High-rate HF |
| `s1224-4800.uf2` | 4800 | 2100 | 3300 | High-rate |

Each baud rate requires a **separate firmware image** — no runtime switching.

## Quick Guide — Firmware Update

### Prerequisites
- Raspberry Pi Pico with USB cable
- T-Modem-s1224 board (or any RP2040 board for testing)

### Flash via BOOTSEL (Recommended)

1. **Disconnect** the Pico from USB.
2. **Hold the BOOTSEL button** on the Pico while plugging in USB.
3. The Pico appears as a USB mass storage device (`RPI-RP2`).
4. **Copy** the appropriate `.uf2` file to the `RPI-RP2` drive:
   ```bash
   # Linux
   cp firmware/s1224/bin/s1224-2400.uf2 /media/$USER/RPI-RP2/
   ```
5. The Pico **automatically reboots** and runs the new firmware.
6. Verify: open `/dev/ttyACM0` at 115200 baud, send `STATUS` → should respond.

### Flash via picotool (Advanced)

```bash
picotool load firmware/s1224/bin/s1224-2400.uf2 -x
```

### Build from Source

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
cd firmware/s1224
mkdir build && cd build
cmake .. -DS1224_BITRATE=2400   # 300|600|1200|2400|3600|4800
make -j$(nproc)
# → s1224-2400.uf2
```

## Operation

1. Power from **12 V** barrel (centre-positive). USB = data only.
2. Connect USB to host. Open `/dev/ttyACM0` at 115200 baud.
3. **KISS mode:** Send `0xC0`-framed packets (standard KISS TNC).
4. **Service mode:** Send ASCII commands (`HELP`, `STATUS`, `PTT ON/OFF`, `ID`).

### ASCII Commands

| Command | Description |
|---------|-------------|
| `HELP` | List available commands |
| `STATUS` | Show modem state, PTT, signal, bitrate |
| `ID` | Show device ID, callsign, bitrate |
| `PTT ON` / `PTT OFF` | Manual PTT control |
| `QUIET ON/OFF` | Enable/disable unsolicited output |
| `INI` | Dump configuration |
| `GET <key>` | Read config value |
| `SET <key>=<value>` | Set config value (RAM only) |
| `SAVE` | Persist config to flash |
| `RELOAD` | Reload config from flash |
| `DEFAULTS` | Reset to factory defaults |

## Hardware

- **Schematic:** `hardware/kicad/`
- **BOM:** `hardware/BOM.md` and `hardware/bom/BOM.csv`
- **AF out:** GP6 (PWM AFSK output)
- **AF in:** GP26 (ADC Goertzel input)
- **PTT:** GP9 → optocoupler → radio PTT input
- **Display:** I2C HD44780 via PCF8574 backpack (optional)

## Architecture

```
Host (USB CDC) ←→ Pico (RP2040) ←→ Radio
                  │
                  ├── KISS parser (0xC0 framing)
                  ├── HDLC encode/decode
                  ├── AFSK softmodem (PWM TX / Goertzel RX)
                  ├── PTT control (GP9)
                  ├── I2C display (optional)
                  └── Config (flash storage)
```

## Tone Profiles (Compile-Time)

| Baud | Mark (Hz) | Space (Hz) | Shift (Hz) | Notes |
|------|-----------|------------|------------|-------|
| 300 | 1600 | 1800 | 200 | HF narrow |
| 600 | 1200 | 1800 | 600 | HF standard |
| 1200 | 1200 | 2200 | 1000 | Bell-202 |
| 2400 | 1775 | 3250 | 1475 | Y2-class |
| 3600 | 1800 | 2700 | 900 | High-rate HF |
| 4800 | 2100 | 3300 | 1200 | High-rate |

## Project Structure

```
T-Modem-s1224/
├── firmware/s1224/
│   ├── src/common/          # C source files
│   ├── include/             # Headers (afsk_soft.h, afsk_profile.h)
│   ├── config/              # Default INI
│   ├── bin/                 # Pre-built UF2 images (6 rates)
│   └── CMakeLists.txt
├── hardware/
│   ├── kicad/               # Schematic + PCB
│   ├── bom/                 # Bill of materials
│   └── BOM.md
├── docs/                    # User guide, figures
└── share/                   # Example INI files
```

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).

**Non-profit project.** Not for commercial use without permission.