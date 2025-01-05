/*!*****************************************************************************
 * @file    48L512.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.1
 * @date    04/01/2025
 * @brief   EERAM48LM01 driver
 * @details SPI-Compatible 512-kbit SPI Serial EERAM
 * Follow datasheet DS20006008C Rev.C (Oct 2019)
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020-2025 Fabien MAILLY
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
 * 1.0.1    Update error management to add context
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef EERAM48L512_H_INC
#define EERAM48L512_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "SPI_Interface.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#  define __EERAM48L512_PACKED__
#  define EERAM48L512_PACKITEM    __pragma(pack(push, 1))
#  define EERAM48L512_UNPACKITEM  __pragma(pack(pop))
#else
#  define __EERAM48L512_PACKED__  __attribute__((packed))
#  define EERAM48L512_PACKITEM
#  define EERAM48L512_UNPACKITEM
#endif

//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#define EERAM48L512_CONTROL_ITEM_SIZE(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }

//-----------------------------------------------------------------------------

// Limits definitions
#define EERAM48L512_SPICLOCK_MAX  ( 66000000u ) //!< Max SPI clock frequency

// Device definitions
#define EERAM48L512_PAGE_SIZE          ( 64 ) //!< The EERAM48L512 is 64 bytes page size (for secured transfer)
#define EERAM48L512_PAGE_SIZE_MASK     ( EERAM48L512_PAGE_SIZE - 1 ) //!< The EERAM48L512 page mask is 63 bytes
#define EERAM48L512_NONVOLATILE_SIZE   ( 16 ) //!< The EERAM48L512 have 16 bytes of Nonvolatile User Space

#define EERAM48L512_EERAM_SIZE         ( 65536 ) //!< 48L512 total memory size
#define EERAM48L512_ADDRESS_BYTE_SIZE  ( 2 ) //!< 2 byte for the address

#define EERAM48L512_STORE_TIMEOUT      ( 10 ) //!< Store Operation Duration: 10ms
#define EERAM48L512_RECALL_TIMEOUT     (  0 ) //!< Store Operation Duration: 50µs

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM48L512 Register list
//********************************************************************************************************************

//! EERAM48L512 Operation Codes list
typedef enum
{
  // Write Control Commands
  EERAM48L512_WREN   = 0x06, //!< Set Write Enable Latch
  EERAM48L512_WRDI   = 0x04, //!< Reset Write Enable Latch
  // SRAM Commands
  EERAM48L512_WRITE  = 0x02, //!< Write to SRAM Array
  EERAM48L512_READ   = 0x03, //!< Read from SRAM Array
  EERAM48L512_SWRITE = 0x12, //!< Secure Write to SRAM Array with CRC
  EERAM48L512_SREAD  = 0x13, //!< Secure Read from SRAM Array with CRC
  // STATUS Register Commands
  EERAM48L512_WRSR   = 0x01, //!< Write STATUS Register (SR)
  EERAM48L512_RDSR   = 0x05, //!< Read STATUS Register (SR)
  // Store/Recall Commands
  EERAM48L512_STORE  = 0x08, //!< Store SRAM data to EEPROM array
  EERAM48L512_RECALL = 0x09, //!< Copy EEPROM data to SRAM array
  // Nonvolatile User Space Commands
  EERAM48L512_WRNUR  = 0xC2, //!< Write Nonvolatile User Space
  EERAM48L512_RDNUR  = 0xC3, //!< Read Nonvolatile User Space
  // Hibernate Commands
  EERAM48L512_HBRNT  = 0xB9, //!< Enter Hibernate Mode
} eEERAM48L512_OPcodes;

#define EERAM48L512_IS_NV_USER_SPACE(opCode)  ( ((uint8_t)(opCode) & 0xC0) == 0xC0 )

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM48L512 Specific Controller Registers
//********************************************************************************************************************

//! Status Register
EERAM48L512_PACKITEM
typedef union __EERAM48L512_PACKED__ EERAM48L512_StatusRegister
{
  uint8_t Status;
  struct
  {
    uint8_t RDY_BSY: 1; //!< 0   - Ready/Busy Status bit (Read-Only): '1' = Device is busy with an internal store or recall operation ; '0' = Device is ready for standard SRAM Read/Write commands
    uint8_t WEL    : 1; //!< 1   - Write Enable Latch bit (Read-Only): '1' = WREN has been executed and device is enabled for writing ; '0' = Device is not write-enabled
    uint8_t BP     : 2; //!< 2-3 - Block Protection bits
    uint8_t SWM    : 1; //!< 4   - Secure Write Monitoring bit (Read-Only): '1' = The last secure write operation has failed ; '0' = No error reported in Secure Write
    uint8_t        : 1; //!< 5
    uint8_t ASE    : 1; //!< 6   - AutoStore Enable bit: '1' = AutoStore is disabled ; '0' = AutoStore is enabled (factory default)
    uint8_t        : 1; //!< 7
  } Bits;
} EERAM48L512_StatusRegister;
EERAM48L512_UNPACKITEM;
EERAM48L512_CONTROL_ITEM_SIZE(EERAM48L512_StatusRegister, 1);

#define EERAM48L512_IS_BUSY        (0x1u << 0) //!< Device is busy with an internal store or recall operation
#define EERAM48L512_IS_READY       (0x0u << 0) //!< Device is ready for standard SRAM Read/Write commands
#define EERAM48L512_WRITE_ENABLE   (0x1u << 1) //!< WREN has been executed and device is enabled for writing
#define EERAM48L512_WRITE_DISABLE  (0x0u << 1) //!< Device is not write-enabled

//! Block Protect
typedef enum
{
  EERAM48L512_NO_WRITE_PROTECT    = 0b00, //!< (Level 0) No SRAM array write protection (factory default)
  EERAM48L512_PROTECT_C000h_FFFFh = 0b01, //!< (Level 1) Upper quarter SRAM memory array protection
  EERAM48L512_PROTECT_8000h_FFFFh = 0b10, //!< (Level 2) Upper half SRAM memory array protection
  EERAM48L512_PROTECT_0000h_FFFFh = 0b11, //!< (Level 3) Entire SRAM memory array protection
} eEERAM48L512_BlockProtect;

#define EERAM48L512_BP_Pos                 2
#define EERAM48L512_BP_Mask                (0x3u << EERAM48L512_BP_Pos)
#define EERAM48L512_BP_SET(value)          (((uint8_t)(value) << EERAM48L512_BP_Pos) & EERAM48L512_BP_Mask) //!< Set Block Protect bits
#define EERAM48L512_BP_GET(value)          (((uint8_t)(value) & EERAM48L512_BP_Mask) >> EERAM48L512_BP_Pos) //!< Get Block Protect bits
#define EERAM48L512_WRITE_SECURE_FAILED    (0x1u << 4) //!< The last secure write operation has failed
#define EERAM48L512_NO_WRITE_SECURE_ERROR  (0x0u << 4) //!< No error reported in Secure Write
#define EERAM48L512_AUTOSTORE_DISABLE      (0x1u << 6) //!< AutoStore is disabled
#define EERAM48L512_AUTOSTORE_ENABLE       (0x0u << 6) //!< AutoStore is enabled (factory default)

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM48L512 Driver API
//********************************************************************************************************************

#define EERAM48L512_DMA_TRANSFER_IN_PROGRESS_Pos        ( 0 )
#define EERAM48L512_DMA_TRANSFER_IN_PROGRESS            ( 1u << EERAM48L512_DMA_TRANSFER_IN_PROGRESS_Pos ) // DMA transfer in progress
#define EERAM48L512_IS_DMA_TRANSFER_IN_PROGRESS(value)  (((uint8_t)(value) & EERAM48L512_DMA_TRANSFER_IN_PROGRESS) > 0) // Is DMA transfer in progress?
#define EERAM48L512_NO_DMA_TRANSFER_IN_PROGRESS_SET     (~EERAM48L512_DMA_TRANSFER_IN_PROGRESS)            // Mask to set no DMA transfer to the device

#define EERAM48L512_TRANSACTION_NUMBER_Pos           ( 1 )
#define EERAM48L512_TRANSACTION_NUMBER_Mask          ( 0x3F << EERAM48L512_TRANSACTION_NUMBER_Pos )
#define EERAM48L512_TRANSACTION_NUMBER_SET(value)    (((uint8_t)(value) << EERAM48L512_TRANSACTION_NUMBER_Pos) & EERAM48L512_TRANSACTION_NUMBER_Mask) // Set the transaction number to internal config
#define EERAM48L512_TRANSACTION_NUMBER_GET(value)    (((uint8_t)(value) & EERAM48L512_TRANSACTION_NUMBER_Mask) >> EERAM48L512_TRANSACTION_NUMBER_Pos) // Get the transaction number to internal config
#define EERAM48L512_TRANSACTION_NUMBER_CLEAR(value)  value &= ~EERAM48L512_TRANSACTION_NUMBER_Mask // Clears the transaction number of internal config

#define EERAM48L512_STATUS_WRITE_ENABLE_Pos        ( 7 )
#define EERAM48L512_STATUS_WRITE_ENABLE            ( 1u << EERAM48L512_STATUS_WRITE_ENABLE_Pos ) // Write enable
#define EERAM48L512_IS_STATUS_WRITE_ENABLE(value)  (((uint8_t)(value) & (1u << EERAM48L512_STATUS_WRITE_ENABLE_Pos)) > 0) // Is write enable?
#define EERAM48L512_STATUS_WRITE_DISABLE_SET       (~EERAM48L512_STATUS_WRITE_ENABLE)            // Mask to set write disable

//-----------------------------------------------------------------------------

typedef struct EERAM48L512 EERAM48L512; //! Typedef of EERAM48L512 device object structure
typedef uint8_t TEERAM48L512DriverInternal; //! Alias for Driver Internal data flags

//-----------------------------------------------------------------------------

/*! @brief Function that gives the current millisecond of the system to the driver
 *
 * This function will be called when the driver needs to get current millisecond
 * @return Returns the current millisecond of the system
 */
