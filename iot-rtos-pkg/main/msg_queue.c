/*

    Implements functions for synchronization queue between sensor polling task
    and message sending task

*/

//#include <FreeRTOS.h>
//#include <queue.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "msg_queue.h"

#define QUEUE_LEN 10
#define QUEUE_ITEM_SIZE sizeof(iot_msg_t)
#define QUEUE_BUF_BYTES (QUEUE_LEN * QUEUE_ITEM_SIZE)

uint8_t queue_buf[QUEUE_BUF_BYTES];

static StaticQueue_t msg_queue_data;

xQueueHandle init_msg_queue(void)
{
    /*xQueueHandle xqueue = xQueueCreateStatic(
        QUEUE_LEN,
        QUEUE_ITEM_SIZE,
        queue_buf,
        &msg_queue_data
    );
    */

    xQueueHandle xqueue = xQueueCreate(QUEUE_LEN, QUEUE_ITEM_SIZE);

    /* 
        create static shouldn't fail unless QUEUE_BUF_BYTES is too large, 
        so no error checking code is necessary
    */

    return xqueue;
}

void set_bool_msg(iot_msg_t * msg, bool bool_)
{
    * (bool *) msg->data = bool_;
}

void set_int_msg(iot_msg_t * msg, int64_t int_)
{
    * (int64_t *) msg->data = int_;
}

void set_float_msg(iot_msg_t * msg, double double_)
{
    * (double *) msg->data = double_;
}

void set_string_msg(iot_msg_t * msg, char * string_)
{
    uint16_t i;
    for (i = 0; i < IOT_MSG_DATA_SIZE - 1; ++i) {
        if(string_ == NULL) break;
        msg->data[i] = *string_;
        ++string_;
    }
    msg->data[i] = 0;
}

bool get_bool_msg(iot_msg_t * msg)
{
    return * (bool *) msg->data;
}

int64_t get_int_msg(iot_msg_t * msg)
{
    return * (int64_t *) msg->data;
}

double get_float_msg(iot_msg_t * msg)
{
    return * (double *) msg->data;
}

char * get_string_msg(iot_msg_t * msg)
{
    return (char *) msg->data;
}
