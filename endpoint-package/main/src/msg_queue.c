/*

    Implements functions for synchronization queue between sensor polling task
    and message sending task

*/

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <string.h>
#include "../inc/msg_queue.h"

#define QUEUE_ITEM_SIZE sizeof(iot_msg_t)
#define QUEUE_BUF_BYTES (QUEUE_LEN * QUEUE_ITEM_SIZE)

uint8_t queue_buf[QUEUE_BUF_BYTES];

static StaticQueue_t msg_queue_data;

xQueueHandle init_msg_queue(void)
{
    xQueueHandle xqueue = xQueueCreateStatic(
        QUEUE_LEN,
        QUEUE_ITEM_SIZE,
        queue_buf,
        &msg_queue_data
    );

    /* 
        create static shouldn't fail unless QUEUE_BUF_BYTES is too large, 
        so no error checking code is necessary
    */

    return xqueue;
}

void set_bool_msg(iot_msg_t * msg, bool bool_)
{
    msg->type = BOOL;
    msg->data.b = bool_;
}

void set_int_msg(iot_msg_t * msg, int64_t int_)
{
    msg->type = INT64;
    msg->data.i64 = int_;
}

void set_float_msg(iot_msg_t * msg, double double_)
{
    msg->type = FLOAT64;
    msg->data.f64 = double_;
}

void set_string_msg(iot_msg_t * msg, char * string_)
{
    msg->type = STRING;
    strncpy(msg->data.s, string_, IOT_MSG_DATA_SIZE);
}

bool get_bool_msg(iot_msg_t * msg)
{
    return msg->data.b;
}

int64_t get_int_msg(iot_msg_t * msg)
{
    return msg->data.i64;
}

double get_float_msg(iot_msg_t * msg)
{
    return msg->data.f64;
}

char * get_string_msg(iot_msg_t * msg)
{
    return msg->data.s;
}
