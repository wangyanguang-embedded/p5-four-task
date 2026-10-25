#include "ina219.h"

#include "main.h"

extern I2C_HandleTypeDef hi2c1;

void ina219_init(void) {
    uint8_t buf[2];

    // 配置寄存器：16V量程，12位分辨率
    uint16_t config = 0x199F;
    buf[0] = config >> 8;
    buf[1] = config & 0xFF;
    HAL_I2C_Mem_Write(&ina219_i2c, ina219_addr, ina219_config, 1, buf, 2, 100);

    // 校准寄存器
    uint16_t cal = 4096;
    buf[0] = cal >> 8;
    buf[1] = cal & 0xFF;
    HAL_I2C_Mem_Write(&ina219_i2c, ina219_addr, ina219_calib, 1, buf, 2, 100);
}

float ina219_read_current(void) {
    uint8_t buf[2];
    HAL_I2C_Mem_Read(&ina219_i2c, ina219_addr, ina219_current, 1, buf, 2, 100);
    int16_t current = (buf[0] << 8) | buf[1];
    return current * 0.1f;  // mA
}

float ina219_read_power(void) {
    uint8_t buf[2];
    HAL_I2C_Mem_Read(&ina219_i2c, ina219_addr, ina219_power, 1, buf, 2, 100);
    int16_t power = (buf[0] << 8) | buf[1];
    return power * 0.2f;  // W
}

float ina219_read_bus_voltage(void) {
    uint8_t buf[2];
    HAL_I2C_Mem_Read(&ina219_i2c, ina219_addr, ina219_bus_volt, 1, buf, 2, 100);
    uint16_t raw = (buf[0] << 8) | buf[1];
    return (raw >> 3) * 0.004f;  // V
}