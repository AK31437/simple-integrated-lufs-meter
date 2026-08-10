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

JUCE is fetched automatically via CMake FetchContent — no manual setup needed.

### Quick reference

| Platform | Configure | Build |
|----------|-----------|-------|
| Linux / macOS | `cmake -B build -DCMAKE_BUILD_TYPE=Release` | `cmake --build build` |
| Windows | `cmake -B build` | `cmake --build build --config Release` |

### Linux

<details><summary><b>Requirements</b></summary>

- CMake 3.22+
- C++17 compiler (GCC or Clang)
- JUCE system libraries: ALSA, freetype2, X11 (X11, Xrandr, Xinerama, Xcursor) development headers

Install them with:

```bash
# Debian / Ubuntu
sudo apt install build-essential cmake libasound2-dev libfreetype-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev

# Arch Linux
sudo pacman -S --needed base-devel cmake alsa-lib freetype2 libx11 libxrandr libxinerama libxcursor

# Fedora
sudo dnf install gcc-c++ cmake alsa-lib-devel freetype-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel
```

</details>

<details><summary><b>Build and install</b></summary>

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The plugin bundle is built at `build/LUFSMeter_artefacts/Release/VST3/Simple Integrated LUFS Meter.vst3`.

Install it to your DAW's plugin path:

```bash
cp -r "build/LUFSMeter_artefacts/Release/VST3/Simple Integrated LUFS Meter.vst3" ~/.vst3/
```

</details>

### macOS

<details><summary><b>Requirements</b></summary>

- [Xcode Command Line Tools](https://developer.apple.com/xcode/): `xcode-select --install` (provides Clang, C++17)
- CMake 3.22+ — e.g. via [Homebrew](https://brew.sh/): `brew install cmake`

</details>

<details><summary><b>Build and install</b></summary>

macOS has no `nproc`, so use `sysctl` to parallelize:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)
```

The plugin bundle is built at `build/LUFSMeter_artefacts/Release/VST3/Simple Integrated LUFS Meter.vst3`.

Install it to your DAW's plugin path:

```bash
cp -r "build/LUFSMeter_artefacts/Release/VST3/Simple Integrated LUFS Meter.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

</details>

### Windows

<details><summary><b>Requirements</b></summary>

- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **"Desktop development with C++"** workload. This bundles the MSVC compiler and CMake, so nothing else is needed.

</details>

<details><summary><b>Build and install</b></summary>

Windows uses the multi-config Visual Studio generator, so there is no `CMAKE_BUILD_TYPE` — the configuration is passed to the build step instead:

```powershell
cmake -B build
cmake --build build --config Release
```

The plugin bundle is built at `build\LUFSMeter_artefacts\VST3\Release\Simple Integrated LUFS Meter.vst3`.

Install it to your DAW's plugin path:

```powershell
Copy-Item -Recurse "build\LUFSMeter_artefacts\VST3\Release\Simple Integrated LUFS Meter.vst3" "$env:PROGRAMFILES\Common Files\VST3\"
```

</details>

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
