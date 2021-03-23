/*

    config.h - defines hardcoded configuration options, such as those pertaining
    to static memory allocation and constants

*/

#ifndef CONFIG_H
#define CONFIG_H

#define taskHz(x)   (1000/x)/portTICK_PERIOD_MS

#define configTASK_PRIORITY 5

#endif
