# 🎵 Audio-Pico: Interactive MIDI Synthesizer

A modern C++17 audio framework for Raspberry Pi Pico featuring an interactive MIDI synthesizer with real-time keyboard control and multi-octave support.

## ✨ Features

### 🎹 Interactive MIDI Synthesizer
- **Multi-Octave Support**: Low (3rd octave), Standard (4th octave), High (5th octave)
- **Real-Time Keyboard Control**: Serial interface with keyboard input mapping
- **Advanced Input Handling**: True combination key support (Shift+1-7, Alt+1-7)
- **Audio Effect Controls**: Volume adjustment, waveform switching, mute control
- **Scale Demonstration**: Complete DO RE MI scale playback
- **Memory Optimization**: LRU cache management with 32kHz stereo, 768-sample buffer

### 🎛️ Audio Capabilities
- **High-Quality Audio**: I2S output at 32kHz/44.1kHz, 16-bit stereo
- **Multiple Waveforms**: Piano synthesis with harmonics and ADSR envelope, Pure sine wave
- **Real-Time Processing**: Low-latency audio generation and processing
- **Volume Control**: Real-time volume adjustment with smooth transitions
- **Mute Control**: Hardware mute pin support (GPIO 22)

### ⚡ C++ Framework Features
- **Modern C++17**: Object-oriented design with inheritance, polymorphism
- **Template Programming**: Type-safe audio processing
- **Smart Pointers**: Automatic memory management with RAII
- **Namespace Organization**: Clean Audio:: namespace structure
- **Fast Compilation**: Ninja build system with parallel compilation

## 🔧 Hardware Requirements

### Required Components
- **Raspberry Pi Pico** (or Pico W)
- **I2S Audio DAC** (e.g., PCM5102, MAX98357A)
- **Speakers or Headphones**
- **USB Cable** (for programming and serial communication)

### Pin Connections
```
Pico GPIO  →  I2S DAC     Description
-----------------------------------------
GPIO 26    →  DIN         Data Input
GPIO 27    →  BCLK        Bit Clock
GPIO 28    →  LRCLK       Left/Right Clock
GPIO 22    →  XMT         Mute Control (PCM5102)
3.3V       →  VCC         Power Supply
GND        →  GND         Ground
```

## 🚀 Quick Start

### Prerequisites
- **Pico SDK v2.1.1** installed at:
  ```
  C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\
  ```
- **MinGW-w64** or equivalent build tools
- **CMake 3.13+**
- **Ninja Build System** (recommended)

### Build and Flash

#### Option 1: Automated Build (Windows)
```batch
# Run the automated build script
.\build_pico.bat
```

#### Option 2: Manual Build
```bash
# Set environment variables
export PICO_SDK_PATH="C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk"
export PICO_EXTRAS_PATH="C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras"

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..

# Build the project
ninja
```

### Flash to Pico
1. Hold the **BOOTSEL** button on your Pico
2. Connect USB cable to computer
3. Copy `build\interactive_midi_synth.uf2` to the **RPI-RP2** drive
4. Pico will reboot automatically and start the synthesizer

### Connect Serial Terminal
1. Open Device Manager and find the new COM port
2. Connect using any serial terminal (115200 baud):
   - **PuTTY**: Set Serial, 115200 baud
   - **Arduino IDE**: Serial Monitor
   - **Windows Terminal**: Use built-in serial

## 🎹 Keyboard Controls

### Method 1: Combination Keys (Recommended)

True implementation of combination key functionality:

| Step | Key | Function |
|------|-----|----------|
| **Activate Low Mode** | `[` | Enable Shift mode for low octave |
| **Activate High Mode** | `]` | Enable Alt mode for high octave |
| **Play Notes** | `1-7` | Play notes in current mode |
| **Cancel Mode** | `ESC` | Reset combination key state |

**Usage Examples:**
- `[ + 1` = Play low C (DO)
- `] + 5` = Play high G (SOL)
- Direct `3` = Play E (MI) in current octave

### Method 2: Direct Key Mapping (Compatibility)

