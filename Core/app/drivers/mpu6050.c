#include "drivers/mpu6050.h"

#include <stdio.h>
#include <string.h>

#include "main.h"


extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
//读id
uint8_t mpu6050_readid() {
    uint8_t id;
    HAL_I2C_Mem_Read(&mpu6050_i2c,mpu6050_addr,mpu6050_who_am_i,1,&id,1,100);
    return id;

}

//初始化
void mpu6050_init() {
    uint8_t data = 0x00;//写电源管理寄存器0x6b,唤醒他
    //(12c地址，)
    HAL_I2C_Mem_Write(&mpu6050_i2c,mpu6050_addr,mpu6050_pwr_mgmt_1,1,&data,1,100);

}
void mpu6050_readaccel(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buf[6];
    HAL_I2C_Mem_Read(&mpu6050_i2c, mpu6050_addr, mpu6050_accel_xout, 1, buf, 6, 100);

      *x= (buf[0] << 8) | buf[1];
      *y= (buf[2] << 8) | buf[3];
      *z= (buf[4] << 8) | buf[5];
}

void i2c_scan() {
    char buf[32];
    for (uint8_t addr = 1; addr <= 127; addr++) {
        if (HAL_I2C_IsDeviceReady(&mpu6050_i2c,addr<<1,1,10)==HAL_OK) {
           sprintf(buf,"found:%d\r\n",addr);
            HAL_UART_Transmit(&huart1,(uint8_t*)buf,strlen(buf),10);
        }
    }

}
