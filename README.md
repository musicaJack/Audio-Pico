# 🎵 Pico Audio I2S DO RE MI Demo

A musical scale demonstration project for Raspberry Pi Pico using official pico-extras audio libraries. This project generates a DO RE MI scale with realistic piano tones through I2S audio output.

## ✨ Features

### 🎹 Audio Capabilities
- **Piano Tone Synthesis**: Multi-harmonic synthesis with ADSR envelope
- **Dual Audio Modes**: Piano tone vs Pure sine wave
- **I2S Audio Output**: 44.1kHz, 16-bit stereo
- **Real-time Audio Generation**: Low-latency audio processing

### 🎛️ Interactive Controls
- **Serial Interface**: USB and UART serial control
- **Speed Control**: Fast (5ms), Medium (10ms), Slow (20ms) note duration
- **Volume Control**: Real-time volume adjustment
- **Manual Note Control**: Skip to next note instantly
- **Tone Switching**: Toggle between piano and sine wave modes

### ⚡ Performance Optimizations
- **Fast Compilation**: Disabled unnecessary picotool downloads
- **Optimized Build**: Using official pico-extras libraries
- **Minimal Latency**: Efficient audio callback processing

## 🔧 Hardware Requirements

### Required Components
- **Raspberry Pi Pico** (or Pico W)
- **I2S Audio DAC** (e.g., PCM5102, MAX98357A)
- **Speakers or Headphones**
- **USB Cable** (for programming and serial communication)

### Pin Connections
```
Pico GPIO  →  I2S DAC
-----------------------
GPIO 26    →  DIN   (Data Input)
GPIO 27    →  BCLK  (Bit Clock)
GPIO 28    →  LRCLK (Left/Right Clock)
3.3V       →  VCC
GND        →  GND
```

## 🚀 Quick Start

### 1. Prerequisites
- **Pico SDK v2.1.1** installed at:
  ```
  C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\
  ```
- **MinGW-w64** or equivalent build tools
- **CMake 3.13+**

### 2. Build the Project
```bash
# Clone or download the project
cd pico_audio_i2s_32b

# Run the build script
.\build_pico.bat
```

### 3. Flash to Pico
1. Hold the **BOOTSEL** button on your Pico
2. Connect USB cable to computer
3. Copy `build\do_re_mi_demo.uf2` to the **RPI-RP2** drive
4. Pico will reboot automatically

### 4. Connect Serial Terminal
1. Open Device Manager and find the new COM port
2. Connect using any serial terminal (115200 baud):
   - **PuTTY**: Set Serial, 115200 baud
   - **Arduino IDE**: Serial Monitor
   - **Windows Terminal**: `wt -p cmd`

## 🎮 Usage Controls

### Serial Commands
Once connected to the serial terminal, use these keys:

| Key | Action |
|-----|--------|
| `+` | Increase volume |
| `-` | Decrease volume |
| `n` | Next note (manual) |
| `s` | Cycle speed (Fast/Medium/Slow) |
| `t` | Toggle tone (Piano/Sine wave) |
| `q` | Quit program |

### Speed Modes
- **Fast**: 5ms notes + 5ms pause (10ms total per note)
- **Medium**: 10ms notes + 5ms pause (15ms total per note)
- **Slow**: 20ms notes + 5ms pause (25ms total per note)

## 🎵 Musical Scale
The demo plays the standard DO RE MI scale:

| Note | Frequency | Musical Note |
|------|-----------|--------------|
| DO   | 261.63 Hz | C4           |
| RE   | 293.66 Hz | D4           |
| MI   | 329.63 Hz | E4           |
| FA   | 349.23 Hz | F4           |
| SOL  | 392.00 Hz | G4           |
| LA   | 440.00 Hz | A4           |
| SI   | 493.88 Hz | B4           |
| DO   | 523.25 Hz | C5           |

## 🎹 Piano Tone Synthesis

### Harmonic Structure
The piano tone uses 6 harmonics with decreasing amplitudes:
- **Fundamental**: 100% amplitude
- **2nd Harmonic**: 50% amplitude
- **3rd Harmonic**: 30% amplitude
- **4th Harmonic**: 20% amplitude
- **5th Harmonic**: 15% amplitude
- **6th Harmonic**: 10% amplitude

### ADSR Envelope
- **Attack**: 20ms rapid rise
- **Decay**: 100ms decay to sustain level
- **Sustain**: 40% of peak volume
- **Release**: 200ms natural fade

## 🛠️ Configuration

### Timing Configuration (in `main.cpp`)
```cpp
// Audio playback timing configuration
#define DEFAULT_NOTE_DURATION_MS 10    // Default note duration (ms)
#define DEFAULT_PAUSE_DURATION_MS 5    // Default pause between notes (ms)

#define SPEED_FAST_NOTE_MS 5           // Fast mode note time
#define SPEED_MEDIUM_NOTE_MS 10        // Medium mode note time  
#define SPEED_SLOW_NOTE_MS 20          // Slow mode note time
```

### Audio Configuration
```cpp
// Piano tone parameters
#define NUM_HARMONICS 6                // Number of harmonics
#define ATTACK_SAMPLES (44100 * 20 / 1000)   // 20ms attack time
#define DECAY_SAMPLES (44100 * 100 / 1000)   // 100ms decay time
#define SUSTAIN_LEVEL 0.4f             // Sustain volume (40%)
```

## 📁 Project Structure

```
pico_audio_i2s_32b/
├── CMakeLists.txt              # Build configuration
├── build_pico.bat             # Windows build script
├── samples/
│   └── do_re_mi_demo/
│       └── main.cpp           # Main application
├── build/                     # Build output directory
│   └── do_re_mi_demo.uf2     # Compiled firmware
├── .gitignore                 # Git ignore rules
└── README.md                  # This file
```

## 🔍 Troubleshooting

### No Serial Port Appears
1. Check that both USB and UART are enabled in CMakeLists.txt:
   ```cmake
   pico_enable_stdio_usb(${example_name} 1)
   pico_enable_stdio_uart(${example_name} 1)
   ```
2. Try a different USB cable
3. Check Device Manager for "Unknown Device"
4. Reinstall Pico drivers if needed

### No Audio Output
1. Verify I2S connections (GPIO 26, 27, 28)
2. Check DAC power supply (3.3V)
3. Test with headphones/speakers
4. Ensure audio format compatibility (44.1kHz, 16-bit)

### Compilation Issues
1. Verify Pico SDK path in environment variables
2. Check MinGW-w64 installation
3. Run `build_pico.bat` as Administrator if needed
4. Clear build directory and rebuild

### Audio Quality Issues
1. Adjust volume using `+`/`-` keys
2. Try switching between piano and sine wave modes (`t` key)
3. Check for electrical interference
4. Verify stable power supply

## 🧑‍💻 Development

### Building from Source
```bash
# Set environment variables (if not already set)
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
set PICO_EXTRAS_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras

# Create build directory
mkdir build
cd build

# Configure and build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### Adding New Features
1. Modify timing constants at the top of `main.cpp`
2. Add new musical scales in the frequency arrays
3. Implement additional audio effects in the synthesis functions
4. Extend serial command interface for new controls

## 📜 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## 🙏 Acknowledgments

- **Raspberry Pi Foundation** for the excellent Pico SDK
- **pico-extras** project for official audio libraries
- **Open source community** for inspiration and support

---

**Enjoy your musical Pico! 🎵**
