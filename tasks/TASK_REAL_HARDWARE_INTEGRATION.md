# Task: Complete Real S3/C5 Hardware Integration in One Goal Run

## 1. Task type

This is one complete goal-mode implementation task.

Do not split the requested product result into a sequence of future tasks. You may internally plan and checkpoint, but continue until all code that can be completed from the repository and supplied hardware references is implemented, reviewed, and verified.

## 2. Final goal

Turn the current software loop and firmware business modules into a field-debuggable real-device EchoForest build.

At completion, the team must be able to take the PC, ESP32-S3, ESP32-C5, screens, and sensors onsite and perform this real loop:

```text
C5 touch starts focus
-> backend confirms focus
-> S3 reads real light/noise/temp/humi
-> S3 uploads environment data
-> backend computes forest_state
-> web, C5, and S3 show the same backend state
-> C5 can enter break and end focus
-> backend generates a log
-> S3 can play a clear local forest-growth animation without breaking communication
```

## 3. Must-read files

Read before changing code:

```text
README.md
docs/API_CONTRACT.md
docs/HARDWARE_SOURCE_OF_TRUTH.md
docs/FIELD_HARDWARE_CHECKLIST.md
docs/ANIMATION_TEST_ASSET_SPEC.md
docs/C5_PACKAGE_CAPABILITY_SUPPLEMENT.md
hardware_reference/README.md
firmware_modules/common/*
firmware_modules/c5/*
firmware_modules/s3/*
backend/src/*
backend/test/*
frontend/src/*
simulator/*
```

Decode and inspect the S3 reference package:

```cmd
scripts\decode_s3_reference.cmd
```

Treat `docs/HARDWARE_SOURCE_OF_TRUTH.md` and the decoded latest S3 package as authoritative when older assumptions conflict.

## 4. Required implementation result

### 4.1 Repository structure

Create maintainable ESP-IDF build targets or integration projects for both boards. Do not leave the result as host-only business modules.

The final structure must make it clear how to:

- build S3 firmware;
- flash S3 firmware;
- build C5 firmware;
- flash C5 firmware;
- configure Wi-Fi and backend URL locally;
- run backend/frontend for onsite testing;
- run a complete smoke test.

Reuse `firmware_modules/common`, `firmware_modules/s3`, and `firmware_modules/c5` rather than duplicating the state protocol in independent projects.

### 4.2 S3 real integration

Implement and wire:

- NVS and Wi-Fi STA initialization;
- configurable SSID/password/backend URL;
- backend heartbeat;
- real BH1750 light read;
- corrected DHT11 read with checksum failure handling;
- real INMP441 RMS/log-amplitude read;
- sensor validity and stale/error handling;
- environment upload to `/api/device_data`;
- backend state polling;
- ST7789 LCD initialization before rendering;
- state-to-main-screen rendering;
- local animation playback;
- reconnect/retry behavior that does not block forever.

Default to camera disabled. Do not allow camera startup to conflict with BH1750 GPIO4/GPIO5 or consume resources needed by the core loop.

Direct UDP must not be used to synchronize session or forest state.

### 4.3 C5 real integration

Use the existing C5 reference package and current C5 business layer to implement and wire:

- real Wi-Fi/backend configuration;
- backend heartbeat;
- real I2C touch read;
- touch coordinate transform and debounce;
- start-focus, start-break, and end-focus requests;
- backend state polling;
- real ST7789V screen rendering;
- clear touch buttons and current state/environment display;
- TTS or prompt playback on meaningful state transition only;
- nonblocking or bounded audio behavior.

A local touch must not update the final UI state until the backend request/state confirms it.

### 4.4 Backend and web support for field debugging

Preserve the current API contract.

Add only the minimum field-debug support needed, such as:

- clear device last-seen/online information;
- sensor validity or stale status if required by the hardware client contract;
- useful error messages;
- configuration guidance;
- an onsite diagnostics view or fields when this materially reduces debugging time.

Do not replace the backend state center with direct S3-C5 communication.

### 4.5 Animation smoke asset

Provide a license-safe, reproducible forest-growth animation following `docs/ANIMATION_TEST_ASSET_SPEC.md`.

Required:

- 8-16 visible frames;
- source frames;
- conversion script;
- generated firmware asset;
- configurable frame delay, default about 100 ms;
- a bounded playback test;
- size report;
- proof that sensor/backend tasks continue during playback.

The existing reference RLE decoder may be reused or improved. Do not commit an unexplained opaque asset with no reproducible conversion path.

## 5. Authoritative hardware values

Use the values in `docs/HARDWARE_SOURCE_OF_TRUTH.md`, including:

- S3 LCD ST7789, 240 x 320, SPI2, MISO48/MOSI47/SCLK21/CS42/DC41;
- BH1750 I2C0, SDA5/SCL4, address 0x23;
- DHT11 current package pin GPIO0, with boot-risk handling and configurable override;
- INMP441 I2S0, BCLK35/WS36/DIN37, 16 kHz, 32-bit mono left;
- C5 LCD and touch pin maps;
- current API paths, payload names, mode values, and forest-state values.

