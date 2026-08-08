# BreakStep User Guide & Workflow Manual

Welcome to **BreakStep**, a modular Drum & Bass workstation and transient sample chopper designed for electronic music producers.

---

## 1. Quick Start Workflow

### 1.1 Master Transport & Global Controls
- **`PLAY` / `STOP`**: Starts and stops the sample-accurate master clock.
- **`TEMPO` (BPM)**: Adjusts speed from 60 to 200 BPM (default: 170 BPM for Drum & Bass / Jungle).
- **`SWING`**: Injects groovy MPC-style shuffle into odd 16th-note steps.
- **`DELAY` & `MASTER FILTER`**: Global master tempo-synced feedback delay and continuous lowpass resonant filter.
- **`CLR ALL` & `RND ALL`**:
  - `CLR ALL` (Red): Instantly wipes all step pattern matrices.
  - `RND ALL` (Green): Generates a full, intelligent Drum & Bass groove across all 7 tracks in one click.

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

## 3. Unified 7-Track Step Sequencer

### 3.1 Track 0: `✂️ CHOP SEQ`
- **Left-Click on a Step Pad**: Toggles the step on/off (glowing cyan).
- **Right-Click / Ctrl-Click on a Step Pad**: Cycles which slice (`S1` to `S16`) is triggered on that step.
- **`1x` / `2x` / `3x` / `4x` (Ratchets)**: Subdivides the step into rapid micro-rolls (1/32 and 1/64 rolls).
- **`100%` / `75%` / `50%` / `25%` (Probability)**: Sets the random trigger chance for dynamic, non-repetitive grooves.

### 3.2 Tracks 1–6: BreakStep Drum Machine
- **6 Drum Voices**: `KICK`, `SNARE`, `HAT`, `OPEN HAT`, `CLAP`, and `PERC`.
- **Vintage Sampler DSP Button**: Cycle between:
  - **`CLEAN`**: Modern 32-bit floating-point Hi-Fi.
  - **`MPC-60`**: 12-bit / 40kHz punch with transformer analog saturation.
  - **`MPC-3K`**: 16-bit warmth with asymmetric op-amp tape saturation.
  - **`EPS-16`**: Ensoniq OTIS variable bit decimation and gritty aliasing crunch.
- **`CRUNCH` Knob**: Dial in the exact amount of vintage distortion and bit reduction.
- **`CLR` & `RND` per Track**: Clear or randomize individual drum patterns independently.

---

## 4. Saving & Loading Sessions

- Click **`SAVE PROJ`** to export a complete `.breakstep` JSON session file (persisting BPM, swing, FX, track settings, slice markers, and sample paths).
- Click **`LOAD PROJ`** to restore any previously saved session instantly.
