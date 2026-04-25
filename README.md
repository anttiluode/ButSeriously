# But Seriously 🎛️

*A Topological Synthesizer & Bost-Connes Resonator*

**But Seriously** is a completely novel digital signal processing (DSP) audio plugin built in **C++ / JUCE**.

It abandons standard amplitude and EQ processing to operate purely on the **phase topology** of the audio signal.  
Originally designed as an esoteric reverb, this plugin features full **MIDI input tracking**, allowing you to play the delay buffer like a tuned physical instrument.  

Feed it a drum loop, route MIDI notes to it — and play your drums like a synthesizer.

---

## 🎛️ The Concept

Standard audio plugins treat audio as a **1D signal over time**.  

**But Seriously** instead uses a **Takens Delay Embedding** to unfold the signal into a **2D complex phase space**:

\[
Z = \text{Real} + i \cdot \text{Imaginary}
\]

Once unfolded into geometry, the plugin applies a **Bost-Connes Phase Transition**:

### 🔹 The Prime Arm (Ergodic)

- Delay lines use **prime-number lengths**
- No shared divisors → no harmonic reinforcement
- Result: infinitely dense, metallic, non-repeating "smear"

### 🔹 The Harmonic Arm (Locked)
- Delay lines use **integer harmonic relationships**
- Produces stable, resonant tonal structures

### 🔹 The EML Gate
A **topological wavefolder** based on the EML Zero Manifold:

\[
|Z| - \arg(Z)
\]

- Distorts only when **magnitude and phase misalign**
- Creates tearing, holes, and phase-dependent clipping

---

## 🎹 MIDI Tuning (The Topological Synth)

**But Seriously** accepts live MIDI input.  
When a note is played:

1. Frequency is calculated  
2. Converted into delay time (ms)  
3. Entire delay network is **retuned to that pitch**

### 🎛️ FL Studio Setup
1. Open Wrapper Settings (⚙ → 🔧)  
2. Set **MIDI Input Port = 1**  
3. Add **MIDI Out** channel → set **Port = 1**  
4. Play notes via MIDI Out to tune the audio

---

## 🕹️ Controls & Features

### 🔥 Cooling (Beta)
The master macro knob controlling phase state:

- **0.0 (Hot)**  
  → Prime delays dominate  
  → Chaotic, whispering metallic wash

- **>1.0 (Critical / Cooled)**  
  → Prime delays collapse into harmonics  
  → Sound crystallizes into tuned chords

### ⚡ EML Gate
- Applies **topological distortion**  
- Attenuates samples off the EML Zero Manifold  
- Produces tearing, glitch-like phase artifacts

### ⏱️ Base Delay (ms)
- Sets overall delay scale  
- **Overridden by MIDI input** when active

### 🔁 Resonance
- Feeds processed signal back into delay system  
- Increases complexity and sustain

### 🎚️ Mix
- Standard dry/wet control

### 💾 Preset System
- Save / load states as **XML files**  
- Capture entire mathematical configurations

---

## 🔭 The Live Visualizer

A real-time **Takens Phase Space scope**:

### 🌀 Ergodic State (Low Beta)
- Chaotic, swirling **purple ellipses**  
- Known as the *Alpha Crystal*

### 🟦 Locked State (High Beta)
- Sharp, geometric **teal squares**  
- Known as the *Theta Box*

### 📐 Responsive GUI
- Fully resizable window  
- Built-in **Auto Gain Control (AGC)**  
- Always scales visualization to fit screen cleanly

---

## 🛠️ Building from Source

### Requirements
- CMake
- C++17 compiler  
  - Visual Studio (Windows)  
  - Xcode (Mac)

### 1. Clone JUCE
```bash
git clone https://github.com/juce-framework/JUCE.git JUCE
2. Clone But Seriously
Bashgit clone https://github.com/yourusername/but-seriously.git
cd but-seriously
3. Build
Bashmkdir build && cd build
cmake .. -DJUCE_ROOT=../../JUCE
cmake --build . --config Release
The resulting VST3 / AU / AAX plugin will appear in the build folder.

📜 License
MIT License — feel free to explore, modify, and build upon the phase topology.

⚠️ Note
This is a highly experimental plugin that lives at the intersection of dynamical systems, number theory, and audio DSP.
Expect the unexpected.
But Seriously.
