# EchoForest On-Site Hardware Checklist

This document is for the person physically checking wiring and running the first real-device test. It intentionally focuses on simple, verifiable hardware facts and field actions.

Record every result. Do not silently change pins in code without updating `docs/S3_HARDWARE_FACTS.md` and the final integration report.

## 1. Required equipment

- ESP32-S3 board and HSD024290/ST7789 display
- ESP32-C5 board and P183B001 display/touch unit
- BH1750
- DHT11
- INMP441
- USB cables capable of data transfer
- PC with ESP-IDF 5.5.4 environment
- Stable Wi-Fi router or phone hotspot
- Backend PC on the same network
- Optional multimeter

## 2. Network decision

Use one shared Wi-Fi or phone hotspot for the first real test unless there is a proven reason not to.

Record:

```text
Wi-Fi SSID:
Backend PC IPv4:
Backend port:
Backend URL used by S3:
Backend URL used by C5:
Windows firewall rule confirmed: YES / NO
S3 can ping/reach backend: YES / NO
C5 can ping/reach backend: YES / NO
```

ESP32 backend URLs must use the PC LAN IP. Do not use `localhost` or `127.0.0.1`.

## 3. S3 display wiring check

Expected from the latest received source:

| Signal | Expected GPIO | Physical confirmation |
| --- | ---: | --- |
| MISO | 48 | |
| MOSI | 47 | |
| SCLK | 21 | |
| CS | 42 | |
| DC | 41 | |
| RST | not connected / -1 | |

Before continuing:

- confirm power voltage and ground;
- confirm no visible short circuit;
- call LCD initialization before any animation;
- display a full-screen solid color;
- display text or a test frame;
- play at least one local animation loop.

Record:

```text
LCD initialization success: YES / NO
Solid color test: PASS / FAIL
Text/frame test: PASS / FAIL
Animation playback: PASS / FAIL
Observed orientation:
Observed color problem, if any:
```

## 4. BH1750 check

Expected:

| Item | Value |
| --- | --- |
| SDA | GPIO5 |
| SCL | GPIO4 |
| Address | 0x23 |
| I2C port | I2C_NUM_0 |

For the first closed-loop test, disable the camera path because the camera SCCB also uses GPIO4/GPIO5.

Record three readings:

```text
Covered/dark reading:
Normal indoor reading:
Phone flashlight reading:
Unit shown in logs:
Read failure distinguishable from zero light: YES / NO
```

The converted reading should be treated as approximately lux, not klx.

## 5. DHT11 check

Expected source pin: GPIO0.

Check that the sensor does not hold GPIO0 low during board reset.

Record:

```text
Physical DATA GPIO:
Board boots normally with DHT11 connected: YES / NO
Temperature reading:
Humidity reading:
Checksum failure is rejected: YES / NO
Failed read avoids uploading garbage: YES / NO
```

If GPIO0 causes boot instability, move the DHT11 DATA line to a safe free GPIO and update both source and hardware facts documentation.

## 6. INMP441 check

Expected:

| Signal | GPIO |
| --- | ---: |
| BCLK | 35 |
| WS/LRCL | 36 |
| DIN | 37 |

Expected configuration:

```text
I2S_NUM_0
16000 Hz
32-bit
mono left slot
512-sample blocks
```

Measure and record at least ten seconds for each condition:

```text
Quiet room RMS range:
Quiet room computed noise range:
Normal speech RMS range:
Normal speech computed noise range:
Loud music/clap RMS range:
Loud music/clap computed noise range:
Recommended backend maxNoise threshold:
```

Do not label the computed value as calibrated dB SPL unless an actual acoustic calibration has been performed.

## 7. C5 physical interface check

Known C5 values from the existing base package:

### LCD

| Signal | GPIO |
| --- | ---: |
| MOSI | 23 |
| SCLK | 24 |
| CS | 25 |
| DC/RS | 26 |
| Backlight | 5 |

### Touch

| Item | Value |
| --- | --- |
| Interface | I2C |
| SDA | GPIO2 |
| SCL | GPIO3 |
| Address | 0x15 |
| Coordinate transform | `y = LCD_HEIGHT - 1 - raw_y` |

Record:

```text
C5 LCD starts: YES / NO
Touch raw coordinates available: YES / NO
Touch coordinate orientation correct: YES / NO
Start button hit region works: YES / NO
Break button hit region works: YES / NO
End button hit region works: YES / NO
TTS function works: YES / NO
TTS blocks the main loop: YES / NO
```

## 8. End-to-end field test

Run the backend and frontend first, then power the boards.

Required result:

1. C5 and S3 report online through heartbeat.
2. C5 touch starts focus mode.
3. Web UI immediately shows focus mode.
4. S3 uploads real light/noise/temp/humi.
5. Web UI displays those values.
6. Covering BH1750 causes `light_warning`.
7. Loud noise causes `noise_warning` after calibration.
8. Temperature/humidity out of range causes `comfort_warning` if safely testable.
9. C5 displays the same backend state.
10. S3 displays the corresponding forest state.
11. C5 starts break and the web/S3/C5 state agrees.
12. C5 or web ends focus and a learning log is generated.
13. A selected local S3 animation can be played on demand for visual verification.

Record failures with:

```text
Step number:
Observed behavior:
Serial log:
Backend log:
Physical condition:
Temporary workaround:
Needs code change: YES / NO
```

## 9. Safety and rollback

- Keep the existing simulator path available throughout the field test.
- Do not destroy the known-working C5 or S3 base projects.
- Keep camera disabled until the BH1750/shared-pin conflict is resolved.
- If real sensor integration is unstable, switch S3 to mock samples while retaining real Wi-Fi and display tests.
- If a screen fails, retain serial logging and backend synchronization so the rest of the loop can still be verified.
