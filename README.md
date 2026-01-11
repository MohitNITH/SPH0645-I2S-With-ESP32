# ESP32 I2S Microphone Bit-Shift Fix (SPH0645 / INMP441)

## Overview

This repository documents and fixes a **known ESP32 I2S microphone issue** where only **1 bit of audio data appears to change**, resulting in:
- Constant `0`, `1`, or `-1` readings  
- Extremely low sensitivity  
- Audio reacting only when physically tapping the microphone  

The issue is **not a faulty microphone module**.  
It is caused by **incorrect bit alignment and I2S data format handling in ESP32**.

This workaround is based on the analysis described here:  
🔗 https://hackaday.io/project/162059-street-sense/log/160705-new-i2s-microphone

---

## Affected Hardware

- ESP32 (ESP32-WROOM-32, ESP32 Dev Module)
- I2S MEMS microphones:
  - **SPH0645**
  - **INMP441** (similar behavior)

---

## Symptoms

- Audio values stuck at `0`, `1`, or `-1`
- No response to voice, only sharp physical taps
- Appears as if microphone has 1-bit resolution
- Happens even with official Adafruit SPH0645 modules

---

## Root Cause (Important)

ESP32’s I2S peripheral:
- Always left-justifies data
- Outputs valid audio bits shifted inside a 32-bit frame
- Many examples read samples without correcting the bit shift

As explained in the Hackaday log, the **actual microphone data is present**, but:
- Shifted incorrectly
- Misinterpreted as noise or single-bit toggles

---

## The Fix / Workaround

### Key Changes Implemented

- Correct I2S configuration:
  - `I2S_COMM_FORMAT_I2S`
  - Proper sample bit depth
- Manual bit shifting of received samples
- Sign-extension correction
- Discarding invalid LSBs

This restores:
- Full dynamic range
- Proper amplitude response
- Normal microphone sensitivity

---

## Wiring (ESP32 ↔ SPH0645)

| SPH0645 Pin | ESP32 Pin |
|------------|----------|
| VDD        | 3.3V     |
| GND        | GND      |
| BCLK       | GPIO 26  |
| LRCL / WS  | GPIO 25  |
| DOUT       | GPIO 33  |
| SEL        | GND (Left Channel) |

> Pin numbers can be changed in code if required.

---

## Code

The fixed implementation is available here:

