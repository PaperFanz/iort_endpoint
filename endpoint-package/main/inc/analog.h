/*

    analog.h - header file for analog smapling functionality

*/

#ifndef ANALOG_H
#define ANALOG_H

#include <stdint.h>

#define MAX_KEY_LEN 256

typedef enum channel {
    CH0,
    CH1,
    CH2,
    CH4,
    ANALOG_CHANNEL_NUM,
} channel_t;

typedef enum soft_avg {
    NONE,
    X2,
    X4,
    X16,
    X32,
    X64,
} soft_avg_t;

typedef enum status {
    INACTIVE,
    ONESHOT,
    PERIODIC,
} ch_status_t;

typedef struct analog_ch {
    channel_t id;                       // identified on PCB and case
    char key[MAX_KEY_LEN];              // data key
    ch_status_t cond;                   // when to sample
    uint32_t tick_reset;                // tick reset value for periodic sample
    uint32_t tick;                      // current tick value
    uint32_t sample;                    // last sample value
    soft_avg_t avg;                     // apply software averaging (unused)

    /*
        sample and process raw ADC values, default to read_voltage
        args: none
        ret: uint32_t sample
    */
    uint32_t (* sampling_func)(void);

    /*
        format data as string for JSON message, default to snprintf("%d")
        args: char * str - write a null terminated string to the string pointer,
                           max length of 32
        ret: none
    */
    void (* formatting_func)(char *);
} analog_ch_t;

int init_channel(channel_t ch, char * key);

int init_channel_periodic(channel_t ch, char * key, uint32_t tick_reset);

void set_sampling_func(channel_t ch, uint32_t (* f)(void));

void set_formatting_func(channel_t ch, void (* f)(char *));

#endif
