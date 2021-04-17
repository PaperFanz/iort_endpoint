/*
    msg_jsonify.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "esp_attr.h"
#include "driver/sdmmc_host.h"

#include "lwip/err.h"
#include "lwip/apps/sntp.h"

#include "msg_task.h"
#include "msg_mqtt.h"
#include "msg.h"
#include "config.h"
#include "analog.h"
#include "epdgl.h"

/*
    GLOBALS
*/
static const char TAG[] = "MSG_JSONIFY";
static const char DEV_UUID[] = DEVICE_UUID;
static const char TOPIC[] = "device/"DEVICE_UUID"/data";
static const char MSG_FORMAT_PRE[] = 
"{"
    "\"uuid\":\""DEVICE_UUID"\","
    "\"time\":%lld,"
    "\"data\":{";

static const char BOOL_DATA_FORMAT[] = 
"\"%s\":%s";

static const char INT64_DATA_FORMAT[] = 
"\"%s\":%lld";

static const char FLOAT64_DATA_FORMAT[] = 
"\"%s\":%g";

static const char STRING_DATA_FORMAT[] = 
"\"%s\":\"%s\"";

static const char MSG_FORMAT_POST[] =
    "}"
"}";

#define UINT64_MAX_CHARS 20
#define TIME_UUID_MAX_SIZE (sizeof(MSG_FORMAT_PRE) + UINT64_MAX_CHARS + 1)
#define KEY_VALUE_MAX_SIZE (IOT_MSG_KEY_SIZE + IOT_MSG_DATA_SIZE + 8)
#define CLOSE_PAR_MAX_SIZE (sizeof(MSG_FORMAT_POST) + 1)

static char MSG[
    TIME_UUID_MAX_SIZE +
    (KEY_VALUE_MAX_SIZE * 4) + 
    CLOSE_PAR_MAX_SIZE
];

static char DATA[ANALOG_CHANNEL_NUM][KEY_VALUE_MAX_SIZE];

text_config_t msg_txt_cfg = {
    .font = &Consolas20,
    .color = EPD_BLACK,
};

void msg_task(void * param)
{
    uint32_t notif = 0;
    uint32_t initd = 0;
    uint32_t i;
    struct timeval tv_now;
    int64_t time_us;
    while (true) {
        /* wait indefinitely and clear all pending messages */
        xTaskNotifyWait(0x0, 0xffffffff, &notif, portMAX_DELAY);

        /* update cached data on change */
        for (i = 0; i < ANALOG_CHANNEL_NUM; ++i) {
            if (notif & (1UL << i)) {
                /* data on channel i has changed */
                iot_msg_t * msg = get_channel_msg(i);
                switch (msg->type) {
                case BOOL:
                    snprintf(DATA[i], KEY_VALUE_MAX_SIZE, BOOL_DATA_FORMAT,
                             msg->key, (msg->data.b ? "true" : "false"));
                    break;
                case INT64:
                    snprintf(DATA[i], KEY_VALUE_MAX_SIZE, INT64_DATA_FORMAT,
                             msg->key, msg->data.i64);
                    break;
                case FLOAT64:
                    snprintf(DATA[i], KEY_VALUE_MAX_SIZE, FLOAT64_DATA_FORMAT,
                             msg->key, msg->data.f64);
                    break;
                case STRING:
                    snprintf(DATA[i], KEY_VALUE_MAX_SIZE, STRING_DATA_FORMAT,
                             msg->key, msg->data.s);
                    break;
                }
                initd |= (1UL << i);
            }
        }

        gettimeofday(&tv_now, NULL);
        time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
        snprintf(MSG, TIME_UUID_MAX_SIZE, MSG_FORMAT_PRE, time_us);
        /* concatenate all key-value pairs and send */
        for (i = 0; i < ANALOG_CHANNEL_NUM; ++i) {
            if (initd & (1UL << i)) {
                strcat(MSG, DATA[i]);
                if (i < ANALOG_CHANNEL_NUM - 1) {
                    strcat(MSG, ",");
                }
            }
        }
        strcat(MSG, MSG_FORMAT_POST);

        mqtt_publish(TOPIC, MSG);

        // ESP_LOGI(TAG, "MSG: %s\n", MSG);
    }
}

//taken from esp_sntp example
void time_init(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    epdgl_left_align_cursor();
    epdgl_draw_string("Initializing SNTP", &msg_txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
    
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        epdgl_draw_string(".", &msg_txt_cfg);
        while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);
    }
    epdgl_draw_string("RTC Synchronized\n", &msg_txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);
}

xTaskHandle msg_task_init()
{
    epdgl_left_align_cursor();
    epdgl_draw_string("connecting to "WIFI_SSID"...", &msg_txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);
    wifi_init();
    epdgl_draw_string("connected\n", &msg_txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);
    mqtt_init();
    time_init();

    xTaskHandle msg_task_handle;

    xTaskCreate(
        &msg_task,
        "msg_task",
        configMINIMAL_STACK_SIZE * 4,
        NULL,
        4,
        &msg_task_handle
    );

    return msg_task_handle;
}
