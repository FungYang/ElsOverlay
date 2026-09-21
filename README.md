# ElsOverlay

**ElsOverlay** is a customizable, external overlay for **Elsword** that shows skill cooldowns, buffs, Transcendence, Atma and distance guides directly on top of the game.

It runs entirely outside the game: it uses global keyboard input, screen capture and on-screen image recognition (template matching and an ONNX/YOLO detector). It does **not** read game memory, inject code or touch game files.

> **Disclaimer:** ElsOverlay is an independent, unofficial fan project and is not affiliated with or endorsed by KOG or the Elsword publishers. Use it at your own risk.

<!-- VERIFY: add a screenshot of the main window / a sample overlay, e.g. ![ElsOverlay](images/screenshot.png) -->

---

## Table of contents

- [Features](#features)
- [How it works](#how-it-works)
- [Compliance note](#compliance-note)
- [Requirements](#requirements)
- [Installation (release build)](#installation-release-build)
- [First-time setup](#first-time-setup)
- [Configuration](#configuration)
- [Building from source](#building-from-source)
- [Project structure](#project-structure)
- [Known limitations](#known-limitations)
- [Related projects](#related-projects)
- [Release history](#release-history)
- [License](#license)

---

## Features

Every system is independent and can be switched **ON/OFF** from the main window without restarting the application. The main window uses Italian labels (*Configura*, *Chiudi*).

| Section | What it does |
|---|---|
| **Atma** | Tracks Atma-related buffs using screen recognition. Includes a dedicated capture/zone setup and a debug window. |
| **Class Buff** | Configurable, per-class buff/cooldown tracking with custom names and icons. |
| **Distance Guides** | Lines, rectangles and circles drawn over the game, organized in groups. |
| **Buff Titles** | Cooldown tracking for the equipped title's buff. Supports transparency and pauses automatically on stage change. |
| **Buff Trascendenza** | Visual buff display for Transcendence, independent from the cooldown timer. |
| **Buff Tracker** | Buff remapping, configured through the companion app [ElsBuffRemapping](https://github.com/FungYang/ElsBuffRemapping). |

### Transcendence

- Transcendence cooldown tracking with reset and pause support.
- Screen-recognition based detection, with dedicated capture setup and precision cropping.
- Movable overlay, position saved automatically.

### Skill cooldowns

- Configurable skill overlay: skills and their keys are set from the skill configuration window.
- Title-aware tracking (Concerto, Night Parade, Setting Sun, and others), selected through a keyboard sequence or configured directly.
- Special cooldown overlay (potions and Invariant), detected via template matching and resonance text recognition.
<!-- VERIFY: confirm which skills/titles are currently supported and whether title selection is still done by key sequence or via the config window. -->

### Distance Guides

Guide types: **vertical line**, **rectangle**, **circle**.

Guides can be created, removed, configured, moved, grouped and enabled/disabled. A group can hold any number of guides and groups can be switched independently, e.g.:

```
Group: Boss Position
 ├── Player Position
 ├── Boss Position
 ├── Left Limit
 ├── Right Limit
 └── Safe Area
```

The group configuration window stays open while adding elements, so you can create several guides in a row.

---

## How it works

ElsOverlay observes the same pixels you see on screen:

1. **Global keyboard hook**: reads your key presses to start, reset and pause timers.
2. **Screen capture**: Windows Direct3D 11 / DXGI capture of the selected screen regions.
3. **Recognition**:
   - *Template matching* against reference images you save during setup (Atma, potions, Invariant, digits).
   - *YOLO detection* through **ONNX Runtime**, using the models in `models/` (`best_1080.onnx` for 1080p, `best_2k.onnx` for 2K).
4. **Overlays**: transparent, click-through-friendly Qt windows managed by a common overlay root.

It does **not**: read or modify game memory, inject code, inspect network traffic, or modify game files.

---

## Compliance note

On 12 August 2026 the Elsword.it support team (Game Admin, after consulting with the CoMa) stated that software obtaining information from the game client through data mining is to be considered illicit, while software that operates solely through screen capture is permitted.

ElsOverlay is designed to stay within that scope: it analyses only what is visible on screen and does not access the game client's internal data.

Please keep in mind that:

- This statement comes from the **Elsword.it** support team and concerns that server. Rules on other servers may differ.
- It is a support answer, not a formal license or endorsement, and rules may change over time. Always check the current terms of service of the server you play on.
- The statement refers to software that analyses *only* information obtained through screen capture. Do not extend ElsOverlay with anything that reads game memory, injects code, inspects network traffic or reads game files.

<!-- VERIFY: the support answer does not explicitly mention the global keyboard hook (globalkeyboard.*). Consider asking support to confirm it, then mention it here. -->

---

## Requirements

**To run**

- Windows 10/11 (64-bit)
- Elsword in windowed or borderless mode
- A **1080p** or **2K** game resolution (the bundled detection models target these two)
<!-- VERIFY: confirm exclusive fullscreen support and whether other resolutions work. -->

**To build**

- CMake 3.19+
- Qt 6.5+ (Core, Widgets, Concurrent)
- A C++17 compiler for Windows (MSVC recommended)
- [ONNX Runtime](https://github.com/microsoft/onnxruntime/releases) for Windows (headers and `onnxruntime.dll`)

---

## Installation (release build)

1. Download the latest archive from the [Releases](https://github.com/FungYang/ElsOverlay/releases) page.
2. Extract it anywhere you have write access.
3. Keep the folder structure intact: the executable expects `images/`, `models/` and `onnxruntime.dll` next to it.
4. Run `ElsOverlay.exe`.

---

## First-time setup

1. Start Elsword and set your final resolution and UI scale first. Changing them later invalidates saved references.
2. Start ElsOverlay and enable only the sections you need.
3. For each section, press **Configura** and follow its setup (see below).
4. Drag overlays where you want them. Positions are saved automatically.

### Moving overlays

Click and hold an overlay, drag it, release. Positions are restored on the next launch.

### Atma setup

1. Enable **Atma** and open its configuration.
2. Place the capture areas over the relevant parts of the screen.
3. Save the reference images with the configured key (default `P`), then confirm with `ENTER`.

Reference images are stored locally and reloaded at startup.
<!-- VERIFY: the Atma flow changed (atmazone* modules, red-zone proxy, debug window). Update these steps and the folder name (previously AtmaFlowBuff/). -->

### Transcendence setup

Open the Transcendence capture setup, select the region that shows the Transcendence indicator and save the reference.
<!-- VERIFY: describe the actual steps of transcendencecapturesetup / precision crop. -->

### Class Buff

`Class Buff → Configura` lets you create class configurations, each with any number of buffs (name, icon, cooldown). Use different configurations for different characters.

### Distance Guides

`Distance Guides → Configura` lets you create groups and add lines, rectangles and circles. Each guide is configured individually and can be removed without deleting the group.

### Buff Tracker

`Buff Tracker → Configura` launches the companion app **ElsBuffRemapping**. See its repository for setup.
<!-- VERIFY: the old README said this was "buffoverla.exe" (typo). Confirm the real executable name and where it must be placed. -->

---

## Controls

Keys depend on the active subsystem and only work while that subsystem is enabled.

| Key | Function |
|---|---|
| `CTRL` | Start / reset the relevant cooldown |
| `CTRL Right` | Reset cooldown systems |
| `8` | Pause / resume Transcendence cooldown |
| `G` | Begin title selection sequence |
| `↑` `←` `↓` `→` | Select title (Concerto / Night Parade / Setting Sun / Other) |
| `P` | Save a reference image during Atma setup |
| `ENTER` | Confirm configuration |
| `0`, `1`-`6` | Atma buff tracking input |

<!-- VERIFY: keys are now configurable via skillconfigwindow/keyedit. Check which of these are still hard-coded defaults and update or remove the table. -->

---

## Configuration

Everything is configured from the GUI; no manual file editing is normally needed.

- `ElsOverlay.ini` stores overlay positions and general settings.
- Reference images and per-class / per-guide configurations are stored locally next to the executable.

To reset a section, close ElsOverlay and delete the corresponding configuration file or reference folder.
<!-- VERIFY: list the actual config/reference file and folder names used today. -->

---

## Building from source

```bash
git clone https://github.com/FungYang/ElsOverlay.git
cd ElsOverlay
```

1. Download ONNX Runtime for Windows and place it as:

   ```
   third_party/onnxruntime/
   ├── include/        (headers)
   └── lib/onnxruntime.dll
   ```

2. Make sure the `models/` directory contains `best_1080.onnx` and `best_2k.onnx`. They are copied next to the executable after each build.

3. Configure and build (Qt 6.5+ kit required):

   ```bash
   cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
   cmake --build build --config Release
   ```

   Or open `CMakeLists.txt` in **Qt Creator** and build with a Qt 6 kit.

The post-build step copies `images/`, `onnxruntime.dll` and the ONNX models into the output directory. `qt_generate_deploy_app_script` is used for Qt deployment (`windeployqt`-style) on install.

---

## Project structure

```
ElsOverlay/
├── images/                     Icons and bundled images
├── models/                     ONNX (YOLO) models: best_1080.onnx, best_2k.onnx
├── third_party/onnxruntime/    ONNX Runtime headers (+ DLL, not tracked)
│
├── main.cpp, mainwindow.*      Entry point and main control window
├── globalkeyboard.*            Global keyboard hook
├── screencapture.*             Screen capture (D3D11/DXGI)
├── overlay.*, overlayroot.*    Common overlay windows and root container
│
├── skill*.*, keyedit.*         Skill overlay and skill/key configuration
├── specialcooldown*.*          Special cooldowns (potions, Invariant)
├── digitdetector.*             Digit recognition
│
├── transcendence*.*            Transcendence capture, cropping and vision manager
├── atma*.*                     Atma zone capture, worker, manager and debug tools
│
├── buff*.*                     Buff overlay and BuffVision (capture, detector, core, manager)
├── class*.*, newbuffdialog.*   Class buff configuration and editors
│
└── distanceguide*.*            Distance guides (line, rectangle, circle, groups, config)
```

---

## Known limitations

- Windows only.
- Recognition depends on resolution, display scaling, UI layout and your saved reference images. Recreate references if any of these change significantly.
- The bundled YOLO models target 1080p and 2K layouts.
- Global keyboard hooks and screen capture may behave differently with some Windows configurations, overlays or security software.

---

## Related projects

- [ElsBuffRemapping](https://github.com/FungYang/ElsBuffRemapping): companion app used by **Buff Tracker → Configura**. It analyses visual information from the screen only.

---

## Release history

| Version | Highlights |
|---|---|
| v1.18 | Transparency option and automatic pause on stage change for titles |
| v1.17 | Template matching for potions; Invariant detection via Resonance text |
| v1.15 | Debug output removed |
| v1.08 | Transcendence bug fix; fully configurable titles |

See the [Releases](https://github.com/FungYang/ElsOverlay/releases) page for the complete list.

---

## License

Released under the **GPL-3.0** license. See [`LICENSE`](LICENSE).

Developed by **FungYang**.
