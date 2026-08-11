# WLEDkitty on the Newsheen puck

The full-featured firmware for the Newsheen puck: [WLED](https://github.com/wled/WLED) with a
Retia personality. Drives the 8 addressable LEDs, reacts to sound through the onboard I2S mic,
takes commands from a 21-key IR remote, and serves the complete WLED web UI over its own Wi-Fi
AP. Boots straight into the **Freqwave** audio visualizer at full brightness.

## Flash it (easiest)

Browser-flash at **[scriptkitty.sh](https://scriptkitty.sh)** — no tools needed. Or grab
`wledkitty-newsheen-puck.factory.bin` from this repo's latest release and:

```bash
esptool --chip esp32s3 write-flash 0x0 wledkitty-newsheen-puck.factory.bin
```

After flashing:
- It boots into **Freqwave** — play music or talk near it and the cat lamp dances to the sound.
  (Audio effects are *dark in a silent room* by design; that's the effect resting, not a fault.)
- Point the **21-key IR remote** at it for colors, white, brightness, and effects.
- Join its **`WLED-<id>`** Wi-Fi (password `wled1234`) and open `http://4.3.2.1` for the full
  web UI + effect picker (pick any ♪/♫ effect for other sound-reactive looks).

## Build from source

Source: **[RetiaLLC/WLEDkitty](https://github.com/RetiaLLC/WLEDkitty)** (branch `wledkitty`),
a WLED fork carrying the Retia device build envs. The puck's env is `pusheen-puck-audio`.

```bash
git clone -b wledkitty https://github.com/RetiaLLC/WLEDkitty && cd WLEDkitty
pio run -e pusheen-puck-audio          # Python 3.12/3.13 (the ESP32 toolchain rejects 3.14+)

# merge a 16 MB DIO factory image
BOOT_APP0=$(find ~/.platformio/packages/framework-arduinoespressif32*/tools/partitions/boot_app0.bin | head -1)
esptool --chip esp32s3 merge-bin -o wledkitty-newsheen-puck.factory.bin \
  --flash-mode dio --flash-freq 80m --flash-size 16MB \
  0x0 .pio/build/pusheen-puck-audio/bootloader.bin \
  0x8000 .pio/build/pusheen-puck-audio/partitions.bin \
  0xE000 "$BOOT_APP0" \
  0x10000 .pio/build/pusheen-puck-audio/firmware.bin
```

## Why this env — key build flags

| Flag | What it does |
|---|---|
| `WLED_BOOTUPDELAY=2500` | Fixes a **softAP boot-settle race**: without it the release build beacons `WLED-AP` but *no client can associate* (the AP comes up before the board settles). Bench-verified 20/20 vs 0/8. |
| `WLED_AP_SSID_UNIQUE` | Per-device AP name (`WLED-<last-3-MAC-bytes>`) so a fleet of units never collide on one SSID. Pass stays `wled1234`. |
| `SR_DMTYPE=3` + `I2S_WSPIN=37 I2S_CKPIN=38 I2S_SDPIN=39` | SPH0645-class I2S mic on the sensor header. Mic type is runtime-switchable (Settings → Usermods) for INMP441 / ICS-43434, etc. |
| `WLED_I2S_SEL_GND=36` | Drives GPIO36 low to ground the mic's **SEL** pin (it lands on the SCL pad when the SPH0645 plugs straight into the header) → left channel, **no rewiring**. |
| `IRPIN=4` + `IRTYPE=5` | 21-key RGB IR remote on GPIO4. |
| `DEFAULT_MODE=137` + `DEFAULT_BOOT_FX=137` + `WLED_DEFAULT_BRIGHTNESS=255` | Boots the **Freqwave** audio visualizer at 100 %. |

Mic sourcing and the sensor-header pinout: see [`hardware/pinout.md`](../../hardware/pinout.md).
