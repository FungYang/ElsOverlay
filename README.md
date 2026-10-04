# ElsOverlay

ElsOverlay is a customizable, external overlay for Elsword that displays skill cooldowns, buffs, Transcendence, Atma, visual trackers and distance guides directly on top of the game.

It runs entirely outside the game. It uses global keyboard input, screen capture and on-screen image recognition, including template matching and ONNX/YOLO detection.

It does **not** read game memory, inject code, inspect network traffic or modify game files.

> **Disclaimer:** ElsOverlay is an independent, unofficial fan project and is not affiliated with or endorsed by KOG or the Elsword publishers. Use it at your own risk.

---

## Table of contents

- [Features](#features)
- [Custom Searcher](#custom-searcher)
- [Distance Guides](#distance-guides)
- [Resonance Gate](#resonance-gate)
- [How it works](#how-it-works)
- [Compliance note](#compliance-note)
- [Requirements](#requirements)
- [Installation](#installation-release-build)
- [First-time setup](#first-time-setup)
- [Configuration](#configuration)
- [Controls](#controls)
- [Building from source](#building-from-source)
- [Project structure](#project-structure)
- [Known limitations](#known-limitations)
- [Release history](#release-history)
- [License](#license)

---

# Features

Every system is independent and can be enabled or disabled from the main window without restarting the application.

The main window uses Italian labels such as **Configura**, **Chiudi**, **ON**, **OFF** and **Pausa**.

| Section | What it does |
|---|---|
| **Atma** | Tracks Atma-related buffs using screen recognition. Includes dedicated capture areas and configuration. |
| **Class Buff** | Configurable, per-class buff and cooldown tracking with custom names, icons and cooldown durations. |
| **Custom Searcher** | Generic screen-based template recognition system for visual UI elements. Supports custom reference images, two independent search zones, cooldowns and movable overlays. |
| **Distance Guides** | Lines, rectangles and circles drawn over the game, organized into independent groups. |
| **Buff Titles** | Cooldown tracking for the equipped title's buff. Supports transparency and automatic pause on stage change. |
| **Buff Trascendenza** | Visual buff display for Transcendence, independent from the Transcendence cooldown timer. |
| **Transcendence** | Transcendence cooldown tracking with reset and pause support. Uses screen-recognition based detection and dedicated capture/cropping tools. |
| **Skill cooldowns** | Configurable skill overlay with custom skills and keyboard assignments. Supports title-aware tracking. |
| **Special cooldowns** | Detects potions and Invariant using template matching and visual recognition of Resonance text. |
| **Resonance Gate** | Detects the visual Gate state and can automatically pause/resume supported cooldown systems. Gate-induced pausing can be independently enabled or disabled. |

---

# Custom Searcher

Custom Searcher is a generic visual recognition system designed for UI elements that are visible on screen but cannot be customized sufficiently through the game's own interface.

It works with reference images configured by the user and does not depend on game memory or internal game data.

A template can be configured with:

- a custom reference image;
- a search zone;
- a cooldown duration;
- an overlay position;
- an enabled/disabled state.

## Two search zones

Custom Searcher supports two independent screen regions:

```text
Zone 1
┌──────────────────────────────┐
│                              │
│       Search area 1          │
│                              │
└──────────────────────────────┘

Zone 2
┌──────────────────────────────┐
│                              │
│       Search area 2          │
│                              │
└──────────────────────────────┘
```

Each template can be assigned independently to **Zone 1** or **Zone 2**.

This allows different parts of the game's interface to be monitored independently while keeping a single reusable recognition system.

## Why Custom Searcher exists

Many games expose useful information visually but provide limited options for customizing or repositioning their UI.

Custom Searcher allows that visible information to be recognized externally and represented through ElsOverlay's own overlays.

It only analyzes pixels captured from the screen.

This makes the system reusable for other games or applications where similar visual recognition is useful.

---

# Distance Guides

Distance Guides provide visual positioning tools directly over the game.

Supported guide types:

- vertical line;
- rectangle;
- circle.

Guides can be:

- created;
- removed;
- configured;
- moved;
- grouped;
- enabled or disabled independently.

A group can contain any number of guides.

Example:

```text
Group: Boss Position
 ├── Player Position
 ├── Boss Position
 ├── Left Limit
 ├── Right Limit
 └── Safe Area
```

The group configuration window remains open while adding elements, allowing several guides to be created in sequence.

---

# Resonance Gate

The Resonance Gate system uses screen-based recognition to determine the Gate state.

When **Pausa** is enabled, a closed Gate can pause the supported cooldown systems and an opened Gate can resume them.

Gate-induced pausing is independent from the `DELETE` overlay visibility control.

This means Gate pausing can be disabled while the normal `DELETE` hide/show behavior remains available.

The **Pausa** preference is saved in `ElsOverlay.ini`.

---

# How it works

ElsOverlay observes the same pixels that are visible on the user's screen.

## Global keyboard hook

The application uses a global keyboard hook for actions such as:

- starting or resetting cooldowns;
- pausing and resuming supported timers;
- selecting titles;
- global reset;
- hiding and showing the overlay.

Keyboard actions are handled by the relevant subsystem and do not require access to the game's internal state.

## Screen capture

Screen regions are captured using Windows Direct3D 11 / DXGI capture.

Only the configured screen regions are analyzed.

## Recognition

ElsOverlay uses several forms of visual recognition.

### Template matching

Reference images configured by the user are used to recognize visual elements such as:

- buffs;
- potions;
- Invariant;
- Custom Searcher templates;
- other configured UI elements.

### YOLO / ONNX detection

The project uses ONNX Runtime for object detection.

The bundled models are:

```text
models/
├── best_1080.onnx
└── best_2k.onnx
```

The models target 1080p and 2K layouts.

## Overlays

Overlay windows are transparent Qt windows managed through a common overlay root.

They can be positioned over the game without modifying the game's own interface.

---

# What ElsOverlay does not do

ElsOverlay is designed to operate exclusively through visible screen information.

It does **not**:

- read game memory;
- modify game memory;
- inject code into the game;
- inspect network traffic;
- modify game files;
- access the game's internal data structures.

---

# Compliance note

On **12 August 2026**, the Elsword.it support team stated, after consulting with the CoMa, that software obtaining information from the game client through data mining is considered illicit, while software operating solely through screen capture is permitted.

ElsOverlay is designed to stay within that scope: it analyzes information obtained from the visible screen and does not access the game's internal data.

Please keep in mind:

- this statement comes from the Elsword.it support team and concerns that server;
- rules on other servers may differ;
- rules may change over time;
- users should always check the current rules and terms of service of the server they play on.

The statement refers to software that analyzes information obtained through screen capture.

ElsOverlay should not be extended with functionality that reads game memory, injects code, inspects network traffic or accesses internal game data.

---

# Requirements

## To run

- Windows 10/11 64-bit
- Elsword in windowed or borderless mode
- 1080p or 2K game resolution for the bundled YOLO models

## To build

- CMake 3.19+
- Qt 6.5+
  - Core
  - Widgets
  - Concurrent
- C++17 compiler for Windows
- MSVC recommended
- ONNX Runtime for Windows
  - headers
  - `onnxruntime.dll`

---

# Installation (release build)

1. Download the latest archive from the **Releases** page.
2. Extract it anywhere you have write access.
3. Keep the folder structure intact.
4. Make sure the executable has access to:

```text
ElsOverlay.exe
images/
models/
onnxruntime.dll
```

5. Run `ElsOverlay.exe`.

The application expects the required resources next to the executable.

---

# First-time setup

Before configuring ElsOverlay:

1. Start Elsword.
2. Set the final game resolution.
3. Set the desired UI scale.
4. Start ElsOverlay.
5. Enable the sections you need.
6. Press **Configura** for each section and complete its configuration.

Recognition-based systems depend on the visual layout of the game. If the resolution, UI scale or relevant interface changes significantly, saved references may need to be recreated.

---

## Moving overlays

Overlay elements can be positioned directly over the game.

Click and hold the overlay, drag it to the desired position and release.

Positions are saved automatically and restored on the next launch.

---

# Atma setup

Enable **Atma** and open its configuration.

Configure the capture areas over the relevant parts of the screen and create the required reference images through the Atma configuration interface.

The Atma system uses screen recognition to identify the configured visual elements and track their state.

Reference images and configuration data are stored locally and reloaded when ElsOverlay starts.

If the game's resolution, UI scale or relevant visual layout changes significantly, the Atma references may need to be recreated.

---

# Transcendence setup

Open the Transcendence capture configuration.

Select the region containing the Transcendence indicator and create the required reference.

The capture configuration includes precision cropping so that only the relevant visual information is used for recognition.

---

# Class Buff

Open:

```text
Class Buff → Configura
```

Class Buff allows you to create separate configurations for different classes or characters.

Each configuration can contain any number of buffs.

Each buff can have:

- a custom name;
- a custom icon;
- a custom cooldown.

Different characters can therefore use different class configurations.

---

# Custom Searcher setup

Open:

```text
Custom Searcher → Configura
```

Create a template and select the appropriate search zone:

```text
Zone 1
Zone 2
```

Capture the visual reference corresponding to the element you want to recognize.

Configure the desired cooldown and overlay position.

Multiple templates can be configured independently.

Custom Searcher can therefore monitor several visual elements at the same time without modifying the game's own interface.

---

# Distance Guides setup

Open:

```text
Distance Guides → Configura
```

Create a group and add the desired guides.

Available types:

```text
Vertical line
Rectangle
Circle
```

Each guide can be configured and positioned independently.

Guides can be moved and enabled or disabled independently.

---

# Configuration

Everything is normally configured through the GUI.

Manual editing of configuration files should not be necessary for normal use.

The main configuration file is:

```text
ElsOverlay.ini
```

It stores persistent application settings such as:

- overlay positions;
- toggle states;
- keyboard configuration;
- Gate pause preference;
- other subsystem settings.

Recognition references and subsystem-specific configurations are stored locally next to the executable.

---

## Resetting a configuration

If a section needs to be completely reset:

1. close ElsOverlay;
2. remove the corresponding configuration file or reference folder;
3. start ElsOverlay again;
4. configure the section from scratch.

Keep a backup of configurations you may want to restore.

---

# Controls

Keyboard controls depend on the active subsystem.

| Key | Function |
|---|---|
| `CTRL` | Start / reset the relevant cooldown |
| `CTRL Right` | Reset supported cooldown and recognition systems |
| `G` | Begin title selection sequence |
| `↑ ← ↓ →` | Select title |
| `DELETE` | Hide / show the overlay |
| Configured Reset key | Reset supported systems |
| Configured Pause key | Pause / resume supported systems |

The Reset and Pause keys can be configured from the main window.

Atma does not require dedicated keyboard controls for its normal operation.

---

# Global reset

ElsOverlay provides a global reset mechanism for supported systems.

The reset returns active cooldown and recognition states to their normal initial state without requiring the application to restart.

Custom Searcher is included in the reset flow: active template cooldowns are cleared and its recognition workers are rebuilt.

---

# Building from source

Clone the repository:

```bash
git clone https://github.com/FungYang/ElsOverlay.git
cd ElsOverlay
```

Download ONNX Runtime for Windows and place it under:

```text
third_party/onnxruntime/
├── include/
└── lib/
    └── onnxruntime.dll
```

Make sure the `models/` directory contains:

```text
models/
├── best_1080.onnx
└── best_2k.onnx
```

The models are copied next to the executable after the build.

## CMake

Configure the project using a Qt 6.5+ kit:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
```

Build Release:

```bash
cmake --build build --config Release
```

Alternatively, open `CMakeLists.txt` in Qt Creator and select an appropriate Qt kit.

The post-build process copies:

- `images/`;
- `models/`;
- `onnxruntime.dll`;

into the output directory.

Qt deployment is handled through `qt_generate_deploy_app_script`.

---

# Project structure

```text
ElsOverlay/
├── images/                         Icons and bundled images
├── models/                         ONNX / YOLO models
│   ├── best_1080.onnx
│   └── best_2k.onnx
│
├── third_party/onnxruntime/        ONNX Runtime headers and DLL
│
├── main.cpp                        Application entry point
├── mainwindow.*                    Main control window
├── globalkeyboard.*                Global keyboard hook
├── screencapture.*                 D3D11 / DXGI screen capture
├── overlay.*                       Common overlay windows
├── overlayroot.*                   Overlay root container
│
├── skill*.*                        Skill overlay and configuration
├── keyedit.*                       Keyboard/key configuration
│
├── specialcooldown*.*              Potion and Invariant detection
├── digitdetector.*                 Digit recognition
│
├── transcendence*.*                Transcendence capture and vision
├── atma*.*                         Atma capture, zones and vision
│
├── buff*.*                         Buff overlay, capture and detection
├── class*.*                        Class buff configuration
├── newbuffdialog.*                 Buff editor
│
├── customsearcher*.*              Generic template search system
│
├── distanceguide*.*               Distance guides
│
└── ...
```

---

# Architecture overview

ElsOverlay is organized as independent subsystems rather than one large cooldown manager.

A simplified runtime flow is:

```text
                    ┌─────────────────────┐
                    │     ElsOverlay      │
                    └──────────┬──────────┘
                               │
             ┌─────────────────┼─────────────────┐
             │                 │                 │
             ▼                 ▼                 ▼
      Global Keyboard     Screen Capture     Overlay Root
             │                 │                 │
             │                 ▼                 │
             │          Visual Recognition       │
             │          ┌───────────────┐        │
             │          │ Template      │        │
             │          │ Matching      │        │
             │          ├───────────────┤        │
             │          │ YOLO / ONNX   │        │
             │          └───────────────┘        │
             │                 │                 │
             └─────────────────┼─────────────────┘
                               ▼
                    Independent subsystems
```

The major subsystems can operate independently and can be enabled or disabled without restarting the application.

This keeps screen recognition, cooldown logic, configuration and visual overlays separated from each other.

---

# Known limitations

ElsOverlay is currently Windows-only.

Recognition accuracy depends on:

- game resolution;
- Windows display scaling;
- game UI scale;
- UI layout;
- saved reference images;
- screen capture conditions.

Reference images may need to be recreated if the game's visual layout changes significantly.

The bundled YOLO models are designed for:

- 1080p;
- 2K.

Other resolutions may require different models or additional configuration.

Global keyboard hooks and screen capture can also behave differently depending on Windows configuration, overlays, graphics drivers or security software.

---

# Release history

| Version | Highlights |
|---|---|
| **v1.18** | Transparency option and automatic pause on stage change for titles |
| **v1.17** | Template matching for potions; Invariant detection via Resonance text |
| **v1.15** | Debug output removed |
| **v1.08** | Transcendence bug fix; fully configurable titles |

See the **Releases** page for the complete release history.

---

# License

ElsOverlay is released under the **GPL-3.0** license.

See [LICENSE](LICENSE) for the full license text.

---

Developed by **FungYang**.