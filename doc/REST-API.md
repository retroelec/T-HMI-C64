# REST API for the T-HMI-C64 Emulator

## Overview

The REST API allows external control of the C64 emulator via HTTP.
It works only on Linux.

**Main use cases:**
- Automated testing (e.g., an LLM fixes a bug, starts the emulator, loads a program, and verifies the result)
- MCP server integration (tools for loading, starting, and controlling C64 programs)
- Remote control of the emulator via HTTP

The server runs in its own thread and communicates thread-safely through the
`ExtCmdQueue` with the emulator's main thread.

## Starting

The REST API server starts automatically with the Linux build of the emulator:

```bash
./c64linux
```

The server listens on **localhost:8080** (not accessible from outside).
The port is currently hardcoded to 8080 (change it in `src/c64linux.cpp` if needed).

## Endpoint Reference

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/load` | Loads and starts a PRG program |
| POST | `/api/reset` | Performs a cold reset of the emulator |
| POST | `/api/pause` | Toggles between pause and resume |
| POST | `/api/key` | Simulates a key press on the C64 keyboard |
| POST | `/api/joy` | Sets the joystick value (keyboard joystick mode) |
| GET | `/api/status` | Returns the emulator status as JSON |
| GET | `/api/registers` | Returns CPU and VIC/CIA registers as JSON |
| GET | `/api/screen/text` | Returns the text screen content as JSON |
| GET | `/api/screen/ppm` | Returns the complete screen as a PPM image |

---

### POST /api/load

Loads a PRG file from the configured program directory (default: `c64prgs/`)
and auto-starts it.

**Request body (JSON):**

```json
{
    "filename": "game.prg"
}
```

The filename may include or omit the `.prg` extension.

**Example:**

```bash
curl -X POST http://localhost:8080/api/load \
  -H 'Content-Type: application/json' \
  -d '{"filename": "game.prg"}'
```

---

### POST /api/reset

Performs a cold reset of the C64 (resets CPU, VIC, CIA, SID).

**Request body:** `{}` (empty)

**Example:**

```bash
curl -X POST http://localhost:8080/api/reset \
  -H 'Content-Type: application/json' \
  -d '{}'
```

---

### POST /api/pause

Toggles between pausing and resuming emulation.
After a pause command, the emulator is halted; a subsequent call resumes it.

**Request body:** `{}` (empty)

**Example:**

```bash
curl -X POST http://localhost:8080/api/pause \
  -H 'Content-Type: application/json' \
  -d '{}'
```

---

### POST /api/key

Simulates a key press on the C64 keyboard. The key remains active for a
configurable number of scans (1 scan = 8 ms).

**Request body (JSON):**

```json
{
    "dc00": 254,
    "dc01": 191,
    "shift": 0,
    "duration": 3
}
```

| Field | Type | Description |
|-------|------|-------------|
| `dc00` | int | C64 matrix column (see keycodes below) |
| `dc01` | int | C64 matrix row (see keycodes below) |
| `shift` | int (optional) | Shift/Ctrl/Commodore code: bit 0=Shift, bit 1=Ctrl, bit 2=Commodore (default: 0) |
| `duration` | int (optional) | Number of scans (8 ms/scan), default: 3 (~24 ms) |

**Example – press key `A`:**

```bash
curl -X POST http://localhost:8080/api/key \
  -H 'Content-Type: application/json' \
  -d '{"dc00": 253, "dc01": 251, "shift": 0}'
```

**Example – press `Shift`+`A`:**

```bash
curl -X POST http://localhost:8080/api/key \
  -H 'Content-Type: application/json' \
  -d '{"dc00": 253, "dc01": 251, "shift": 1}'
```

---

### POST /api/joy

Sets the joystick value in keyboard joystick mode. The value remains active until
a new command overwrites it. Send `255` to release all directions.

**Request body (JSON):**

```json
{
    "value": 239
}
```

| Field | Type | Description |
|-------|------|-------------|
| `value` | int | Joystick bitfield: bit 0=Up, 1=Down, 2=Left, 3=Right, 4=Fire1. 255 = release all |

**Joystick bitfield values:**

| Value | Meaning |
|-------|---------|
| 255 | neutral (all released) |
| 239 | Fire pressed (255 - (1<<4)) |
| 251 | Up pressed (255 - (1<<0)) |
| 127 | Up + Fire (255 - (1<<0) - (1<<4)) |
| 254 | Down pressed |
| 253 | Left pressed |
| 247 | Right pressed |

**Example – press fire:**

```bash
curl -X POST http://localhost:8080/api/joy \
  -H 'Content-Type: application/json' \
  -d '{"value": 239}'
