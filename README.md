
# Endpoint Firmware

This repository contains the firmware for the IoT-Robotics Integration project. It is built around [Amazon FreeRTOS](https://github.com/aws/amazon-freertos), [Amazon Web Services IoT Device SDK for Embedded C](https://github.com/aws/aws-iot-device-sdk-embedded-C), and [esp-idf](https://github.com/espressif/esp-idf). The main files for the project can be found under `endpoint-package/main`. Other major components of the project are available at the links below:

1. [`iort_endpoint`](https://github.com/PaperFanz/iort_endpoint) - this repository
2. [`insitu`](https://github.com/PaperFanz/insitu) - open source situational awareness package with extension support
3. [`iort_lib`](https://github.com/PaperFanz/iort_lib) - a ROS library that wraps calls to a web API for fetching IoT data from AWS Timestream
4. [`iort_filters`](https://github.com/PaperFanz/iort_filters) - example `insitu` extensions using `iort_lib` to construct real time data overlays

You should go through these four links in order and follow the setup isntructions in `README.md`.

## Hardware

This firmware targets the ESP32 Devkit-C V1 development board. It should be fairly easy to configure the project for a different development board or a later ESP32 MCU such as the ESP32-S2 or the ESP32-C3.

## Setting up the Project

There is some manual setup required to connect the endpoint firmware to AWS through your WiFi network. The below instructions are up to date as of May 18th, 2021.

### Connecting to AWS

The project uses AWS as the primary web component, so you will need to create a root AWS account at [https://portal.aws.amazon.com/billing/signup#/start](https://portal.aws.amazon.com/billing/signup#/start). Follow the instructions [here](https://aws.amazon.com/cli/) to set up the AWS command line interface.

Open a terminal on your local machine and run:

```sh
aws iot create-thing --thing-name "myEndpoint"
```

Open a web browser and log in to the AWS console, then navigate to the [AWS IoT Core page](https://us-west-2.console.aws.amazon.com/iot/home?region=us-west-2#/dashboard). ***Make sure that your CLI and Web Console are connected to the same AWS region.*** You can easily switch the Web Console to the right region from a dropdown menu in the upper right corner of the page. I used `us-west-2` for the full system demo.

In the sidebar, navigate to `Manage>Things` and verify that the name of the thing (`myEndpoint`) you just created in the CLI is there. **DO NOT** attempt to use the `Create` button on this page to create any endpoints; the web backend is bugged and will not work. Click on `myEndpoint>Security`, then select `Create certificate`. Download and save for later the:

* Certificate for this thing
* Private Key
* Root CA for AWS IoT

then click `Attach a policy`. Click `Create a new policy` and paste in the following, replacing `us-west-2` with your region:

```
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:Connect",
      "Resource": "arn:aws:iot:us-west-2:729534713919:*"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Publish",
      "Resource": "arn:aws:iot:us-west-2:729534713919:*"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Subscribe",
      "Resource": "arn:aws:iot:us-west-2:729534713919:*"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Receive",
      "Resource": "arn:aws:iot:us-west-2:729534713919:*"
    }
  ]
}
```

### Configuring the Project

Now we need to point the endpoint firmware to the certificate files you just generated and configure the WiFi settings in order to establish the connection with AWS.

#### AWS Certificates

Copy the certificate into `endoint-package/main/certs/certificate.pem.crt`.

Copy the private key into `endoint-package/main/certs/private.pem.key`.

Copy the root CA into `endoint-package/main/certs/aws-root-ca.pem`.

#### WiFi

Modify `endpoint-package/main/inc/config.h`:

Generate a new Version 4 UUID at [https://www.uuidgenerator.net/](https://www.uuidgenerator.net/) and paste it into the `DEVICE_UUID` definition:

```c
#define DEVICE_UUID "db24801e-c62c-4fc0-83e8-7866448e4fbf" // your uuid here
```

Navigate to `Manage>Things>myEndpoint>Interact` and copy the Thing Shadow URI into the `MQTT_HOST_ADDR` definition:

```c
#define MQTT_HOST_ADDR "adlphu38p2o1d-ats.iot.us-west-2.amazonaws.com" // your Rest API Endpoint here
```

Then fill in the following for your endoint deployment environment:

```c
#define WIFI_SSID "ssid"            // your WiFi network name
#define WIFI_PASSWORD "pass"        // your WiFi passowrd
#define AWS_CLIENT_ID "myEndpoint"  // your thing name
```

#### Editing Analog Channels

You may also configure the analog channels with different channel names, rates, and functions in `endpoint-package/main/src/main.c`. An example is given below:

```c
//           Channel id, channel name, data type, sample rate
init_channel(CH0, "light", STRING, taskHz(10));

//                Channel id, pointer to sampling function
set_sampling_func(CH0, &light_sample);

//                  Channel id, pointer to formatting function
set_formatting_func(CH0, &light_format);
```

See `analog.h` for full details on how these can be used to alter the behavior of the endpoint.

## Building the Project

First, clone the repository to your machine:

```sh
git clone --recursive https://github.com/PaperFanz/iort_endpoint
```

and add the following lines to your `.bashrc`:

```sh
export PATH="$HOME/<path-to-repository>/xtensa-esp32-elf/bin:$PATH"

export IDF_PATH="$HOME/<path-to-repository>/esp-idf"

export PATH="$IDF_PATH/tools:$PATH"
```

This will add the esp-idf binaries and scripts to your system path. After re-sourcing your `.bashrc`, you can build the project by running

```sh
idf.py -p /dev/<port> build
```

flash the firmware to the board by running 

```sh
idf.py -p /dev/<port> flash
```

and launch a serial monitor by running

```sh
idf.py -p /dev/<port> monitor
```

Note: `<port>` will most likely be `ttyUSB0` or something, you can find it by running `ls /dev` while the board is connected and disconnected and seeing which interface disappears when you disconnect.

## Testing

At this point, after building and flashing the endpoint firmware, you should be able to log on the the AWS Console, navigate to `IoT Core>Test>MQTT test client`, and see the uploaded messages by subscribing to `device/+/data`.

### Troubleshooting

```c
// TODO
```

## Modifying Firmware Functionality

Program execution starts in `main.c>app_main`. The interesting stuff largely happens in `analog.c` and `msg_task.c`. The `lib` directory contains functions for interacting with the e-ink display, ADC, and generating QR codes. Happy hacking!
