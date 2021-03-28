/*******************************************************************************
 * @file    EEPROM.h
 * @author  FMA
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   Generic EEPROM driver
 *
 * Generic I2C-Compatible (2-wire) Serial EEPROM
 * It can work with every memory with an address 1010xxx_ compatibility
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020 Fabien MAILLY
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
#ifndef EEPROM_H_INC
#define EEPROM_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------



// Device definitions
#define EEPROM_I2C_READ   ( 0x01 ) //!< Standard I2C LSB bit to set
#define EEPROM_I2C_WRITE  ( 0xFE ) //!< Standard I2C bit mask which clear the LSB



/*! @brief Generate the EEPROM chip configurable address following the state of A0, A1, and A2
 * You shall set '1' (when corresponding pin is connected to +V) or '0' (when corresponding pin is connected to Ground) on each parameter
 */
#define EEPROM_ADDR(A2, A1, A0)  ( (uint8_t)((((A2) & 0x01) << 3) | (((A1) & 0x01) << 2) | (((A0) & 0x01) << 1)) )

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EEPROM Specific Driver Registers
//********************************************************************************************************************

//! Enumerator of all possible chip address selection pin that the device can have
typedef enum
{
  EEPROM_NO_CHIP_ADDRESS_SELECT = 0x00,                                                                     //!< No chip select address
  EEPROM_CHIP_ADDRESS_A0        = 0x01,                                                                     //!< Chip select use A0
  EEPROM_CHIP_ADDRESS_A1        = 0x02,                                                                     //!< Chip select use A1
  EEPROM_CHIP_ADDRESS_A2        = 0x04,                                                                     //!< Chip select use A2
  EEPROM_CHIP_ADDRESS_A1A0      = EEPROM_CHIP_ADDRESS_A1 | EEPROM_CHIP_ADDRESS_A0,                          //!< Chip select use A1, and A0
  EEPROM_CHIP_ADDRESS_A2A0      = EEPROM_CHIP_ADDRESS_A2 | EEPROM_CHIP_ADDRESS_A0,                          //!< Chip select use A2, and A0
  EEPROM_CHIP_ADDRESS_A2A1      = EEPROM_CHIP_ADDRESS_A2 | EEPROM_CHIP_ADDRESS_A1,                          //!< Chip select use A2, and A1
  EEPROM_CHIP_ADDRESS_A2A1A0    = EEPROM_CHIP_ADDRESS_A2 | EEPROM_CHIP_ADDRESS_A1 | EEPROM_CHIP_ADDRESS_A0, //!< Chip select use A2, A1, and A0
} eEEPROM_ChipSelect;


//! Enumerator of all possible chip address configuration type
typedef enum
{
  EEPROM_ADDRESS_1Byte             =    1, //!< EEPROM Address is  8-bits: S (1010A2A1A0.) (xxxxxxxx)
  EEPROM_ADDRESS_1Byte_plus_A0     = 0x21, //!< EEPROM Address is  9-bits: S (1010A2A1x .) (xxxxxxxx)
  EEPROM_ADDRESS_1Byte_plus_A1A0   = 0x61, //!< EEPROM Address is 10-bits: S (1010A2x x .) (xxxxxxxx)
  EEPROM_ADDRESS_1Byte_plus_A2A1A0 = 0xE1, //!< EEPROM Address is 11-bits: S (1010x x x .) (xxxxxxxx)
  EEPROM_ADDRESS_2Bytes            =    2, //!< EEPROM Address is 16-bits: S (1010A2A1A0.) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_2Byte_plus_A0     = 0x22, //!< EEPROM Address is 17-bits: S (1010A2A1x .) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_2Byte_plus_A1A0   = 0x62, //!< EEPROM Address is 18-bits: S (1010A2x x .) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_2Byte_plus_A2A1A0 = 0xE2, //!< EEPROM Address is 19-bits: S (1010x x x .) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Bytes            =    3, //!< EEPROM Address is 24-bits: S (1010A2A1A0.) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Byte_plus_A0     = 0x23, //!< EEPROM Address is 25-bits: S (1010A2A1x .) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Byte_plus_A1A0   = 0x63, //!< EEPROM Address is 26-bits: S (1010A2x x .) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Byte_plus_A2A1A0 = 0xE3, //!< EEPROM Address is 27-bits: S (1010x x x .) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_4Bytes            =    4, //!< EEPROM Address is 32-bits: S (1010A2A1A0.) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)

  EEPROM_ADDRESS_Bytes_MASK        = 0x0F, //!< Mask for EEPROM Address to bytes
  EEPROM_ADDRESS_plus_Ax_MASK      = 0xE0, //!< Mask for EEPROM Address to A0, A1, and/or A2
} eEEPROM_AddressType;


