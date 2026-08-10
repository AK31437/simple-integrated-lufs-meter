# Simple Integrated LUFS Meter

A minimal VST3 plugin that measures integrated loudness according to [ITU-R BS.1770-4](https://www.itu.int/rec/R-REC-BS.1770-4-201510-I) / [EBU R 128](https://tech.ebu.ch/docs/r/r128.pdf). Does one thing, does it well.

## Features

- Integrated LUFS measurement with proper K-weighting and gating
- Transport-aware: measures while playing, freezes on pause/stop, resets on next play
- Manual reset button
- Audio passes through unmodified (pure measurement, no signal coloring)
- Works at any sample rate (filter coefficients adapt via bilinear transform)
- Stereo by default, 5.1 channel weights supported

## Building

### Requirements

- CMake 3.22+
- C++17 compiler (GCC, Clang, MSVC)
- Linux: ALSA, freetype2, X11 development headers (for JUCE)

JUCE is fetched automatically via CMake FetchContent — no manual setup needed.

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Install

Copy the built `.vst3` bundle to your DAW's plugin path:

```bash
# Linux
cp -r build/LUFSMeter_artefacts/Release/VST3/Simple\ Integrated\ LUFS\ Meter.vst3 ~/.vst3/

# macOS
cp -r build/LUFSMeter_artefacts/Release/VST3/Simple\ Integrated\ LUFS\ Meter.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Windows (PowerShell)
Copy-Item -Recurse build\LUFSMeter_artefacts\Release\VST3\Simple\ Integrated\ LUFS\ Meter.vst3 $env:PROGRAMFILES\Common Files\VST3\
```

## How It Works

The plugin implements the full ITU-R BS.1770-4 integrated loudness algorithm:

1. **K-weighting** — two cascaded biquad filters (pre-filter for head-related acoustic effects + revised low-frequency B-curve high-pass)
2. **Mean square** — per-channel energy accumulation in 100ms bins
3. **Channel weighting** — stereo: L=1.0, R=1.0 (surround channels weighted +1.5 dB per spec)
4. **Gating** — 400ms blocks with 75% overlap; absolute gate at -70 LUFS, then relative gate at -10 dB below the absolute-gated average
5. **Histogram** — block loudnesses are binned at 0.1 LU resolution for the final gated average

The measurement uses `double` precision internally. Cross-thread communication (audio → GUI) uses `std::atomic` with no locks on the audio thread.

## Usage

1. Insert on any track or bus in your DAW
2. Start playback — the integrated LUFS value accumulates in real time
3. Pause/stop — the measurement freezes on screen
4. Start again — measurement resets automatically
5. Use the round button to manually reset at any time

## Project Structure

```
CMakeLists.txt              # Build config, fetches JUCE 8.0.8
Source/
  LufsCalculator.h/cpp      # Pure DSP engine (no JUCE dependency)
  LufsMeterProcessor.h/cpp  # JUCE AudioProcessor wrapper
  LufsMeterEditor.h/cpp     # Minimal GUI
```

## License

MIT License. See [LICENSE](LICENSE) for details.
