/*!*****************************************************************************
 * @file    23LCxxx.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    09/01/2022
 * @brief   Generic SRAM 23LCxxx driver
 * @details Generic driver for Microchip (c) Serial SRAM 23LCxxx. Works with:
 *   23A640/23K640: 64K SPI Bus Low-Power Serial SRAM
 *   23A256/23K256: 256K SPI Bus Low-Power Serial SRAM
 *   23A512/23LC512: 512Kbit SPI Serial SRAM with SDI and SQI Interface
 *   23A1024/23LC1024: 1Mbit SPI Serial SRAM with SDI and SQI Interface
 *   23LCV512: 512-Kbit SPI Serial SRAM with Battery Backup and SDI Interface
 *   23LCV1024: 1Mbit SPI Serial SRAM with Battery Backup and SDI Interface
 * Follow datasheet DS22126 Rev.E (October 2010)
 *                  DS22100 Rev.F (October 2011)
 *                  DS20005155 Rev.B (November 2013)
 *                  DS20005142 Rev.C (January 2015)
 *                  DS20005157 Rev.B (June 2021)
 *                  DS20005156 Rev.B (June 2021)
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020-2022 Fabien MAILLY
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
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef SRAM23LCxxx_H_INC
#define SRAM23LCxxx_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "SPI_Interface.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#  define __SRAM23LCxxx_PACKED__
#  define SRAM23LCxxx_PACKITEM    __pragma(pack(push, 1))
#  define SRAM23LCxxx_UNPACKITEM  __pragma(pack(pop))
#else
#  define __SRAM23LCxxx_PACKED__  __attribute__((packed))
#  define SRAM23LCxxx_PACKITEM
#  define SRAM23LCxxx_UNPACKITEM
#endif

//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#define SRAM23LCxxx_CONTROL_ITEM_SIZE(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// 23LCxxx Specific Driver Registers
//********************************************************************************************************************

//! I/O modes enumerator
typedef enum
{
  SRAM23LCxxx_SPI = 0x01, //!< Support SPI
  SRAM23LCxxx_SDI = 0x02, //!< Support SDI
  SRAM23LCxxx_SQI = 0x04, //!< Support SQI
} eSRAM23LCxxx_IOmodes;


//! 23LCxxx configuration of a device
typedef struct SRAM23LCxxx_Conf
{
  eSRAM23LCxxx_IOmodes ModeSet; //!< Indicate the SPI mode of the 23LCxxx memory to set
  bool UseHold;                  //!< Indicate that the device can disable its hold functionality
  uint8_t AddressBytes;          //!< Byte count for the address to send to the SRAM
  uint16_t PageSize;             //!< This is the page size of the device memory in bytes
  uint32_t ArrayByteSize;        //!< This is the memory total size in bytes
  uint32_t MaxSPIclockSpeed;     //!< This is the maximum SPI SCL clock speed of the device in Hertz
} SRAM23LCxxx_Conf;

//-----------------------------------------------------------------------------



//=== AT24CXX(A) devices ======================================================
// 23x640 configurations
extern const SRAM23LCxxx_Conf SRAM23A640_Conf, SRAM23K640_Conf;
// 23x256 configurations
extern const SRAM23LCxxx_Conf SRAM23A256_Conf, SRAM23K256_Conf;
// 23x512 configurations
extern const SRAM23LCxxx_Conf SRAM23A512_Conf, SRAM23LC512_Conf;
// 23x1024 configurations
extern const SRAM23LCxxx_Conf SRAM23A1024_Conf, SRAM23LC1024_Conf;
// 23LCV512 configuration
extern const SRAM23LCxxx_Conf SRAM23LCV512_Conf;
// 23LCV512 configuration
extern const SRAM23LCxxx_Conf SRAM23LCV1024_Conf;
//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SRAM23LCxxx Register list
//********************************************************************************************************************

//! SRAM23LCxxx Instruction list
typedef enum
{
  // SRAM Commands
  SRAM23LCxxx_READ  = 0b00000011, //!< Read data from memory array beginning at selected address
  SRAM23LCxxx_WRITE = 0b00000010, //!< Write data to memory array beginning at selected address
  // I/O Modes Commands
  SRAM23LCxxx_EDIO  = 0b00111011, //!< [23x512/23x1024/23LCV512/23LCV1024 only] Enter Dual I/O access
  SRAM23LCxxx_EQIO  = 0b00111000, //!< [23x512/23x1024 only] Enter Quad I/O access
  SRAM23LCxxx_RSTIO = 0b11111111, //!< [23x512/23x1024/23LCV512/23LCV1024 only] Reset Dual and Quad I/O access
  // STATUS Register Commands
  SRAM23LCxxx_RDSR  = 0b00000101, //!< Read STATUS register
  SRAM23LCxxx_WRSR  = 0b00000001, //!< Write STATUS register
} eSRAM23LCxxx_InstructionSet;

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SRAM23LCxxx Specific Controller Registers
//********************************************************************************************************************

//! Status Register
SRAM23LCxxx_PACKITEM
typedef union __SRAM23LCxxx_PACKED__ SRAM23LCxxx_StatusRegister
{
  uint8_t Status;
  struct
  {
    uint8_t HOLD: 1; //!< 0   - [23x640/23x256 only] The HOLD bit enables the Hold pin functionality. It must be set to a '0' before HOLD pin is brought low for HOLD function to work properly. Setting HOLD to '1' disables feature
    uint8_t     : 5; //!< 1-5
    uint8_t MODE: 2; //!< 6-7 - The mode bits indicate the operating mode of the SRAM
  } Bits;
} SRAM23LCxxx_StatusRegister;
SRAM23LCxxx_UNPACKITEM;
SRAM23LCxxx_CONTROL_ITEM_SIZE(SRAM23LCxxx_StatusRegister, 1);

#define SRAM23LCxxx_HOLD_FEATURE_DISABLE  (0x1u << 0) //!< Disable hold feature
#define SRAM23LCxxx_HOLD_FEATURE_ENABLE   (0x0u << 0) //!< Enable hold feature

//! Block Protect
typedef enum
{
  SRAM23LCxxx_BYTE_MODE       = 0b00, //!< Byte mode (default operation)
  SRAM23LCxxx_SEQUENTIAL_MODE = 0b01, //!< Sequential mode
  SRAM23LCxxx_PAGE_MODE       = 0b10, //!< Page mode
  SRAM23LCxxx_RESERVED        = 0b11, //!< Reserved
} eSRAM23LCxxx_Modes;

#define SRAM23LCxxx_MODE_Pos         6
#define SRAM23LCxxx_MODE_Mask        (0x3u << SRAM23LCxxx_MODE_Pos)
#define SRAM23LCxxx_MODE_SET(value)  (((uint8_t)(value) << SRAM23LCxxx_MODE_Pos) & SRAM23LCxxx_MODE_Mask) //!< Set mode
#define SRAM23LCxxx_MODE_GET(value)  (((uint8_t)(value) & SRAM23LCxxx_MODE_Mask) >> SRAM23LCxxx_MODE_Pos) //!< Get mode

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SRAM23LCxxx Driver API
//********************************************************************************************************************

#define SRAM23LCxxx_DMA_TRANSFER_IN_PROGRESS_Pos        ( 8 )
#define SRAM23LCxxx_DMA_TRANSFER_IN_PROGRESS            ( 1u << SRAM23LCxxx_DMA_TRANSFER_IN_PROGRESS_Pos ) // DMA transfer in progress
#define SRAM23LCxxx_IS_DMA_TRANSFER_IN_PROGRESS(value)  (((uint16_t)(value) & SRAM23LCxxx_DMA_TRANSFER_IN_PROGRESS) > 0) // Is DMA transfer in progress?
#define SRAM23LCxxx_NO_DMA_TRANSFER_IN_PROGRESS_SET     (~SRAM23LCxxx_DMA_TRANSFER_IN_PROGRESS)            // Mask to set no DMA transfer to the device

#define SRAM23LCxxx_TRANSACTION_NUMBER_Pos           ( 9 )
#define SRAM23LCxxx_TRANSACTION_NUMBER_Mask          ( 0x3Fu << SRAM23LCxxx_TRANSACTION_NUMBER_Pos )
#define SRAM23LCxxx_TRANSACTION_NUMBER_SET(value)    (((uint16_t)(value) << SRAM23LCxxx_TRANSACTION_NUMBER_Pos) & SRAM23LCxxx_TRANSACTION_NUMBER_Mask) // Set the transaction number to internal config
#define SRAM23LCxxx_TRANSACTION_NUMBER_GET(value)    (((uint16_t)(value) & SRAM23LCxxx_TRANSACTION_NUMBER_Mask) >> SRAM23LCxxx_TRANSACTION_NUMBER_Pos) // Get the transaction number to internal config
#define SRAM23LCxxx_TRANSACTION_NUMBER_CLEAR(value)  value &= ~SRAM23LCxxx_TRANSACTION_NUMBER_Mask // Clears the transaction number of internal config

#define SRAM23LCxxx_IO_MODE_Pos           ( 3 )
#define SRAM23LCxxx_IO_MODE_Mask          ( 0x7u << SRAM23LCxxx_IO_MODE_Pos )
#define SRAM23LCxxx_IO_MODE_SET(value)    (((uint16_t)(value) << SRAM23LCxxx_IO_MODE_Pos) & SRAM23LCxxx_IO_MODE_Mask) // Set the IO mode to internal config
#define SRAM23LCxxx_IO_MODE_GET(value)    (((uint16_t)(value) & SRAM23LCxxx_IO_MODE_Mask) >> SRAM23LCxxx_IO_MODE_Pos) // Get the IO mode to internal config

//-----------------------------------------------------------------------------

typedef struct SRAM23LCxxx SRAM23LCxxx; //! Typedef of SRAM23LCxxx device object structure
typedef uint16_t TSRAM23LCxxxDriverInternal; //! Alias for Driver Internal data flags

//-----------------------------------------------------------------------------

//! SRAM23LCxxx device object structure
struct SRAM23LCxxx
{
  void *UserDriverData;                      //!< Optional, can be used to store driver data or NULL
  TSRAM23LCxxxDriverInternal InternalConfig; //!< DO NOT USE OR CHANGE THIS VALUE, IT'S THE INTERNAL DRIVER CONFIGURATION

  //--- SRAM23LCxxx configuration ---
  const SRAM23LCxxx_Conf *Conf;              //!< This is the SRAM23LCxxx configuration, this parameter is mandatory

  //--- Interface driver call functions ---
  uint8_t SPIchipSelect;                     //!< This is the Chip Select index that will be set at the call of a transfer
#ifdef USE_DYNAMIC_INTERFACE
  SPI_Interface* SPI;                        //!< This is the SPI_Interface descriptor pointer that will be used to communicate with the device
#else
  SPI_Interface SPI;                         //!< This is the SPI_Interface descriptor that will be used to communicate with the device
#endif
  uint32_t SPIclockSpeed;                    //!< Clock frequency of the SPI interface in Hertz
};

//-----------------------------------------------------------------------------

//! SRAM23LCxxx Controller configuration structure
typedef struct SRAM23LCxxx_Config
{
  bool RecoverSPIbus;               //!< If 'true' the driver try to recover the previous I/O mode. It will send the SRAM23LCxxx_RSTIO instruction in SDI and SQI (depending on the device) to reset to SPI before setting the SRAM23LCxxx_Config.IOmode. This parameter have no effect on device that does not support SDI and SQI
  eSRAM23LCxxx_IOmodes IOmode;      //!< SPI I/O access mode
  eSRAM23LCxxx_Modes OperationMode; //!< Operating mode of the SRAM
  bool DisableHold;                 //!< Set to 'true' to disable the Hold pin functionality else set to 'false'
} SRAM23LCxxx_Config;

//-----------------------------------------------------------------------------


/*! @brief SRAM23LCxxx initialization
 *
 * This function initializes the SRAM23LCxxx driver and call the initialization of the interface driver (SPI)
 * Next it checks parameters and configures the SRAM23LCxxx
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @param[in] *pConf Is the pointed structure of the device configuration
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_SRAM23LCxxx(SRAM23LCxxx *pComp, const SRAM23LCxxx_Config* pConf);

//********************************************************************************************************************


/*! @brief Read SRAM data from the SRAM23LCxxx device
 *
 * This function reads data from the SRAM area of a SRAM23LCxxx device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read (can be inside a page)
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_ReadSRAMData(SRAM23LCxxx *pComp, uint32_t address, uint8_t* data, size_t size);

//********************************************************************************************************************


/*! @brief Write SRAM data to the SRAM23LCxxx device
 *
 * This function writes data to the SRAM area of a SRAM23LCxxx device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be written (can be inside a page)
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_WriteSRAMData(SRAM23LCxxx *pComp, uint32_t address, const uint8_t* data, size_t size);

/*! @brief Write an instruction to the SRAM23LCxxx device
 *
 * This function sends an instruction to a SRAM23LCxxx device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] instruction Is the instruction to send
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_WriteInstruction(SRAM23LCxxx *pComp, const eSRAM23LCxxx_InstructionSet instruction);

//********************************************************************************************************************


/*! @brief Read Status register from the SRAM23LCxxx device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *status Is where the status register will be stored
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_GetStatus(SRAM23LCxxx *pComp, SRAM23LCxxx_StatusRegister* status);

/*! @brief Write Status register to the SRAM23LCxxx device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] status Is the status register to send
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_SetStatus(SRAM23LCxxx *pComp, const SRAM23LCxxx_StatusRegister status);

//********************************************************************************************************************


/*! @brief Set the I/O mode of the SRAM23LCxxx
 *
 * The function takes care of the process to exchange the IO mode
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] mode Is the new IO mode to apply
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_SetIOmode(SRAM23LCxxx *pComp, const eSRAM23LCxxx_IOmodes mode);

/*! @brief Set the SRAM operation mode of the SRAM23LCxxx
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] mode Is the new operation mode of the SRAM to apply
 * @param[in] disableHold Set to 'true' to disable the hold function (only used on 23x640/23x256)
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SRAM23LCxxx_SetOperationMode(SRAM23LCxxx *pComp, const eSRAM23LCxxx_Modes mode, const bool disableHold);

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* SRAM23LCxxx_H_INC */