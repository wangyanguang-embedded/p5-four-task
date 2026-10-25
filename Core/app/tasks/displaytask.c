#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "usart.h"
#include "drivers/oled.h"
#include "types/sensortype.h"

void startdisplaytask(void *argument) {
    sensordata_t *data ;
    char buf [64];

    for (;;) {
        osMessageQueueGet(displayqueueHandle,&data,NULL,osWaitForever);
        OLED_NewFrame();

        sprintf(buf, "T:%.1fc V:%.2fv", data->temperature, data->voltage);
        OLED_PrintString(0, 0, buf, &font16x16, OLED_COLOR_NORMAL);

        sprintf(buf, "G:%.2fg I:%.1fma", data->vibration, data->current);
        OLED_PrintString(0, 20, buf, &font16x16, OLED_COLOR_NORMAL);

        OLED_ShowFrame();
        vPortFree(data);


    }
}
