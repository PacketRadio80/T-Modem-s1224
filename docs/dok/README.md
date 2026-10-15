# T-Modem-s1224 — complete documentation

**Audience:** anyone · **Version:** v1.0-prototype · **AS-IS** · GPLv3 · non-profit

Everything needed to understand, build, flash and run the **T-Modem-s1224**:
what it is · parts · assembly · firmware build · flashing · commissioning ·
operation · host software · troubleshooting. Companion docs stay valid —
this page is the one-stop route.

---

## 1. What it is

The **T-Modem-s1224** is a **KISS half-TNC with a built-in AFSK modem** on a
Raspberry Pi Pico (RP2040). "Half-TNC" means the board owns everything on the
radio side (modulation, demodulation, HDLC framing, PTT); the connected PC runs
the AX.25/network stack. The host link is plain **USB CDC** (a virtual serial
port) speaking **KISS** — the standard packet-radio TNC protocol. **No driver
stack, no special host software** is required; any KISS-capable program works.

**s1224 vs c1224:** the s1224 is the **softmodem** sibling. There is **no modem
chip** on board — the Pico generates and decodes the AFSK tones itself
(PWM output / ADC Goertzel demodulator). The c1224 does the same job with a
classic **TCM3105-class modem chip**. One board family, one PCB base.

```
Host PC (KISS software) ←USB CDC→ Pico (RP2040) ←AF/PTT→ Radio
                                   ├── KISS parser (0xC0 framing)
                                   ├── HDLC encode/decode (flags, bit-stuff, FCS)
                                   ├── AFSK softmodem (PWM TX GP6 / Goertzel RX GP26)
                                   ├── PTT control (GP9)
                                   ├── Optional I2C LCD (GP0/GP1)
                                   └── Config (flash blob)
```

Key facts:

- **Baud rates:** 300 / 600 / 1200 / 2400 / 3600 / 4800 — one **separate
  firmware image per rate**, no runtime switching.
- **Power:** **12 V** barrel (centre-positive). USB is **data only**.
- **Service CLI:** ASCII commands (`HELP`, `STATUS`, `PTT`, …) on the same
  port, interleaved by protocol rules (section 7).
- **Standalone:** no MAX25-Stack or other suite required.

## 2. What you need

- Assembled **T-Modem-s1224** board with Raspberry Pi Pico
  (modem IC / socket / crystals stay **DNI** on this variant)
- **12 V** centre-positive supply
- USB cable (data, not charge-only) to any host
- Radio with AF in/out + PTT input; 3 wires to the board (section 4)
- Optional: HD44780 16×2 LCD with PCF8574 I2C backpack

Part lists: [`../../hardware/BOM.md`](../../hardware/BOM.md) (s1224 = c1224
kit **minus** modem IC, socket, crystals) · CSV [`../../hardware/bom/BOM.csv`](../../hardware/bom/BOM.csv).

## 3. Hardware

### Stuffing rules (softmodem variant)

1. Power path on board · **no** USB VBUS power (see [`../POWER.md`](../POWER.md)).
2. Leave **modem IC / socket / crystal area unstuffed** (DNI).
3. Pico on headers/socket, display optional.

Assembly steps with photos: [`../figures/`](../figures/) (steps 1–8) ·
PCB: [`../../hardware/kicad/t-modem-s1224/README.md`](../../hardware/kicad/t-modem-s1224/README.md).

### Pins

| GPIO / rail | Role |
|-------------|------|
| GP0 / GP1 | I2C LCD (optional) |
| GP6 | **AF_TX** — PWM AFSK out → radio TX AF (mic) |
| GP26 | **AF_RX** — ADC0 soft demod in ← radio RX AF |
| GP7 / GP8 | Legacy chip pads — leave open on softmodem boards |
| GP9 | **PTT** → optocoupler → radio PTT input (auto on TX + manual CLI) |
| GP10–12 | Optional LEDs |
| — | LED_PWR on **5 V rail** |

### Power

| Net / path | Role |
|------------|------|
| `VIN_12` | Barrel jack **5.5×2.1** centre-**positive** |
| `U_REG` | Buck **12 V → 5 V** (Mini560 5 V class / MP1584-class) |
| USB `VBUS` | **Isolated** — never feeds the 5 V rail |

### Radio wiring

| Board | Radio |
|-------|-------|
| GP6 AF out | TX AF / microphone input (watch level, start low) |
| GP26 AF in | RX AF (flat/discriminator audio preferred) |
| GP9 PTT | PTT input (optocoupler, active per radio) |
| GND | Common ground with radio |

## 4. Build the firmware

