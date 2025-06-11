#include <stdio.h>
#include <memory>

#include "pico/stdlib.h"

/**
 * @brief 调试测试程序
 * 用于诊断串口和基本功能问题
 */
int main() {
    // 初始化标准I/O
    stdio_init_all();
    
    // 等待更长时间以确保串口初始化完成
    sleep_ms(2000);

    printf("\n========================================\n");
    printf("🔧 调试测试程序启动\n");
    printf("========================================\n");
    printf("如果您看到这条消息，说明串口工作正常\n\n");

    // 测试基本串口输出
    for (int i = 1; i <= 10; i++) {
        printf("⏰ 测试输出 %d/10\n", i);
        sleep_ms(1000);
    }

    printf("\n📋 系统信息:\n");
    printf("  - Pico SDK 工作正常\n");
    printf("  - 串口输出功能正常\n");
    printf("  - 延时功能正常\n");

    printf("\n🔄 开始循环测试 (每5秒输出一次):\n");
    
    int counter = 1;
    while (true) {
        printf("⭕ 循环 %d - 时间戳: %llu ms\n", counter, time_us_64() / 1000);
        
        // 测试GPIO (如果连接了LED)
        printf("💡 测试板载LED\n");
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        
        // 闪烁LED
        for (int i = 0; i < 3; i++) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(200);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            sleep_ms(200);
        }
        
        printf("✅ LED测试完成\n\n");
        
        counter++;
        sleep_ms(5000);
    }

    return 0;
} 