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
# C++ Audio Framework Library (Static Library)
# ==============================================================================
add_library(pico_audio_framework STATIC
    src/AudioAPI.cpp
    src/MusicSequencer.cpp
    src/PicoAudioCore.cpp
    src/WAVPlayer.cpp
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
    hardware_gpio
    hardware_spi         # SPI for SD card
    # TODO: Add FatFS library when available
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

# ==============================================================================
# Build C++ Audio Framework Demo (Main Program)
# ==============================================================================
set(cpp_demo_name "audio_demo_cpp")
add_executable(${cpp_demo_name}
    src/main.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${cpp_demo_name} 1)
pico_enable_stdio_uart(${cpp_demo_name} 1)

# Link the audio framework
target_link_libraries(${cpp_demo_name} PRIVATE
    pico_audio_framework
)

# Set program information
pico_set_program_name(${cpp_demo_name} "C++ Audio Framework Demo")
pico_set_program_version(${cpp_demo_name} "3.0")

# Generate UF2 file
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${cpp_demo_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${cpp_demo_name}> ${cpp_demo_name}.uf2
        COMMENT "Generating UF2 file: ${cpp_demo_name}.uf2")
else()
    add_custom_command(TARGET ${cpp_demo_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${cpp_demo_name}> ${cpp_demo_name}.bin
        COMMENT "Generating BIN file: ${cpp_demo_name}.bin")
endif()

# ==============================================================================
# Build Simple API Demo
# ==============================================================================
set(api_demo_name "simple_api_demo")
add_executable(${api_demo_name}
    examples/simple_api_demo.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${api_demo_name} 1)
pico_enable_stdio_uart(${api_demo_name} 1)

# Link the audio framework
target_link_libraries(${api_demo_name} PRIVATE
    pico_audio_framework
)

# Set program information
pico_set_program_name(${api_demo_name} "Simple API Demo")
pico_set_program_version(${api_demo_name} "1.0")

# Generate UF2 file
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${api_demo_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${api_demo_name}> ${api_demo_name}.uf2
        COMMENT "Generating UF2 file: ${api_demo_name}.uf2")
endif()

# ==============================================================================
# Build Debug Test (Basic Diagnostics)
# ==============================================================================
set(debug_test_name "debug_test")
add_executable(${debug_test_name}
    examples/debug_test.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${debug_test_name} 1)
pico_enable_stdio_uart(${debug_test_name} 1)

# Link basic libraries only
target_link_libraries(${debug_test_name} PRIVATE
    pico_stdlib
)

# Set program information
pico_set_program_name(${debug_test_name} "Debug Test")
pico_set_program_version(${debug_test_name} "1.0")

# ==============================================================================
# Build WAV Player Demo
# ==============================================================================
set(wav_demo_name "wav_player_demo")
add_executable(${wav_demo_name}
    examples/wav_player_demo.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${wav_demo_name} 1)
pico_enable_stdio_uart(${wav_demo_name} 1)

# Link the audio framework
target_link_libraries(${wav_demo_name} PRIVATE
    pico_audio_framework
)

# Set program information
pico_set_program_name(${wav_demo_name} "WAV Player Demo")
pico_set_program_version(${wav_demo_name} "1.0")

# Generate UF2 file
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${wav_demo_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${wav_demo_name}> ${wav_demo_name}.uf2
        COMMENT "Generating UF2 file: ${wav_demo_name}.uf2")
endif()

# Generate UF2 file
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${debug_test_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${debug_test_name}> ${debug_test_name}.uf2
        COMMENT "Generating UF2 file: ${debug_test_name}.uf2")
endif()

# ==============================================================================
# Build Mute Test (Static Control Test)
# ==============================================================================
set(mute_test_name "mute_test")
add_executable(${mute_test_name}
    examples/mute_test.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${mute_test_name} 1)
pico_enable_stdio_uart(${mute_test_name} 1)

# Link the audio framework
target_link_libraries(${mute_test_name} PRIVATE
    pico_audio_framework
)

# Set program information
pico_set_program_name(${mute_test_name} "Mute Control Test")
pico_set_program_version(${mute_test_name} "1.0")

# Generate UF2 file
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${mute_test_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${mute_test_name}> ${mute_test_name}.uf2
        COMMENT "Generating UF2 file: ${mute_test_name}.uf2")
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
message(STATUS "API demo: ${api_demo_name}")
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/samples/do_re_mi_demo/main.cpp)
    message(STATUS "Legacy demo: ${legacy_demo_name} (optional)")
endif()
message(STATUS "Build outputs:")
message(STATUS "  - ${cpp_demo_name}.uf2")
message(STATUS "  - ${api_demo_name}.uf2")
message(STATUS "  - ${debug_test_name}.uf2 (调试用)")
message(STATUS "  - ${mute_test_name}.uf2 (静音测试)")
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/samples/do_re_mi_demo/main.cpp)
    message(STATUS "  - ${legacy_demo_name}.uf2")
endif()
message(STATUS "===========================================")