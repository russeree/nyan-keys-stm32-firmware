# Why is this folder here?

## Reasoning
Using STM32 Cube MX to change peripherals causes the standard THIRD_PARTY directory to get overwritten and this breaks the USB HID composite driver. 

## Usage
After using STM32CUBEMX 
 1. Delete the ```Third_Party``` directory.
 2. Make a copy of this ```Third_Party_Master``` directory.
 3. Rename the newly copied directory to ```Third_Party```.