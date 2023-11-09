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

### EEPROM Address Layout
| Block | Address     | Description            | Length |
| ----  | ----------- | ---------------------- | ------ |
| 0     | 0x0000      | Board Serial Number    | 32     |
| 0     | 0x0020      | Board Owner            | 64     |
| 0     | 0x0060      | Board Build Block      | 16     |
| 0     | 0x0070      | Board Version          | 16     |
| 0     | 0x0080      | Total Keystrokes       | 16     |
| 0     | 0x0090      | Total USB Connections  | 16     |
| 0     | 0x00A0      | Total Times Powered On | 16     |
| 0     | 0x00B0      | FPGA Bitstream Len     | 16     |
| 0     | 0x00C0      | Reserved 0             | 16     |
| 0     | 0x00D0      | Reserved 1             | 16     |
| 0     | 0x00E0      | Reserved 2             | 16     |
| 0     | 0x00F0      | Reserved 3             | 16     |
| 0     | 0x0100      | Reserved 4             | 16     |
| 0     | 0x0110      | Reserved 5             | 16     |
| 0     | 0x0120      | Reserved 6             | 16     |
| 0     | 0x0130      | Reserved 7             | 16     |
| 0     | 0x0140      | Reserved 8             | 16     |
| 0     | 0x0150      | Reserved 9             | 16     |
| 0     | 0x0160      | Reserved 10            | 16     |
| 0     | 0x0170      | Reserved 11            | 16     |
| 0     | 0x0180      | Reserved 12            | 16     |
| 0     | 0x0190      | Reserved 13            | 16     |
| 0     | 0x01A0      | Reserved 14            | 16     |
| 0     | 0x01B0      | Reserved 15            | 16     |
| 0     | 0x01C0      | Reserved 16            | 16     |
| 0     | 0x01D0      | Reserved 17            | 16     |
| 0     | 0x01E0      | Reserved 18            | 16     |
| 0     | 0x01F0      | Reserved 19            | 16     |
| 1     | 0x0000      | FPGA Bitstream         | 8192   |

