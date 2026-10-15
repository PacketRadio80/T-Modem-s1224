# T-Modem-s1224 — Pico USB KISS half-TNC + softmodem

**AS-IS** · GPLv3 (repo `LICENSE`) · no support, no warranty · **v1.0-prototype**.

**Three** UF2 images for Raspberry Pi Pico (RP2040). Softmodem does air AFSK; Pico is **USB KISS half-TNC** (HDLC) plus display / PTT / sense. **No** TCM3105 chip path.

| Owns | Does **not** |
|------|----------------|
| HDLC TX/RX (flags, bit-stuff, FCS-CCITT) | Chip bitbang / TCM3105 TXD-RXD |
| Soft AFSK: PWM TX (GP6) · Goertzel RX (GP26 ADC) | Runtime baud switch |
| USB CDC: KISS `0xC0` + ASCII service | XTAL-rate crystal steering |
| Auto PTT GP9 during KISS TX · `PTT ON`/`OFF` | Modem IC / socket |
| Display / INI texts · carrier sense · fixed boot banner | INI/CDC boot-banner toggle |

## Tone profiles

| `-DS1224_BITRATE=` | UF2 | Mark / Space Hz |
|--------------------|-----|-----------------|
| `300` | `s1224-300.uf2` | 1600 / 1800 |
| `1200` (default) | `s1224-1200.uf2` | 1200 / 2200 (Bell 202) |
| `2400` | `s1224-2400.uf2` | ~1775 / 3250 (c1224 Y2-class) |

## Build

Requires [Pico SDK](https://github.com/raspberrypi/pico-sdk) and `arm-none-eabi-gcc`.

```bash
export PICO_SDK_PATH=/path/to/pico-sdk   # e.g. T-Modem-c1224/.deps/pico-sdk
cd firmware/s1224
mkdir -p build && cd build
cmake .. -DS1224_BITRATE=1200    # or 300 / 2400
make -j$(nproc)
# → s1224-1200.uf2  s1224-1200.elf
```

Rebuild separately for each bitrate (clean build dir or out-of-tree dirs recommended).

### Flash

1. Hold **BOOTSEL**, plug USB (board power = **12 V** barrel).
2. Copy `s1224-*.uf2` to RPI-RP2 (or `picotool load -f …`).
3. Host opens `/dev/ttyACM*` (USB CDC).

**Unattended:** CLI `BOOTSEL`/`DFU` enters the bootloader · `REBOOT` restarts · 1200bps DTR-drop touches the bootloader too.

## Any KISS terminal (standalone — no MAX25)

**MAX25-Stack is not required.** **All terminals should work insofar as applicable** — flash UF2 → plug USB → open `/dev/ttyACM*` → KISS `0xC0` or ASCII `HELP`.

| Step | Detail |
|------|--------|
| Device | `/dev/ttyACM*` |
| Framing | Raw **8N1** · binary **KISS** (`0xC0` … `0xC0`) · DATA cmd `0x00` |
| Boot | Fixed source banner **always** once · `quiet=1` damps extra STATUS/CONFIG ASCII only |

## Pins

| GPIO / rail | Role |
|-------------|------|
| GP0 / GP1 | I2C LCD |
| GP6 | **AF_TX** — PWM AFSK out (bypass DNI modem footprint) |
| GP26 | **AF_RX** — ADC0 soft demod in |
| GP7 / GP8 | Legacy chip pads — leave open on softmodem boards |
| GP9 | **PTT** (auto on TX + manual CDC) |
| GP10–12 | Optional LEDs |
| — | LED_PWR on **5 V rail** |

## Boot banner

Fixed source text in `src/common/banner.c` / `include/version.h`. **Always** printed once after USB CDC ready.

## USB CDC ASCII

| Command | Effect |
|---------|--------|
| `HELP` | Command list |
| `STATUS` | state, PTT, signal, packet, carrier, bitrate |
| `ID` | `device_id` / `callsign` / bitrate / quiet |
| `PTT ON` / `PTT OFF` | Key GP9 |
| `QUIET ON` / `OFF` | Suppress extra unsolicited CDC ASCII |
| `INI` / `GET` / `SET` / `SAVE` / `RELOAD` / `DEFAULTS` | Config |
| `BOOTSEL` / `DFU` | Enter UF2 bootloader (unattended flash) |
| `REBOOT` | Restart the firmware |

**`SAVE`:** writes a config **blob** into the **last 4 KiB** of Pico flash — **not** a filesystem.

## INI (`[terminal]`)

| Key | Default | Role |
|-----|---------|------|
| `callsign` | empty | Operator call / station string for `ID` |
| `device_id` | `s1224` | Short device mark |
| `quiet` | `1` | Suppress extra unsolicited CDC ASCII (not boot banner) |

See [`share/s1224.ini.example`](../../share/s1224.ini.example).

## Rate

Firmware does not switch baud at runtime. Flash the UF2 that matches the intended tone profile.
