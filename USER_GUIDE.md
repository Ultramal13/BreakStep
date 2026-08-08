# BreakStep User Guide & Workflow Manual

Welcome to **BreakStep**, a modular Drum & Bass workstation, transient sample chopper, and song arranger designed for electronic music producers.

---

## 1. Quick Start Workflow

### 1.1 Master Transport & Global Controls
- **`PLAY` / `STOP`**: Starts and stops the sample-accurate master clock.
- **`PATTERN` vs. `SONG` Mode**:
  - `[ PATTERN ]`: Loops the currently selected pattern indefinitely for live jam sessions and beat creation.
  - `[ SONG ]`: Plays the 32-bar linear arrangement from Bar 1 to 32, automatically switching patterns across the song timeline.
- **`STYLE PRESETS` Dropdown**: 1-click loading of authentic electronic genres (**DnB Roller 174**, **Jungle Amen 168**, **Dubstep 140**, **UK Garage 132**, **UK Bass 138**, **Nu-Skool Breaks 135**, **Liquid DnB 172**).
- **`P1` to `P8` Pattern Selectors**: Instantly audition and edit any of the 8 pattern banks.
- **`DRUMS` & `CHOP` Bus Volume Knobs**: Independently balance the overall volume of the drum machine and the loop slicer with single rotary controls.
- **`TEMPO` (BPM)**: Adjusts speed from 60 to 200 BPM.
- **`SWING`**: Injects groovy MPC-style shuffle into odd 16th-note steps.
- **`DELAY` & `MASTER FILTER`**: Global master tempo-synced feedback delay and continuous lowpass resonant filter.

---

## 2. ReCycle-Style Waveform Lab & MPC Chopper

### 2.1 Multi-Slot Sample Loading
- Click **`SLOT 1`**, **`SLOT 2`**, **`SLOT 3`**, or **`SLOT 4`** to switch between 4 sample memory banks.
- Click **`LOAD AUDIO`** or drag-and-drop any `.wav`, `.aiff`, `.mp3`, or `.flac` audio loop directly onto the waveform display.

### 2.2 Slicing & Transient Editing
- **`AUTO SLICE` & `SENSITIVITY` Knob**: Automatically detects attack onsets using high-frequency spectral pre-emphasis and snaps cut markers to zero-crossings.
- **Mouse Drag**: Click and drag any yellow slice line to adjust its cut point in real time.
- **`NUDGE` Knob**: Fine-tune the selected slice marker forward or backward sample-by-sample.
- **Double-Click**: Inserts a new slice marker at that exact cursor position.
- **Right-Click on Marker**: Deletes the slice marker.
- **16 MPC Slice Pads (`PAD 1` .. `PAD 16`)**: Click to audition any slice instantly with real-time length readouts.

---

## 3. Propellerhead Reason-Style Linear Song Timeline Arranger

The **Song Timeline** allows arranging full tracks using visual pattern blocks across 32 bars:
- **`DRUMS` Lane**:
  - Left-click a bar: Cycles the drum pattern (`D1` $\to$ `D2` $\to$ ... $\to$ `D8` $\to$ `---`).
  - Right-click a bar: Mutes that bar (`---`).
- **`CHOPS` Lane**:
  - Left-click a bar: Cycles the slice chop pattern (`S1` $\to$ `S2` $\to$ ... $\to$ `S8` $\to$ `---`).
  - Right-click a bar: Mutes that bar (`---`).
- **`CLEAR ARR`**: Wipes the timeline arrangement clean.
- **`STD DNB ARR`**: Generates a standard electronic song structure (Intro $\to$ Build $\to$ Drop $\to$ Breakdown $\to$ Drop 2 $\to$ Outro).
- **`REPEAT 1-4`**: Copies the first 4 bars across the entire 32-bar timeline.

---

## 4. Saving & Loading Sessions

- Click **`SAVE`** to export a complete `.breakstep` JSON session file (persisting BPM, swing, FX, track settings, all 8 pattern banks, timeline arrangement, and sample paths).
- Click **`LOAD`** to restore any previously saved session instantly.
