/*

    Declarations for synchronization queue between sensor polling task and 
    message sending task

*/

#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum ros_msg_type {
    BOOL,
    INT64,
    FLOAT64,
    STRING
} ros_msg_t;

#define IOT_MSG_KEY_SIZE 64     // adjustable depending on memory usage
#define IOT_MSG_DATA_SIZE 128   // adjustable, but no smaller than 4

typedef struct iot_msg {
    char key [IOT_MSG_KEY_SIZE];
    ros_msg_t type;
    uint8_t data[IOT_MSG_DATA_SIZE];
} iot_msg_t;

xQueueHandle init_msg_queue(void);

void set_bool_msg(iot_msg_t * msg, bool bool_);

void set_int_msg(iot_msg_t * msg, int64_t int_);

void set_float_msg(iot_msg_t * msg, double double_);

void set_string_msg(iot_msg_t * msg, char * string_);

bool get_bool_msg(iot_msg_t * msg);

int64_t get_int_msg(iot_msg_t * msg);

double get_float_msg(iot_msg_t * msg);

char * get_string_msg(iot_msg_t * msg);

#endif // MSG_QUEUE_H