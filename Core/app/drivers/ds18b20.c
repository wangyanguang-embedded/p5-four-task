


#include <stdint.h>
#include <stdio.h>
#include <string.h>

# include "DS18B20.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"
// 微秒延时（基于DWT周期计数器）
void DelayUs(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
    //dwt初始化函数，放在init函数里面初始化一下就行
    // CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // DWT->CYCCNT = 0;
    // DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;


}
uint8_t ds18b20_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 输出模式，拉低480us
    GPIO_InitStruct.Pin = ds18b20_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ds18b20_port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_RESET);
    DelayUs(480);

    // 切换成输入模式，释放总线
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ds18b20_port, &GPIO_InitStruct);

    DelayUs(70);

    if (HAL_GPIO_ReadPin(ds18b20_port, ds18b20_pin) == GPIO_PIN_SET) {
        return 1;  // 没检测到
    }

    DelayUs(410);
    return 0;
}

void ds18b20_write_bit(uint8_t bit) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ds18b20_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ds18b20_port, &GPIO_InitStruct);

    if (bit == 1) {
        HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_RESET);
        DelayUs(2);
        HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_SET);
        DelayUs(60);
    } else {
        HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_RESET);
        DelayUs(60);
        HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_SET);
        DelayUs(2);
    }
}
uint8_t ds18b20_read_bit(void) {
    uint8_t bit;

    // 拉低2us
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ds18b20_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ds18b20_port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_RESET);
    DelayUs(2);

    // 切换成输入模式
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ds18b20_port, &GPIO_InitStruct);
    DelayUs(10);

    bit = HAL_GPIO_ReadPin(ds18b20_port, ds18b20_pin);

    DelayUs(50);
    return bit;
}

void ds18b20_write_byte(uint8_t dat) {
    for (uint8_t i = 0; i < 8; i++) {
        ds18b20_write_bit(dat & 0x01);  // 从最低位开始写
        dat >>= 1;
    }
}

uint8_t ds18b20_read_byte(void) {
    uint8_t dat = 0;
    for (uint8_t i = 0; i < 8; i++) {
        dat >>= 1;
        if (ds18b20_read_bit() == 1) {
            dat |= 0x80;  // 从最低位开始读
        }
    }
    return dat;
}
void ds18b20_start(void) {
    ds18b20_init();
    ds18b20_write_byte(0xCC);
    ds18b20_write_byte(0x44);
}

float ds18b20_read_temp(void) {
    uint8_t low, high;
    int16_t temp;


    taskENTER_CRITICAL();  // 关中断

    ds18b20_init();
    ds18b20_write_byte(0xCC);
    ds18b20_write_byte(0xBE);
    low = ds18b20_read_byte();
    high = ds18b20_read_byte();

    taskEXIT_CRITICAL();  // 开中断

    temp = (high << 8) | low;

    // 读完立刻启动下一次转换
    ds18b20_start();

    return temp * 0.0625f;
}

// float ds18b20_test(void) {
//     uint8_t low, high;
//     int16_t temp;
//     // 测试：不接传感器，写高电平，读引脚
//     HAL_GPIO_WritePin(ds18b20_port, ds18b20_pin, GPIO_PIN_SET);
//     DelayUs(10);
//     uint8_t pin = HAL_GPIO_ReadPin(ds18b20_port, ds18b20_pin);
//
//     char buf[32];
//     sprintf(buf, "no sensor, pin:%d\r\n", pin);
//     HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);
//
//
//     taskENTER_CRITICAL();
//
//     ds18b20_init();
//     ds18b20_write_byte(0xCC);
//     ds18b20_write_byte(0x44);  // 启动转换
//
//     taskEXIT_CRITICAL();
//
//     HAL_Delay(750);  // 等转换完成
//
//     taskENTER_CRITICAL();
//
//     ds18b20_init();
//     ds18b20_write_byte(0xCC);
//     ds18b20_write_byte(0xBE);  // 读温度
//     low = ds18b20_read_byte();
//     high = ds18b20_read_byte();
//
//     taskEXIT_CRITICAL();
//
//     temp = (high << 8) | low;
//     return temp * 0.0625f;
// }