//! EEPROM configuration of a device
typedef struct EEPROM_Conf
{
  uint8_t ChipAddress;             //!< This is the base chip address
  eEEPROM_ChipSelect ChipSelect;   //!< Indicate which chip select pins are used by the chip
  eEEPROM_AddressType AddressType; //!< Indicate the EEPROM address type
  uint8_t PageWriteTime;           //!< Maximum time to write a page (for timeout) in millisecond
  uint16_t PageSize;
  uint32_t ArrayByteSize;
  uint32_t MaxI2CclockSpeed;
} EEPROM_Conf;

//-----------------------------------------------------------------------------



//=== AT24CXX(A) devices ======================================================
// AT24C01A EEPROM configurations
extern const EEPROM_Conf AT24C01A_1V8_Conf, AT24C01A_Conf;
// AT24C02 EEPROM configurations
extern const EEPROM_Conf AT24C02_1V8_Conf, AT24C02_Conf;
// AT24C04 EEPROM configurations
extern const EEPROM_Conf AT24C04_1V8_Conf, AT24C04_Conf;
// AT24C08A EEPROM configurations
extern const EEPROM_Conf AT24C08A_1V8_Conf, AT24C08A_Conf;
// AT24C16A EEPROM configurations
extern const EEPROM_Conf AT24C16A_1V8_Conf, AT24C16A_Conf;


//=== 24XX256 devices =========================================================
// 24AA256 EEPROM configurations
extern const EEPROM_Conf _24AA256_1V8_Conf, _24AA256_Conf;
// 24LC256 EEPROM configurations
extern const EEPROM_Conf _24LC256_Conf;
// 24FC256 EEPROM configurations
extern const EEPROM_Conf _24FC256_1V8_Conf, _24FC256_Conf;


//=== AT24CM02 devices ========================================================
extern const EEPROM_Conf AT24CM02_1V7_Conf, AT24CM02_Conf;


//=== AT24MACX02 devices ======================================================
// AT24MAC402 EEPROM configurations
extern const EEPROM_Conf AT24MAC402_1V7_Conf, AT24MAC402_Conf;
// AT24MAC602 EEPROM configurations
extern const EEPROM_Conf AT24MAC602_1V7_Conf, AT24MAC602_Conf;


//=== 47(L/C)04 devices =======================================================
extern const EEPROM_Conf EERAM47L04_Conf, EERAM47C04_Conf;


//=== 47(L/C)16 devices =======================================================
extern const EEPROM_Conf EERAM47L16_Conf, EERAM47C16_Conf;


//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EEPROM Driver API
//********************************************************************************************************************
typedef struct EEPROM EEPROM; //! Typedef of EEPROM device object structure



