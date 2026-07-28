# ElsOverlay

ElsOverlay is a customizable overlay for Elsword that allows you to track skill and buff cooldowns through visual elements displayed on top of the game.

The program allows separate configuration of:

- Transcendence overlay.
- Main 4-buff group.
- Class-specific buff boxes.

All positions are automatically saved and restored on the next launch through the file:

ElsOverlay.ini
No manual editing of the file is required.

---

# First Launch

On the first launch, all elements are created using their default positions.

Every overlay element can be moved by dragging it with the mouse.

Once the desired position is set, it will be automatically saved.

---

# Transcendence Overlay

The Transcendence overlay is a single element that is always available.

To configure it:

1. Drag the overlay to the desired position.
2. Close the program.

The position will automatically be restored on future launches.

---

# Buff Group

The 4-buff group works independently from the selected class.

To configure it:

1. Move the group to the desired position.
2. Close the program.

The position will be preserved on future launches.

---

# Class Selection

The class selector allows you to choose the class configuration you want to use.

Each class has an independent configuration.

Example:No manual editing of the file is required.

---

# First Launch

On the first launch, all elements are created using their default positions.

Every overlay element can be moved by dragging it with the mouse.

Once the desired position is set, it will be automatically saved.

---

# Transcendence Overlay

The Transcendence overlay is a single element that is always available.

To configure it:

1. Drag the overlay to the desired position.
2. Close the program.

The position will automatically be restored on future launches.

---

# Buff Group

The 4-buff group works independently from the selected class.

To configure it:

1. Move the group to the desired position.
2. Close the program.

The position will be preserved on future launches.

---

# Class Selection

The class selector allows you to choose the class configuration you want to use.

Each class has an independent configuration.

Example:
BQ
├── A
├── D

FL
├── A
├── B
└── C


When selecting a class:

- Only the buff boxes associated with that class are loaded.
- Saved positions for that class are restored.
- Other class configurations remain unchanged.

---

# Class Buff Box Configuration

When a class is loaded for the first time:

1. The buff boxes are created in their default positions.
2. Drag each box to the desired location.
3. Press `ENTER` to confirm the configuration.

After confirmation, the boxes become active and can receive inputs.

Pressing `ENTER` without selecting a class has no effect.

---

# Controls

## LCTRL

Starts the tracking system.

When pressed:

- Enables sequence detection.
- Starts the configured cooldown tracking.

If tracking is already active, pressing LCTRL again will not restart the system.

---

## RCTRL

Returns the program to its initial state.

Effects:

- Disables tracking.
- Resets cooldowns.
- Clears stored sequences.

---

## ENTER

Confirms the placement of the current class buff boxes.

Use this after positioning the boxes to make them active.

---

## SPACE

Pauses or resumes overlay input detection.

While paused:

- The overlay ignores all other keyboard inputs.
- `ESC` continues to work normally.

Press SPACE again to resume input detection.

---

## ESC

A command that remains available even while the overlay is paused.

---

# Configuration Saving

The `ElsOverlay.ini` file automatically stores all configurations.

Example:
[Overlay]
Transcendence\position=@Point(...)
BuffGroup\position=@Point(...)

[Classes]
BQ\A\position=@Point(...)
BQ\D\position=@Point(...)


Buff box positions are stored separately for each class.

Deleting the `ElsOverlay.ini` file will reset the program to its default positions.

---

# Changing Class

When selecting a new class:

1. The previous class buff boxes are removed.
2. The new class buff boxes are created.
3. Saved positions for the selected class are restored.
4. Press `ENTER` to confirm the configuration.

---

# Notes

ElsOverlay is designed as a visual support and cooldown tracking overlay.

The program does not modify game files and does not directly interact with the game client.

