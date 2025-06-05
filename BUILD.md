# Pico Audio I2S 32b 构建说明

## 🚀 快速构建

直接在项目根目录运行构建脚本：

```batch
build_pico.bat
```

## 📋 构建过程详解

构建脚本会执行以下步骤：

### 1. 环境变量设置
- `PICO_SDK_PATH`: C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
- `PICO_EXTRAS_PATH`: C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras
- `PICO_TOOLCHAIN_PATH`: C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\gcc-arm-none-eabi

### 2. SDK 导入文件复制
脚本会自动从 Pico SDK 目录复制必要的导入文件到项目根目录：
- `pico_sdk_import.cmake` - Pico SDK 导入文件
- `pico_extras_import.cmake` - Pico Extras 导入文件（如果可用）

**复制逻辑：**
- 优先从 SDK 的 `external/` 目录复制
- 如果不存在，则从 SDK 根目录复制
- 这样可以让项目更加独立，减少对环境变量的依赖

**项目结构简化：**
- 所有库定义都在根目录的单一 `CMakeLists.txt` 中
- 包含 `pico_audio_32b` 和 `pico_audio_i2s_32b` 库
- 包含 `sine_wave_i2s_32b` 示例项目

### 3. 清理构建
- 删除旧的 `build/` 目录
- 创建新的构建目录

### 4. CMake 配置
- 使用 MinGW Makefiles 生成器
- 自动检测和使用本地复制的导入文件

### 5. 编译
- 使用 `make -j4` 并行编译
- 生成 UF2 文件

## 📁 输出文件

构建成功后，生成的文件位于：
- **UF2 文件**: `build/sine_wave_i2s_32b.uf2`
- **ELF 文件**: `build/sine_wave_i2s_32b.elf`
- **其他调试文件**: `build/` 目录中

## 🔧 手动构建

如果需要手动构建，可以按以下步骤：

```batch
# 1. 设置环境变量
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-sdk
set PICO_EXTRAS_PATH=C:\Program Files\Raspberry Pi\Pico SDK v2.1.1\pico-extras

# 2. 复制导入文件（可选，CMakeLists.txt 会自动处理）
copy "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" .

# 3. 创建构建目录
mkdir build
cd build

# 4. 运行 CMake
cmake -G "MinGW Makefiles" ..

# 5. 编译
make
```

## 📝 注意事项

- 确保已安装 Pico SDK v2.1.1
- 确保已安装 MinGW 和 CMake
- 构建脚本会自动处理路径和依赖问题
- 生成的导入文件已添加到 `.gitignore`，不会被版本控制

## 🔌 硬件连接

项目使用 I2S 接口输出音频，需要按以下方式连接硬件：

```
Pico 引脚     -->   I2S 设备
GPIO 26      -->   DIN   (数据输入)
GPIO 27      -->   BCLK  (位时钟)
GPIO 28      -->   LRCLK (左右声道时钟)
```

**注意事项：**
- 确保 I2S 设备的电源电压与 Pico 兼容（通常是 3.3V）
- 如需要，可添加适当的电平转换电路
- 建议使用短导线连接，减少信号干扰

## 🎯 部署到 Pico

1. 按住 Pico 的 BOOTSEL 按钮
2. 将 Pico 连接到电脑
3. 将生成的 `do_re_me_demo.uf2` 文件拖拽到出现的 RPI-RP2 驱动器
4. Pico 会自动重启并运行程序
5. 通过串口可以看到引脚连接信息 