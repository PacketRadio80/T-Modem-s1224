# Power — T-Modem-s1224

| Net / path | Role |
|------------|------|
| `VIN_12` | Barrel jack **5.5×2.1** centre-**positive** |
| `U_REG` | Buck **12 V → 5 V** (Mini560 5 V class / MP1584-class on headers) |
| `+5V` / `+5V_REG` | Board rail after buck (+ optional series PTC) |
| `GND` | Common ground |
| USB `VBUS` | **Isolated** — **DNI** OR diode · **no** feed into 5 V rail |
| USB D+/D− / GND | Flash + CDC command path to Pico · LCD service via Pico |

Pico **VSYS/VBUS pin on headers** is fed from the **board 5 V rail**, not from the Type-A receptacle VBUS.
