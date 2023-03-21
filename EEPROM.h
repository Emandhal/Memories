/*!*****************************************************************************
 * @file    EEPROM.h
 * @author  FMA
 * @version 1.1.0
 * @date    30/10/2021
 * @brief   Generic EEPROM driver
 * @details Generic I2C-Compatible (2-wire) Serial EEPROM
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

/* Revision history:
 * 1.1.0    I2C interface rework
 *          Add EEPROM_WaitEndOfWrite() function
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef EEPROM_H_INC
#define EEPROM_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "I2C_Interface.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------



/*! @brief Generate the EEPROM chip configurable address following the state of A0, A1, and A2
 * You shall set '1' (when corresponding pin is connected to +V) or '0' (when corresponding pin is connected to Ground) on each parameter
 */
#define EEPROM_ADDR(A2, A1, A0)  ( (uint8_t)((((A2) & 0x01) << 3) | (((A1) & 0x01) << 2) | (((A0) & 0x01) << 1)) )

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EEPROM Specific Driver Registers
//********************************************************************************************************************

//! Enumerator of all possible user configurable chip address selection pin that the device can have
typedef enum
{
  EEPROM_NO_CHIP_ADDRESS_SELECT = 0x00,                                                                     //!< No chip select address
  EEPROM_CHIP_ADDRESS_A0        = 0x01,                                                                     //!< User configurable chip select use A0
  EEPROM_CHIP_ADDRESS_A1        = 0x02,                                                                     //!< User configurable chip select use A1
  EEPROM_CHIP_ADDRESS_A2        = 0x04,                                                                     //!< User configurable chip select use A2
  EEPROM_CHIP_ADDRESS_A1A0      = EEPROM_CHIP_ADDRESS_A1 | EEPROM_CHIP_ADDRESS_A0,                          //!< User configurable chip select use A1, and A0
  EEPROM_CHIP_ADDRESS_A2A0      = EEPROM_CHIP_ADDRESS_A2 | EEPROM_CHIP_ADDRESS_A0,                          //!< User configurable chip select use A2, and A0
  EEPROM_CHIP_ADDRESS_A2A1      = EEPROM_CHIP_ADDRESS_A2 | EEPROM_CHIP_ADDRESS_A1,                          //!< User configurable chip select use A2, and A1
  EEPROM_CHIP_ADDRESS_A2A1A0    = EEPROM_CHIP_ADDRESS_A2 | EEPROM_CHIP_ADDRESS_A1 | EEPROM_CHIP_ADDRESS_A0, //!< User configurable chip select use A2, A1, and A0
} eEEPROM_ChipSelect;


//! Enumerator of all possible address configuration type
typedef enum
{
  EEPROM_ADDRESS_1Byte             =    1, //!< EEPROM Address is  8-bits: S (1010A2A1A0_) (xxxxxxxx)
  EEPROM_ADDRESS_1Byte_plus_A0     = 0x21, //!< EEPROM Address is  9-bits: S (1010A2A1x _) (xxxxxxxx)
  EEPROM_ADDRESS_1Byte_plus_A1A0   = 0x61, //!< EEPROM Address is 10-bits: S (1010A2x x _) (xxxxxxxx)
  EEPROM_ADDRESS_1Byte_plus_A2A1A0 = 0xE1, //!< EEPROM Address is 11-bits: S (1010x x x _) (xxxxxxxx)
  EEPROM_ADDRESS_2Bytes            =    2, //!< EEPROM Address is 16-bits: S (1010A2A1A0_) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_2Byte_plus_A0     = 0x22, //!< EEPROM Address is 17-bits: S (1010A2A1x _) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_2Byte_plus_A1A0   = 0x62, //!< EEPROM Address is 18-bits: S (1010A2x x _) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_2Byte_plus_A2A1A0 = 0xE2, //!< EEPROM Address is 19-bits: S (1010x x x _) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Bytes            =    3, //!< EEPROM Address is 24-bits: S (1010A2A1A0_) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Byte_plus_A0     = 0x23, //!< EEPROM Address is 25-bits: S (1010A2A1x _) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Byte_plus_A1A0   = 0x63, //!< EEPROM Address is 26-bits: S (1010A2x x _) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_3Byte_plus_A2A1A0 = 0xE3, //!< EEPROM Address is 27-bits: S (1010x x x _) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)
  EEPROM_ADDRESS_4Bytes            =    4, //!< EEPROM Address is 32-bits: S (1010A2A1A0_) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx) (xxxxxxxx)

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
  uint16_t PageSize;               //!< This is the page size of the device memory in bytes
  uint32_t ArrayByteSize;          //!< This is the memory total size in bytes
  uint32_t MaxI2CclockSpeed;       //!< This is the maximum I2C SCL clock speed of the device in Hertz
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


/*! @brief Function that gives the current millisecond of the system to the driver
 *
 * This function will be called when the driver needs to get current millisecond
 * @return Returns the current millisecond of the system
 */
typedef uint32_t (*GetCurrentms_Func)(void);



//! EEPROM device object structure
struct EEPROM
{
  void *UserDriverData;             //!< Optional, can be used to store driver data or NULL

  //--- EEPROM configuration ---
  const EEPROM_Conf *Conf;          //!< This is the EEPROM configuration, this parameter is mandatory

  //--- Interface driver call functions ---
#ifdef USE_DYNAMIC_INTERFACE
  I2C_Interface* I2C;               //!< This is the I2C_Interface descriptor pointer that will be used to communicate with the device
#else
  I2C_Interface I2C;                //!< This is the I2C_Interface descriptor that will be used to communicate with the device
#endif
  uint32_t I2CclockSpeed;           //!< Clock frequency of the I2C interface in Hertz

  //--- Time call function ---
  GetCurrentms_Func fnGetCurrentms; //!< This function will be called when the driver need to get current millisecond

  //--- Device address ---
  uint8_t AddrA2A1A0;               //!< Device configurable address A2, A1, and A0. You can use the macro EEPROM_ADDR() to help filling this parameter. Only these 3 lower bits are used: ....210_ where 2 is A2, 1 is A1, 0 is A0. '.' and '_' are fixed by device
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



/*! @brief Wait the end of write to the EEPROM device
 * This function exists because the last page write of the EEPROM_WriteData() function to the EEPROM do not wait the end of write so a shutdown can corrupt the last page data
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EEPROM_WaitEndOfWrite(EEPROM *pComp);

//********************************************************************************************************************





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* EEPROM_H_INC */