| Octave Range | Key Mapping | Corresponding Notes |
|-------------|-------------|-------------------|
| **Low Octave** | `W E R T Y U I` | Low DO RE MI FA SOL LA SI |
| **Standard Octave** | `C F G J K L ;` | DO RE MI FA SOL LA SI |
| **High Octave** | `Z X V B N` | High DO RE MI FA SOL |

### Function Control Keys

| Key | Function | Description |
|-----|----------|-------------|
| `[` | Activate Shift | Next 1-7 will play low octave |
| `]` | Activate Alt | Next 1-7 will play high octave |
| `ESC` | Cancel Combination | Reset Shift/Alt state |
| `W` | Toggle Waveform | Piano synthesis ↔ Sine wave |
| `+` / `-` | Volume Control | Adjust volume ±10% |
| `M` | Mute Toggle | Mute/unmute audio |
| `O` | Octave Switch | Cycle through octaves 3/4/5 |
| `D` | Demo Scale | Play DO RE MI scale |
| `S` | Stop Playback | Stop current note |
| `I` | Memory Info | Show LRU cache and memory usage |
| `H` / `?` | Show Help | Display control instructions |
| `Q` | Quit Program | Exit synthesizer |

## 🎼 Note Frequency Reference

### 3rd Octave (Low Range)
| Note | Frequency | Musical Note |
|------|-----------|--------------|
| Low DO | 130.81 Hz | C3 |
| Low RE | 146.83 Hz | D3 |
| Low MI | 164.81 Hz | E3 |
| Low FA | 174.61 Hz | F3 |
| Low SOL | 196.00 Hz | G3 |
| Low LA | 220.00 Hz | A3 |
| Low SI | 246.94 Hz | B3 |

### 4th Octave (Standard Range)
| Note | Frequency | Musical Note |
|------|-----------|--------------|
| DO | 261.63 Hz | C4 |
| RE | 293.66 Hz | D4 |
| MI | 329.63 Hz | E4 |
| FA | 349.23 Hz | F4 |
| SOL | 392.00 Hz | G4 |
| LA | 440.00 Hz | A4 |
| SI | 493.88 Hz | B4 |

### 5th Octave (High Range)
| Note | Frequency | Musical Note |
|------|-----------|--------------|
| High DO | 523.25 Hz | C5 |
| High RE | 587.33 Hz | D5 |
| High MI | 659.25 Hz | E5 |
| High FA | 698.46 Hz | F5 |
| High SOL | 783.99 Hz | G5 |
| High LA | 880.00 Hz | A5 |
| High SI | 987.77 Hz | B5 |

## 🎵 Usage Examples

### Playing Simple Melodies

#### Using Combination Keys:
1. **Twinkle Twinkle Little Star**: `1 1 5 5 6 6 5` (standard octave)
2. **Happy Birthday (fragment)**: `1 1 2 1 4 3` (standard octave)
3. **Mary Had a Little Lamb**: `3 2 1 2 3 3 3` (standard octave)

#### Using Direct Keys:
1. **Twinkle Twinkle**: `C C H H L L H` (DO DO SOL SOL LA LA SOL)
2. **Happy Birthday**: `C C F C J G` (DO DO RE DO FA MI)
3. **Mary Had a Little Lamb**: `G F C F G G G` (MI RE DO RE MI MI MI)

### Multi-Octave Performance

#### Combination Key Method:
1. Press `[` to activate low mode, then press `1-7`
2. Press `]` to activate high mode, then press `1-7`
3. Direct press `1-7` for current octave

#### Traditional Method:
1. Press `O` to switch octaves
2. Press `1-7` to play in current octave
3. Or use dedicated octave letter keys

## 🔧 Technical Implementation

### Core Components

- **AudioAPI**: Unified audio interface with event system
- **PicoAudioCore**: Pico platform audio driver with I2S support
- **InteractiveMIDISynth**: Real-time keyboard interaction controller
- **WaveGenerator**: Multi-harmonic synthesis with ADSR envelope
- **MusicSequencer**: Note sequence playback management

### Key Features

- **Non-blocking Input**: Uses `getchar_timeout_us(0)` for real-time response
- **Multi-octave Mapping**: Complete 3-octave frequency mapping
- **Event Callback System**: Real-time playback status feedback
- **Memory Optimization**: LRU cache with smart resource management
- **Hardware Integration**: Direct GPIO mute control and I2S audio output