typedef uint32_t (*GetCurrentms_Func)(void);

#ifdef USE_EXTERNAL_CRC16
/*! @brief Function that compute CRC16-IBM3740 for the driver
 *
 * This function will be called when a CRC16-IBM3740 computation is needed
 * @param[in,out] *pCRC Is the current computation of the CRC. If this is the first data computed, set the value to 0xFFFF
 * @param[in] *data Is the byte steam of data to compute
 * @param[in] size Is the size of the byte stream
 */
typedef uint16_t (*ComputeCRC16_Func)(uint16_t* pCRC, const uint8_t* data, size_t size);
#endif

//-----------------------------------------------------------------------------

//! EERAM48L512 device object structure
struct EERAM48L512
{
  void *UserDriverData;                      //!< Optional, can be used to store driver data or NULL
  TEERAM48L512DriverInternal InternalConfig; //!< DO NOT USE OR CHANGE THIS VALUE, IT'S THE INTERNAL DRIVER CONFIGURATION

  //--- Interface driver call functions ---
  uint8_t SPIchipSelect;                     //!< This is the Chip Select index that will be set at the call of a transfer
#ifdef USE_DYNAMIC_INTERFACE
  SPI_Interface* SPI;                        //!< This is the SPI_Interface descriptor pointer that will be used to communicate with the device
#else
  SPI_Interface SPI;                         //!< This is the SPI_Interface descriptor that will be used to communicate with the device
#endif
  uint32_t SPIclockSpeed;                    //!< Clock frequency of the SPI interface in Hertz

