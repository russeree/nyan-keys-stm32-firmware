# NyanOS Firmware (NOS)
## Nyan Keys Keyboard Firmware
STM32F723 Firmware for the Nyan Keys keyboard. This architecture places the highest priority on peformance and reliability.
Some of th primary responsibilies of NOS are.
 - USB 2.0 HS - HID/CDC composite device
 - Serial console
 - EEPROM master
 - FPGA bitstream programmer
 - Status indication - 5 Leds

### Status Indication
On the Nyan Keys 0.8x - 0.9x boards there are 5 status leds that are activated upon boot. The labels for these LED(s) are as follows
| ID   | Name        | Description            |
| ---- | ----------- | ---------------------- |
| 0    | LED_0       | MCU Functional POST    |
| 1    | LED_1       | FPGA Configured        |

The system status LED should pulse at a rate of 1.287hz and have a period of 777ms. This is driven by TIM1 and TIM6 using interupts.

TIM1 (Timer 1)

