cmake_minimum_required(VERSION 3.13)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default generator to Ninja if not specified
if(NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja")
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
set(project_name "pico_audio_i2s_demo" C CXX ASM)
project(${project_name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

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
# Build example project: DO RE MI scale demo
# ==============================================================================
set(example_name "do_re_mi_demo")
add_executable(${example_name}
    samples/do_re_mi_demo/main.cpp
)

# Enable USB and UART serial output
pico_enable_stdio_usb(${example_name} 1)
pico_enable_stdio_uart(${example_name} 1)

# Link official pico-extras audio libraries
target_link_libraries(${example_name} PRIVATE
    pico_stdlib
    pico_audio           # Official audio base library
    pico_audio_i2s       # Official I2S library
)

# Set custom pin configuration and compile definitions
target_compile_definitions(${example_name} PRIVATE
    PICO_AUDIO_I2S_DATA_PIN=26
    PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
    # Disable WiFi features to avoid pioasm issues
    PICO_CYW43_SUPPORTED=0
    LIB_PICO_CYW43_ARCH=0
    LIB_PICO_CYW43_DRIVER=0
)

# Disable some features that might cause compilation issues
set(PICO_NO_FLASH 0)
set(PICO_NO_HARDWARE 0)
set(PICO_COPY_TO_RAM 0)

# Set program information
pico_set_program_name(${example_name} "DO RE MI Demo")
pico_set_program_version(${example_name} "2.0")

# Generate UF2 file (disable picotool to speed up compilation)
# pico_add_extra_outputs(${example_name})

# Manually generate UF2 file, avoiding picotool dependency
find_program(ELF2UF2_EXECUTABLE elf2uf2 PATHS ${PICO_SDK_PATH}/build/tools/elf2uf2)
if(ELF2UF2_EXECUTABLE)
    add_custom_command(TARGET ${example_name} POST_BUILD
        COMMAND ${ELF2UF2_EXECUTABLE} $<TARGET_FILE:${example_name}> ${example_name}.uf2
        COMMENT "Generating UF2 file: ${example_name}.uf2")
else()
    # If elf2uf2 not found, use basic binary output
    add_custom_command(TARGET ${example_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${example_name}> ${example_name}.bin
        COMMENT "Generating BIN file: ${example_name}.bin")
endif()

# ==============================================================================
# Project information output
# ==============================================================================
message(STATUS "===========================================")
message(STATUS "Pico Audio I2S DO RE MI Demo Project")
message(STATUS "===========================================")
message(STATUS "Using official pico-extras audio library")
message(STATUS "Example: ${example_name}")
message(STATUS "Output file: ${example_name}.uf2")
message(STATUS "===========================================")