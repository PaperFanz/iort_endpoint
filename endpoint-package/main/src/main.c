/*

    main.c - entry point of the endpoint package

*/

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

#include "config.h"
#include "analog.h"
#include "epd.h"
#include "epdgl.h"
#include "qrcode.h"

text_config_t txt_cfg = {
    .font = &Consolas20,
    .color = EPD_BLACK,
};

void display_qr(void)
{
    QRCode qr;
    uint8_t qrbuf[qrcode_getBufferSize(3)];

    qrcode_initText(&qr, qrbuf, 3, ECC_QUARTILE, "aws-iot-dev");

    int sq = 300 / qr.size;
    for (uint8_t y = 0; y < qr.size; y++) {
        for (uint8_t x = 0; x < qr.size; x++) {
            if (qrcode_getModule(&qr, x, y)) {
                epdgl_fill_rect(50+(x*sq), (y*sq), sq, sq, EPD_BLACK);
            } else {
                epdgl_fill_rect(50+(x*sq), (y*sq), sq, sq, EPD_WHITE);
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

    display_qr();

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    xQueueHandle iotMsgQueue = init_msg_queue();

    analog_init(iotMsgQueue);

    init_channel(CH0, "channel 0", INT64, taskHz(1));
}
