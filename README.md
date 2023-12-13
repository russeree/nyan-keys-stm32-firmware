<div align="center">
 <img src="assets/images/icon_square.png" width="120" height="120">
</div>

<br>

<div align="center">
    <h1>NyanOS 🐱 (NOS) - A Fast Keyboard Firmware</h1>
</div>

## Nyan Keys Keyboard Firmware
STM32F723 Firmware for the Nyan Keys keyboard. This architecture places the highest priority on performance, latency, and reliability.

### Supported Hardware

#### Keyboard PCBs
| PCB            | Description               |
| -------------- | ------------------------- |
| NyanKeys Proto | Nyan Keys Prototype 0     |

_Please make a PR if you decide to use NyanOS for your keyboard PCB_

#### MCUs
| MCU         | Description               |
| ----------- | ------------------------- |
| STM32F723xx | STM32 F7 w/ USB2.0 HS PHY |

### Responsibilities 
 - __USB 2.0 HS HID/CDC composite device__
 - __Serial console via USB__
 - __EEPROM master - FPGA Bitstream Storage__
 - __FPGA bitstream programmer - SPI Master__
 - __Status indication - 5 Leds__
 - __Bitcoin Miner - opt-in__
 - __USB HID Interface @ 8000hz Polling__
 - __SPI Master to FPGA switch serializer and debouncer__



### NyanOS Terminal
One of the nicer features of NyanOS is a fully functional USB-CDC (_serial_) interface to interact with NyanOSk. Currently functionality is limited to only the most necessary commands for keyboard operation and configuration. 

### FPGA Bitstream Loading
The NyanOS out of the box should support any Lattice Ice40HX FPGAs that are also supported by [IceStorm](https://github.com/YosysHQ/icestorm). For a complete hardware support list visit. [https://clifford.at/icestorm](https://clifford.at/icestorm) The flow for synthesizing, placing, and routing is outlined below

1. ```yosys <args>```
2. ```nextpnr <args>```
3. ```icepack <bitstream.asc> <bitstream.bin```
4. ```icecompr.py < bitstream.bin > bitstream_compr.bin```

The icecompr tool is utilized to compress the bitstream, typically achieving a final ratio of approximately 25-30%. This compression allows an Ice40HX4K bitstream to fit on a 1Mbit I2C EEPROM. Without compression, the bitstream would occupy 131070 bytes, exceeding the capacity of the I2C EEPROM.

In NyanOS, the FPGA is treated as an SPI slave. The system manages all dummy bits both before (8 bits) and after (47 bits) the bitstream programming. NyanOS sends 48 dummy bits to the slave, as 47 is not divisible by 8, and thus it rounds up.

The FPGA bitstream programming in NyanOS occurs at startup and typically takes 2-3 seconds. This duration is primarily due to loading the bitstream from the I2C bus at 200KHz. Speed improvements might be possible in future updates by using lower value pull-up resistors. Currently, 10K resistors are used in Nyan Keys hardware.

__NOTE:__ The time to load the Bitstream is roughly 2-3 seconds and will occur on device power-on. The FPGA can be reprogrammed without a complete device reset, by setting the nos_fpga->configured to false. The main loop will eventually catch this after the interrupts complete and reload the bitstream from the contents of the EEPROM IC that are in Bank 1, using the value stored in the EEPROM bank 0 EEPROM FPGA Bitstream Len address 0x00B0 aligned as 4 Words, where each word is little endian encoded. This will be fixed later but current functions correct and you can use the ```write-bitstream <size>``` command and this will all be handled. __THE MAXIMUM BITSTREAM SIZE IS 65536 BYTES__ anything more and you will get a size error returned.

User input to keys is not handled until the FPGA bitstream is loaded. Any keys pressed before configuration will not be relayed via the HID peripheral.

### Persistent Windows Logo Key Disable
Nyan Keys now supports Windows logo key disablement. The user just has to press [FN + Windows Logo Key] to toggle the state between enabled and disabled. Each time this is done, the state is saved to the onboard EEPROM, ensuring it persists across reboots

### Status Indication
On the Nyan Keys 0.8x - 0.9x boards there are 5 status leds that are activated upon boot. The labels for these LED(s) are as follows
| ID   | Name        | Description            |
| ---- | ----------- | ---------------------- |
| 0    | LED_0       | MCU Functional POST    |
| 5    | LED_1       | FPGA Configured        |

The system status LED should pulse at a rate of 1.287hz and have a period of 777ms. This is driven by TIM1 and TIM6 using interrupts.

The FPGA configuration LED will always match the pin status of ```c_done``` of the Lattice FPGA. ```c_done``` is an active high signal and will only go high once the FPGA has been programmed __AND__ the 47 dummy bits have been sent over the SPI bus. NyanOS handles all of this without any additional programming using the ```FPGAInit``` function in ```lattice_ice_hx.c```

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
| 1     | 0x0000      | FPGA Bitstream         | 65535  |

