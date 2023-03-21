/*!*****************************************************************************
 * @file    48LM01.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    26/12/2021
 * @brief   EERAM48LM01 driver
 * @details SPI-Compatible 1-Mbit SPI Serial EERAM
 * Follow datasheet DS20006008C Rev.C (Oct 2019)
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
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef EERAM48LM01_H_INC
#define EERAM48LM01_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "SPI_Interface.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#  define __EERAM48LM01_PACKED__
#  define EERAM48LM01_PACKITEM    __pragma(pack(push, 1))
#  define EERAM48LM01_UNPACKITEM  __pragma(pack(pop))
#else
#  define __EERAM48LM01_PACKED__  __attribute__((packed))
#  define EERAM48LM01_PACKITEM
#  define EERAM48LM01_UNPACKITEM
#endif

//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#define EERAM48LM01_CONTROL_ITEM_SIZE(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }

//-----------------------------------------------------------------------------



// Limits definitions
#define EERAM48LM01_SPICLOCK_MAX  ( 66000000u ) //!< Max SPI clock frequency



// Device definitions
#define EERAM48LM01_PAGE_SIZE          ( 128 ) //!< The EERAM48LM01 is 128 bytes page size (for secured transfer)
#define EERAM48LM01_PAGE_SIZE_MASK     ( EERAM48LM01_PAGE_SIZE - 1 ) //!< The EERAM48LM01 page mask is 127 bytes
#define EERAM48LM01_NONVOLATILE_SIZE   (  16 ) //!< The EERAM48LM01 have 16 bytes of Nonvolatile User Space

#define EERAM48LM01_EERAM_SIZE         ( 131072 ) //!< 48LM01 total memory size
#define EERAM48LM01_ADDRESS_BYTE_SIZE  ( 3 ) //!< 3 byte for the address

#define EERAM48LM01_STORE_TIMEOUT      ( 10 ) //!< Store Operation Duration: 10ms
#define EERAM48LM01_RECALL_TIMEOUT     (  0 ) //!< Store Operation Duration: 50µs

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM48LM01 Register list
//********************************************************************************************************************

//! EERAM48LM01 Operation Codes list
typedef enum
{
  // Write Control Commands
  EERAM48LM01_WREN   = 0x06, //!< Set Write Enable Latch
  EERAM48LM01_WRDI   = 0x04, //!< Reset Write Enable Latch
  // SRAM Commands
  EERAM48LM01_WRITE  = 0x02, //!< Write to SRAM Array
  EERAM48LM01_READ   = 0x03, //!< Read from SRAM Array
  EERAM48LM01_SWRITE = 0x12, //!< Secure Write to SRAM Array with CRC
  EERAM48LM01_SREAD  = 0x13, //!< Secure Read from SRAM Array with CRC
  // STATUS Register Commands
  EERAM48LM01_WRSR   = 0x01, //!< Write STATUS Register (SR)
  EERAM48LM01_RDSR   = 0x05, //!< Read STATUS Register (SR)
  // Store/Recall Commands
  EERAM48LM01_STORE  = 0x08, //!< Store SRAM data to EEPROM array
  EERAM48LM01_RECALL = 0x09, //!< Copy EEPROM data to SRAM array
  // Nonvolatile User Space Commands
  EERAM48LM01_WRNUR  = 0xC2, //!< Write Nonvolatile User Space
  EERAM48LM01_RDNUR  = 0xC3, //!< Read Nonvolatile User Space
  // Hibernate Commands
  EERAM48LM01_HBRNT  = 0xB9, //!< Enter Hibernate Mode
} eEERAM48LM01_OPcodes;

#define EERAM48LM01_IS_NV_USER_SPACE(opCode)  ( ((uint8_t)(opCode) & 0xC0) == 0xC0 )





//********************************************************************************************************************
// EERAM48LM01 Specific Controller Registers
//********************************************************************************************************************

//! Status Register
EERAM48LM01_PACKITEM
typedef union __EERAM48LM01_PACKED__ EERAM48LM01_StatusRegister
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
} EERAM48LM01_StatusRegister;
EERAM48LM01_UNPACKITEM;
EERAM48LM01_CONTROL_ITEM_SIZE(EERAM48LM01_StatusRegister, 1);

#define EERAM48LM01_IS_BUSY        (0x1u << 0) //!< Device is busy with an internal store or recall operation
#define EERAM48LM01_IS_READY       (0x0u << 0) //!< Device is ready for standard SRAM Read/Write commands
#define EERAM48LM01_WRITE_ENABLE   (0x1u << 1) //!< WREN has been executed and device is enabled for writing
#define EERAM48LM01_WRITE_DISABLE  (0x0u << 1) //!< Device is not write-enabled

//! Block Protect
typedef enum
{
  EERAM48LM01_NO_WRITE_PROTECT      = 0b00, //!< (Level 0) No SRAM array write protection (factory default)
  EERAM48LM01_PROTECT_18000h_1FFFFh = 0b01, //!< (Level 1) Upper quarter SRAM memory array protection
  EERAM48LM01_PROTECT_10000h_1FFFFh = 0b10, //!< (Level 2) Upper half SRAM memory array protection
  EERAM48LM01_PROTECT_00000h_1FFFFh = 0b11, //!< (Level 3) Entire SRAM memory array protection
} eEERAM48LM01_BlockProtect;

#define EERAM48LM01_BP_Pos                 2
#define EERAM48LM01_BP_Mask                (0x3u << EERAM48LM01_BP_Pos)
#define EERAM48LM01_BP_SET(value)          (((uint8_t)(value) << EERAM48LM01_BP_Pos) & EERAM48LM01_BP_Mask) //!< Set Block Protect bits
#define EERAM48LM01_BP_GET(value)          (((uint8_t)(value) & EERAM48LM01_BP_Mask) >> EERAM48LM01_BP_Pos) //!< Get Block Protect bits
#define EERAM48LM01_WRITE_SECURE_FAILED    (0x1u << 4) //!< The last secure write operation has failed
#define EERAM48LM01_NO_WRITE_SECURE_ERROR  (0x0u << 4) //!< No error reported in Secure Write
#define EERAM48LM01_AUTOSTORE_DISABLE      (0x1u << 6) //!< AutoStore is disabled
#define EERAM48LM01_AUTOSTORE_ENABLE       (0x0u << 6) //!< AutoStore is enabled (factory default)

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM48LM01 Driver API
//********************************************************************************************************************

#define EERAM48LM01_DMA_TRANSFER_IN_PROGRESS_Pos        ( 0 )
#define EERAM48LM01_DMA_TRANSFER_IN_PROGRESS            ( 1u << EERAM48LM01_DMA_TRANSFER_IN_PROGRESS_Pos ) // DMA transfer in progress
#define EERAM48LM01_IS_DMA_TRANSFER_IN_PROGRESS(value)  (((uint8_t)(value) & EERAM48LM01_DMA_TRANSFER_IN_PROGRESS) > 0) // Is DMA transfer in progress?
#define EERAM48LM01_NO_DMA_TRANSFER_IN_PROGRESS_SET     (~EERAM48LM01_DMA_TRANSFER_IN_PROGRESS)            // Mask to set no DMA transfer to the device

#define EERAM48LM01_TRANSACTION_NUMBER_Pos           ( 1 )
#define EERAM48LM01_TRANSACTION_NUMBER_Mask          ( 0x3F << EERAM48LM01_TRANSACTION_NUMBER_Pos )
#define EERAM48LM01_TRANSACTION_NUMBER_SET(value)    (((uint8_t)(value) << EERAM48LM01_TRANSACTION_NUMBER_Pos) & EERAM48LM01_TRANSACTION_NUMBER_Mask) // Set the transaction number to internal config
#define EERAM48LM01_TRANSACTION_NUMBER_GET(value)    (((uint8_t)(value) & EERAM48LM01_TRANSACTION_NUMBER_Mask) >> EERAM48LM01_TRANSACTION_NUMBER_Pos) // Get the transaction number to internal config
#define EERAM48LM01_TRANSACTION_NUMBER_CLEAR(value)  value &= ~EERAM48LM01_TRANSACTION_NUMBER_Mask // Clears the transaction number of internal config

#define EERAM48LM01_STATUS_WRITE_ENABLE_Pos        ( 7 )
#define EERAM48LM01_STATUS_WRITE_ENABLE            ( 1u << EERAM48LM01_STATUS_WRITE_ENABLE_Pos ) // Write enable
#define EERAM48LM01_IS_STATUS_WRITE_ENABLE(value)  (((uint8_t)(value) & (1u << EERAM48LM01_STATUS_WRITE_ENABLE_Pos)) > 0) // Is write enable?
#define EERAM48LM01_STATUS_WRITE_DISABLE_SET       (~EERAM48LM01_STATUS_WRITE_ENABLE)            // Mask to set write disable

//-----------------------------------------------------------------------------


typedef struct EERAM48LM01 EERAM48LM01; //! Typedef of EERAM48LM01 device object structure
typedef uint8_t TEERAM48LM01DriverInternal; //! Alias for Driver Internal data flags

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



//! EERAM48LM01 device object structure
struct EERAM48LM01
{
  void *UserDriverData;                      //!< Optional, can be used to store driver data or NULL
  TEERAM48LM01DriverInternal InternalConfig; //!< DO NOT USE OR CHANGE THIS VALUE, IT'S THE INTERNAL DRIVER CONFIGURATION

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





/*! @brief EERAM48LM01 initialization
 *
 * This function initializes the EERAM48LM01 driver and call the initialization of the interface driver (SPI).
 * Next it checks parameters and configures the EERAM48LM01
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_EERAM48LM01(EERAM48LM01 *pComp);

//********************************************************************************************************************



/*! @brief Read SRAM data from the EERAM48LM01 device
 *
 * This function reads data from the SRAM area of a EERAM48LM01 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_ReadSRAMData(EERAM48LM01 *pComp, uint32_t address, uint8_t* data, size_t size);


/*! @brief Secure read SRAM data to the EERAM48LM01 device
 *
 * This function reads data from the SRAM area of a EERAM48LM01 device with a CRC check
 * @warning The size of data to read shall be a multiple of 128 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_ReadSecure(EERAM48LM01 *pComp, uint32_t address, uint8_t* data, size_t size);


/*! @brief Write NonVolatile User Space data to the EERAM48LM01 device
 *
 * This function reads data from the NonVolatile User Space area of a EERAM48LM01 device
 * @warning The size of data buffer shall be 16 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *data Is where the data will be stored
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_ReadNVUSData(EERAM48LM01 *pComp, uint8_t* data);


/*! @brief Read SRAM data with DMA from the EERAM48LM01 device
 *
 * To know the state of the DMA transfer, call this function. When the function returns ERR_OK, it means that the transfer is complete else it returns the current state/error
 * In case of no DMA, the function act like a EERAM48LM01_ReadSRAMData() function
 * @warning Never touch the data processed by the DMA before its completion
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_ReadSRAMDataWithDMA(EERAM48LM01 *pComp, uint32_t address, uint8_t* data, size_t size);

//********************************************************************************************************************



/*! @brief Write SRAM data to the EERAM48LM01 device
 *
 * This function writes data to the SRAM area of a EERAM48LM01 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_WriteSRAMData(EERAM48LM01 *pComp, uint32_t address, const uint8_t* data, size_t size);


/*! @brief Secure write SRAM data to the EERAM48LM01 device
 *
 * This function writes data to the SRAM area of a EERAM48LM01 device with a CRC check
 * @warning The size of data buffer to write shall be a multiple of 128 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_WriteSecure(EERAM48LM01 *pComp, uint32_t address, const uint8_t* data, size_t size);


/*! @brief Write NonVolatile User Space data to the EERAM48LM01 device
 *
 * This function writes data to the NonVolatile User Space area of a EERAM48LM01 device
 * @warning The size of data to write shall be 16 bytes
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] *data Is the data array to store
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_WriteNVUSData(EERAM48LM01 *pComp, const uint8_t* data);


/*! @brief Write a command to the EERAM48LM01 device
 *
 * This function asserts the device, write the OP code and deasserts the device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] command Is the OP code to send
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_WriteCommand(EERAM48LM01 *pComp, const eEERAM48LM01_OPcodes command);


/*! @brief Write SRAM data with DMA to the EERAM48LM01 device
 *
 * To know the state of the DMA transfer, call this function. When the function returns ERR_OK, it means that the transfer is complete else it returns the current state/error
 * In case of no DMA, the function act like a EERAM48LM01_WriteSRAMData() function
 * @warning Never touch the data processed by the DMA before its completion
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_WriteSRAMDataWithDMA(EERAM48LM01 *pComp, uint32_t address, const uint8_t* data, size_t size);

//********************************************************************************************************************



/*! @brief Get the status of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *status Is where the result will be saved
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_GetStatus(EERAM48LM01 *pComp, EERAM48LM01_StatusRegister* status);


/*! @brief Set the status of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] status Is the status to store
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_SetStatus(EERAM48LM01 *pComp, const EERAM48LM01_StatusRegister status);

//********************************************************************************************************************



/*! @brief Store all the SRAM to the EEPROM of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] waitEndOfStore This indicate that the function do not return before the end of store operation duration. If 'false', the function return directly after the send of store operation
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_StoreSRAMtoEEPROM(EERAM48LM01 *pComp, bool waitEndOfStore);


/*! @brief Recall all data from EEPROM to SRAM of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] waitEndOfRecall This indicate that the function do not return before the end of recall operation duration. If 'false', the function return directly after the send of recall operation
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_RecallEEPROMtoSRAM(EERAM48LM01 *pComp, bool waitEndOfRecall);


/*! @brief Activate the Auto-Store of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_ActivateAutoStore(EERAM48LM01 *pComp);


/*! @brief Deactivate the Auto-Store of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_DeactivateAutoStore(EERAM48LM01 *pComp);


/*! @brief Set block write protect of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] blockProtect Indicate what location to write protect
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM48LM01_SetBlockWriteProtect(EERAM48LM01 *pComp, eEERAM48LM01_BlockProtect blockProtect);


/*! @brief Set write enable of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM48LM01_SetWriteEnable(EERAM48LM01 *pComp)
{
  eERRORRESULT Error = EERAM48LM01_WriteCommand(pComp, EERAM48LM01_WREN); // Send OP code of write enable
  pComp->InternalConfig |= EERAM48LM01_STATUS_WRITE_ENABLE;               // Set write enable flag
  return Error;
}


/*! @brief Set write disable of the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM48LM01_SetWriteDisable(EERAM48LM01 *pComp)
{
  return EERAM48LM01_WriteCommand(pComp, EERAM48LM01_WRDI); // Send OP code of write disable
}


/*! @brief Put the EERAM48LM01 device in hibernation
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM48LM01_Hibernate(EERAM48LM01 *pComp)
{
  return EERAM48LM01_WriteCommand(pComp, EERAM48LM01_HBRNT); // Send OP code of hibernation
}

//********************************************************************************************************************





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* EERAM48LM01_H_INC */