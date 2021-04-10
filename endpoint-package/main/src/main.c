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

#include "config.h"
#include "msg_task.h"
#include "msg_mqtt.h"
#include "msg.h"
#include "analog.h"
#include "epd.h"
#include "epdgl.h"
#include "qrcode.h"

/*
    GLOBALS
*/
text_config_t txt_cfg = {
    .font = &Consolas20,
    .color = EPD_BLACK,
};

/*
    clear display and draw centerd QR code encoding the device UUID
*/
void display_qr(void)
{
    epdgl_clear();
    QRCode qr;
    uint8_t qrbuf[qrcode_getBufferSize(3)];

    qrcode_initText(&qr, qrbuf, 3, ECC_MEDIUM, DEVICE_UUID);

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

void app_main(void)
{

    // initialize display
    epd_init();
    epdgl_init();
   
    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    epdgl_draw_string("nvs init done\n", &txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);

    /*
        msg task init depends on message array
    */
    xTaskHandle msg_task_handle = msg_task_init();

    epdgl_draw_string("msg task init done\n", &txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);

    /*
        analog init depends on message task handle and message array
    */
    analog_init(msg_task_handle);

    epdgl_draw_string("analog init done\n", &txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);

    /*
        channel init depends on analog init
    */ 
    init_channel(CH0, "channel 0", INT64, taskHz(10));
    init_channel(CH1, "channel 1", INT64, taskHz(4));

    epdgl_draw_string("channel init done\n", &txt_cfg);
    while (!epdgl_update_screen(EPD_FAST)) vTaskDelay(10);

    display_qr();
}
