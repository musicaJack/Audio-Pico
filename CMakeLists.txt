cmake_minimum_required(VERSION 3.13)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ninja-specific optimizations
if(CMAKE_GENERATOR STREQUAL "Ninja")
    # Enable parallel builds by default
    set(CMAKE_JOB_POOLS compile=4 link=2)
    message(STATUS "Ninja detected - enabling parallel compilation pools")
endif()

# Import Pico SDK - prefer local copied files
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
    message(STATUS "Using local pico_sdk_import.cmake")
elseif(DEFINED ENV{PICO_SDK_PATH})
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
    message(STATUS "Using pico_sdk_import.cmake from environment variable")
else()
    message(FATAL_ERROR "pico_sdk_import.cmake not found, please ensure PICO_SDK_PATH environment variable is set correctly")
endif()

# Enable pico-extras support - prefer local copied files
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/pico_extras_import.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/pico_extras_import.cmake)
    message(STATUS "Using local pico_extras_import.cmake")
elseif(DEFINED ENV{PICO_EXTRAS_PATH})
    include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
    message(STATUS "Using pico_extras_import.cmake from environment variable")
endif()

# Project configuration
set(project_name "pico_audio_framework" C CXX ASM)
project(${project_name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Enable C++17 features
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable fetching SDK and tools from Git (must be set before pico_sdk_init)
set(PICO_SDK_FETCH_FROM_GIT OFF)
set(PICO_TINYUSB_FETCH_FROM_GIT OFF)
set(PICOTOOL_FETCH_FROM_GIT OFF)

# Force disable picotool features to speed up compilation
set(PICO_NO_PICOTOOL ON CACHE BOOL "Disable picotool to speed up compilation")
set(PICOTOOL_FETCH_FROM_GIT_PATH OFF)
set(SKIP_PICOTOOL ON)

# Configure local picotool path (but don't enforce usage)
set(PICOTOOL_DIR "C:/Program Files/Raspberry Pi/Pico SDK v2.1.1/picotool")
if(EXISTS "${PICOTOOL_DIR}/picotool.exe")
    message(STATUS "✓ Found local picotool: ${PICOTOOL_DIR}/picotool.exe")
    set(PICOTOOL_FOUND TRUE)
    set(PICOTOOL_EXECUTABLE "${PICOTOOL_DIR}/picotool.exe")
else()
    message(STATUS "⚠️ Local picotool not found")
endif()

# Initialize Pico SDK
pico_sdk_init()

# ==============================================================================
# C++ Audio Framework Library (Static Library) - Simplified Build
# ==============================================================================
add_library(pico_audio_framework STATIC
    src/AudioAPI.cpp
    src/MusicSequencer.cpp
    src/PicoAudioCore.cpp
    # Note: WAV player components excluded due to missing pico_fatfs dependency
    # src/WAVPlayer.cpp
    # src/wav_stream_player.cpp
    # src/simple_wav_player.cpp
    # src/MicroSD/micro_sd.cpp
)

# Include directories for the framework
target_include_directories(pico_audio_framework PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Link required Pico libraries
target_link_libraries(pico_audio_framework PUBLIC
    pico_stdlib
    pico_audio           # Official audio base library
    pico_audio_i2s       # Official I2S library
    pico_multicore       # Multicore support for streaming
    hardware_gpio
)

# Set compile definitions
target_compile_definitions(pico_audio_framework PUBLIC
    PICO_AUDIO_I2S_DATA_PIN=26
    PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
    # Disable WiFi features to avoid pioasm issues
    PICO_CYW43_SUPPORTED=0
    LIB_PICO_CYW43_ARCH=0
    LIB_PICO_CYW43_DRIVER=0
)

# Enable C++17 features
target_compile_features(pico_audio_framework PUBLIC cxx_std_17)

# Compiler specific options
target_compile_options(pico_audio_framework PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-field-initializers>  # Suppress pico-extras warnings
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>  # Disable exceptions for embedded
)

# Disable some features that might cause compilation issues
set(PICO_NO_FLASH 0)
set(PICO_NO_HARDWARE 0)
set(PICO_COPY_TO_RAM 0)

# Manually generate UF2 file, avoiding picotool dependency
find_program(ELF2UF2_EXECUTABLE elf2uf2 PATHS ${PICO_SDK_PATH}/build/tools/elf2uf2)

# 注意：原始的C++音频框架演示程序已被移除
# 现在主要程序是 interactive_midi_synth

# ==============================================================================
# Build Interactive MIDI Synthesizer
# ==============================================================================
set(midi_synth_name "interactive_midi_synth")
add_executable(${midi_synth_name}
    examples/interactive_midi_synth.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${midi_synth_name} 1)
pico_enable_stdio_uart(${midi_synth_name} 1)

# Link the audio framework
target_link_libraries(${midi_synth_name} PRIVATE
    pico_audio_framework
)

# Set program information
pico_set_program_name(${midi_synth_name} "Interactive MIDI Synthesizer")
pico_set_program_version(${midi_synth_name} "1.0")

# Generate UF2 file
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${midi_synth_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${midi_synth_name}> ${midi_synth_name}.uf2
        COMMENT "Generating UF2 file: ${midi_synth_name}.uf2")
endif()

# ==============================================================================
# Build Legacy DO RE MI Demo (Original Version - Optional)
# ==============================================================================
# Uncomment the following section if you want to keep the original C-style demo
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/samples/do_re_mi_demo/main.cpp)
    set(legacy_demo_name "do_re_mi_demo_legacy")
    add_executable(${legacy_demo_name}
        samples/do_re_mi_demo/main.cpp
    )
    
    # Enable USB and UART serial output
    pico_enable_stdio_usb(${legacy_demo_name} 1)
    pico_enable_stdio_uart(${legacy_demo_name} 1)
    
    # Link official pico-extras audio libraries
    target_link_libraries(${legacy_demo_name} PRIVATE
        pico_stdlib
        pico_audio           # Official audio base library
        pico_audio_i2s       # Official I2S library
    )
    
    # Set custom pin configuration and compile definitions
    target_compile_definitions(${legacy_demo_name} PRIVATE
        PICO_AUDIO_I2S_DATA_PIN=26
        PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
        # Disable WiFi features to avoid pioasm issues
        PICO_CYW43_SUPPORTED=0
        LIB_PICO_CYW43_ARCH=0
        LIB_PICO_CYW43_DRIVER=0
    )
    
    # Set program information
    pico_set_program_name(${legacy_demo_name} "DO RE MI Demo (Legacy)")
    pico_set_program_version(${legacy_demo_name} "2.0")
    
    # Generate UF2 file
    if(ELF2UF2_EXECUTABLE)
        add_custom_command(TARGET ${legacy_demo_name} POST_BUILD
            COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${legacy_demo_name}> ${legacy_demo_name}.uf2
            COMMENT "Generating UF2 file: ${legacy_demo_name}.uf2")
    else()
        add_custom_command(TARGET ${legacy_demo_name} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${legacy_demo_name}> ${legacy_demo_name}.bin
            COMMENT "Generating BIN file: ${legacy_demo_name}.bin")
    endif()
    
    message(STATUS "Legacy demo enabled: ${legacy_demo_name}")
endif()

# ==============================================================================
# Documentation and Installation
# ==============================================================================

# Create an installation target
install(DIRECTORY include/ DESTINATION include/pico_audio_framework)
install(FILES 
    ${CMAKE_CURRENT_LIST_DIR}/README_CPP.md
    DESTINATION share/doc/pico_audio_framework)

# ==============================================================================
# Project information output
# ==============================================================================
message(STATUS "===========================================")
message(STATUS "Pico Audio C++ Framework (Unified Build)")
message(STATUS "===========================================")
message(STATUS "Using C++17 with modern OOP features")
message(STATUS "Framework: pico_audio_framework (header-only)")
message(STATUS "Main demo: ${cpp_demo_name}")
message(STATUS "MIDI Synth: ${midi_synth_name}")
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/samples/do_re_mi_demo/main.cpp)
    message(STATUS "Legacy demo: ${legacy_demo_name} (optional)")
endif()
message(STATUS "Build outputs:")
message(STATUS "  - ${cpp_demo_name}.uf2 (C++音频框架演示)")
message(STATUS "  - ${midi_synth_name}.uf2 (交互式MIDI合成器)")
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/samples/do_re_mi_demo/main.cpp)
    message(STATUS "  - ${legacy_demo_name}.uf2")
endif()
message(STATUS "===========================================")