# EchoForest Field Hardware Checklist

## 1. Purpose

This checklist is for the onsite hardware checker before and during the first real S3/C5 integration test.

It intentionally focuses on physical wiring, power, network reachability, and observable serial behavior. Do not change firmware architecture or backend protocol during this check.

Record every final result in the tables instead of relying on memory.

## 2. Safety rules

- Power off the board before moving any wire.
- Use a shared ground between each board and its attached modules.
- Use 3.3 V logic. Do not apply 5 V to ESP32 GPIO pins.
- Do not connect or disconnect the LCD or microphone while the board is powered unless the hardware design explicitly supports it.
- Keep the camera disconnected for the first full EchoForest test unless camera coexistence has been separately validated.

## 3. S3 expected wiring

### 3.1 S3 LCD

| Signal | Expected GPIO | Field result |
|---|---:|---|
| MISO | 48 | |
| MOSI | 47 | |
| SCLK | 21 | |
| CS | 42 | |
| DC | 41 | |
| RST | Not connected / software-controlled | |
| Power | Board-approved 3.3 V supply | |
| GND | Common ground | |

Checklist:

- [ ] Screen model/resolution matches 240 x 320.
- [ ] The display is connected to the pin set above.
- [ ] Backlight is visibly powered.
- [ ] No old wiring remains on the previously assumed LCD SCLK/MOSI pins.
- [ ] A simple full-screen color test works before animation testing.
- [ ] `lcd_display_init()` succeeds before any draw call.
- [ ] Red/blue are not swapped; if they are, record it rather than rewiring immediately.
- [ ] Screen orientation is correct.

### 3.2 BH1750

| Signal | Expected GPIO/value | Field result |
|---|---:|---|
| SDA | GPIO5 | |
| SCL | GPIO4 | |
| Address | 0x23 | |
| Bus | I2C_NUM_0, 100 kHz | |
| Power | 3.3 V | |
| GND | Common ground | |

Checklist:

- [ ] Camera is disconnected or disabled because camera SCCB also uses GPIO4/GPIO5.
- [ ] I2C scan or BH1750 init finds address 0x23.
- [ ] Covering the sensor causes a clear lux decrease.
- [ ] Shining a lamp causes a clear lux increase.
- [ ] A read failure is not recorded as a valid 0-lux sample.

### 3.3 DHT11

| Signal | Expected current GPIO/value | Field result |
|---|---:|---|
| DATA | GPIO0 | |
| Power | 3.3 V | |
| GND | Common ground | |
| Pull-up | Present internally or on module | |

Checklist:

- [ ] Board boots normally with DHT11 connected.
- [ ] Reset does not enter download/bootloader mode unexpectedly.
- [ ] Temperature and humidity readings pass checksum validation.
- [ ] At least ten consecutive reads are stable and plausible.

If GPIO0 causes boot problems and rewiring is physically easy:

1. Power off.
2. Move DHT11 DATA to a verified free non-strapping GPIO.
3. Do not use GPIO13 while the camera is enabled, because GPIO13 is camera PCLK in the received package.
4. Record the new GPIO below.
5. Update the single firmware configuration constant; do not scatter the new pin through multiple files.

```text
Final DHT11 DATA GPIO: __________
Reason for change: ______________________________________________
```

### 3.4 INMP441

| Signal | Expected GPIO/value | Field result |
|---|---:|---|
| BCLK / SCK | GPIO35 | |
| WS / LRCL | GPIO36 | |
| DOUT / SD | GPIO37 | |
| L/R select | Left-channel wiring | |
| Sample rate | 16000 Hz | |
| Power | 3.3 V | |
| GND | Common ground | |

Checklist:

- [ ] L/R selection matches the firmware left-slot configuration.
- [ ] Quiet room produces nonzero but stable RMS/log-amplitude values.
- [ ] Normal speech produces a clear increase.
- [ ] Clap/music/noisy condition produces a clear further increase.
- [ ] No constant saturation or all-zero stream occurs.

Record calibration values after at least 10 seconds per condition:

