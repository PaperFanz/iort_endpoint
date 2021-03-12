#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "msg_mqtt.h"
#include "msg_queue.h"

static const char* TAG = "MSG";

volatile xQueueHandle xMsgQueue;

char* msg_jsonify(iot_msg_t *msg){
    static char json_string[IOT_MSG_KEY_SIZE + IOT_MSG_DATA_SIZE + 100];
    char payload[IOT_MSG_DATA_SIZE + 5];

    switch(msg->type){
        case BOOL:
            sprintf(payload, "%s", get_bool_msg(msg) ? "true" : "false");
            break;
        case INT64:
            sprintf(payload, "%" PRId64, get_int_msg(msg)); 
            break;
        case FLOAT64:
            sprintf(payload, "%f", get_float_msg(msg));
            break;
        case STRING:
            sprintf(payload, "\"%s\"", get_string_msg(msg));
            break;
    }

    ESP_LOGI(TAG, "Key is %s, Payload is %s\n", msg->key, payload);

    sprintf(json_string, "{\"%s\" : %s}", msg->key, payload);
    
    return json_string;
}

void msg_sender(void *param){

    const char* topic = "test_topic";
    mqtt_subscribe(topic);

    iot_msg_t *rx_msg;
    
    for(;;){
        
        if(xMsgQueue == NULL){
            ESP_LOGI(TAG, "Queue is fucked\n");
        }
        
        if(xQueueReceive(xMsgQueue, (void *) &rx_msg, ( TickType_t ) 10)){
            ESP_LOGI(TAG, "Recieve %s %s\n", rx_msg->key, rx_msg->data);
            mqtt_publish(topic, msg_jsonify(rx_msg)); 
        }
    }
}

void msg_create(void *param){

    iot_msg_t messages[16];

    for(int i = 0; i < 16; i++){

        sprintf(messages[i].key, "%s", "test");
        messages[i].type = STRING;
        set_string_msg(&(messages[i]), "test");

        ESP_LOGI(TAG, "Send %s %s \n", messages[i].key, messages[i].data);

        if(xMsgQueue == NULL){
            ESP_LOGI(TAG, "Queue is fucked\n");
        }

        iot_msg_t *temp = &(messages[i]);

        xQueueSend(xMsgQueue, (void *) &temp, (TickType_t) 10); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


    abort();

}

void msg_init(){

    wifi_init();
    mqtt_init();

    xMsgQueue = init_msg_queue();

    if(xMsgQueue == NULL){
        ESP_LOGI(TAG, "Queue is fucked\n");
    }


    xTaskCreate(&msg_sender, "msg_sender", 20000, NULL, 5, NULL);
    xTaskCreate(&msg_create, "msg_create", 9216, NULL, 4, NULL);

}

void app_main(){

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    msg_init(); 
}


