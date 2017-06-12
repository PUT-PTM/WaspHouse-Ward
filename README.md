# WaspHouse-Ward

## Overview 
Distributed IoT system - Wasp.
![19142231_1280419338741212_706988342_n 1](https://user-images.githubusercontent.com/13883700/27058331-d70e2140-4fd0-11e7-8553-0cac0a095475.png)

## Description
Distributed IoT system that allows meansurment and processing data from sensors like temperature, humidity, light intensity on the photocurrent and distance from the distance sensor.

This data can be use to turn on/off home windmill (to reduce the temperature or humidity), turn on/off lamp (when someone is near the entrance) or turn warning light - when someone is too close to our house.

Everything is sent, received and processed by three microcontrollers (STM32) equipped with WiFi module (ESP8266).

## Tools 
CooCox 
STM Studio
IntelliJ IDEA
STM32 ST-LINK Utility
Hercules

## How to run
1. Connect all sensors to two microcontrollers (which will be WaspHouse-Wards).
2. Upload WaspHouse-Center to one of microcontrollers (which will be WaspHouse-Center).
3. Upload WaspHouse-Ward to WaspHouse-Wards.
4. Recieve and display all data.

## How to compile
1. Run main.c in CooCox.
2. Read the data in console.

## Future improvements
1. Receive and process data from the pressure sensor.

## Attributions 
Łukasz Śmierzchalski, Łukasz Wolniak, Grzegorz Osak, Szymon Zieliński

## License 
GPU General Public License

## Credits
Łukasz Śmierzchalski, Łukasz Wolniak, Grzegorz Osak, Szymon Zieliński

The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.

Supervisor: Tomasz Mańkowski
