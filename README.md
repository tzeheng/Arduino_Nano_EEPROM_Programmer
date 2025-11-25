# Arduino_Nano_EEPROM_Programmer
This repo contains the schematics as well as the Arduino C/ino and Python code to program 28c16 and other similar EEPROM chips.  
This EEPROM programmer was designed as part of a larger project to build a breadboard computer based on the 6802 inspired by [Ben Eater](https://eater.net)

Copyright 2025 Tze Heng Tan

Code and schematic are [MIT licensed](http://en.wikipedia.org/wiki/MIT_License)

## Hardware
This is a simple circuit for programming the 28C16, 28C64, 28C256, and similar parallel EEPROMs using an Arduino

### Parts list
|Part|Quantity |Notes |
|--|--|--|
|Arduino Nano|1|The brain|
|74HC595|2|8-bit shift registers|
|CAT28C16|1|16kbit parallel EEPROM chip I used|
|Breadboard|1|830-contact version used here|
|Breadboard wires|||

### Schematic
<img width="1744" height="932" alt="image" src="https://github.com/user-attachments/assets/457468ae-01da-4a4c-9f4a-6c107ecebfda" />
Since the Arduino doesn’t have enough pins to directly control all of the address, data, and control lines of the EEPROM, two 74HC595 shift registers are used for the 11 address lines (15 for the 28C256) and the output enable control line.

Here's a photo of my spaghetti (with wiring for a 28c16) 
<img width="4015" height="1440" alt="image" src="https://github.com/user-attachments/assets/5e9f91ff-6b4f-4634-a04e-dc41ceec0693" />

## Software
This repo contains the software written for this project and it consists of 2 main parts.
1. The firmware for the Arduino Nano for hosting the communication and actual EEPROM programming
2. The Python client code to communicate with the Arduino via USB-serial

### Requirements
1. Arduino IDE
2. Python > 3.8

### EEPROM_Programmer Firmware
the EEPROM_Programmer folder contains the .ino file for the basic programmer code to be compiled and written to the Arduino Nano, heavily inspired by [Ben Eater](https://github.com/beneater/eeprom-programmer) and [Tom Nisbet](https://github.com/beneater/eeprom-programmer)'s TommyPROM


### Python client scripts
- EEPROM.py: the python based client to communicate with the Arduino with the EEPROM_Programmer firmware.
  > python EEPROM.py -h will display a short usage guide
  > Example:  **python EEPROM.py -p COM9 -m r -s 1**   will connect to the Arduino Nano on **COM9** in **r**ead mode and returns the first **1**kb (yes bits) of data on the EEPROM.
- CreateROM.py: A simple few-liner to generate a binary ROM file to be burnt to the EERPOM
  > currently only creates non-sensical binary files. Used it to create binary files of various sizes for testing.
- firmware.bin: A 1kB (8kb) bin file containing 0x55s

