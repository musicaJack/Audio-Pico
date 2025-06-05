@echo off
chcp 65001 >nul
echo =============================================
echo    Pico Audio I2S DO RE MI 项目构建脚本
echo =============================================
echo.

echo 步骤 1/6: 设置环境变量...
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
set PICO_EXTRAS_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras
set PICOTOOL_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\picotool
echo ✓ PICO_SDK_PATH=%PICO_SDK_PATH%
echo ✓ PICO_EXTRAS_PATH=%PICO_EXTRAS_PATH%

echo.
echo 步骤 2/6: 复制SDK导入文件...
if exist "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" (
    copy "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" . >nul 2>&1
    echo ✓ 已复制 pico_sdk_import.cmake
) else (
    echo ❌ 未找到 pico_sdk_import.cmake
    goto error
)

if exist "%PICO_EXTRAS_PATH%\external\pico_extras_import.cmake" (
    copy "%PICO_EXTRAS_PATH%\external\pico_extras_import.cmake" . >nul 2>&1
    echo ✓ 已复制 pico_extras_import.cmake
) else (
    echo ❌ 未找到 pico_extras_import.cmake
    goto error
)

echo.
echo 步骤 3/6: 清理旧的构建目录...
if exist build (
    rmdir /s /q build
    echo ✓ 已删除旧的构建目录
)

echo.
echo 步骤 4/6: 创建构建目录...
mkdir build
cd build
echo ✓ 已创建并进入构建目录

echo.
echo 步骤 5/6: 运行 CMake 配置...
cmake -G "MinGW Makefiles" ..
if %ERRORLEVEL% neq 0 (
    echo ❌ CMake 配置失败
    goto error
)
echo ✓ CMake 配置完成

echo.
echo 步骤 6/6: 编译项目...
mingw32-make -j4
if %ERRORLEVEL% neq 0 (
    echo ❌ 编译失败
    goto error
)

echo.
echo =============================================
echo ✅ 构建成功完成！
echo =============================================
echo 输出文件: build\do_re_mi_demo.uf2
echo.
echo 硬件连接提醒：
echo   GPIO 26 ^-^> DIN   ^(数据输入^)
echo   GPIO 27 ^-^> BCLK  ^(位时钟^)
echo   GPIO 28 ^-^> LRCLK ^(左右声道时钟^)
echo.
echo 烧录方法：
echo 1. 按住 Pico 上的 BOOTSEL 按钮
echo 2. 连接 USB 电缆到电脑
echo 3. 将 do_re_mi_demo.uf2 复制到 RPI-RP2 驱动器
echo =============================================
goto end

:error
echo.
echo ❌ 构建过程中出现错误，请检查上述输出信息
exit /b 1

:end
pause 