### Audio Configuration

```cpp
// Optimized audio settings for memory efficiency
AudioConfig config;
config.sample_rate = 32000;    // Balanced quality/memory (32kHz)
config.channels = 2;           // Stereo output
config.bit_depth = 16;         // 16-bit samples
config.buffer_size = 768;      // Optimized buffer size
```

### Piano Synthesis

#### Harmonic Structure
- **Fundamental**: 100% amplitude
- **2nd Harmonic**: 50% amplitude
- **3rd Harmonic**: 30% amplitude
- **4th Harmonic**: 20% amplitude
- **5th Harmonic**: 15% amplitude
- **6th Harmonic**: 10% amplitude

#### ADSR Envelope
- **Attack**: 20ms rapid rise
- **Decay**: 100ms decay to sustain
- **Sustain**: 40% of peak volume
- **Release**: 200ms natural fade

## 📁 Project Structure

```
Audio-Pico/
├── CMakeLists.txt                  # Build configuration
├── build_pico.bat                  # Windows build script
├── include/                        # C++ header files
│   ├── pin_config.hpp             # Unified pin configuration
│   ├── AudioAPI.hpp               # Main audio interface
│   ├── PicoAudioCore.hpp          # Pico audio driver
│   ├── WaveGenerator.hpp          # Audio synthesis
│   ├── MusicSequencer.hpp         # Sequence playback
│   ├── Notes.hpp                  # Note definitions
│   └── AudioCore.hpp              # Base audio interface
├── src/                           # C++ source files
│   ├── AudioAPI.cpp               # Audio API implementation
│   ├── PicoAudioCore.cpp          # Pico driver implementation
│   └── MusicSequencer.cpp         # Sequencer implementation
├── examples/                      # Example programs
│   └── interactive_midi_synth.cpp # Main MIDI synthesizer
├── build/                         # Build output directory
│   └── interactive_midi_synth.uf2 # Compiled firmware
└── README.md                      # This file
```

## 🔍 Troubleshooting

### No Serial Port Appears
1. Check that both USB and UART are enabled in CMakeLists.txt:
   ```cmake
   pico_enable_stdio_usb(interactive_midi_synth 1)
   pico_enable_stdio_uart(interactive_midi_synth 1)
   ```
2. Try a different USB cable (data-capable, not power-only)
3. Check Device Manager for "Unknown Device"
4. Reinstall Pico drivers if needed

### No Audio Output
1. Verify I2S connections (GPIO 26, 27, 28)
2. Check DAC power supply (3.3V stable)
3. Test with headphones/speakers
4. Ensure audio format compatibility
5. Check mute status (press `M` to unmute)
6. Adjust volume (press `+` to increase)

### No Keyboard Response
1. Ensure serial terminal is connected and active
2. Try pressing `H` to display help menu
3. Check serial monitor configuration (115200 baud)
4. Verify USB cable supports data transmission

### Compilation Issues
1. Verify Pico SDK v2.1.1 installation path
2. Check MinGW-w64 and CMake installation
3. Ensure Ninja build system is installed
4. Run `build_pico.bat` as Administrator if needed
5. Clear build directory and rebuild: `rmdir /s build && .\build_pico.bat`

### Audio Quality Issues
1. Adjust volume using `+`/`-` keys (optimal range: 30-80%)
2. Switch between piano and sine wave modes (`W` key)
3. Check for electrical interference on I2S lines
4. Verify stable 3.3V power supply
5. Use short, shielded wires for I2S connections

### Memory Issues
1. Monitor memory usage with `I` key
2. Check LRU cache status (should show active resources)
3. Restart synthesizer if memory warnings appear
4. Consider reducing polyphony if experiencing issues

## 🧑‍💻 Development and Customization

### Building from Source