```

**Example – release joystick:**

```bash
curl -X POST http://localhost:8080/api/joy \
  -H 'Content-Type: application/json' \
  -d '{"value": 255}'
```

> **Note:** For joystick control via REST API to work, the keyboard joystick mode
> must be active (not enabled by default). Enable it via the emulator key combo
> `RCTRL+K` or set `kbjoystickmode` in the source code.

---

### GET /api/status

Returns the current emulator status as JSON.

**Example:**

```bash
curl -s http://localhost:8080/api/status | python3 -m json.tool
```

**Response:**

```json
{
    "running": true,
    "paused": false,
    "joystickMode": 0,
    "kbJoystickMode": 0,
    "volume": 10,
    "perf": false,
    "debug": false,
    "deactivateTemp": false,
    "detectReleaseKey": true
}
```

| Field | Type | Description |
|-------|------|-------------|
| `running` | bool | Emulator is running (not paused) |
| `paused` | bool | Emulator is paused |
| `joystickMode` | int | Joystick mode (0=off, 1=port 1, 2=port 2) |
| `kbJoystickMode` | int | Keyboard joystick mode (0=off, 1=port 1, 2=port 2) |
| `volume` | int | Volume (0–255) |
| `perf` | bool | Performance display active |
| `debug` | bool | Debug mode active |
| `deactivateTemp` | bool | Deactivate-temp mode active |
| `detectReleaseKey` | bool | Key release detection active |

---

### GET /api/registers

Returns the current CPU registers and important VIC/CIA registers as JSON.

**Example:**

```bash
curl -s http://localhost:8080/api/registers | python3 -m json.tool
```

**Response:**

```json
{
    "pc": 64885,
    "a": 85,
    "x": 0,
    "y": 80,
    "sr": 0,
    "d011": 27,
    "d016": 5,
    "d018": 21,
    "d019": 113,
    "d01a": 240,
    "register1": 231,
    "dc0d": 129,
    "dc0e": 17,
    "dc0f": 8,
    "dd0d": 0,
    "dd0e": 8,
    "dd0f": 8
}
```

| Field | Description |
|-------|-------------|
| `pc` | Program counter (16 bit) |
| `a`, `x`, `y` | CPU accumulator and index registers |
| `sr` | Status register (flags) |
| `d011`–`d01a` | VIC-II registers (video chip) |
| `register1` | CPU port 1 (memory configuration) |
| `dc0d`–`dc0f` | CIA #1 registers (timers, interrupts) |
| `dd0d`–`dd0f` | CIA #2 registers (timers, interrupts) |

---

### GET /api/screen/text

Returns the C64 screen content as a 40×25 character grid in JSON format.
This endpoint reads directly from video RAM ($0400) and color RAM ($D800)
and is therefore **independent of the current video mode** – it always shows
the text RAM contents.

**Example:**

```bash
curl -s http://localhost:8080/api/screen/text | python3 -c "
import sys, json
d = json.load(sys.stdin)
for row in d['chars']:
    line = ''.join(chr(c) if 32 <= c < 128 else '.' for c in row)
    print(line)
