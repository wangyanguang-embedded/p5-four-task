
#ifndef P5_FOUR_TASK_INA219_H
#define P5_FOUR_TASK_INA219_H
// ========== 硬件宏定义，换I2C或者换地址只改这里 ==========
#define ina219_i2c        hi2c1    // I2C句柄
#define ina219_addr       0x40<<1  // 设备地址（7位地址左移1位）

// ========== 寄存器地址 ==========
#define ina219_config     0x00    // 配置寄存器：量程、分辨率
#define ina219_bus_volt   0x02    // 母线电压寄存器
#define ina219_power      0x03    // 功率寄存器
#define ina219_current    0x04    // 电流寄存器
#define ina219_calib      0x05    // 校准寄存器

// ========== 函数声明 ==========
// 初始化ina219：写配置寄存器和校准寄存器
void ina219_init(void);

// 读电流：返回电流值，单位mA
float ina219_read_current(void);

// 读功率：返回功率值，单位mW
float ina219_read_power(void);

// 读母线电压：返回电压值，单位V
float ina219_read_bus_voltage(void);

#endif //P5_FOUR_TASK_INA219_H
