This project uses the amazon embedded sdk (https://github.com/aws/aws-iot-device-sdk-embedded-C) to implement mqtt communication to IoT Core. At the moment, the program puts dummy data into a FreeRTOS queue and sends it to IoT Core. The stack allocation is wack though, and the program will fail pretty often. TODO.


Setting up the project:

-Add files to iot-rtos-pkg/main/CMakeLists.txt

-Change FreeRTOS headers from #include <*.h> -> include "freertos/*.h" 

-Set WIFI SSID and password in "msg_mqtt.c"

How to build and flash:

-Make sure that the following lines are in your .bashrc:

    export PATH="$HOME/<repo>/xtensa-esp32-elf/bin:$PATH"

    export IDF_PATH=~/<repo>/esp-idf

    export PATH="$IDF_PATH/tools:$PATH"

    *Note that you cannot run the last two lines for the other freertos project

-If idf.py is on your path as well, then you build using:

    idf.py -p /dev/<port ESP is connected on> build

    idf.py -p /dev/<port ESP is connected on> flash

    idf.py -p /dev/<port ESP is connected on> monitor

    *I personally just aliased idf as "alias idf='python ~/<repo>/esp-idf/tools/idf.py'", and run all the commands with idf instead of idf.py

Some Misc Notes:

-QueueCreateStatic doesn't work unless some config flag is set, for the moment, I replaced it with it's dynamic brother

-The JSON function is pretty basic, I will augment it to include more

-The topic is currently set to one thing, but it would be best to figure out how to name different topics

Notes on Porting the AWS Embedded SDK into the other project:

Honestly, if you're feeling extra motivated, you can try to set this up, but no sweat if not. We can try to figure it out after Spring Break.

What I've done so far:

-Copied the aws_iot folder from <repo>/esp-idf/components/aws_iot into <other_repo>/freertos/vendors/espressif/esp-idf/components

-The CMakeFiles.txt in the aws_iot folder has an if and else based of a CONFIG_..., removed the if-else so it is always built

-The Kconfig file in the aws_iot folder has statements that say "depends on" for all configs, removed the "depends on" line so the config is always set

-I've been testing by just including one of the aws headers in the main, for example you can add the following includes

    #include "aws_iot_config.h"

    #include "aws_iot_log.h"

    #include "aws_iot_version.h"

    #include "aws_iot_mqtt_client_interface.h"

-After doing this, the linker fails to find the mbedtls functions, causing the build to fail

-The library mbedtls seems to already exist in the project under <other_repo>/freertos/library/3rdparty, but I think since the esp-idf gets built seperately from FreeRTOS, there is some issue in linking? Not too sure what the issue is 

-I have also tried to simply copy the mbedtls folder from this repo's components to the other repo's components, but no luck