/*! @brief Interface function for driver initialization of the EEPROM
 *
 * This function will be called at driver initialization to configure the interface driver
 * @param[in] *pIntDev Is the EEPROM.InterfaceDevice of the device that call the interface initialization
 * @param[in] sclFreq Is the SCL frequency in Hz to set at the interface initialization
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*EEPROM_I2CInit_Func)(void *pIntDev, const uint32_t sclFreq);


/*! @brief Interface function for I2C transfer of the EEPROM
 *
 * This function will be called when the driver needs to transfer data over the I2C communication with the device
 * Can be a read of data or a transmit of data. It also indicates if it needs a start and/or a stop
 * @warning A I2CInit_Func() must be called before using this function
 * @param[in] *pIntDev Is the EEPROM.InterfaceDevice of the device that call the I2C transfer
 * @param[in] deviceAddress Is the device address on the bus (8-bits only). The LSB bit indicate if it is a I2C Read (bit at '1') or a I2C Write (bit at '0')
 * @param[in,out] *data Is a pointer to memory data to write in case of I2C Write, or where the data received will be stored in case of I2C Read (can be NULL if no data transfer other than chip address)
 * @param[in] byteCount Is the byte count to write over the I2C bus or the count of byte to read over the bus
 * @param[in] start Indicate if the transfer needs a start (in case of a new transfer) or restart (if the previous transfer has not been stopped)
 * @param[in] stop Indicate if the transfer needs a stop after the last byte sent
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*EEPROM_I2CTranfert_Func)(void *pIntDev, const uint8_t deviceAddress, uint8_t *data, size_t byteCount, bool start, bool stop);


/*! @brief Function that gives the current millisecond of the system to the driver
 *
 * This function will be called when the driver needs to get current millisecond
 * @return Returns the current millisecond of the system
 */
typedef uint32_t (*GetCurrentms_Func)(void);



//! EEPROM device object structure
struct EEPROM
{
  void *UserDriverData;                   //!< Optional, can be used to store driver data or NULL

  //--- EEPROM configuration ---
  const EEPROM_Conf *Conf;                //!< This is the EEPROM configuration, this parameter is mandatory

  //--- Interface clocks ---
  uint32_t I2C_ClockSpeed;                //!< Clock frequency of the I2C interface in Hertz

  //--- Interface driver call functions ---
  void *InterfaceDevice;                  //!< This is the pointer that will be in the first parameter of all interface call functions
  EEPROM_I2CInit_Func fnI2C_Init;         //!< This function will be called at driver initialization to configure the interface driver
  EEPROM_I2CTranfert_Func fnI2C_Transfer; //!< This function will be called when the driver needs to transfer data over the I2C communication with the device

  //--- Time call function ---
  GetCurrentms_Func fnGetCurrentms;       //!< This function will be called when the driver need to get current millisecond

  //--- Device address ---
  uint8_t AddrA2A1A0;                     //!< Device configurable address A2, A1, and A0. You can use the macro EEPROM_ADDR() to help filling this parameter. Only these 3 lower bits are used: ....210_ where 2 is A2, 1 is A1, 0 is A0. '.' and '_' are fixed by device
};
//-----------------------------------------------------------------------------





/*! @brief EEPROM initialization
 *
 * This function initializes the EEPROM driver and call the initialization of the interface driver (I2C). It also checks the presence of the device
 * Next it checks parameters and configures the EEPROM
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_EEPROM(EEPROM *pComp);



/*! @brief Is the EEPROM device ready
 *
 * Poll the acknowledge from the EEPROM
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns 'true' if ready else 'false'
 */
bool EEPROM_IsReady(EEPROM *pComp);

//********************************************************************************************************************



/*! @brief Read data from the EEPROM device
 *
 * This function reads data from the EEPROM area of a EEPROM device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read (can be inside a page)
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EEPROM_ReadData(EEPROM *pComp, uint32_t address, uint8_t* data, size_t size);



/*! @brief Write data to the EEPROM device
 *
 * This function writes data to the EEPROM area of a EEPROM device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be written (can be inside a page)
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EEPROM_WriteData(EEPROM *pComp, uint32_t address, const uint8_t* data, size_t size);

//********************************************************************************************************************





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* EEPROM_H_INC */