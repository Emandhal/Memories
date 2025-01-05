/*!*****************************************************************************
 * @file    47x04.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.2.1
 * @date    04/01/2025
 * @brief   EERAM47x04 driver
 * @details I2C-Compatible (2-wire) 4-Kbit (512B x 8) Serial EERAM
 * Follow datasheet 47L04/47C04/47L16/47C16 Rev.C (Jun 2016)
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
 * 1.2.1    Update error management to add context
 * 1.2.0    Add EEPROM genericness
 * 1.1.0    I2C interface rework for I2C DMA use
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef EERAM47x04_H_INC
#define EERAM47x04_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "I2C_Interface.h"
//-----------------------------------------------------------------------------
#ifdef USE_EEPROM_GENERICNESS
#  include "EEPROM.h"
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#  define __EERAM47x04_PACKED__
#  define EERAM47x04_PACKITEM    __pragma(pack(push, 1))
#  define EERAM47x04_UNPACKITEM  __pragma(pack(pop))
#else
#  define __EERAM47x04_PACKED__  __attribute__((packed))
#  define EERAM47x04_PACKITEM
#  define EERAM47x04_UNPACKITEM
#endif
//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#define EERAM47x04_CONTROL_ITEM_SIZE(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }

//-----------------------------------------------------------------------------

#ifdef USE_EEPROM_GENERICNESS
// 47(L/C)04 EERAM configurations
extern const EEPROM_Conf EERAM47L04_Conf, EERAM47C04_Conf;
#endif

//-----------------------------------------------------------------------------

// Limits definitions
#define EERAM47x04_I2CCLOCK_MAX  ( 1000000u ) //!< Max I2C clock frequency

// Device definitions
#define EERAM47x04_SRAM_CHIPADDRESS_BASE  ( 0xA0 ) //!< SRAM chip base address
#define EERAM47x04_REG_CHIPADDRESS_BASE   ( 0x30 ) //!< Control Register chip base address
#define EERAM47x04_CHIPADDRESS_BASE_MASK  ( 0xF0 ) //!< Base chip address mask
#define EERAM47x04_CHIPADDRESS_MASK       ( 0xFC ) //!< Chip address mask

#define EERAM47x04_STATUS_REGISTER_ADDR   ( 0x00 ) //!< Address to the status register
#define EERAM47x04_COMMAND_REGISTER_ADDR  ( 0x55 ) //!< Address to the command register

#define EERAM47x04_STORE_COMMAND          ( 0b00110011 ) //!< Command to store SRAM data to EEPROM
#define EERAM47x04_RECALL_COMMAND         ( 0b11011101 ) //!< Command to recall data from EEPROM to SRAM

#define EERAM47x04_EERAM_SIZE             ( 512 ) //!< 47x04 total memory size

#define EERAM47x04_STORE_TIMEOUT          ( 8 ) //!< Store Operation Duration: 8ms
#define EERAM47x04_RECALL_TIMEOUT         ( 2 ) //!< Recall Operation Duration: 2ms

/*! @brief Generate the EERAM47x04 chip configurable address following the state of A1, and A2
 * You shall set '1' (when corresponding pin is connected to +V) or '0' (when corresponding pin is connected to Ground) on each parameter
 */
#define EERAM47x04_ADDR(A2, A1)  ( (uint8_t)((((A2) & 0x01) << 3) | (((A1) & 0x01) << 2)) )

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM47x04 Specific Controller Registers
//********************************************************************************************************************

//! Status Register
EERAM47x04_PACKITEM
typedef union __EERAM47x04_PACKED__ EERAM47x04_Status_Register
{
  uint8_t Status;
  struct
  {
    uint8_t EVENT: 1; //!< 0   - Event Detect bit: '1' = An event was detected on the HS pin ; '0' = No event was detected on the HS pin
    uint8_t ASE  : 1; //!< 1   - Auto-Store Enable bit: '1' = Auto-Store feature is enabled ; '0' = Auto-Store feature is disabled
    uint8_t BP   : 3; //!< 2-4 - Block Protect bits
    uint8_t      : 2; //!< 5-6
    uint8_t AM   : 1; //!< 7   - Array Modified bit: '1' = SRAM array has been modified ; '0' = SRAM array has not been modified
  } Bits;
} EERAM47x04_Status_Register;
EERAM47x04_UNPACKITEM;
EERAM47x04_CONTROL_ITEM_SIZE(EERAM47x04_Status_Register, 1);

#define EERAM47x04_EVENT_DETECTED  (0x1u << 0) //!< An event was detected on the HS pin
#define EERAM47x04_ASE_ENABLE      (0x1u << 1) //!< Enable Auto-Store feature
#define EERAM47x04_ASE_DISABLE     (0x0u << 1) //!< Disable Auto-Store feature