If the real field wiring differs, centralize the override in configuration and update `docs/FIELD_HARDWARE_CHECKLIST.md`. Do not silently scatter changed pin values.

## 6. Constraints

1. Backend remains the only state center.
2. Preserve the canonical API unless a proven blocker requires a coordinated migration with tests and documentation.
3. Do not commit real Wi-Fi credentials or a personal LAN IP.
4. Do not use `localhost` or `127.0.0.1` as an ESP32 backend URL.
5. Camera is disabled by default for this task.
6. Direct UDP is diagnostic-only or removed from the integrated runtime.
7. Sensor read failure is not a valid environmental measurement.
8. DHT11 checksum failure must not publish uninitialized data.
9. TTS must be deduplicated by state transition.
10. Animation must not be an infinite tight loop and must not starve network/sensor tasks.
11. Keep all generated asset steps reproducible.
12. Do not add large application frameworks unrelated to the hardware loop.
13. Do not remove existing backend, frontend, simulator, or host firmware tests.
14. Do not claim real-hardware PASS without actual build output and a clearly labeled field verification status.

## 7. Required configuration behavior

Provide one documented configuration path for each board, using Kconfig, sdkconfig defaults, or a local untracked config header.

It must cover at least:

```text
Wi-Fi SSID
Wi-Fi password
backend base URL
device ID
device role
sensor polling interval
backend polling interval
heartbeat interval
S3 pin overrides
C5 pin/orientation overrides
camera enabled flag, default false
animation frame delay
```

Repository examples must contain placeholders only.

## 8. Required tests and verification

### 8.1 Existing software verification

Run and preserve:

```text
backend tests
frontend build/tests
simulator tests
host firmware business-logic tests
```

### 8.2 ESP-IDF verification

For both S3 and C5:

- clean configure;
- compile with the intended ESP-IDF version;
- report target and sdkconfig used;
- report binary and partition sizes;
- report warnings and failures;
- ensure the build fits configured flash.

### 8.3 Static contract verification

Add a review script that verifies at least:

- canonical API paths are present;
- old API/state names are not accidentally reintroduced into active firmware;
- no real credentials are tracked;
- camera defaults off;
- S3/C5 configs exist;
- animation source and conversion script exist;
- field checklist and runbook exist;
- existing software directories were not accidentally replaced.

### 8.4 Hardware-independent integration test

Use mock transport and/or a local backend to verify:

- C5 touch action -> backend control endpoint -> confirmed state;
- S3 environment sample -> backend -> computed state;
- state parsing and rendering frame generation;
- TTS state-transition deduplication;
- animation decoder bounds and frame count;
- retry behavior.

### 8.5 Field runbook

Produce a concise `docs/FIELD_RUNBOOK.md` with exact CMD commands and observable expected results for:

- backend startup;
- frontend startup;
- finding the PC LAN IP;
- firewall check;
- board configuration;
- build and flash;
- serial monitor;
- sensor calibration;
- complete demo loop;
- rollback to simulator if a device is blocked.

## 9. Acceptance criteria

The task is complete only when all code-supported acceptance items are met:

1. S3 and C5 have real ESP-IDF buildable entry projects.
2. Both use the existing common protocol/cloud logic rather than duplicate incompatible APIs.
3. S3 builds with real LCD and all three sensor adapters.
4. C5 builds with real LCD, touch, and speaker/TTS integration.
5. Wi-Fi/backend config is externalized.
6. Camera is disabled by default.
7. S3 can upload a real or hardware-adapter sample and parse backend state.
8. C5 can map a real touch point to a backend-confirmed action.
9. Both render the same canonical backend state.
10. Learning end creates a backend log visible on the web.
11. A reproducible forest-growth animation asset builds and can be played at a human-visible speed.
12. Playback does not block the core communication loop by design and test.
13. Existing software tests pass.
14. Host firmware tests pass.
15. S3 and C5 ESP-IDF builds pass, or a precise external toolchain blocker is documented as BLOCKED.
16. Field wiring and runbook documents are complete.
17. Git status contains only intended changes.

Actual physical behavior must be reported separately as:

```text
NOT YET FIELD-TESTED
FIELD-TESTED PASS
FIELD-TESTED RISKY
FIELD-TESTED BLOCKED
```

Do not convert a compile-only result into a false field-test claim.

## 10. Rollback

The existing backend/frontend/simulator loop must remain runnable throughout the task.

If real hardware is blocked onsite:

- keep backend and web unchanged;
- use the simulator to demonstrate the state loop;
- switch S3 to mock sensor adapter if necessary;
- use static screen/status output if animation is blocked;
- document the exact hardware blocker without inventing a success.

## 11. Final report format

Report:

```text
Changed files
Architecture decisions
S3 integration result
C5 integration result
Animation asset result
Backend/frontend/simulator verification
Host firmware test result
S3 ESP-IDF build result
C5 ESP-IDF build result
Binary/flash size result
Field-test status
Open risks
Git status --short --untracked-files=all
Final verdict: PASS / RISKY / BLOCKED
```

Use `PASS` only for the code/build task when its acceptance criteria are satisfied. Keep physical field-test status explicit and separate.
