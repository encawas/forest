# EchoForest Hardware Source of Truth

## 1. Purpose and precedence

This document is the authoritative hardware baseline for the next real-device integration task.

When hardware facts conflict, use this precedence order:

1. The latest inspected real ESP-IDF package and its source code.
2. This document.
3. Existing repository implementation and API contract.
4. Older planning documents, screenshots, verbal assumptions, and inferred pin maps.

For the current task, the latest accepted S3 package is:

```text
hardware_reference/s3_base_project_20260710.zip.b64.part00 ... part07
SHA-256: a75ff2ef026ee1d97d8b7d01eeab1fadea44af19e34c6fab8bc4668fa4ab3453
```

Decode it with:

```cmd
scripts\decode_s3_reference.cmd
```

The decoded archive is reference input, not the final EchoForest project structure. Its useful drivers and verified configuration must be integrated into the existing `firmware_modules/` architecture rather than replacing the backend state model.

## 2. Canonical software protocol

The current repository API and enum names are authoritative.

### Endpoints

```text
GET  /api/state
GET  /api/logs
POST /api/control/start_focus
POST /api/control/start_break
POST /api/control/end_focus
POST /api/device_data
POST /api/devices/heartbeat
```

### Mode values

```text
idle
focus
break
completed
```

### Forest-state values

```text
idle
growing
light_warning
noise_warning
comfort_warning
break
completed
```

### S3 environment payload

```json
{
  "light": 300,
  "noise": 40,
  "temp": 24,
  "humi": 50,
  "source": "s3",
  "deviceId": "s3-main"
}
```

Do not reintroduce old names such as `/api/session/start`, `low_light`, `noisy`, `uncomfortable`, `recovering`, or `device_id` unless the whole API contract and all clients are intentionally migrated together.

## 3. Final hardware responsibility split

### ESP32-S3

- Read the real light, noise, temperature, and humidity sensors.
- Upload environment data to the backend.
- Read backend state.
- Drive the 240 x 320 main forest screen.
- Play local screen animation selected from backend state.
- Never decide the final session mode or final forest state locally.

### ESP32-C5

- Read touch input.
- Send start-focus, start-break, and end-focus requests to the backend.
- Read backend state.
- Drive the 240 x 284 status/control screen.
- Trigger TTS or prompt audio only on meaningful state changes.
- Never become the state center.

### Backend

- Remains the only state center.
- Computes forest state from session mode and S3 environment data.
- Supplies the same state to web, S3, and C5.

## 4. Verified ESP32-S3 base project

### 4.1 Project baseline

| Item | Verified value |
|---|---|
| Target | ESP32-S3 |
| ESP-IDF | 5.5.4 environment used by the received package |
| Project name | `1_LED` |
| Entry file | `main/main.c` |
| Entry function | `app_main()` |
| Flash configuration | 2 MB |
| PSRAM | Disabled |
| Partition table | Single app |
| Camera dependency | `espressif/esp32-camera` 2.1.7 |
| Main existing capabilities | Wi-Fi STA, UDP, camera web server, ST7789 LCD, BH1750, DHT11, INMP441 |

The package has useful drivers, but its current `app_main()` is a hardware demo, not an EchoForest backend client.

### 4.2 S3 main LCD

The received code uses the ESP-IDF `esp_lcd` ST7789 driver.

| Item | Verified value |
|---|---|
| Controller used in code | ST7789 |
| Resolution | 240 x 320 |
| Interface | SPI |
| SPI host | `SPI2_HOST` |
| MISO | GPIO48 |
| MOSI | GPIO47 |
| SCLK | GPIO21 |
| CS | GPIO42 |
| DC | GPIO41 |
| RST | `-1`, no dedicated reset GPIO |
| Pixel clock | 27 MHz |
| Pixel format | RGB565 |
| Element order | BGR |
| Touch | Disabled in the received S3 project |

Existing LCD APIs:

```c
esp_err_t lcd_display_init(void);
void lcd_fill_screen(uint16_t color);
void lcd_fill_rect(int x, int y, int w, int h, uint16_t color);
void lcd_draw_rect(int x, int y, int w, int h, uint16_t color);
void lcd_draw_string(int x, int y, const char *text, uint16_t fg, uint16_t bg, int scale);
void lcd_draw_string_center(int cx, int y, const char *text, uint16_t fg, uint16_t bg, int scale);
void lcd_show_picture(const uint16_t *img);
void lcd_show_picture_area(int x, int y, int w, int h, const uint16_t *img);
void lcd_show_echoforest_demo(int light, int temp, int humi, int noise, const char *state);
void lcd_show_animation_frame(int frame_index);
void lcd_play_boot_animation(void);
```

