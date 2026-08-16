# ElsOverlay

**ElsOverlay** is a customizable overlay for **Elsword** designed to help track skill cooldowns, buffs, transcendence and distance-related information directly on screen.

The application is designed to remain completely external to the game: it relies on keyboard input and screen recognition and does not modify game files or directly interact with the game client.

---

## Features

ElsOverlay currently provides several independent overlay systems that can be enabled or disabled from the main window.

### Transcendence

Tracks the cooldown of the Transcendence system.

The overlay displays the remaining cooldown and can be controlled through keyboard input.

Features:

- Transcendence cooldown tracking
- Reset support
- Ability to pause the cooldown
- Movable overlay
- Automatic position saving

---

### Main Skill Overlay

Tracks the cooldowns of the main skills associated with the character.

The overlay currently supports:

- Concerto
- Artifact
- Night Parade
- Setting Sun

The skill overlay uses keyboard sequences to determine which title/skill is currently active.

#### Title selection

After the awakening sequence, the title can be selected using the directional keys:

| Direction | Title |
|---|---|
| `↑` | Concerto |
| `←` | Night Parade |
| `↓` | Setting Sun |
| `→` | Other |

The overlay then tracks the corresponding cooldown according to the detected input sequence.

---

### Class Buffs

ElsOverlay includes a configurable system for tracking class-specific buffs.

Class buffs can be configured from the main window and displayed through a dedicated overlay.

The system supports:

- Multiple class configurations
- Custom buff entries
- Custom names
- Custom icons
- Cooldown tracking
- Enable/disable toggle
- Configuration editor

---

### Buff Titles

Buff Titles can be independently enabled or disabled from the main window.

This allows the user to control exactly which buff information is displayed without affecting the other overlay systems.

---

### Buff Trascendenza

The Transcendence buff overlay can also be independently enabled or disabled.

This makes it possible to keep the Transcendence cooldown system separate from the visual buff display.

---

### Atma / BuffVision

The Atma system uses **BuffVision**, a screen-recognition system designed to detect visual changes in selected areas of the screen.

BuffVision does not read game memory.

Instead, it:

1. Captures selected areas of the screen.
2. Stores reference images.
3. Compares the current screen with the stored references.
4. Detects state transitions.
5. Triggers the corresponding buff logic.

This allows the system to react to visual changes without directly interacting with the game client.

---

### Distance Guides

Distance Guides provide visual guides that can be placed over the game.

Supported guide types include:

- Vertical lines
- Rectangles
- Circles

Guides can be:

- Created
- Removed
- Configured
- Moved
- Grouped
- Enabled or disabled

#### Groups

Distance Guides can be organized into groups.

Each group can contain any number of guides.

For example:

```text
Group: Boss Position

    ├── Player Position
    ├── Boss Position
    ├── Left Limit
    ├── Right Limit
    └── Safe Area
```

Groups can be configured independently, making it possible to quickly switch between different guide setups.

---

## Main Window

The main window provides independent controls for the different overlay systems.

Each system can be turned ON or OFF without affecting the others.

Available sections include:

- Atma
- Class Buff
- Distance Guides
- Buff Tracker
- Buff Titles
- Buff Trascendenza

The configuration windows can be opened separately using the corresponding **Configura** buttons.

---

# Controls

The exact keyboard behavior depends on the active overlay system.

Common controls include:

| Key | Function |
|---|---|
| `CTRL` | Start/reset relevant cooldown tracking |
| `CTRL Right` | Reset relevant cooldown systems |
| `0` | Start AtmaFlowBuff tracking |
| `1` - `6` | AtmaFlowBuff action input |
| `G` | Begin title selection sequence |
| `↑` | Select Concerto |
| `←` | Select Night Parade |
| `↓` | Select Setting Sun |
| `→` | Select Other |
| `P` | Save AtmaFlowBuff reference |
| `ENTER` | Confirm configuration |
| `8` | Pause/resume Transcendence cooldown |

> Some keys are handled only when the corresponding subsystem is enabled.

---

# Moving Overlays

Overlay elements can generally be moved directly with the mouse.

To move an overlay:

1. Click and hold the overlay.
2. Drag it to the desired position.
3. Release the mouse button.

Positions are automatically saved.

The next time ElsOverlay is launched, the overlays are restored to their previous positions.

---

# Configuration

Most configuration is performed directly through the application's graphical interface.

No manual editing of configuration files is normally required.

The application stores positional and configuration data locally.

The main position file is:

```text
ElsOverlay.ini
```

---

# AtmaFlowBuff Setup

To configure Atma/AtmaFlowBuff:

1. Enable **Atma** from the main window.
2. Open the Atma configuration window.
3. Position the capture areas over the desired parts of the screen.
4. Save the first reference using `P`.
5. Save the second reference using `P`.
6. Confirm the configuration with `ENTER`.
7. Enable the Atma overlay when needed.

