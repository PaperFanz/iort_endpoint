/*

    main.c - entry point of the endpoint package

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

#include "nvs.h"
#include "nvs_flash.h"

#include "msg_mqtt.h"
#include "msg_queue.h"

#include "config.h"
#include "analog.h"
#include "epd.h"
#include "epdgl.h"
#include "qrcode.h"

/*
    GLOBALS
*/
static const char * DEV_UUID = DEVICE_UUID;

static const char * TOPIC = "device/"DEVICE_UUID"/data";

static const int UUID = 0;

static const char* TAG = "MAIN";

text_config_t txt_cfg = {
    .font = &Consolas20,
    .color = EPD_BLACK,
};

#define FMT_OVERALL1 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s]}"
#define FMT_OVERALL2 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s]}"
#define FMT_OVERALL3 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s]}"
#define FMT_OVERALL4 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s]}"
#define FMT_OVERALL5 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s%s]}"
#define FMT_OVERALL6 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s%s%s]}"
#define FMT_OVERALL7 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s%s%s%s]}"
#define FMT_OVERALL8 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s%s%s%s%s]}"
#define FMT_OVERALL9 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s%s%s%s%s%s]}"
#define FMT_OVERALL10 "{\"uuid\": %d, \"time\": %lld, \"data\": [%s%s%s%s%s%s%s%s%s%s]}"
#define FMT_OVERALL_SZ 32
#define FMT_MSG_COMMA "{\"%s\" : %s},"
#define FMT_MSG_COMMA_SZ 8
#define FMT_MSG "{\"%s\" : %s}"
#define FMT_MSG_SZ 7



void msg_jsonify_send(xQueueHandle queue){
    //static char* json_string;
    
    iot_msg_t msg;
    iot_msg_t temp_msg;

    char* json_array[QUEUE_LEN];
    int json_array_idx = 0;

    while(xQueueReceive(queue, (void *) &msg, (TickType_t) 0)){
        
        char payload[IOT_MSG_DATA_SIZE + 5];

        switch(msg.type){
            case BOOL:
                sprintf(payload, "%s", get_bool_msg(&msg) ? "true" : "false");
                break;
            case INT64:
                sprintf(payload, "%lld", get_int_msg(&msg)); 
                break;
            case FLOAT64:
                sprintf(payload, "%f", get_float_msg(&msg));
                break;
            case STRING:
                sprintf(payload, "\"%s\"", get_string_msg(&msg));
                break;
        }

        ESP_LOGI(TAG, "Key is %s, Payload is %s\n", msg.key, payload);

        
        char keyPayBuf[strlen(msg.key) + strlen(payload) + FMT_MSG_COMMA_SZ];
        
        if(xQueuePeek(queue, (void *) &temp_msg, (TickType_t) 10)){
            sprintf(keyPayBuf, FMT_MSG_COMMA, msg.key, payload);
        }
        else {
            sprintf(keyPayBuf, FMT_MSG, msg.key, payload);
        }

        ESP_LOGI(TAG, "Buffer is %s\n", keyPayBuf);
        
        size_t keyPayBufSz = strlen(keyPayBuf);
        
        json_array[json_array_idx] = (char *) malloc(keyPayBufSz);
        strncpy(json_array[json_array_idx], keyPayBuf, keyPayBufSz+1);
        json_array_idx++; 
        
    }
    
    for(int i = 0; i < json_array_idx; i++){
        ESP_LOGI(TAG, "Array at %d is %s\n", i, json_array[i]);    
    }

    size_t array_sz = 0;

    for(int i = 0; i < json_array_idx; i++){
        array_sz += strlen(json_array[i]);
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;

    char json_string[array_sz + FMT_OVERALL_SZ + 5];

    switch(json_array_idx){
        case 1:
            sprintf(json_string, FMT_OVERALL1, UUID, time_us, json_array[0]);
            break;
        case 2:
            sprintf(json_string, FMT_OVERALL2, UUID, time_us, json_array[0], json_array[1]);
            break;
        case 3:
            sprintf(json_string, FMT_OVERALL3, UUID, time_us, json_array[0], json_array[1], json_array[2]);
            break;
        case 4:
            sprintf(json_string, FMT_OVERALL4, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3]);
            break;
        case 5:
            sprintf(json_string, FMT_OVERALL5, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3], json_array[4]);
            break;
        case 6:
            sprintf(json_string, FMT_OVERALL6, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3], json_array[4], json_array[5]);
            break;
        case 7:
            sprintf(json_string, FMT_OVERALL7, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3], json_array[4], json_array[5], json_array[6]);
            break;
        case 8:
            sprintf(json_string, FMT_OVERALL8, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3], json_array[4], json_array[5], json_array[6], json_array[7]);
            break;
        case 9:
            sprintf(json_string, FMT_OVERALL9, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3], json_array[4], json_array[5], json_array[6], json_array[7], json_array[8]);
            break;
        case 10:
            sprintf(json_string, FMT_OVERALL10, UUID, time_us, json_array[0], json_array[1], json_array[2], json_array[3], json_array[4], json_array[5], json_array[6], json_array[7], json_array[8], json_array[9]);
            break;
    }

    vTaskDelay(10);

    for(int i = 0; i < json_array_idx; i++){
        free(json_array[i]);
    }

    ESP_LOGI(TAG, "Json String is %s", json_string);

    mqtt_publish(TOPIC, json_string);
    //return json_string;
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

void display_qr(void)
{
    QRCode qr;
    uint8_t qrbuf[qrcode_getBufferSize(3)];

    qrcode_initText(&qr, qrbuf, 3, ECC_QUARTILE, "WE <3 VENKATA");

    int sq = 300 / qr.size;
    int vm = (300 - (sq * qr.size)) / 2;
    for (uint8_t y = 0; y < qr.size; y++) {
        for (uint8_t x = 0; x < qr.size; x++) {
            if (qrcode_getModule(&qr, x, y)) {
                epdgl_fill_rect(50+(x*sq), vm + (y*sq), sq, sq, EPD_BLACK);
            } else {
                epdgl_fill_rect(50+(x*sq), vm + (y*sq), sq, sq, EPD_WHITE);
            }
        }
    }

    while (!epdgl_update_screen(EPD_SLOW)) vTaskDelay(10);
}

void consumer(void * param)
{
    xQueueHandle queue = (xQueueHandle) param;

    // mqtt_subscribe(topic);
    // mqtt_publish(topic, "Hello!");


    iot_msg_t msg;

    for (;;) {
        if (xQueuePeek(queue, (void *) &msg, (TickType_t) (5 / portTICK_PERIOD_MS))) {
            msg_jsonify_send(queue);
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{
   /*
    // initialize display
    epd_init();
    epdgl_init();
    display_qr();
    */
    
   
    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // inits for mqtt
    wifi_init();
    mqtt_init();
    time_init();

    xQueueHandle iotMsgQueue = init_msg_queue();

    analog_init(iotMsgQueue);

    xTaskCreate(
        &consumer,
        "consumer",
        configMINIMAL_STACK_SIZE * 4,
        (void *) iotMsgQueue,
        4,
        NULL
    );
    
    init_channel(CH0, "channel 0", INT64, taskHz(1));
    init_channel(CH1, "channel 1", INT64, taskHz(4));
}
