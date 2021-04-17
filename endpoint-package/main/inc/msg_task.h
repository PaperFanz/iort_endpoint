/*
    msg_task.h
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "analog.h"
#include "msg.h"

xTaskHandle msg_task_init(void);