```bash
# Clone repository
git clone <repository-url>
cd Audio-Pico

# Set environment variables (Windows)
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
set PICO_EXTRAS_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras

# Build with automated script
.\build_pico.bat

# Or manual build
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### Customization Options

#### Audio Configuration
```cpp
// In AudioAPI constructor or configuration
AudioConfig config;
config.sample_rate = 44100;    // Higher quality: 44.1kHz
config.channels = 2;           // Stereo output
config.bit_depth = 16;         // 16-bit samples
config.buffer_size = 1156;     // Larger buffer for stability
```

#### Note Duration and Volume
```cpp
// In InteractiveMIDISynth class
#define AUDIO_NOTE_DURATION_MS  300     // Note duration (ms)
#define AUDIO_DEFAULT_VOLUME    70      // Default volume (0-100)
#define AUDIO_LRU_CACHE_SIZE    12      // LRU cache size
```

#### Pin Configuration
Edit `include/pin_config.hpp` to change hardware pin assignments:
```cpp
// I2S Audio pins
#define AUDIO_PIN_MUTE          22      // Mute control pin
#define AUDIO_PIN_DATA          26      // I2S data pin
#define AUDIO_PIN_BCLK          27      // I2S bit clock pin
#define AUDIO_PIN_LRCLK         28      // I2S LR clock pin
```

### Extension Ideas

1. **Multi-Note Playback**: Implement polyphonic synthesis for chords
2. **Recording and Playback**: Add performance recording capabilities
3. **MIDI File Support**: Import and play standard MIDI files
4. **Additional Waveforms**: Implement sawtooth, square wave synthesis
5. **Effects Processing**: Add reverb, delay, chorus effects
6. **Hardware Controls**: Add physical buttons and potentiometers
7. **Display Integration**: Add OLED/LCD display for visual feedback

### API Integration

The framework can be easily integrated into other projects:

```cpp
// Minimal integration example
#include "AudioAPI.hpp"
#include "PicoAudioCore.hpp"

// Create audio system
auto audio_core = std::make_unique<PicoAudioCore>();
auto audio_api = std::make_unique<AudioAPI>(std::move(audio_core));

// Initialize with default configuration
AudioConfig config;
config.sample_rate = 32000;
config.channels = 2;
config.bit_depth = 16;
config.buffer_size = 768;

audio_api->initialize(config);

// Play a note
audio_api->playNote(440.0f, 1000);  // Play A4 for 1 second
```

## 📋 Build Script Details

The `build_pico.bat` script automates the entire build process:

### Build Process Steps

1. **Environment Setup**: Sets PICO_SDK_PATH and PICO_EXTRAS_PATH
2. **Tool Verification**: Checks for Ninja, CMake, and SDK installation
3. **SDK Import**: Copies necessary import files to project root
4. **Project Validation**: Verifies required C++ files exist
5. **Clean Build**: Removes old build directory and creates new one
6. **CMake Configuration**: Configures project with Ninja generator
7. **Compilation**: Builds project with parallel compilation
8. **Output Verification**: Checks for successful UF2 generation

### Generated Files

After successful build, you'll find:
- **interactive_midi_synth.uf2**: Main firmware file for flashing
- **interactive_midi_synth.elf**: ELF executable for debugging
- **Additional debug files**: Map files, symbol tables, etc.

## 🎯 Performance Optimization

### Memory Management
- **LRU Cache**: Automatically manages audio resources with least-recently-used eviction
- **Smart Pointers**: Automatic memory cleanup with RAII principles
- **Optimized Buffers**: Balanced buffer sizes for quality vs. memory usage

### Real-Time Performance
- **Non-blocking I/O**: Zero-latency keyboard input processing
- **Efficient Synthesis**: Optimized harmonic generation algorithms
- **Hardware Integration**: Direct GPIO control for minimal latency

### Build Optimization
- **Ninja Build System**: Parallel compilation for faster builds
- **Release Configuration**: Optimized code generation (-O3)
- **Minimal Dependencies**: Only essential libraries included

## 📜 License and Credits

This project builds upon the Raspberry Pi Pico SDK and pico-extras libraries. The synthesizer implementation uses modern C++17 features for clean, maintainable code.

### Third-Party Libraries
- **Pico SDK v2.1.1**: Core Pico functionality
- **pico-extras**: Official audio libraries (pico_audio, pico_audio_i2s)
- **Standard C++ Libraries**: STL containers and smart pointers

---

🎵 **Happy Music Making with Audio-Pico!** 🎵
