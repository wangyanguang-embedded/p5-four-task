#include "stdint.h"

#ifndef P5_FOUR_TASK_DS18B20_H
#define P5_FOUR_TASK_DS18B20_H

#define  ds18b20_port GPIOB
#define  ds18b20_pin GPIO_PIN_0

uint8_t ds18b20_init(void);
void    ds18b20_write_bit(uint8_t bit);
uint8_t ds18b20_read_bit(void);
void ds18b20_start(void);
float ds18b20_read_temp(void);
float ds18b20_test(void);







#endif //P5_FOUR_TASK_DS18B20_H
