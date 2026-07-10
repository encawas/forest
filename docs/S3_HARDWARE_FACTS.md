# EchoForest ESP32-S3 Hardware Facts

This document records the latest confirmed information from the newly received ESP32-S3 ESP-IDF project. When this document conflicts with earlier planning notes, this document takes precedence.

## Source-of-truth rule

- Newly received S3 project information is authoritative for the S3 board.
- Existing repository API names and state names remain authoritative for backend interaction.
- Do not silently reuse older pin mappings, older endpoint names, or older state enum names.
- Hardware facts that still require on-site confirmation are explicitly marked.

## Project facts

| Item | Confirmed value |
| --- | --- |
| Target | ESP32-S3 |
| ESP-IDF | 5.5.4 |
| Project name | `1_LED` |
| Entry file | `main/main.c` |
| Entry function | `app_main()` |
| Flash | 2 MB |
| PSRAM | Disabled |
| Partition style | Single app |
| Existing subsystems | Wi-Fi, UDP, BH1750, DHT11, INMP441, LCD, camera |
| Camera component | `espressif/esp32-camera` 2.1.7 |

## Main display

| Item | Confirmed value |
| --- | --- |
| Panel use | EchoForest S3 main forest screen |
| Resolution | 240 x 320 |
| Driver used by code | ST7789 |
| Interface | SPI |
| SPI host | `SPI2_HOST` |
| SPI clock | 27 MHz |
| Pixel format | RGB565 |
| Color order | BGR |
| Touch | Not used by current S3 project |
| Hardware reset pin | Not used; `LCD_PIN_RST = -1` |

### Display GPIO mapping from received source

| Signal | GPIO |
| --- | ---: |
| LCD MISO | 48 |
| LCD MOSI | 47 |
| LCD SCLK | 21 |
| LCD CS | 42 |
| LCD DC | 41 |
| LCD RST | -1 |

This mapping replaces earlier inferred S3 display mappings for implementation purposes. It must still be checked against the current physical unit before flashing because the source package may represent a particular wiring revision.

### Existing LCD capability

The received project already contains functions equivalent to:

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

The project includes local RLE-compressed animation support. Future real-hardware integration should reuse this capability rather than replacing the LCD driver.

### Known display entry-point defect

The received `app_main()` creates an animation task but does not clearly initialize the LCD before playback. Before using the animation path, ensure `lcd_display_init()` succeeds before any frame is rendered.

## BH1750

| Item | Confirmed value |
| --- | --- |
| Controller | ESP32-S3 |
| I2C port | `I2C_NUM_0` |
| SDA | GPIO5 |
| SCL | GPIO4 |
| I2C frequency | 100 kHz |
| Address | `0x23` |
| Mode | High-resolution mode, command `0x10` |
| Existing init | `bh1750_init()` |
| Existing read | `bh1750_read_data()` |
| Conversion | raw / 1.2, approximately lux |

The existing log label that says `klx` is incorrect; the converted value should be treated as approximately lux.

A read failure must not be represented as valid zero lux. The integrated adapter must preserve the last valid value or report sensor failure separately.

## DHT11

| Item | Confirmed value |
| --- | --- |
| Controller | ESP32-S3 |
| DATA pin in received source | GPIO0 |
| Pin mode | Open-drain input/output with pull-up |
| Existing init | `dht11_init()` |
| Existing read | `dht11_read_data(&temperature, &humidity)` |
| Returned precision | Integer temperature and humidity |
| Current sampling period | About 1 second |

GPIO0 is a boot-strapping pin. The on-site wiring must confirm that the DHT11 module does not hold GPIO0 low during reset.

The received driver must be hardened before upload to the backend:

- checksum failure must return failure;
- the caller must check the return value;
- failed reads must not upload uninitialized values;
- the last valid sample may be retained with an explicit stale/error state.

This GPIO0 mapping replaces the earlier unconfirmed GPIO13 note.

## INMP441

| Item | Confirmed value |
| --- | --- |
| Controller | ESP32-S3 |
| I2S port | `I2S_NUM_0` |
| BCLK | GPIO35 |
| WS / LRCL | GPIO36 |
| DIN | GPIO37 |
| Sample rate | 16000 Hz |
| Sample width | 32 bit |
| Channel | Mono, left slot |
| Block size | 512 `int32_t` samples |
| Approximate block duration | 32 ms |
| Existing init | `i2s_mic_init()` |
| Existing read | `i2s_mic_read_rms(float *rms_out)` |

The current processing path includes DC removal, RMS calculation, spike filtering and exponential smoothing. The candidate noise value is:

```c
noise = 20.0f * log10f(rms + 1.0f);
```

This is not calibrated dB SPL. On-site calibration must record representative values for quiet, normal speech and loud/noisy conditions before the backend threshold is finalized.

## Current network behavior in the received S3 project

The received project currently connects as a station to:

```text
SSID: C5_AP
PASSWORD: 12345678
```

It also uses direct UDP communication with C5:

```text
C5 address: 192.168.4.1
UDP port: 3333
```

The UDP handshake includes `S3_CONNECTED` and `ACK_FROM_C5`.

This is legacy/test connectivity, not the final EchoForest state architecture. The backend remains the only state center. Direct S3-C5 UDP must not become an independent source of `mode` or `forest_state`.

For real field debugging, prefer placing the PC, S3 and C5 on the same ordinary Wi-Fi or phone hotspot so both boards can directly reach the backend LAN address. If C5 AP/NAT is retained, its routability to the PC backend must be explicitly proven on site.

Real Wi-Fi credentials and the PC LAN IP must remain local configuration, not committed source.

## Camera

The received project includes an OV2640-style camera path with QVGA JPEG streaming and no PSRAM.

Camera GPIOs include:

| Signal | GPIO |
| --- | ---: |
| XCLK | 15 |
| SIOD | 4 |
| SIOC | 5 |
| D7 | 16 |
| D6 | 17 |
| D5 | 18 |
| D4 | 12 |
| D3 | 10 |
| D2 | 8 |
| D1 | 9 |
| D0 | 11 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |

BH1750 uses GPIO4/GPIO5 while the camera uses the same pins for SCCB. The first real EchoForest closed-loop build should disable camera initialization unless on-site testing proves the shared-bus setup is safe. Camera recognition is not required for the current target.

## Backend contract to use

The current repository contract takes precedence over older planning documents:

```text
GET  /api/state
GET  /api/logs
POST /api/control/start_focus
POST /api/control/start_break
POST /api/control/end_focus
POST /api/device_data
POST /api/devices/heartbeat
```

Current wire states:

```text
idle
growing
light_warning
noise_warning
comfort_warning
break
completed
```

Environment upload uses camelCase `deviceId`, not `device_id`.

Example:

```json
{
  "source": "s3",
  "deviceId": "s3-main",
  "light": 320,
  "noise": 42,
  "temp": 24,
  "humi": 50
}
```

## Remaining facts that require on-site confirmation

1. The current physical display really uses GPIO47/21/48/42/41 as in the received source.
2. The physical DHT11 is connected to GPIO0 and does not interfere with boot.
3. The chosen final network topology lets both boards reach the PC backend.
4. Windows firewall permits inbound access to the backend port.
5. Quiet, speech and loud INMP441 values are measured and recorded.
6. Camera is disabled for the first closed-loop test unless coexistence is proven.
7. The integrated firmware fits the 2 MB flash layout and builds under ESP-IDF 5.5.4.
8. LCD initialization and at least one local animation play successfully on the real S3 display.
