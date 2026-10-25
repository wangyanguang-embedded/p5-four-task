#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "usart.h"
#include "types/sensortype.h"

void startcommutask(void *argument) {
    sensordata_t *data ;
    char buf[64];
    for (;;) {
        osMessageQueueGet(commuqueueHandle,&data,NULL,osWaitForever);
       sprintf(buf,"comm:t:%.1f\r\n",data->temperature);
        //HAL_UART_Transmit(&huart1,(uint8_t*)buf,strlen(buf),100);
        vPortFree(data);


    }

}
