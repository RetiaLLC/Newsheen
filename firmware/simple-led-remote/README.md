# Newsheen — Simple LED Remote firmware

A tiny, self-contained firmware that lets the bundled **21-key IR remote** drive the Newsheen
board's 8 addressable LEDs. No Wi-Fi, no app — point the remote and press a button.

## Flash it

Easiest: flash the pre-built image with [esptool](https://github.com/espressif/esptool) or any
ESP32 web flasher:

```bash
esptool --chip esp32s3 write-flash 0x0 dist/newsheen-simple-led-remote.factory.bin
```

Or build from source: `pio run -e newsheen-led-remote` (PlatformIO).

## What the buttons do

| Button | Action | Button | Action |
|---|---|---|---|
| ON / OFF | LEDs on / off | R, G, B | red / green / blue |
| + / − | brightness up / down | W | white |
| SMOOTH | flowing rainbow | color grid | 12 preset colors |
| FLASH | strobe | 30MIN / 60MIN | sleep timer |

Boots to a warm white. Holding **+ / −** ramps brightness.

## Requirements

- IR receiver: a **VS1838B** (or any GND-in-the-middle 38 kHz demodulating receiver) on **GPIO4**,
  installed with **OUT in the square pad**.
- LEDs: 8× WS2812B on **GPIO16**. On v2 boards, tie the LED level-shifter DIR pin to **+3V3** so
  the LEDs render (see `../../hardware/pinout.md`).

Codes are the standard NEC `0xFF…` 21-key RGB-remote set (address `0x00`); the same codes work
with WLED's built-in "21-key" IR remote type if you later run WLED on the board.
