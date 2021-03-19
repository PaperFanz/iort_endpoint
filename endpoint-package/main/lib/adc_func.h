/* Description yada yada yada
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

/* ESP32 includes */
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_interface.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"


void adc_init();
void setup_adc(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_bits_width_t width);
uint32_t read_adc1_raw(adc_channel_t channel);
int read_adc1_voltage(adc_channel_t channel);
int read_adc2_raw(adc_channel_t channel);
int read_adc2_voltage(adc_channel_t channel);