  //--- Time call function ---
  GetCurrentms_Func fnGetCurrentms;          //!< This function will be called when the driver need to get current millisecond

  //--- CRC call functions ---
#ifdef USE_EXTERNAL_CRC16
  ComputeCRC16_Func fnComputeCRC16;          //!< This function will be called when a CRC16-IBM3740 computation is needed
#endif
};
//-----------------------------------------------------------------------------


/*! @brief EERAM48L512 initialization
 *
 * This function initializes the EERAM48L512 driver and call the initialization of the interface driver (SPI).
 * Next it checks parameters and configures the EERAM48L512
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_EERAM48L512(EERAM48L512 *pComp);

//********************************************************************************************************************


/*! @brief Read SRAM data from the EERAM48L512 device
 *
 * This function reads data from the SRAM area of a EERAM48L512 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_ReadSRAMData(EERAM48L512 *pComp, uint16_t address, uint8_t* data, size_t size);

/*! @brief Secure read SRAM data to the EERAM48L512 device
 *
 * This function reads data from the SRAM area of a EERAM48L512 device with a CRC check
 * @warning The size of data to read shall be a multiple of 64 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_ReadSecure(EERAM48L512 *pComp, uint16_t address, uint8_t* data, size_t size);

/*! @brief Write NonVolatile User Space data to the EERAM48L512 device
 *
 * This function reads data from the NonVolatile User Space area of a EERAM48L512 device
 * @warning The size of data buffer shall be 16 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *data Is where the data will be stored
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_ReadNVUSData(EERAM48L512 *pComp, uint8_t* data);

/*! @brief Read SRAM data with DMA from the EERAM48L512 device
 *
 * To know the state of the DMA transfer, call this function. When the function returns ERR_NONE, it means that the transfer is complete else it returns the current state/error
 * In case of no DMA, the function act like a EERAM48L512_ReadSRAMData() function
 * @warning Never touch the data processed by the DMA before its completion
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_ReadSRAMDataWithDMA(EERAM48L512 *pComp, uint16_t address, uint8_t* data, size_t size);

//********************************************************************************************************************


/*! @brief Write SRAM data to the EERAM48L512 device
 *
 * This function writes data to the SRAM area of a EERAM48L512 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_WriteSRAMData(EERAM48L512 *pComp, uint16_t address, const uint8_t* data, size_t size);

/*! @brief Secure write SRAM data to the EERAM48L512 device
 *
 * This function writes data to the SRAM area of a EERAM48L512 device with a CRC check
 * @warning The size of data buffer to write shall be a multiple of 64 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_WriteSecure(EERAM48L512 *pComp, uint16_t address, const uint8_t* data, size_t size);

/*! @brief Write NonVolatile User Space data to the EERAM48L512 device
 *
 * This function writes data to the NonVolatile User Space area of a EERAM48L512 device
 * @warning The size of data to write shall be 16 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] *data Is the data array to store
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_WriteNVUSData(EERAM48L512 *pComp, const uint8_t* data);

/*! @brief Write a command to the EERAM48L512 device
 *
 * This function asserts the device, write the OP code and deasserts the device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] command Is the OP code to send
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_WriteCommand(EERAM48L512 *pComp, const eEERAM48L512_OPcodes command);

/*! @brief Write SRAM data with DMA to the EERAM48L512 device
 *
 * To know the state of the DMA transfer, call this function. When the function returns ERR_NONE, it means that the transfer is complete else it returns the current state/error
 * In case of no DMA, the function act like a EERAM48L512_WriteSRAMData() function
 * @warning Never touch the data processed by the DMA before its completion
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_WriteSRAMDataWithDMA(EERAM48L512 *pComp, uint16_t address, const uint8_t* data, size_t size);

//********************************************************************************************************************


/*! @brief Get the status of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *status Is where the result will be saved
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_GetStatus(EERAM48L512 *pComp, EERAM48L512_StatusRegister* status);

/*! @brief Set the status of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] status Is the status to store
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_SetStatus(EERAM48L512 *pComp, const EERAM48L512_StatusRegister status);

//********************************************************************************************************************


/*! @brief Store all the SRAM to the EEPROM of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] waitEndOfStore This indicate that the function do not return before the end of store operation duration. If 'false', the function return directly after the send of store operation
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_StoreSRAMtoEEPROM(EERAM48L512 *pComp, bool waitEndOfStore);

/*! @brief Recall all data from EEPROM to SRAM of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] waitEndOfRecall This indicate that the function do not return before the end of recall operation duration. If 'false', the function return directly after the send of recall operation
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_RecallEEPROMtoSRAM(EERAM48L512 *pComp, bool waitEndOfRecall);

/*! @brief Activate the Auto-Store of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_ActivateAutoStore(EERAM48L512 *pComp);

/*! @brief Deactivate the Auto-Store of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_DeactivateAutoStore(EERAM48L512 *pComp);

/*! @brief Set block write protect of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] blockProtect Indicate what location to write protect
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48L512_SetBlockWriteProtect(EERAM48L512 *pComp, eEERAM48L512_BlockProtect blockProtect);

/*! @brief Set write enable of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM48L512_SetWriteEnable(EERAM48L512 *pComp)
{
  eERRORRESULT Error = EERAM48L512_WriteCommand(pComp, EERAM48L512_WREN); // Send OP code of write enable
  pComp->InternalConfig |= EERAM48L512_STATUS_WRITE_ENABLE;               // Set write enable flag
  return Error;
}

/*! @brief Set write disable of the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM48L512_SetWriteDisable(EERAM48L512 *pComp)
{
  return EERAM48L512_WriteCommand(pComp, EERAM48L512_WRDI); // Send OP code of write disable
}

/*! @brief Put the EERAM48L512 device in hibernation
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM48L512_Hibernate(EERAM48L512 *pComp)
{
  return EERAM48L512_WriteCommand(pComp, EERAM48L512_HBRNT); // Send OP code of hibernation
}
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* EERAM48L512_H_INC */