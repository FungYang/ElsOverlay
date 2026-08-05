# ElsOverlay

ElsOverlay is a customizable overlay for Elsword designed to track skill cooldowns and buffs through visual elements displayed above the game.

The program uses external information only, such as keyboard input and screen recognition features. It does not modify game files and does not directly interact with the game client.

---

# Features

ElsOverlay currently provides:

* Transcendence cooldown tracking.
* Main skill/buff overlay group.
* Class-specific buff overlays.
* Movable overlay elements.
* Automatic position saving.
* Keyboard sequence recognition.
* Title-based skill selection system.
* Awakening-based Artifact tracking.
* BuffVision visual confirmation system.

BuffVision uses selected screen areas and image comparison to detect visual states without interacting with the game client.

---

# First Launch

On first launch, all overlay elements are created using default positions.

Every element can be moved by dragging it with the mouse.

Positions are automatically saved in:

```
ElsOverlay.ini
```

No manual configuration editing is required.

---

# BuffVision System

BuffVision is a visual detection system used to confirm buff states through screen recognition.

The system works by:

1. Selecting specific screen areas.
2. Saving reference images for different visual states.
3. Capturing the selected areas during tracking.
4. Comparing current images with stored references.
5. Triggering events only when a valid state transition is detected.

Example:

```
State 1
   |
   | visual change
   v
State 2
   |
   v
Buff event
```

The system does not read game memory and does not modify the game client.

---

# BuffVision Configuration

During BuffVision setup:

1. Move the capture boxes over the desired screen areas.
2. Press `P` to save the first reference image.
3. Press `P` again to save the second reference image.
4. Press `ENTER` to confirm the configuration.

Reference images are stored in:

```
BuffVision/
```

The program automatically loads existing references on startup.

---

# Overlay Groups

## Transcendence Overlay

The Transcendence overlay tracks the cooldown timer.

Features:

* Automatic cooldown counting.
* Manual reset through dedicated key.
* Independent from other overlays.

The position is automatically saved.

---

## Main Skill Group

The main group contains:

* Concerto
* Artifact
* Night Parade
* Setting Sun

The group can be moved freely.

Each position is automatically restored on the next launch.

---

# Title Selection System

Skill activation uses a title selection system.

Pressing:

```
G + Direction
```

selects the current title.

Available titles:

```
G + ↑ -> Concerto

G + ← -> Night Parade

G + ↓ -> Setting Sun

G + → -> Empty title
```

The selected title remains active until another title is chosen.

This prevents accidental activation of previously selected skills.

---

# Skill Activation

## Concerto

Available combinations:

```
G + ↑ + CTRL
G + ↑ + 6
```

---

## Night Parade

Available combinations:

```
G + ← + F
G + ← + T
```

---

## Setting Sun

Available combinations:

```
G + ↓ + CTRL
G + ↓ + 6
```

---

# Artifact

Artifact works independently from the selected title.

When awakening tracking is active:

```
CTRL
6
```

activate Artifact cooldown tracking.

Artifact does not depend on:

* Concerto selection.
* Night Parade selection.
* Setting Sun selection.

---

# Controls

## LCTRL

Starts awakening/tracking mode.

First activation:

* Enables sequence detection.
* Starts initial tracking cooldowns.

Further presses continue normal tracking.

---

## RCTRL

Full system reset.

Effects:

* Stops tracking.
* Clears selected title.
* Resets skill cooldowns.
* Resets BuffVision states.
* Restores the initial state.

---

## ENTER

Confirms BuffVision configuration.

After moving BuffVision capture boxes:

1. Position the capture areas.
2. Save the two reference images using `P`.
3. Press ENTER.
4. The configuration becomes active.

---

## SPACE

Pauses overlay input detection.

While paused:

* Skill detection is disabled.
* ESC remains functional.

Press SPACE again to resume.

---

## ESC + P

Exit command.

The application closes only when:

```
ESC
then
P
```

are pressed.

ESC alone does not stop the program or block other inputs.

---

## 7

Manual Transcendence reset.

This reset only affects the Transcendence timer.

Useful when:

* Awakening ends unexpectedly.
* The character dies.
* Manual synchronization is needed.

It does not reset other overlays.

---

## 0

Manual Atma Buff start/reset.

This reset only affects the Atma Flow buff timer.

Useful when:

* Finished Raid
* Before Starting Raid

It does not reset other overlays.

---

# Class System

Classes have independent buff configurations.

Example:

```
BQ
├── A
├── D

FL
├── A
├── B
└── C
```

When changing class:

* Previous buff boxes are removed.
* New boxes are created.
* Saved positions are restored.

---

# Configuration File

All settings are stored in:

```
ElsOverlay.ini
```

Example:

```
[Overlay]
Transcendence\position=@Point(...)

BuffGroup\position=@Point(...)

[Classes]
BQ\A\position=@Point(...)
```

Deleting this file restores default positions.

---

# Technical Information

Developed with:

* C++
* Qt Framework
* Windows Keyboard Hooks
* Qt Screen Capture API

The application is designed to remain lightweight and independent from the game client.

---

# License

License information will be specified before public release.

The project is intended to remain open-source and transparent.
