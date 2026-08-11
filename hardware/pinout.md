# Newsheen — pinout & build notes

**MCU:** Espressif **ESP32-S3-WROOM-1**, **N16R2** (16 MB quad flash, 2 MB quad PSRAM).
Round ~55 mm PCB. USB-C (native USB). Designed to sit in the 3D-printed base (`../hardware/3d`).

> Verify flash/PSRAM from the real module with `esptool flash-id` before building — the assembled
> part is the **quad-PSRAM N16R2**. An octal-PSRAM (`…R8`) part would consume GPIO33–37 and break
> the I²C / I²S header.

## GPIO map

| Function | GPIO | Notes |
|---|---|---|
| **Addressable LEDs** | **16** | 8× WS2812B, via a 3V3→5V level shifter |
| **IR receiver (OUT)** | **4** | VS1838B (GND-middle, 38 kHz); OUT → the square pad |
| User button | 17 | active-low, pulled up |
| BOOT | 0 | strapping |
| Reset | EN | — |
| Status LED | 48 | plain LED |
| Buzzer pad | 15 | not populated by default |
| USB D− / D+ | 19 / 20 | native USB |
| I²C SDA / SCL | 35 / 36 | sensor header, 5.1 kΩ pull-ups to 3V3 |
| I²S WS / SCK / SD | 37 / 38 / 39 | sensor header (external mic) |
| Battery sense / charge | 1 / 2 | test points |
| **LoRa** (Wio-SX1262) | NSS 10, MOSI 11, MISO 12, SCK 13, RF_SW 14, NRST 9, DIO1 21, BUSY 47 | 3V3; ANT = wire-antenna pad |

## Build notes (v2 boards)

- **LEDs:** the WS2812 level-shifter's direction pin must be tied **HIGH (+3V3)** for LED data to
  reach the strip. (Corrected in later revisions.)
- **IR:** the receiver footprint expects a **GND-in-the-middle** part (VS1838B / TSOP38238). Install
  with the **OUT/signal leg in the square pad** (toward the module); VCC and GND take the outer pads.
  Breakout-style receivers with VCC in the middle will not work.
