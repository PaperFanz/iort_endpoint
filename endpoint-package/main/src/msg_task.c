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
#include "msg_arr.h"
#include "config.h"

/*
    GLOBALS
*/
static const char TAG[] = "MSG_JSONIFY";
static const char DEV_UUID[] = DEVICE_UUID;
static const char TOPIC[] = "device/"DEVICE_UUID"/data";
static const char MSG_FORMAT[] = 
"{"
    "\"uuid\":\""DEVICE_UUID"\","
    "\"time\":%lld,"
    "\"data\":["
        "%s"
    "]"
"}";

static char MSG[sizeof(MSG_FORMAT) + ((IOT_MSG_DATA_SIZE + 8) * 4) * sizeof(char)];

void msg_task(void * param)
{
    iot_msg_arr_t msgs = (iot_msg_arr_t) param;
    iot_msg_t msg;

    uint32_t notif = 0;
    while (true) {
        /* wait indefinitely and clear all pending messages */
        xTaskNotifyWait(0x0, 0xffffffff, &notif, portMAX_DELAY);

    }
}

const char * msg_jsonify(iot_msg_arr_t queue)
{
    /* get current time */
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;

    return MSG;
}

//taken from esp_sntp example
void time_init(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
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
    }
}

xTaskHandle msg_task_init(iot_msg_arr_t msgs)
{
    wifi_init();
    mqtt_init();
    time_init();

    xTaskHandle msg_task_handle;

    xTaskCreate(
        &msg_task,
        "msg_task",
        configMINIMAL_STACK_SIZE * 4,
        (void *) msgs,
        4,
        &msg_task_handle
    );

    return msg_task_handle;
}