**Prerequisites:** [Pico SDK](https://github.com/raspberrypi/pico-sdk) ·
`arm-none-eabi-gcc` · `cmake` · `make` (Linux: `apt install cmake
build-essential gcc-arm-none-eabi git`).

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
cd firmware/s1224
mkdir -p build-2400 && cd build-2400
cmake .. -DS1224_BITRATE=2400    # 300 | 600 | 1200 | 2400 | 3600 | 4800
make -j$(nproc)
# → s1224-2400.uf2  s1224-2400.elf
```

Use **one build dir per rate** (the rate is compile-time). Full detail:
[`../../firmware/s1224/README.md`](../../firmware/s1224/README.md).

### Tone profiles (compile-time)

| `-DS1224_BITRATE=` | UF2 | Mark / Space (Hz) | Profile |
|--------------------|-----|-------------------|---------|
| `300` | `s1224-300.uf2` | 1600 / 1800 | HF narrow AFSK |
| `600` | `s1224-600.uf2` | 1200 / 1800 | HF standard |
| `1200` (default) | `s1224-1200.uf2` | 1200 / 2200 | Bell-202 |
| `2400` | `s1224-2400.uf2` | ~1775 / 3250 | Y2-class (not Dire Wolf 2400) |
| `3600` | `s1224-3600.uf2` | 1800 / 2700 | High-rate HF |
| `4800` | `s1224-4800.uf2` | 2100 / 3300 | High-rate |

Pre-built images ship in [`../../firmware/s1224/bin/`](../../firmware/s1224/bin/).

## 5. Flash

### BOOTSEL (recommended)

1. **Disconnect** USB.
2. **Hold the BOOTSEL button** on the Pico while plugging USB in.
3. The Pico appears as USB mass storage (**`RPI-RP2`**).
4. Copy the matching `.uf2` onto it:
   ```bash
   # Linux
   cp firmware/s1224/bin/s1224-2400.uf2 /media/$USER/RPI-RP2/
   ```
5. The Pico **reboots automatically** into the new firmware.

### picotool (advanced)

```bash
picotool load firmware/s1224/bin/s1224-2400.uf2 -x
```

### Unattended reflash (CLI)

No BOOTSEL button press needed — over the same USB port:

| Trigger | Effect |
|---------|--------|
| CLI `BOOTSEL` (alias `DFU`) | Enters the UF2 bootloader (`OK BOOTSEL`) |
| CLI `REBOOT` | Restarts the firmware (`OK REBOOT`) |
| Open port at **1200 baud**, then drop **DTR** | Enters the UF2 bootloader (classic 1200bps touch) |

Typical cycle: send `BOOTSEL` → wait for `RPI-RP2` → copy UF2 (or
`picotool load -x`) → board runs new firmware. **Careful:** modem/dial tools
that open the port at 1200 baud can trigger the touch by accident.

## 6. Commissioning (first start)

1. Set the switches: **12 V** on, USB plugged to host.
2. Open the CDC port — **`/dev/ttyACM0`** (Linux) / equivalent — raw **8N1**,
   115200 does not matter (CDC ignores baud). A fixed **boot banner** prints
   **once** when USB is ready.
3. Send `STATUS` → `STATUS state=… ptt=… signal=…` answers.
4. `PTT ON` → PTT LED / radio keys · `PTT OFF` → releases. (`OK PTT ON`)
5. KISS test: send a raw KISS DATA frame (`0xC0 0x00 … 0xC0`) →
   `STATUS`/events show `is sending`, PTT keys, ~tones on the air.
6. Wire the radio (section 3) and repeat 5 listening on another receiver.

**Hint:** the first 1–2 bytes after opening the port can be eaten by the boot
banner — settle 1–2 s or send a dummy `STATUS` first.

## 7. Operation

### Two protocols on one port (dual protocol)

| Traffic | Rule |
|---------|------|
| Packet | Bytes framed with **`0xC0`** → KISS (DATA command `0x00`) |
| Service | Line-oriented ASCII when **not** mid-frame → CLI commands |
| Mixing | Never send ASCII while inside a KISS frame |

### KISS contract (for host authors)

| Topic | Convention |
|-------|------------|
| Framing | Standard KISS: `FEND 0x00 <payload> FEND`, `FESC/TFEND/TFESC` escaping |
| TX FCS | **Firmware appends the HDLC FCS** — host sends payload only |
| RX FCS | **Firmware validates and strips the FCS** — host receives payload only |
| Mode | KISS is active **from power-up** — no init sequence, no `MYCALL` handshake |

### ASCII service commands

| Command | Description |
|---------|-------------|
| `HELP` | List available commands |
| `STATUS` | Modem state, PTT, signal, packet, carrier, bitrate |
| `ID` | Device ID, callsign, bitrate, quiet |
| `PTT ON` / `PTT OFF` | Manual PTT control (GP9) |
| `QUIET ON/OFF` | Enable/disable unsolicited output (never the boot banner) |
| `VERBOSE ON/OFF` | Alias: `QUIET OFF` / `QUIET ON` |
| `INI` | Dump configuration |
| `GET <key>` | Read config value |
| `SET <key>=<value>` | Set config value (RAM only) |
| `SAVE` | Persist config to flash |
| `RELOAD` | Reload config from flash |
| `DEFAULTS` | Reset to factory defaults |
| `BOOTSEL` / `DFU` | Enter UF2 bootloader (unattended flash) |
| `REBOOT` | Restart the firmware |

**TX note:** the firmware transmits synchronously — `STATUS` polls during a
transmission are delayed; judge TX by events or check after TX.

## 8. Configuration (INI)

`SET key=value` then **`SAVE`**. Persist storage = **last 4 KiB of Pico
flash** as a **config blob** — **not** a filesystem. Full example:
[`../../share/s1224.ini.example`](../../share/s1224.ini.example) · firmware
defaults [`../../firmware/s1224/config/default.ini`](../../firmware/s1224/config/default.ini).

| Key | Default | Role |
|-----|---------|------|
| `callsign` | empty | Operator call / station string for `ID` |
| `device_id` | `s1224` | Short device mark |
| `quiet` | `1` | Suppress extra unsolicited CDC ASCII |
| `cdt_active_high` | `0` | Sense polarity (legacy chip pad) |
| `packet_hold_ms` | `500` | Packet-flag hold time |
| `packet_edge_min` | `4` | Packet detect edges |
| `packet_window_ms` | `50` | Packet detect window |

Display texts (`starting`, `ready`, `sending`, …) are `[display]` keys.

## 9. Host software

**All terminals work insofar as applicable** — serial apps for service ASCII;
KISS tools for packet. Examples:

```bash
stty -F /dev/ttyACM0 raw -echo
kissattach /dev/ttyACM0 tm0      # ax25-tools (names vary by distro)
# or: kissutil -p /dev/ttyACM0
# or: Dire Wolf serial KISS, any packet terminal, custom code
```

Known-good host driver reference: PRTERM rig driver `tmodem`
(station config `driver=tmodem`, `kiss_init=native`). Optional convenience:
`max25d` device **`tmodem`** / `max25-terminal`. Standalone is enough.

## 10. Troubleshooting

| Symptom | Likely cause / action |
|---------|----------------------|
| No `/dev/ttyACM*` | Board needs **12 V** (USB does not power it) · data cable · try other USB port |
| CLI dead, port alive | Boot banner ate the first bytes — wait, send `STATUS` again |
| `PTT ON` works, KISS does not | Old firmware (pre KISS fix) — reflash current UF2 |
| PTT keys, no tone | AF wiring GP6 → radio mic · level/trimmer · check on a second receiver |
| No RX | Radio AF → GP26 · squelch open · check `STATUS signal/packet` |
| `signal` always 0 | Prototype PCB: GP7 sense pad is **NC** — sense is unavailable on s1224 |
| Board leaves bootloader after seconds | Old firmware — current builds disable the watchdog before the bootrom |
| Port vanishes / pokes at 1200 baud | ModemManager or similar probing the CDC port — stop/disable it |
| Wrong tones on air | Flashed the wrong rate — UF2 must match the intended profile (section 4) |
| TX/RX distorted | AF level too hot — start with low mic gain, then set deviation |

## 11. Document map

| Doc | Role |
|-----|------|
| [`../USER-GUIDE.md`](../USER-GUIDE.md) | One-page user guide |
| [`../POWER.md`](../POWER.md) | 12 V barrel / no USB power |
| [`../../firmware/s1224/README.md`](../../firmware/s1224/README.md) | Firmware details · pins · INI |
| [`../../hardware/BOM.md`](../../hardware/BOM.md) | Bill of materials |
| [`../../hardware/kicad/t-modem-s1224/README.md`](../../hardware/kicad/t-modem-s1224/README.md) | PCB · DNI modem footprint |
| [`../../share/s1224.ini.example`](../../share/s1224.ini.example) | Operator INI example |
| [`../figures/`](../figures/) | Board overview + assembly steps 1–8 |

**AS-IS** · no warranty · no support · standard reference only ·
sibling of **c1224** · GPLv3 non-profit.
