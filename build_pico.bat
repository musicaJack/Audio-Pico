@echo off
echo ========================================
echo   Pico Audio I2S 32b 项目构建脚本
echo ========================================

echo.
echo [1/6] 设置 Pico SDK v2.1.1 环境变量...
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
set PICO_EXTRAS_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras
set PICO_TOOLCHAIN_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\gcc-arm-none-eabi

rem 禁用网络下载，使用本地工具
set PICO_SDK_FETCH_FROM_GIT=OFF
set PICO_TINYUSB_FETCH_FROM_GIT=OFF
set PICOTOOL_FETCH_FROM_GIT=OFF

echo   PICO_SDK_PATH=%PICO_SDK_PATH%
echo   PICO_EXTRAS_PATH=%PICO_EXTRAS_PATH%
echo   PICO_TOOLCHAIN_PATH=%PICO_TOOLCHAIN_PATH%
echo   ✓ 环境变量设置完成

echo.
echo [2/5] 复制 Pico SDK 导入文件...
rem 复制pico_sdk_import.cmake文件(如果存在)
if exist "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" (
    echo   正在从 SDK external 目录复制 pico_sdk_import.cmake...
    copy "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" . >nul
    echo   ✓ 已复制 pico_sdk_import.cmake 文件
) else if exist "%PICO_SDK_PATH%\pico_sdk_import.cmake" (
    echo   正在从 SDK 根目录复制 pico_sdk_import.cmake...
    copy "%PICO_SDK_PATH%\pico_sdk_import.cmake" . >nul
    echo   ✓ 已复制 pico_sdk_import.cmake 文件
) else (
    echo   ⚠️  警告：未找到 pico_sdk_import.cmake 文件
)

rem 复制pico_extras_import.cmake文件(如果存在且启用)
if defined PICO_EXTRAS_PATH (
    if exist "%PICO_EXTRAS_PATH%\external\pico_extras_import.cmake" (
        echo   正在从 EXTRAS external 目录复制 pico_extras_import.cmake...
        copy "%PICO_EXTRAS_PATH%\external\pico_extras_import.cmake" . >nul
        echo   ✓ 已复制 pico_extras_import.cmake 文件
    ) else if exist "%PICO_EXTRAS_PATH%\pico_extras_import.cmake" (
        echo   正在从 EXTRAS 根目录复制 pico_extras_import.cmake...
        copy "%PICO_EXTRAS_PATH%\pico_extras_import.cmake" . >nul
        echo   ✓ 已复制 pico_extras_import.cmake 文件
    )
)

echo.
echo [3/5] 清理旧的构建文件...
if exist "build" (
    rmdir /s /q "build"
    echo   ✓ 已删除旧的构建目录
) else (
    echo   ✓ 没有发现旧的构建文件
)

echo.
echo [4/6] 创建新的构建目录...
mkdir "build"
cd "build"
echo   ✓ 构建目录创建完成

echo.
echo [5/6] 运行 CMake 配置...
cmake -G "MinGW Makefiles" ..
if %errorlevel% neq 0 (
    echo   ❌ CMake 配置失败！
    pause
    exit /b %errorlevel%
)
echo   ✓ CMake 配置完成

echo.
echo [6/6] 开始编译项目...
make -j4
if %errorlevel% neq 0 (
    echo   ❌ 编译失败！
    pause
    exit /b %errorlevel%
)

echo.
echo ========================================
echo   🎉 构建成功完成！
echo ========================================
echo   UF2 文件位置：build\sine_wave_i2s_32b.uf2
echo   
echo   现在您可以将 UF2 文件拖拽到 Pico 的 RPI-RP2 驱动器中
echo ========================================

pause 