//! Block Protect
typedef enum
{
  EERAM47x04_NO_WRITE_PROTECT  = 0b000, //!< No write protect
  EERAM47x04_PROTECT_1F8h_1FFh = 0b001, //!< Protect range: Upper 1/64 (1F8h-1FFh)
  EERAM47x04_PROTECT_1F0h_1FFh = 0b010, //!< Protect range: Upper 1/32 (1F0h-1FFh)
  EERAM47x04_PROTECT_1E0h_1FFh = 0b011, //!< Protect range: Upper 1/16 (1E0h-1FFh)
  EERAM47x04_PROTECT_1C0h_1FFh = 0b100, //!< Protect range: Upper 1/8  (1C0h-1FFh)
  EERAM47x04_PROTECT_180h_1FFh = 0b101, //!< Protect range: Upper 1/4  (180h-1FFh)
  EERAM47x04_PROTECT_100h_1FFh = 0b110, //!< Protect range: Upper 1/2  (100h-1FFh)
  EERAM47x04_PROTECT_000h_1FFh = 0b111, //!< Protect range: All blocks
} eEERAM47x04_BlockProtect;

#define EERAM47x04_BP_Pos          2
#define EERAM47x04_BP_Mask         (0x7u << EERAM47x04_BP_Pos)
#define EERAM47x04_BP_SET(value)   (((uint8_t)(value) << EERAM47x04_BP_Pos) & EERAM47x04_BP_Mask) //!< Set Block Protect bits
#define EERAM47x04_BP_GET(value)   (((uint8_t)(value) & EERAM47x04_BP_Mask) >> EERAM47x04_BP_Pos) //!< Get Block Protect bits
#define EERAM47x04_ARRAY_MODIFIED  (0x1u << 7) //!< SRAM array has been modified

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// EERAM47x04 Driver API
//********************************************************************************************************************

#define EERAM47x04_DMA_TRANSFER_IN_PROGRESS_Pos        ( 0 )
#define EERAM47x04_DMA_TRANSFER_IN_PROGRESS            ( 1u << EERAM47x04_DMA_TRANSFER_IN_PROGRESS_Pos ) // DMA transfer in progress
#define EERAM47x04_IS_DMA_TRANSFER_IN_PROGRESS(value)  (((uint16_t)(value) & (1u << EERAM47x04_DMA_TRANSFER_IN_PROGRESS_Pos)) > 0) // Is DMA transfer in progress?
#define EERAM47x04_NO_DMA_TRANSFER_IN_PROGRESS_SET     (~EERAM47x04_DMA_TRANSFER_IN_PROGRESS)            // Mask to set no DMA transfer to the device

#define EERAM47x04_TRANSACTION_NUMBER_Pos           ( 1 )
#define EERAM47x04_TRANSACTION_NUMBER_Mask          ( 0x3F << EERAM47x04_TRANSACTION_NUMBER_Pos )
#define EERAM47x04_TRANSACTION_NUMBER_SET(value)    (((uint16_t)(value) << EERAM47x04_TRANSACTION_NUMBER_Pos) & EERAM47x04_TRANSACTION_NUMBER_Mask) // Set the transaction number to internal config
#define EERAM47x04_TRANSACTION_NUMBER_GET(value)    (((uint16_t)(value) & EERAM47x04_TRANSACTION_NUMBER_Mask) >> EERAM47x04_TRANSACTION_NUMBER_Pos) // Get the transaction number to internal config
#define EERAM47x04_TRANSACTION_NUMBER_CLEAR(value)  value &= ~EERAM47x04_TRANSACTION_NUMBER_Mask // Clears the transaction number of internal config

//-----------------------------------------------------------------------------

typedef struct EERAM47x04 EERAM47x04; //! Typedef of EERAM47x04 device object structure
typedef uint8_t TEERAM47x04DriverInternal; //! Alias for Driver Internal data flags

//-----------------------------------------------------------------------------

#if !defined(USE_EEPROM_GENERICNESS)
/*! @brief Function that gives the current millisecond of the system to the driver
 *
 * This function will be called when the driver needs to get current millisecond
 * @return Returns the current millisecond of the system
 */
typedef uint32_t (*GetCurrentms_Func)(void);
#endif

//-----------------------------------------------------------------------------

//! EERAM47x04 device object structure
struct EERAM47x04
{
#ifdef USE_EEPROM_GENERICNESS
  struct EEPROM Eeprom;
#else
  struct EEPROM_EERAM47x04
  {
    void *UserDriverData;                     //!< Optional, can be used to store driver data or NULL
    TEERAM47x04DriverInternal InternalConfig; //!< DO NOT USE OR CHANGE THIS VALUE, IT'S THE INTERNAL DRIVER CONFIGURATION

  //--- Interface driver call functions ---
# ifdef USE_DYNAMIC_INTERFACE
    I2C_Interface* I2C;                       //!< This is the I2C_Interface descriptor pointer that will be used to communicate with the device
# else
    I2C_Interface I2C;                        //!< This is the I2C_Interface descriptor that will be used to communicate with the device
# endif
    uint32_t I2CclockSpeed;                   //!< Clock frequency of the I2C interface in Hertz

