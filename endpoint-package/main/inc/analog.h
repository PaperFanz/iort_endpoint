/*

    analog.h - header file for analog smapling functionality

*/

#ifndef ANALOG_H
#define ANALOG_H

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "msg_queue.h"

#define MAX_KEY_LEN 256

typedef enum analog_error {
    ANALOG_OK = 0,
    INVALID_CHANNEL,
    CHANNEL_NOT_INITD,
    TASK_CREATION_FAILED,
} analog_err_t;

typedef enum channel {
    CH0,
    CH1,
    CH2,
    CH4,
    ANALOG_CHANNEL_NUM,
} channel_id_t;

typedef enum soft_avg {
    NONE = 1,
    X2 = 2,
    X4 = 4,
    X8 = 8,
    X16 = 16,
    X32 = 32,
    X64 = 64,
} soft_avg_t;

typedef struct analog_ch {
    channel_id_t id;
    char key[MAX_KEY_LEN];              // data key
    uint32_t sample;                    // last sample value
    soft_avg_t avg;                     // apply software averaging (unused)
    iot_msg_t msg;                      // stores current message
    TickType_t rate;                    // tracks sample rate
    TaskHandle_t taskHandle;            // used to shutdown the channel

    /*
        sample and process raw ADC values, default implementation in analog.c
        args: channel_id_t id - this channel's id
        ret: uint32_t - processed ADC sample
    */
    uint32_t (* sampling_func)(channel_id_t, soft_avg_t);

    /*
        format data as you'd like for the iot message
        args: iot_msg_t * msg - pointer to the message in this channel, default
                                implementation in analog.c
              uint32_t sample - last sample stored in the channel
        ret: bool - y/n write message to queue
    */
    bool (* formatting_func)(iot_msg_t *, uint32_t);
} analog_ch_t;

void analog_init(xQueueHandle iotMsgQueue);

analog_err_t init_channel(channel_id_t ch, char * key, ros_msg_t type, TickType_t rate);

analog_err_t shutdown_channel(channel_id_t ch);

analog_err_t set_sampling_func(channel_id_t ch, uint32_t (* f)(channel_id_t, soft_avg_t));

analog_err_t set_formatting_func(channel_id_t ch, bool (* f)(iot_msg_t *, uint32_t));

#endif