| Condition | RMS range | `20*log10(rms+1)` range | Notes |
|---|---:|---:|---|
| Quiet | | | |
| Normal speech | | | |
| Clearly noisy | | | |

Suggested backend noise threshold after measurement: __________

Do not call this value calibrated dB SPL unless a reference sound-level meter has been used.

## 4. C5 expected wiring

### 4.1 C5 LCD

| Signal | Expected GPIO | Field result |
|---|---:|---|
| MOSI / SDI | 23 | |
| SCLK | 24 | |
| CS | 25 | |
| DC / RS | 26 | |
| Backlight | 5 | |
| MISO | Not used | |
| Reset | Software reset | |

Checklist:

- [ ] P183B001 240 x 284 screen powers on.
- [ ] Solid-color and rectangle tests work.
- [ ] Screen orientation is correct.
- [ ] Touch coordinates line up with rendered buttons.

### 4.2 C5 touch

| Item | Expected value | Field result |
|---|---|---|
| Interface | I2C | |
| Port | I2C_NUM_0 | |
| SDA | GPIO2 | |
| SCL | GPIO3 | |
| Address | 0x15 | |
| Y transform | `LCD_HEIGHT - 1 - raw_y` | |

Checklist:

- [ ] Address 0x15 responds.
- [ ] Top-left, center, and bottom-right coordinates are recorded.
- [ ] Start, break, and end buttons each trigger only their own action.
- [ ] One physical tap produces one backend request.
- [ ] Touch release/debounce prevents repeated requests.

### 4.3 C5 speaker/TTS

| Signal/API | Expected value | Field result |
|---|---|---|
| PA control | GPIO1 | |
| PDM_P | GPIO7 | |
| PDM_N | GPIO8 | |
| TTS API | `tts_say(const char *text)` | |

Checklist:

- [ ] Startup test phrase is audible.
- [ ] Playback does not permanently block touch polling or backend polling.
- [ ] The same forest state is not spoken repeatedly on every state poll.
- [ ] Volume is safe and intelligible onsite.

## 5. Network checklist

Preferred topology:

```text
PC backend + S3 + C5 -> same router or phone hotspot
```

Record:

```text
Wi-Fi SSID: __________________________
Backend PC IPv4: ____________________
Backend port: 3001 / other: _________
Backend base URL: http://____________________:________
```

Checklist:

- [ ] PC, S3, and C5 are on the same LAN.
- [ ] AP/client isolation is disabled.
- [ ] Backend listens on an address reachable from the LAN, not only loopback.
- [ ] Windows firewall permits inbound TCP on the backend port.
- [ ] S3 can reach `/health`.
- [ ] C5 can reach `/health`.
- [ ] Neither board uses `localhost` or `127.0.0.1` for the PC backend.
- [ ] Wi-Fi credentials and backend URL are injected by local config and not committed.

Fallback topology only when explicitly required:

```text
C5 AP/NAT -> S3 and possibly PC
```

Do not use the fallback unless backend reachability from both boards is proven.

## 6. First onsite smoke sequence

1. Start backend and open the web console.
2. Verify `/health` and `/api/state` from the PC.
3. Boot C5; verify screen, touch, backend heartbeat, and current state display.
4. Boot S3 with camera disabled; verify LCD init, BH1750, DHT11, INMP441, heartbeat, and environment upload.
5. Start focus from C5.
6. Confirm web, C5, and S3 all show the backend-confirmed focus/growing state.
7. Cover BH1750 and confirm `light_warning` everywhere.
8. Restore light and create a noisy condition; confirm `noise_warning` everywhere.
9. Enter break from C5; confirm `break` everywhere.
10. End focus; confirm `completed` and a generated log.
11. Play the local S3 animation smoke test and confirm it remains responsive to backend/sensor tasks.

## 7. Final field record

```text
Date/time:
Location:
Checker:
S3 board identifier:
C5 board identifier:
Backend PC:
S3 firmware commit:
C5 firmware commit:
Backend commit:
Final result: PASS / RISKY / BLOCKED
```

Open issues:

```text
1.
2.
3.
```
