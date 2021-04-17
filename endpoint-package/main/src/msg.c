/*

    Implements functions for synchronization queue between sensor polling task
    and message sending task

*/
#include <string.h>

#include "msg.h"
#include "analog.h"

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