"
```

**Response:**

```json
{
    "cols": 40,
    "rows": 25,
    "chars": [[0, 0, ...], [32, 32, ...], ...],
    "colors": [[0, 0, ...], [14, 14, ...], ...]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `cols` | int | Columns (always 40) |
| `rows` | int | Rows (always 25) |
| `chars` | array | 25×40 PETSCII character codes (0–255) |
| `colors` | array | 25×40 color indices (0–15) |

The characters are PETSCII codes:
- 0–31: Control characters (also used for umlauts and graphic characters in the C64 charset)
- 32–63: Space, digits, special characters
- 64–95: Uppercase letters, @, [, \, ], ^, _
- 96–127: Lowercase letters
- 128–255: Reverse variants of the first 128 characters

The color indices correspond to the 16 C64 colors:

| Index | Color |
|-------|-------|
| 0 | Black |
| 1 | White |
| 2 | Red |
| 3 | Cyan |
| 4 | Purple |
| 5 | Green |
| 6 | Blue |
| 7 | Yellow |
| 8 | Orange |
| 9 | Brown |
| 10 | Light Red |
| 11 | Dark Gray |
| 12 | Medium Gray |
| 13 | Light Green |
| 14 | Light Blue |
| 15 | Light Gray |

---

### GET /api/screen/ppm

Returns the complete screen (320×200 pixels) as a **PPM image** (Portable Pixmap, binary).
The format is `P6` with 8 bits per color channel (RGB).

**Example:**

```bash
# Save to file
curl -s http://localhost:8080/api/screen/ppm -o screenshot.ppm

# Convert with ImageMagick (if available)
curl -s http://localhost:8080/api/screen/ppm | convert ppm:- screenshot.png
```

**Response headers:**
- `Content-Type: image/x-portmap`
- Content-Length = 192,015 bytes (header 15 + 320×200×3 = 192,000)

The image is generated from the raw framebuffer of the VIC-II chip.
The 16 C64 colors are converted to 24-bit RGB.

---

## Typical LLM/MCP Workflow

An LLM that wants to automatically fix a bug and test it could use the
following workflow:

```python
import requests, time, json

BASE = "http://localhost:8080"

def wait_for_text(text, timeout=10):
    """Wait until a specific text appears on the C64 screen."""
    for _ in range(timeout * 5):  # check every 200ms
        resp = requests.get(f"{BASE}/api/screen/text")
        data = resp.json()
        screen_text = ""
        for row in data["chars"]:
            screen_text += "".join(chr(c) if 32 <= c < 128 else "." for c in row)
            screen_text += "\n"
        if text in screen_text:
            return True
        time.sleep(0.2)
    return False

def type_text(text):
    """Type text on the C64 keyboard."""
    # Simplified: each character would need to be sent via /api/key
    keycodes = {
        "A": {"dc00": 253, "dc01": 251},
        "B": {"dc00": 247, "dc01": 239},
        # ... full table in the project documentation
    }
    for ch in text.upper():
        if ch in keycodes:
            requests.post(f"{BASE}/api/key", json=keycodes[ch])
            time.sleep(0.05)
    # RETURN
    requests.post(f"{BASE}/api/key", json={"dc00": 254, "dc01": 253})

# 1. Reset
requests.post(f"{BASE}/api/reset", json={})

# 2. Wait for READY.
wait_for_text("READY.")

# 3. Load program
requests.post(f"{BASE}/api/load", json={"filename": "test.prg"})

# 4. Wait for loading to complete
wait_for_text("READY.")

# 5. Start the program
type_text("RUN")
```

---

## C64 Keycode Reference (Important Keys)

Each key on the C64 keyboard is defined by two values:
- **dc00** (matrix column)
- **dc01** (matrix row)

| Key | dc00 | dc01 | Shift Code |
|-----|------|------|------------|
| `A` | 253 (0xFD) | 251 (0xFB) | – |
| `B` | 247 (0xF7) | 239 (0xEF) | – |
| `C` | 251 (0xFB) | 239 (0xEF) | – |
| `D` | 251 (0xFB) | 251 (0xFB) | – |
| `E` | 253 (0xFD) | 191 (0xBF) | – |
| `F` | 251 (0xFB) | 223 (0xDF) | – |
| `G` | 247 (0xF7) | 251 (0xFB) | – |
| `H` | 247 (0xF7) | 223 (0xDF) | – |
| `I` | 239 (0xEF) | 253 (0xFD) | – |
| `J` | 239 (0xEF) | 251 (0xFB) | – |
| `K` | 239 (0xEF) | 223 (0xDF) | – |
| `L` | 223 (0xDF) | 251 (0xFB) | – |
| `M` | 239 (0xEF) | 239 (0xEF) | – |
| `N` | 239 (0xEF) | 127 (0x7F) | – |
| `O` | 239 (0xEF) | 191 (0xBF) | – |
| `P` | 223 (0xDF) | 253 (0xFD) | – |
| `Q` | 127 (0x7F) | 191 (0xBF) | – |
| `R` | 251 (0xFB) | 253 (0xFD) | – |
| `S` | 253 (0xFD) | 223 (0xDF) | – |
| `T` | 251 (0xFB) | 191 (0xBF) | – |
| `U` | 247 (0xF7) | 191 (0xBF) | – |
| `V` | 247 (0xF7) | 127 (0x7F) | – |
| `W` | 253 (0xFD) | 253 (0xFD) | – |
| `X` | 251 (0xFB) | 127 (0x7F) | – |
| `Y` | 247 (0xF7) | 253 (0xFD) | – |
| `Z` | 253 (0xFD) | 239 (0xEF) | – |
| `0` | 239 (0xEF) | 247 (0xF7) | – |
| `1` | 127 (0x7F) | 254 (0xFE) | – |
| `2` | 127 (0x7F) | 247 (0xF7) | – |
| `3` | 253 (0xFD) | 254 (0xFE) | – |
| `4` | 253 (0xFD) | 247 (0xF7) | – |
| `5` | 251 (0xFB) | 254 (0xFE) | – |
| `6` | 251 (0xFB) | 247 (0xF7) | – |
| `7` | 247 (0xF7) | 254 (0xFE) | – |
| `8` | 247 (0xF7) | 247 (0xF7) | – |
| `9` | 239 (0xEF) | 254 (0xFE) | – |
| RETURN | 254 (0xFE) | 253 (0xFD) | – |
| SPACE | 127 (0x7F) | 239 (0xEF) | – |
| RUN/STOP | 127 (0x7F) | 127 (0x7F) | 0x80 |
| F1 | 254 (0xFE) | 239 (0xEF) | – |
| F3 | 254 (0xFE) | 223 (0xDF) | – |
| F5 | 254 (0xFE) | 191 (0xBF) | – |
| F7 | 254 (0xFE) | 247 (0xF7) | – |
| CURSOR RIGHT | 254 (0xFE) | 251 (0xFB) | – |
| CURSOR LEFT | 254 (0xFE) | 251 (0xFB) | 1 |
| CURSOR DOWN | 254 (0xFE) | 127 (0x7F) | – |
| CURSOR UP | 254 (0xFE) | 127 (0x7F) | 1 |
| DEL | 254 (0xFE) | 254 (0xFE) | – |
| INS | 254 (0xFE) | 254 (0xFE) | 1 |
| HOME | 191 (0xBF) | 247 (0xF7) | – |
| CLR | 191 (0xBF) | 247 (0xF7) | 1 |
| SHIFT (left) | – | – | Bit 0 |
| SHIFT (right) | – | – | Bit 0 |
| CTRL | – | – | Bit 1 |
| COMMODORE | – | – | Bit 2 |

The Shift/Ctrl/Commodore modifiers are controlled via the `shift` parameter.
For uppercase letters (`Shift`+`A` = `A` in C64 mode): set `shift: 1`.

> **Note:** In C64 BASIC mode, **unshifted** letters (e.g., `dc00=253, dc01=251, shift=0`)
> produce lowercase characters, while **shifting** produces uppercase. In **Shift Lock** mode
> (Commodore+Shift) the mapping is reversed.

---

## Build Notes

The REST API is only enabled in the Linux build (`PLATFORM_LINUX`).
It uses **no external libraries** – only POSIX sockets and
`nlohmann/json.hpp` (already included in the project).

```bash
# Standard build (includes REST API automatically)
make cleanlinux && make

# Run
./c64linux
```

The server starts automatically and logs:

```
[I][RestAPI] started on port 8080 (localhost only)
[I][RestAPI] listening on http://localhost:8080
```

---

## Implemented Files

| File | Description |
|------|-------------|
| `src/RestAPI.h` | Header of the REST API class |
| `src/RestAPI.cpp` | Implementation of the HTTP server and all endpoints |
| `src/ExtCmd.h` | Extended with `SENDKEY` and `SENDJOY` commands |
| `src/C64Sys.h` | New `restJoyValue` atomic |
| `src/C64Sys.cpp` | Evaluation of `restJoyValue` in the keyboard joystick path |
| `src/ExternalCmds.cpp` | Processing of `SENDKEY` and `SENDJOY` |
| `src/VIC.h` | New `getBitmap()` getter |
| `src/c64linux.cpp` | Integration of the REST API startup |