Important current defect:

- The received `app_main()` starts the animation task but does not call `lcd_display_init()` first.
- Therefore the integration must initialize and verify the panel before any screen or animation task starts.

### 4.3 Existing S3 animation format

The received project already contains an RLE animation implementation:

| Item | Verified value |
|---|---|
| Frame count | 10 |
| Frame size | Full-screen 240 x 320 |
| Source format in firmware | RLE pairs of `{count, RGB565 color}` |
| Playback API | `lcd_show_animation_frame()` and `lcd_play_boot_animation()` |
| Existing frame delay | 10 ms |

The existing 10 ms delay makes the whole ten-frame sequence approximately 100 ms and is too fast for a useful visual test. The next integration task must make the delay configurable and use a visibly testable value, normally 80-150 ms per frame.

### 4.4 BH1750 light sensor

| Item | Verified value |
|---|---|
| I2C port | `I2C_NUM_0` |
| SDA | GPIO5 |
| SCL | GPIO4 |
| Bus frequency | 100 kHz |
| Address | `0x23` |
| Mode | High-resolution mode, command `0x10` |
| Read API | `bh1750_read_data()` |
| Conversion currently used | raw value divided by 1.2 |
| Intended unit | Approximate lux |

Required integration correction:

- The current log text says `klx`; this is wrong for the current conversion and should be `lux`.
- The current read function returns zero on I2C failure, which is indistinguishable from true darkness. The integrated sensor adapter must expose validity/error state and must not silently upload a communication failure as a valid zero-lux sample.

### 4.5 DHT11 temperature and humidity sensor

| Item | Verified value |
|---|---|
| DATA GPIO in latest package | GPIO0 |
| GPIO mode | Open-drain input/output with pull-up |
| Read API | `dht11_read_data(&temperature, &humidity)` |
| Current data type | Integer temperature and humidity |
| Current demo interval | Approximately one second |

Required integration corrections:

- GPIO0 is a boot-strapping pin. The field team must verify that the attached DHT11 does not hold it low during reset.
- The current driver returns success even when checksum validation fails. It can leave output values unchanged or uninitialized.
- The integrated adapter must only publish a new sample after handshake and checksum success. On failure it must retain the last valid sample with a stale/error indicator or omit the upload.

The older GPIO13 claim is superseded by the received source package. GPIO13 is also used as camera PCLK in that package, so it is not automatically available unless the camera is disabled and field wiring is intentionally changed.

### 4.6 INMP441 microphone

| Item | Verified value |
|---|---|
| I2S port | `I2S_NUM_0` |
| BCLK | GPIO35 |
| WS / LRCL | GPIO36 |
| DIN | GPIO37 |
| Sample rate | 16000 Hz |
| Sample width | 32 bit |
| Channel mode | Mono |
| Slot mask | Left |
| Block size | 512 samples |
| Approximate raw window | 32 ms |
| Read API | `i2s_mic_read_rms(float *rms_out)` |

The existing algorithm:

- shifts the 32-bit sample by 16 bits;
- removes DC mean;
- rejects extreme spikes;
- computes RMS;
- applies exponential smoothing with 0.55 previous / 0.45 new;
- computes the current demo value as `20 * log10(rms + 1)`.

The resulting value is not calibrated acoustic dB SPL. It is a logarithmic digital amplitude score. The final backend threshold must be based on onsite measurements for quiet, normal speech, and clearly noisy conditions.

The INMP441 L/R selection pin must be physically checked. The code reads the left slot, so the microphone module must be wired for the left channel or the slot mask must be changed.

### 4.7 Current S3 Wi-Fi and UDP behavior

The received project currently:

- connects as STA to SSID `C5_AP`;
- uses test password `12345678`;
- sends UDP to `192.168.4.1:3333`;
- uses `S3_CONNECTED` / `ACK_FROM_C5` as a direct S3-C5 handshake;
- starts a camera web server after Wi-Fi connection.

This is a demo topology and is not the final EchoForest state path.

Default decision for the next integration task:

