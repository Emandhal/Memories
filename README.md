# Drivers
Drivers in this repository are fully hardware independant **Drivers** primarily aimed at embedded world.
All memories features are supported.
Here are drivers of:
## I2C memories
### I2C EEPROM drivers
* Generic I2C EEPROM compatible with:
  * 47L04 and 47C04
  * 47L16 and 47C16
  * AT24MAC402
  * AT24MAC602
  * AT24C01A
  * AT24C02
  * AT24C04
  * AT24C08A
  * AT24C16A
  * 24AA256, 24LC256, 24FC256
  * AT24CM02
  * ...
* AT24MAC402
* AT24MAC602

### I2C EERAM drivers
* 47L04 and 47C04
* 47L16 and 47C16

## SPI memories
### SPI EERAM drivers
* 48L512
* 48LM01

### SPI SRAM drivers
* 23A640/23K640, 23A256/23K256, 23A512/23LC512, 23A1024/23LC1024
* 23LCV512, 23LCV1024

# Presentation
This driver only takes care of configuration and check of the internal registers and the formatting of the communication with the device. That means it does not directly take care of the physical communication, there is functions interfaces to do that.
Each driver's functions need a device structure that indicate with which device he must threat and communicate. Each device can have its own configuration.

## Feature
This driver has been designed to:
* Only take care of the memory, not the communication with it
* Configuration is very simplified
* All memories features are covered
* Can communicate with virtually an infinite count of memories
* Different configurations can be used with different memories (no duplication of the driver needed)
* Direct communication with the memories, the driver has no buffer
* Can use the driver defines, enums, structs to create your own functions
* Contiguous memories can be used as 1 unique memory by the driver under certain conditions (I2C EEPROM only)
* Driver will take care of page access to minimize write process and save time
* Driver will take care of address composition of the data

## Installation
### Get the sources
Get and add the 4 following files to your project
```
The memory source .c
The memory source .h
For I2C memories: I2C_Interface.h
For SPI memories: SPI_Interface.h
ErrorsDef.h
```