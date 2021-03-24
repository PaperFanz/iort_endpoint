/*

    analog.c - source file for analog sampling functionality

*/

#include "esp_log.h"

#include "analog.h"
#include "adc_func.h"
#include "config.h"

static analog_ch_t channels[ANALOG_CHANNEL_NUM];

static const adc_channel_t id_to_channel_remap[ANALOG_CHANNEL_NUM] = {
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
};

static volatile xQueueHandle iotMsgQueue;

uint32_t default_sampling_func(channel_id_t id, soft_avg_t avg)
{
    return read_adc1_raw(id_to_channel_remap[id], avg);
}

void default_formatting_func(iot_msg_t * msg, uint32_t sample)
{
    switch (msg->type) {
    case BOOL:
        msg->data.b = sample & 0x01;
        break;
    case INT64:
        msg->data.i64 = sample;
        break;
    case FLOAT64:
        msg->data.f64 = (double) sample / (double) UINT32_MAX;
        break;
    case STRING:
        snprintf(msg->data.s, IOT_MSG_DATA_SIZE, "%d units", sample);
        break;
    }
}

void analog_task(void * param)
{
    /*
        initialize
    */
    analog_ch_t * ch = (analog_ch_t *) param;

    TickType_t lastLoopTime = xTaskGetTickCount();
    /*
        main task loop
    */
    for (;;) {
        vTaskDelayUntil(&lastLoopTime, ch->rate);

        ch->sample = ch->sampling_func(ch->id, ch->avg);
        ch->formatting_func(&ch->msg, ch->sample);

        xQueueSend(iotMsgQueue, (void *) &ch->msg, (TickType_t) 10);
    }
}

void analog_init(xQueueHandle msgQueue)
{
    iotMsgQueue = msgQueue;

    adc_init();
    for (int i = 0; i < ANALOG_CHANNEL_NUM; ++i) {
        channels[i].id = i;
        channels[i].avg = X64;
        channels[i].sampling_func = &default_sampling_func;
        channels[i].formatting_func = &default_formatting_func;
        setup_adc(ADC_UNIT_1, 
                  id_to_channel_remap[i], 
                  ADC_ATTEN_11db, 
                  ADC_WIDTH_BIT_12);
    }
}

analog_err_t init_channel(channel_id_t ch, char * key, ros_msg_t type, TickType_t rate)
{
    if (ch >= ANALOG_CHANNEL_NUM) return INVALID_CHANNEL;
    snprintf(channels[ch].key, MAX_KEY_LEN, "%s", key);
    channels[ch].rate = rate;
    channels[ch].taskHandle = NULL;

    channels[ch].msg.key = channels[ch].key;
    channels[ch].msg.type = type;

    xTaskCreate(
        &analog_task, 
        "", 
        configMINIMAL_STACK_SIZE * 4, 
        &channels[ch], 
        configTASK_PRIORITY, 
        &channels[ch].taskHandle
    );

    if (channels[ch].taskHandle == NULL) return TASK_CREATION_FAILED;
    return ANALOG_OK;
}

analog_err_t shutdown_channel(channel_id_t ch)
{
    if (ch >= ANALOG_CHANNEL_NUM) return INVALID_CHANNEL;
    if (channels[ch].taskHandle == NULL) return CHANNEL_NOT_INITD;

    vTaskDelete(channels[ch].taskHandle);
    return ANALOG_OK;
}

analog_err_t set_sampling_func(channel_id_t ch, uint32_t (* f)(channel_id_t, soft_avg_t))
{
    if (ch >= ANALOG_CHANNEL_NUM) return INVALID_CHANNEL;
    channels[ch].sampling_func = f;
    return ANALOG_OK;
}

analog_err_t set_formatting_func(channel_id_t ch, void (* f)(iot_msg_t *, uint32_t))
{
    if (ch >= ANALOG_CHANNEL_NUM) return INVALID_CHANNEL;
    channels[ch].formatting_func = f;
    return ANALOG_OK;
}