- The PC backend, S3, and C5 should connect to the same ordinary LAN or phone hotspot.
- S3 and C5 should access the backend with the PC LAN address, for example `http://192.168.x.x:3001`.
- `localhost` and `127.0.0.1` are invalid backend addresses on the ESP32 devices.
- Direct UDP must not carry session or forest state. It may remain disabled or diagnostic-only.
- Wi-Fi credentials and backend URL must be local configuration, not committed secrets.

If the field environment requires C5 AP + NAT instead, that topology must be explicitly confirmed and tested; it must not be silently assumed.

### 4.8 Camera conflict

The received camera configuration uses:

```text
SIOD GPIO4
SIOC GPIO5
PCLK GPIO13
```

BH1750 uses the same GPIO4/GPIO5 pair for I2C. The camera and BH1750 cannot be assumed to coexist safely with the current independent driver initialization.

Default decision for the first complete real-device loop:

- Camera support is disabled by default.
- GPIO4/GPIO5 are reserved for BH1750.
- Camera code may remain in reference source but must not block or destabilize the learning-environment loop.

If camera functionality is later required, move the BH1750 bus to two verified free pins or redesign the shared-bus ownership after hardware validation.

## 5. Verified ESP32-C5 baseline

The existing `docs/C5_PACKAGE_CAPABILITY_SUPPLEMENT.md` remains useful, but this section contains the concise authoritative values already verified from the C5 package.

| Item | Verified value |
|---|---|
| Target | ESP32-C5 |
| ESP-IDF | 5.5.4 |
| Screen | P183B001, 240 x 284 |
| LCD controller | ST7789/ST7789V-style initialization |
| SPI host | `SPI2_HOST` |
| LCD MOSI | GPIO23 |
| LCD SCLK | GPIO24 |
| LCD CS | GPIO25 |
| LCD DC / RS | GPIO26 |
| LCD backlight | GPIO5 |
| LCD MISO | Not used |
| LCD reset | Software reset command, no dedicated reset pin in the inspected code |
| Touch interface | I2C |
| Touch port | `I2C_NUM_0` |
| Touch SDA | GPIO2 |
| Touch SCL | GPIO3 |
| Touch address | `0x15` |
| Touch point-count register | `0x02` |
| Touch coordinate register | `0x03` |
| Coordinate transform | `y = LCD_HEIGHT - 1 - raw_y` |
| TTS API found | `tts_say(const char *text)` |
| Audio output | PDM/I2S speaker path |
| PA control | GPIO1 |
| PDM_P | GPIO7 |
| PDM_N | GPIO8 |
| TTS sample rate | 16000 Hz |

The file named `mic.c` in the inspected C5 SPEAKER component is TTS/audio playback code, not an INMP441-style environment microphone capture implementation.

The C5 package currently contains hard-coded Wi-Fi demo values and AP+STA/NAPT behavior. The integrated build must use configuration and the backend as the state center.

## 6. Known integration blockers and required field decisions

The following values cannot be proven from source alone and must be checked on the real assembled devices:

1. The S3 LCD is actually wired to GPIO48/47/21/42/41 as the latest code expects.
2. DHT11 on GPIO0 does not interfere with reset/boot.
3. INMP441 L/R is strapped for the left slot.
4. BH1750 is present at address `0x23` on GPIO5/GPIO4.
5. Camera is disconnected or disabled while BH1750 uses GPIO4/GPIO5.
6. The final backend PC LAN IP and TCP port are reachable from both boards.
7. Windows firewall permits inbound backend traffic on port 3001.
8. The real INMP441 quiet/speech/noisy values are recorded for threshold calibration.
9. The 2 MB flash build fits after combining HTTP, sensors, LCD, and selected animation assets.
10. The C5 screen orientation and touch coordinate transform match the physical orientation.

Use `docs/FIELD_HARDWARE_CHECKLIST.md` to record these results.

## 7. Integration decisions that must not regress

- Backend remains the only state center.
- Current repository API paths and enum names remain canonical.
- Real hardware errors must be distinguishable from valid sensor values.
- Camera is non-goal for the first real closed loop and disabled by default.
- Direct S3-C5 UDP is not a business-state channel.
- Both boards must support configurable Wi-Fi and backend URL.
- S3 main-screen animation is local; backend sends state, not video frames.
- Animation playback must not block sensor upload or backend polling.
- TTS must not replay on every poll; it triggers on state transition with deduplication.
