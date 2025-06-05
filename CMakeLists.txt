cmake_minimum_required(VERSION 3.13)

# 设置导出编译命令，方便IDE使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 导入 Pico SDK - 优先使用本地复制的文件
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)
    message(STATUS "使用本地的 pico_sdk_import.cmake")
elseif(DEFINED ENV{PICO_SDK_PATH})
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
    message(STATUS "使用环境变量中的 pico_sdk_import.cmake")
else()
    message(FATAL_ERROR "未找到 pico_sdk_import.cmake 文件，请确保 PICO_SDK_PATH 环境变量正确设置")
endif()

# 启用pico-extras支持 - 优先使用本地复制的文件
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/pico_extras_import.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/pico_extras_import.cmake)
    message(STATUS "使用本地的 pico_extras_import.cmake")
elseif(DEFINED ENV{PICO_EXTRAS_PATH})
    include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
    message(STATUS "使用环境变量中的 pico_extras_import.cmake")
endif()

# 项目配置
set(project_name "pico_audio_i2s_32b_project" C CXX ASM)
project(${project_name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 禁用从Git获取SDK和工具
set(PICO_SDK_FETCH_FROM_GIT OFF)
set(PICO_TINYUSB_FETCH_FROM_GIT OFF)

# 指定本地picotool路径（如果需要）
if(DEFINED ENV{PICO_TOOLCHAIN_PATH})
    set(PICOTOOL_FETCH_FROM_GIT OFF)
    # 尝试找到本地的picotool
    find_program(PICOTOOL_EXECUTABLE 
        NAMES picotool picotool.exe
        PATHS 
            "C:/Program Files/Raspberry Pi/Pico SDK v2.1.1/picotool"
            "C:/Program Files/Raspberry Pi/Pico SDK v2.1.1/picotool/bin"
            "${PICO_TOOLCHAIN_PATH}/../picotool"
            "${PICO_TOOLCHAIN_PATH}/../picotool/bin"
        NO_DEFAULT_PATH
    )
    if(PICOTOOL_EXECUTABLE)
        message(STATUS "找到本地picotool: ${PICOTOOL_EXECUTABLE}")
    endif()
endif()

# 初始化 Pico SDK
pico_sdk_init()

# ==============================================================================
# 定义 pico_audio_32b 库和头文件库
# ==============================================================================
if (NOT TARGET pico_audio_32b_headers)
    add_library(pico_audio_32b_headers INTERFACE)
    target_include_directories(pico_audio_32b_headers INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/pico_audio_32b/include
    )
endif()

if (NOT TARGET pico_audio_32b)
    add_library(pico_audio_32b INTERFACE)

    # 添加源文件
    target_sources(pico_audio_32b INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/pico_audio_32b/audio.cpp
        $<$<NOT:$<BOOL:${PICO_NO_HARDWARE}>>:${CMAKE_CURRENT_LIST_DIR}/src/pico_audio_32b/audio_utils.S>
    )

    # 链接依赖库
    target_link_libraries(pico_audio_32b INTERFACE
        pico_audio_32b_headers
        pico_sync
    )

    # 添加头文件目录
    target_include_directories(pico_audio_32b INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/pico_audio_32b
    )
endif()

# ==============================================================================
# 定义 pico_audio_i2s_32b 库
# ==============================================================================
if (NOT TARGET pico_audio_i2s_32b)
    add_library(pico_audio_i2s_32b INTERFACE)

    # 生成 PIO 头文件
    pico_generate_pio_header(pico_audio_i2s_32b
        ${CMAKE_CURRENT_LIST_DIR}/src/audio_i2s.pio
    )

    # 添加源文件
    target_sources(pico_audio_i2s_32b INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/audio_i2s.c
    )

    # 链接依赖库
    target_link_libraries(pico_audio_i2s_32b INTERFACE
        pico_stdlib
        pico_multicore
        hardware_dma
        hardware_pio
        hardware_irq
        pico_audio_32b
    )

    # 添加头文件目录
    target_include_directories(pico_audio_i2s_32b INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/include
    )
endif()

# ==============================================================================
# 构建示例项目：sine_wave_i2s_32b
# ==============================================================================
set(example_name "sine_wave_i2s_32b")
add_executable(${example_name}
    samples/sine_wave_i2s_32b/sine_wave.cpp
)

# 启用 USB 和 UART 串口输出
pico_enable_stdio_usb(${example_name} 1)
pico_enable_stdio_uart(${example_name} 1)

# 链接所需的库
target_link_libraries(${example_name} PRIVATE
    pico_stdlib
    pico_audio_32b
    pico_audio_i2s_32b
)

# 设置自定义引脚配置和编译定义
target_compile_definitions(${example_name} PRIVATE
    PICO_AUDIO_I2S_DATA_PIN=26
    PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
    # 禁用WiFi相关功能以避免pioasm问题
    PICO_CYW43_SUPPORTED=0
    LIB_PICO_CYW43_ARCH=0
    LIB_PICO_CYW43_DRIVER=0
)

# 禁用一些可能导致编译问题的功能
set(PICO_NO_FLASH 0)
set(PICO_NO_HARDWARE 0)
set(PICO_COPY_TO_RAM 0)

# 可选的 PIO 和 DMA 配置（当前注释掉）
#target_compile_definitions(${example_name} PRIVATE
#    PICO_AUDIO_I2S_PIO=1
#    PICO_AUDIO_I2S_DMA_IRQ=1
#    CORE1_PROCESS_I2S_CALLBACK
#)

# 设置程序信息
pico_set_program_name(${example_name} "sine_wave_i2s_32b")
pico_set_program_version(${example_name} "1.0")

# 生成 UF2 文件
pico_add_extra_outputs(${example_name})

# ==============================================================================
# 项目信息输出
# ==============================================================================
message(STATUS "===========================================")
message(STATUS "Pico Audio I2S 32b 项目配置完成")
message(STATUS "===========================================")
message(STATUS "库: pico_audio_i2s_32b")
message(STATUS "示例: ${example_name}")
message(STATUS "输出文件: ${example_name}.uf2")
message(STATUS "===========================================")