    //--- Time call function ---
    GetCurrentms_Func fnGetCurrentms;         //!< This function will be called when the driver need to get current millisecond

    //--- Device address ---
    uint8_t AddrA2A1A0;                       //!< Device configurable address A2, and A1. A0 is not used. You can use the macro EERAM47x04_ADDR() to help filling this parameter. Only these 3 lower bits are used: ....21.. where 2 is A2, 1 is A1, and '.' are fixed by device
  } Eeprom;
#endif // USE_EEPROM_GENERICNESS
};

//-----------------------------------------------------------------------------


/*! @brief EERAM47x04 initialization
 *
 * This function initializes the EERAM47x04 driver and call the initialization of the interface driver (I2C).
 * Next it checks parameters and configures the EERAM47x04
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_EERAM47x04(EERAM47x04 *pComp);

/*! @brief Is the EERAM47x04 device ready
 *
 * Poll the acknowledge from the EERAM47x04
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns 'true' if ready else 'false'
 */
bool EERAM47x04_IsReady(EERAM47x04 *pComp);

//-----------------------------------------------------------------------------


/*! @brief Read SRAM data from the EERAM47x04 device
 *
 * This function reads data from the SRAM area of a EERAM47x04 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_ReadSRAMData(EERAM47x04 *pComp, uint16_t address, uint8_t* data, size_t size);

/*! @brief Read Control register from the EERAM47x04 device
 *
 * This function reads the content of the Status register of a EERAM47x04 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *data Is where the data will be stored
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_ReadRegister(EERAM47x04 *pComp, uint8_t* data);

/*! @brief Read SRAM data with DMA from the EERAM47x04 device
 *
 * To know the state of the DMA transfer, call this function. When the function returns ERR_NONE, it means that the transfer is complete else it returns the current state/error
 * In case of no DMA, the function act like a EERAM47x04_ReadSRAMData() function
 * @warning Never touch the data processed by the DMA before its completion
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_ReadSRAMDataWithDMA(EERAM47x04 *pComp, uint16_t address, uint8_t* data, size_t size);

//-----------------------------------------------------------------------------


/*! @brief Write SRAM data to the EERAM47x04 device
 *
 * This function writes data to the SRAM area of a EERAM47x04 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_WriteSRAMData(EERAM47x04 *pComp, uint16_t address, const uint8_t* data, size_t size);

/*! @brief Write Control register to the EERAM47x04 device
 *
 * This function writes data to the status or the command register of a EERAM47x04 device
 * Use the address EERAM47x04_STATUS_REGISTER_ADDR (0x00) to write to the status register
 * Use the address EERAM47x04_COMMAND_REGISTER_ADDR (0x55) to write to the command register
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] data Is the data array to store
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_WriteRegister(EERAM47x04 *pComp, uint8_t address, const uint8_t data);

/*! @brief Write SRAM data with DMA to the EERAM47x04 device
 *
 * To know the state of the DMA transfer, call this function. When the function returns ERR_NONE, it means that the transfer is complete else it returns the current state/error
 * In case of no DMA, the function act like a EERAM47x04_WriteSRAMData() function
 * @warning Never touch the data processed by the DMA before its completion
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be stored
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_WriteSRAMDataWithDMA(EERAM47x04 *pComp, uint16_t address, const uint8_t* data, size_t size);

//-----------------------------------------------------------------------------


/*! @brief Store all the SRAM to the EEPROM of the EERAM47x04 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] force This force the store of SRAM regardless of its status modified or not
 * @param[in] waitEndOfStore This indicate that the function do not return before the end of store operation duration. If 'false', the function return directly after the send of store operation
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_StoreSRAMtoEEPROM(EERAM47x04 *pComp, bool force, bool waitEndOfStore);

/*! @brief Recall all data from EEPROM to SRAM of the EERAM47x04 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] waitEndOfRecall This indicate that the function do not return before the end of recall operation duration. If 'false', the function return directly after the send of recall operation
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_RecallEEPROMtoSRAM(EERAM47x04 *pComp, bool waitEndOfRecall);

/*! @brief Activate the Auto-Store of the EERAM47x04 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_ActivateAutoStore(EERAM47x04 *pComp);

/*! @brief Deactivate the Auto-Store of the EERAM47x04 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_DeactivateAutoStore(EERAM47x04 *pComp);

/*! @brief Set block write protect of the EERAM47x04 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] blockProtect Indicate what location to write protect
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT EERAM47x04_SetBlockWriteProtect(EERAM47x04 *pComp, eEERAM47x04_BlockProtect blockProtect);

/*! @brief Get the status of the EERAM47x04 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] *status Is where the result will be saved
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT EERAM47x04_GetStatus(EERAM47x04 *pComp, EERAM47x04_Status_Register* status)
{
  return EERAM47x04_ReadRegister(pComp, &status->Status); // Get the status register
}

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* EERAM47x04_H_INC */