@echo off
chcp 65001 >nul
echo ========================================
echo  🎵 Pico Audio C++框架 - 构建脚本
echo ========================================
echo 基于现代C++特性重构的音频项目
echo 支持继承、模板、智能指针等特性
echo 使用 Ninja 高速编译系统
echo ========================================
echo.

echo 步骤 1/8: 设置环境变量...
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
set PICO_EXTRAS_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras
set PICOTOOL_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\picotool
echo ✓ PICO_SDK_PATH=%PICO_SDK_PATH%
echo ✓ PICO_EXTRAS_PATH=%PICO_EXTRAS_PATH%

echo.
echo 步骤 2/8: 检查构建工具...
:: 检查Ninja是否安装
ninja --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ 错误: 未找到Ninja构建工具
    echo    请安装Ninja: choco install ninja
    echo    或从 https://github.com/ninja-build/ninja/releases 下载
    goto error
) else (
    for /f %%i in ('ninja --version') do echo ✓ Ninja版本: %%i
)

:: 检查CMake是否安装
cmake --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ 错误: 未找到CMake
    goto error
) else (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr "cmake version"') do echo ✓ CMake版本: %%i
)

:: 检查SDK路径
if not exist "%PICO_SDK_PATH%" (
    echo ❌ 错误: Pico SDK 路径不存在
    echo    请检查路径: %PICO_SDK_PATH%
    goto error
)

if not exist "%PICO_EXTRAS_PATH%" (
    echo ❌ 错误: Pico Extras 路径不存在  
    echo    请检查路径: %PICO_EXTRAS_PATH%
    goto error
)

echo.
echo 步骤 3/8: 复制SDK导入文件...
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
echo 步骤 4/8: 验证C++项目文件...
if not exist "include\AudioAPI.h" (
    echo ❌ 未找到C++头文件 include\AudioAPI.h
    goto error
)
if not exist "src\main.cpp" (
    echo ❌ 未找到C++主程序 src\main.cpp
    goto error
)
if not exist "CMakeLists.txt" (
    echo ❌ 未找到CMake配置文件 CMakeLists.txt
    goto error
)
echo ✓ C++项目文件验证通过

echo.
echo 步骤 5/8: 清理旧的构建目录...
if exist build (
    rmdir /s /q build
    echo ✓ 已删除旧的构建目录
)

echo.
echo 步骤 6/8: 创建构建目录...
mkdir build
cd build
echo ✓ 已创建并进入构建目录

echo.
echo 步骤 7/8: 运行 CMake 配置 (C++17模式, Ninja)...
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
if %ERRORLEVEL% neq 0 (
    echo ❌ CMake 配置失败
    echo 请检查是否安装了支持C++17的编译器和Ninja构建工具
    goto error
)
echo ✓ CMake 配置完成 (C++17, Release模式, Ninja)

echo.
echo 步骤 8/8: 编译C++音频框架...
echo 🔨 正在使用Ninja高速编译，请稍候...
ninja
if %ERRORLEVEL% neq 0 (
    echo ❌ 编译失败
    goto error
)

echo.
echo ========================================
echo ✅ C++音频框架构建成功！(Ninja高速编译)
echo ========================================
echo.

:: 检查输出文件
echo 📁 输出文件检查:
if exist "audio_demo_cpp.uf2" (
    echo ✓ 主程序: audio_demo_cpp.uf2
) else (
    echo ⚠️  警告: 主程序UF2文件未生成
)

if exist "simple_api_demo.uf2" (
    echo ✓ API演示: simple_api_demo.uf2
) else (
    echo ⚠️  警告: API演示UF2文件未生成
)

echo.
echo 🏗️ C++框架特性:
echo   ✓ 面向对象设计 (继承、多态)
echo   ✓ 模板编程 (类型安全)
echo   ✓ 智能指针 (自动内存管理)
echo   ✓ RAII (资源自动管理)
echo   ✓ 命名空间 (Audio::)
echo   ✓ 现代C++17语法
echo   ✓ Ninja高速并行编译
echo.
echo 🎵 硬件连接:
echo   GPIO 26 ^-^> DIN   ^(数据输入^)
echo   GPIO 27 ^-^> BCLK  ^(位时钟^)
echo   GPIO 28 ^-^> LRCLK ^(左右声道时钟^)
echo   GPIO 22 ^-^> XMT   ^(PCM5102静音控制^)
echo.
echo 🚀 烧录方法:
echo   1. 按住 Pico 上的 BOOTSEL 按钮
echo   2. 连接 USB 电缆到电脑
echo   3. 将 UF2 文件复制到 RPI-RP2 驱动器
echo      • audio_demo_cpp.uf2  - 完整功能演示
echo      • simple_api_demo.uf2 - 简单API示例
echo.
echo 📖 程序控制 (audio_demo_cpp.uf2):
echo   1-8键: 播放音符 ^(DO到DO5^)
echo   a键  : 自动播放DO RE MI音阶
echo   l键  : 循环播放音阶
echo   +/-键: 音量控制
echo   t键  : 切换音色 ^(钢琴/正弦波^)
echo   m键  : 静音切换
echo   s/p键: 停止/暂停
echo   q键  : 退出程序
echo.
echo 💡 API集成 (只需3行代码):
echo   auto core = std::make_unique^<PicoAudioCore^>^(^);
echo   auto api = std::make_unique^<AudioAPI^>^(std::move^(core^)^);
echo   api-^>initialize^(^); // 完成！
echo ========================================
goto end

:error
echo.
echo ========================================
echo ❌ C++音频框架构建失败
echo ========================================
echo 可能的原因:
echo   • Pico SDK路径不正确
echo   • 缺少C++17编译器支持
echo   • 缺少必要的项目文件
echo   • 缺少Ninja构建工具
echo   • CMake配置问题
echo.
echo 💡 解决建议:
echo   1. 检查Pico SDK v2.1.1是否正确安装
echo   2. 确保MinGW支持C++17标准
echo   3. 安装Ninja构建工具 (choco install ninja)
echo   4. 验证所有include/*.h文件存在
echo   5. 检查CMakeLists.txt文件完整性
echo ========================================
cd .. 2>nul
exit /b 1

:end
cd .. 2>nul
echo.
echo 🎉 构建完成！按任意键退出...
pause >nul 