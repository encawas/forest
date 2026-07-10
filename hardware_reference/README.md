# Hardware Reference Inputs

This directory stores source packages that are useful for real-hardware integration but are not themselves the canonical EchoForest application structure.

## S3 base project

The base64 archive is stored in these ordered segments:

```text
s3_base_project_20260710.zip.b64.part00
s3_base_project_20260710.zip.b64.part01
s3_base_project_20260710.zip.b64.part02a
s3_base_project_20260710.zip.b64.part02b
s3_base_project_20260710.zip.b64.part02c
s3_base_project_20260710.zip.b64.part02d
s3_base_project_20260710.zip.b64.part03
s3_base_project_20260710.zip.b64.part04
s3_base_project_20260710.zip.b64.part05
s3_base_project_20260710.zip.b64.part06
s3_base_project_20260710.zip.b64.part07
```

Source package checksum after decoding:

```text
SHA-256: a75ff2ef026ee1d97d8b7d01eeab1fadea44af19e34c6fab8bc4668fa4ab3453
```

Decode on Windows CMD:

```cmd
scripts\decode_s3_reference.cmd
```

The script creates:

```text
hardware_reference\s3_base_project_20260710.zip
```

Then extract it into a temporary working directory. Do not unpack it directly over `firmware_modules/`.

Useful contents include:

- ESP32-S3 ESP-IDF project configuration;
- ST7789 240 x 320 LCD driver and RLE animation playback;
- BH1750 driver;
- DHT11 driver;
- INMP441 I2S/RMS code;
- Wi-Fi STA demo;
- UDP S3-C5 demo;
- optional camera web demo.

The received package contains demo credentials and a camera/BH1750 pin conflict. Read `docs/HARDWARE_SOURCE_OF_TRUTH.md` before reusing it.
