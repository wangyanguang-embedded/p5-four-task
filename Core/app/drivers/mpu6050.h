

#ifndef P5_FOUR_TASK_MPU6050_H
#define P5_FOUR_TASK_MPU6050_H
#include "main.h"
//硬件宏定义
#define mpu6050_i2c hi2c1
#define mpu6050_addr 0x68<<1         //设备地址
//寄存器地址
#define mpu6050_who_am_i 0x75        //WHO_AM_I寄存器地址，读id使用
#define mpu6050_pwr_mgmt_1 0x6B      //电源管理寄存器地址，唤醒使用
#define mpu6050_accel_xout 0x3B      //x加速夫寄存器起始地址


uint8_t mpu6050_readid();

void mpu6050_init();

void mpu6050_readaccel(int16_t *x, int16_t *y, int16_t *z);

void i2c_scan();


#endif //P5_FOUR_TASK_MPU6050_H
