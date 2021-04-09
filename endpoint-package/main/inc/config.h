/*

    config.h - defines hardcoded configuration options, such as those pertaining
    to static memory allocation and constants

*/

#ifndef CONFIG_H
#define CONFIG_H

#define DEVICE_UUID "db24801e-c62c-4fc0-83e8-7866448e4fbf"

#define taskHz(x)   (1000/x)/portTICK_PERIOD_MS

#define configTASK_PRIORITY 5

/*
    AWS config
*/
#define MQTT_HOST_ADDR "a3p4yg5a5hm2rp-ats.iot.us-east-2.amazonaws.com"
#define MQTT_HOST_PORT 8883

/*
    WiFi config
*/
#define WIFI_SSID "godbox"
#define WIFI_PASSWORD "we<3venkata"
#define AWS_CLIENT_ID "myesp32"

#endif
