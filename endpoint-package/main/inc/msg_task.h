/*
    msg_task.h
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "analog.h"
#include "msg_arr.h"

xTaskHandle msg_task_init(iot_msg_arr_t msgs);

void msg_task(void * param);

const char * msg_jsonify(iot_msg_arr_t queue);