Reference images are stored in:

```text
AtmaFlowBuff/
```

Existing references are automatically loaded when the application starts.

---

# Class Buff Configuration

Open:

```text
Class Buff → Configura
```

From there you can create and manage class configurations.

Each configuration can contain multiple buffs.

The system is intended to allow different setups to be prepared for different characters or situations.

---

# Distance Guide Configuration

Open:

```text
Distance Guides → Configura
```

From there you can create guide groups and add as many guides as required.

Available guide types:

- Line
- Rectangle
- Circle

Each guide can be configured individually.

Guides can also be removed from their group without deleting the entire group.

The group configuration window remains open while adding multiple elements, allowing several guides to be created consecutively.

---

# Buff Tracker

The Buff Tracker can be opened from the main window.

It is implemented as a separate executable:

```text
buffoverla.exe
```

The main application launches it when the **Configura** button is pressed.

---

# Architecture

ElsOverlay is structured around several independent components.

The main components include:

```text
MainWindow
    |
    +-- Atma / AtmaFlowBuff
    |
    +-- Class Buff
    |
    +-- Distance Guides
    |
    +-- Buff Titles
    |
    +-- Buff Trascendenza
    |
    +-- Buff Tracker
```

The overlay elements are managed through a common overlay root, allowing the different visual components to coexist independently.

This architecture makes it possible to enable or disable individual systems without shutting down the entire application.

---

# External Operation

ElsOverlay is designed to operate externally to Elsword.

The application uses:

- Global keyboard input
- Screen capture
- Image comparison
- Visual overlays

It does **not**:

- Modify Elsword game files
- Read game memory
- Inject code into the game client
- Directly interact with the game's internal systems

AtmaFlowBuff specifically relies on visual recognition rather than game-memory access.

---

# Requirements

The project is developed using **Qt/C++** and is intended for Windows.

A compiled release should include all required Qt runtime dependencies and application resources.

For developers building from source, a compatible Qt development environment is required.

---

# Building From Source

Clone the repository:

```bash
git clone https://github.com/FungYang/ElsOverlay.git
```

Open the project with Qt Creator and build it using the configured Qt kit.

The project contains the source files for the main application, overlay systems, configuration managers and AtmaFlowBuff components.

---

# Project Structure

The repository contains several major components:

```text
ElsOverlay/
│
├── images/
│
├── buffoverlay.*
├── buffbox.*
│
├── skilloverlay.*
├── skillbox.*
│
├── overlay.*
├── overlayroot.*
│
├── buffvisioncore.*
├── buffvisioncapture.*
├── buffvisiondetector.*
├── buffvisionmanager.*
├── buffvisionoverlay.*
├── buffvisioncapturesetup.*
│
├── classconfigurationmanager.*
├── classbuffconfigwindow.*
├── classbuffeditorwindow.*
├── classselector.*
│
├── distanceguidemanager.*
├── distanceguideoverlay.*
├── distanceguideconfigwindow.*
├── distanceguidegroupconfigwindow.*
├── distanceguidegroup.*
├── distanceguideline.*
├── distanceguiderectangle.*
├── distanceguidecircle.*
│
├── globalkeyboard.*
│
├── mainwindow.*
└── main.cpp
```

---

# Important Notes

ElsOverlay is intended as a personal utility for Elsword players who want additional visual information while playing.

Because the application relies on global keyboard hooks and screen recognition, behavior can depend on:

- Windows configuration
- Game resolution
- Display scaling
- Overlay positioning
- Screen layout
- The configured BuffVision reference images

BuffVision references may need to be recreated if the visual appearance or screen layout changes significantly.
## Buff Remapping

The Buff Tracker configuration uses a separate companion application for buff remapping:

**ElsBuffRemapping**

[https://github.com/FungYang/ElsBuffRemapping](https://github.com/FungYang/ElsBuffRemapping)

ElsBuffRemapping is an external Qt application launched by ElsOverlay when the **Buff Tracker → Configura** button is pressed.

It is responsible for configuring and managing the buff remapping system.

### Visual-only detection

ElsBuffRemapping does **not** perform game data mining or access the game's internal data.

It does not:

- Read or modify game memory
- Inject code into the game process
- Inspect network packets
- Intercept game traffic
- Extract internal game data

The system works exclusively by analyzing **visual information captured from the screen**.

In other words, it observes the same visual information that is available to the user and uses that information to identify and manage buffs.

For more information about the implementation and configuration of the external application, see the dedicated repository:

**[ElsBuffRemapping](https://github.com/FungYang/ElsBuffRemapping)**

---

# License

See the [`LICENSE`](LICENSE) file included in the repository.

---

# Repository

GitHub:

https://github.com/FungYang/ElsOverlay

---

# Credits

Developed by **FungYang**.

ElsOverlay is an independent external utility created to provide customizable visual assistance for Elsword.