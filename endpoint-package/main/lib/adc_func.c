#include <stdio.h>
#include <stdlib.h>

/* Project includes */
#include "debug.h"

/* ESP32 includes */
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_interface.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define DEFAULT_VREF    1099        //Used adc2_vref_to_gpio() to get value
#define NO_OF_SAMPLES   64          //Multisampling


static adc_bits_width_t adc2_width_config[ADC_CHANNEL_MAX];
static esp_adc_cal_characteristics_t adc_chars[2][ADC_CHANNEL_MAX];

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printdf(DEBUG_INFO, "eFuse Two Point: Supported\n");
    } else {
        printdf(DEBUG_INFO, "eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printdf(DEBUG_INFO, "eFuse Vref: Supported\n");
    } else {
        printdf(DEBUG_INFO, "eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printdf(DEBUG_INFO, "Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printdf(DEBUG_INFO, "Characterized using eFuse Vref\n");
    } else {
        printdf(DEBUG_INFO, "Characterized using Default Vref\n");
    }
}

void adc_init(){
    check_efuse();

    //Used for calibration
    /*esp_err_t err = adc2_vref_to_gpio(25);

    if(err != ESP_OK)
        printf("GPIO not supported\n");
    */
}


void setup_adc(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width){


    if (unit & ADC_UNIT_1) {
        
        if(adc1_config_width(width) != ESP_OK){
            printdf(DEBUG_ERR, "ADC1 channel %d adc1_config_width fail", channel);
            return;
        }
        
        if(adc1_config_channel_atten(channel, atten) != ESP_OK){
            printdf(DEBUG_ERR, "ADC1 channel %d adc1_config_channel_atten fail", channel);
            return;
        }

        esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, &(adc_chars[0][channel]));
        
        //NOTE: ADC1 channels 0 and 3 are used for the internal hall effect sensor
        if((channel == 0) || (channel == 3)){
            printdf(DEBUG_WARN, "ADC1 channel 0 and 3 are used by hall effect sensor");
        }

        printdf(DEBUG_DEBUG, "Configured ADC1 channel %d\n", channel);
        #if (DEBUG_LOG_LEVEL >= DEBUG_DEBUG) 
        print_char_val_type(val_type);
        #endif
    }

    if (unit & ADC_UNIT_2) {
        //the width for adc2 is given during reading so store it
        adc2_width_config[channel] = width;
        
        if(adc2_config_channel_atten(channel, atten) != ESP_OK){
            printdf(DEBUG_ERR, "ADC2 channel %d adc2_config_channel_atten fail", channel);
            return;
        }

        esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, &(adc_chars[1][channel]));
       
        printdf(DEBUG_WARN, "ADC2 is used by WiFi module");
        printdf(DEBUG_DEBUG, "Configured ADC2 channel %d\n", channel);
        #if (DEBUG_LOG_LEVEL >= DEBUG_DEBUG) 
        print_char_val_type(val_type);
        #endif
    }
    

}

uint32_t read_adc1_raw(adc_channel_t channel, uint32_t avg){
    
    uint32_t adc1_reading = 0;
    //Multisampling
    for (int i = 0; i < avg; i++) {
        int raw = adc1_get_raw((adc1_channel_t)channel);
        
        if(raw < 0){
            printdf(DEBUG_ERR, "Error in ADC1 Reading\n");
            return -1;
        }
        
        adc1_reading += raw;

    }

    adc1_reading /= avg;

    printdf(DEBUG_INFO, "Read %d on ADC1 channel %d\n", adc1_reading, channel);

    return adc1_reading;
}

uint32_t read_adc1_voltage(adc_channel_t channel){
    int raw = read_adc1_raw(channel, 64);
    
    if(raw < 0){
        return -1;
    }

    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &(adc_chars[0][channel]));

    printdf(DEBUG_INFO, "Read %dmV on ADC1 channel %d\n", voltage, channel);

    return voltage;
}

int read_adc2_raw(adc_channel_t channel){
    
    uint32_t adc2_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        int raw;
        esp_err_t err = adc2_get_raw((adc2_channel_t)channel, adc2_width_config[channel], &raw);
        
        if(err != ESP_OK){
            printdf(DEBUG_ERR, "Error in ADC2 Reading\n");
            return -1;
        }
        
        adc2_reading += raw;
    }

    adc2_reading /= NO_OF_SAMPLES;

    printdf(DEBUG_INFO, "Read %d on ADC2 channel %d\n", adc2_reading, channel);

    return adc2_reading;
}

uint32_t read_adc2_voltage(adc_channel_t channel){
    int raw = read_adc2_raw(channel);
    
    if(raw < 0){
        return -1;
    }

    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &(adc_chars[1][channel]));

    printdf(DEBUG_INFO, "Read %dmV on ADC2 channel %d\n", voltage, channel);

    return voltage;
}

/*
void app_main()
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
*/

