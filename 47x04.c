/*!*****************************************************************************
 * @file    47x04.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.1.0
 * @date    23/10/2021
 * @brief   EERAM47x04 driver
 * @details I2C-Compatible (2-wire) 4-Kbit (512B x 8) Serial EERAM
 * Follow datasheet 47L04/47C04/47L16/47C16 Rev.C (Jun 2016)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "47x04.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
//-----------------------------------------------------------------------------

#ifdef USE_DYNAMIC_INTERFACE
#  define GET_I2C_INTERFACE  pComp->I2C
#else
#  define GET_I2C_INTERFACE  &pComp->I2C
#endif

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// EERAM47x04 initialization
//=============================================================================
eERRORRESULT Init_EERAM47x04(EERAM47x04 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  if (pComp->I2CclockSpeed > EERAM47x04_I2CCLOCK_MAX) return ERR__I2C_FREQUENCY_ERROR;
  Error = pI2C->fnI2C_Init(pI2C, pComp->I2CclockSpeed);
  if (Error != ERR_OK) return Error; // If there is an error while calling fnI2C_Init() then return the Error

  pComp->InternalConfig = 0;
  return (EERAM47x04_IsReady(pComp) ? ERR_OK : ERR__NO_DEVICE_DETECTED);
}



//=============================================================================
// Is the EERAM47x04 device ready
//=============================================================================
bool EERAM47x04_IsReady(EERAM47x04 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return false;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return false;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return false;
#endif
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER),
    I2C_MEMBER(ChipAddr    ) (EERAM47x04_SRAM_CHIPADDRESS_BASE | pComp->AddrA2A1) & I2C_WRITE_ANDMASK,
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) NULL,
    I2C_MEMBER(BufferSize  ) 0,
    I2C_MEMBER(Stop        ) true,
  };
  return (pI2C->fnI2C_Transfer(pI2C, &PacketDesc) == ERR_OK); // Send only the chip address and get the Ack flag
}





//**********************************************************************************************************************************************************
// Write 2-bytes address the EERAM47x04 (DO NOT USE DIRECTLY)
static eERRORRESULT __EERAM47x04_WriteAddress(EERAM47x04 *pComp, const uint8_t chipAddr, const uint16_t address, const bool usePolling, const eI2C_TransferType transferType)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  const uint8_t AddrBytes = ((chipAddr & EERAM47x04_CHIPADDRESS_BASE_MASK) == EERAM47x04_SRAM_CHIPADDRESS_BASE ? 2 : 1); // If the base chip address is the SRAM then the address is 2 bytes else 1 byte (control registers)
  eERRORRESULT Error;

  //--- Create address ---
  uint8_t Address[sizeof(uint16_t)];
  for (int_fast8_t z = AddrBytes; --z >= 0;) Address[z] = (uint8_t)((address >> ((AddrBytes - z - 1) * 8)) & 0xFF);
  //--- Send the address ---
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) (usePolling ? I2C_USE_POLLING : I2C_NO_POLLING) | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(transferType),
    I2C_MEMBER(ChipAddr    ) chipAddr & I2C_WRITE_ANDMASK,
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) &Address[0],
    I2C_MEMBER(BufferSize  ) AddrBytes,
    I2C_MEMBER(Stop        ) false,
  };
  Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc);                  // Transfer the address
  if (Error == ERR__I2C_NACK) return ERR__NOT_READY;                // If the device receive a NAK, then the device is not ready
  if (Error == ERR__I2C_NACK_DATA) return ERR__I2C_INVALID_ADDRESS; // If the device receive a NAK while transferring data, then this is an invalid address
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read SRAM data from the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_ReadSRAMData(EERAM47x04 *pComp, uint16_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint16_t)size) > EERAM47x04_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  const uint8_t ChipAddrW = ((EERAM47x04_SRAM_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x04_CHIPADDRESS_MASK);
  eERRORRESULT Error;

  //--- Read data ---
  Error = __EERAM47x04_WriteAddress(pComp, ChipAddrW, address, false, I2C_WRITE_THEN_READ_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_OK)                                                                                 // If there is no error while writing address then
  {
    I2CInterface_Packet PacketDesc =
    {
      I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_READ_SECOND_PART),
      I2C_MEMBER(ChipAddr    ) (ChipAddrW | I2C_READ_ORMASK),
      I2C_MEMBER(Start       ) true,
      I2C_MEMBER(pBuffer     ) data,
      I2C_MEMBER(BufferSize  ) size,
      I2C_MEMBER(Stop        ) true,
    };
    Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Restart a read transfer, get the data and stop transfer
  }
  return Error;
}



//=============================================================================
// Read Control register from the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_ReadRegister(EERAM47x04 *pComp, uint8_t* data)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  //--- Read data from I2C ---
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER),
    I2C_MEMBER(ChipAddr    ) (((EERAM47x04_REG_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x04_CHIPADDRESS_MASK) | I2C_READ_ORMASK),
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) data,
    I2C_MEMBER(BufferSize  ) 1,
    I2C_MEMBER(Stop        ) true,
  };
  return pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Start a read transfer, get the data and stop transfer
}



//=============================================================================
// Read SRAM data with DMA from the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_ReadSRAMDataWithDMA(EERAM47x04 *pComp, uint16_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint16_t)size) > EERAM47x04_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  const uint8_t ChipAddrW = ((EERAM47x04_SRAM_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x04_CHIPADDRESS_MASK);
  const uint8_t ChipAddrR = (ChipAddrW | I2C_READ_ORMASK);
  I2CInterface_Packet PacketDesc;
  eERRORRESULT Error;

  //--- Check DMA ---
  if (EERAM47x04_IS_DMA_TRANSFER_IN_PROGRESS(pComp->InternalConfig))
  {
    const uint16_t CurrTransactionNumber = EERAM47x04_TRANSACTION_NUMBER_GET(pComp->InternalConfig);
    PacketDesc.Config.Value = I2C_USE_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER) | I2C_TRANSACTION_NUMBER_SET(CurrTransactionNumber);
    PacketDesc.ChipAddr     = ChipAddrR;
    PacketDesc.Start        = true;
    PacketDesc.pBuffer      = NULL;
    PacketDesc.BufferSize   = 0;
    PacketDesc.Stop         = true;
    Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Send only the chip address and get the Ack flag, to return the status of the current transfer
    if ((Error != ERR__I2C_BUSY) && (Error != ERR__I2C_OTHER_BUSY)) pComp->InternalConfig &= EERAM47x04_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    return Error;
  }

  //--- Read data ---
  Error = __EERAM47x04_WriteAddress(pComp, ChipAddrW, address, true, I2C_WRITE_THEN_READ_FIRST_PART); // Start a read at address with the device
  if (Error == ERR_OK)                                                                                // If there is no error while writing address then
  {
    PacketDesc.Config.Value = I2C_USE_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_READ_SECOND_PART);
    PacketDesc.ChipAddr     = ChipAddrR;
    PacketDesc.pBuffer      = data;
    PacketDesc.BufferSize   = size;
    PacketDesc.Start        = true;
    PacketDesc.Stop         = true;
    Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Restart at first data read transfer, get the data and stop transfer at last data
    if (Error != ERR__I2C_OTHER_BUSY) pComp->InternalConfig &= EERAM47x04_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    if (Error == ERR__I2C_BUSY) pComp->InternalConfig |= EERAM47x04_DMA_TRANSFER_IN_PROGRESS;
    EERAM47x04_TRANSACTION_NUMBER_CLEAR(pComp->InternalConfig);
    pComp->InternalConfig |= EERAM47x04_TRANSACTION_NUMBER_SET(I2C_TRANSACTION_NUMBER_GET(PacketDesc.Config.Value));
  }
  return Error;
}





//**********************************************************************************************************************************************************
// Write data to the EERAM47x04 (DO NOT USE DIRECTLY, use EERAM47x04_WriteSRAMData() or EERAM47x04_WriteRegister() instead)
static eERRORRESULT __EERAM47x04_WriteData(EERAM47x04 *pComp, const uint8_t chipAddr, uint16_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint16_t)size) > EERAM47x04_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  const uint8_t ChipAddr = (chipAddr | pComp->AddrA2A1) & EERAM47x04_CHIPADDRESS_MASK;
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Write data ---
  Error = __EERAM47x04_WriteAddress(pComp, ChipAddr, address, false, I2C_WRITE_THEN_WRITE_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_OK)                                                                                 // If there is no error while writing address then
  {
    I2CInterface_Packet PacketDesc =
    {
      I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_WRITE_SECOND_PART),
      I2C_MEMBER(ChipAddr    ) ChipAddr, // Chip address will not be used
      I2C_MEMBER(Start       ) false,
      I2C_MEMBER(pBuffer     ) pData,
      I2C_MEMBER(BufferSize  ) size,
      I2C_MEMBER(Stop        ) true,
    };
    Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Continue the transfer by sending the data and stop transfer
  }
  return Error;
}



//=============================================================================
// Write SRAM data to the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_WriteSRAMData(EERAM47x04 *pComp, uint16_t address, const uint8_t* data, size_t size)
{
  return __EERAM47x04_WriteData(pComp, EERAM47x04_SRAM_CHIPADDRESS_BASE, address, data, size);
}



//=============================================================================
// Write Control register to the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_WriteRegister(EERAM47x04 *pComp, uint8_t address, const uint8_t data)
{
  eERRORRESULT Error = __EERAM47x04_WriteData(pComp, EERAM47x04_REG_CHIPADDRESS_BASE, address, &data, 1);
  if (Error == ERR__I2C_NACK_DATA) Error = ERR__I2C_INVALID_COMMAND; // Here a NACK indicate that the command (aka 'data') is invalid. It cannot be anything else
  return Error;
}



//=============================================================================
// Write SRAM data with DMA to the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_WriteSRAMDataWithDMA(EERAM47x04 *pComp, uint16_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint16_t)size) > EERAM47x04_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  const uint8_t ChipAddrW = ((EERAM47x04_SRAM_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x04_CHIPADDRESS_MASK);
  const uint8_t ChipAddrR = (ChipAddrW | I2C_READ_ORMASK);
  uint8_t* pData = (uint8_t*)data;
  I2CInterface_Packet PacketDesc;
  eERRORRESULT Error;

  //--- Check DMA ---
  if (EERAM47x04_IS_DMA_TRANSFER_IN_PROGRESS(pComp->InternalConfig))
  {
    const uint16_t CurrTransactionNumber = EERAM47x04_TRANSACTION_NUMBER_GET(pComp->InternalConfig);
    PacketDesc.Config.Value = I2C_USE_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER) | I2C_TRANSACTION_NUMBER_SET(CurrTransactionNumber);
    PacketDesc.ChipAddr     = ChipAddrR;
    PacketDesc.Start        = true;
    PacketDesc.pBuffer      = NULL;
    PacketDesc.BufferSize   = 0;
    PacketDesc.Stop         = true;
    Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Send only the chip address and get the Ack flag, to return the status of the current transfer
    if ((Error != ERR__I2C_BUSY) && (Error != ERR__I2C_OTHER_BUSY)) pComp->InternalConfig &= EERAM47x04_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    return Error;
  }

  //--- Write data ---
  Error = __EERAM47x04_WriteAddress(pComp, ChipAddrW, address, true, I2C_WRITE_THEN_WRITE_FIRST_PART); // Start a read at address with the device
  if (Error == ERR_OK)                                                                                 // If there is no error while writing address then
  {
    PacketDesc.Config.Value = I2C_USE_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_WRITE_SECOND_PART);
    PacketDesc.ChipAddr     = ChipAddrW;
    PacketDesc.pBuffer      = pData;
    PacketDesc.BufferSize   = size;
    PacketDesc.Start        = true;
    PacketDesc.Stop         = true;
    Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc); // Restart at first data read transfer, get the data and stop transfer at last data
    if (Error != ERR__I2C_OTHER_BUSY) pComp->InternalConfig &= EERAM47x04_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    if (Error == ERR__I2C_BUSY) pComp->InternalConfig |= EERAM47x04_DMA_TRANSFER_IN_PROGRESS;
    EERAM47x04_TRANSACTION_NUMBER_CLEAR(pComp->InternalConfig);
    pComp->InternalConfig |= EERAM47x04_TRANSACTION_NUMBER_SET(I2C_TRANSACTION_NUMBER_GET(PacketDesc.Config.Value));
  }
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Store all the SRAM to the EEPROM of the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_StoreSRAMtoEEPROM(EERAM47x04 *pComp, bool forceStore, bool waitEndOfStore)
{
#ifdef CHECK_NULL_PARAM
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error = ERR_OK;
  EERAM47x04_Status_Register Reg;
  bool Store = forceStore;

  //--- Check the need of store operation ---
  if (forceStore == false)                                                           // Check register Status.AM if the store is not forced
  {
    Error = EERAM47x04_ReadRegister(pComp, &Reg.Status);                             // Get the status register
    if (Error != ERR_OK) return Error;                                               // If there is an error while reading register, then return the error
    Store = ((Reg.Status & EERAM47x04_ARRAY_MODIFIED) > 0);                          // If the array has been modified, flag the store
  }
  //--- Send the store operation if necessary or asked ---
  if (Store)
  {
    uint8_t Data = EERAM47x04_STORE_COMMAND;
    Error = EERAM47x04_WriteRegister(pComp, EERAM47x04_COMMAND_REGISTER_ADDR, Data); // Execute a store
    if (Error != ERR_OK) return Error;                                               // If there is an error while reading register, then return the error
    //--- Wait the end of store if asked ---
    if (waitEndOfStore)
    {
      Reg.Status = EERAM47x04_ARRAY_MODIFIED;
      uint32_t Timeout = pComp->fnGetCurrentms() + EERAM47x04_STORE_TIMEOUT + 1; // Wait at least STORE_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
      while (true)
      {
        Error = EERAM47x04_ReadRegister(pComp, &Reg.Status);                // Get the status register
        if ((Error != ERR_OK) && (Error != ERR__I2C_NACK)) return Error;    // If there is an error while calling EERAM47x04_ReadRegister() then return the error
        if ((Reg.Status & EERAM47x04_ARRAY_MODIFIED) == 0) break;           // The store is finished, all went fine
        if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
      }
    }
  }
  return Error;
}



//=============================================================================
// Recall all data from EEPROM to SRAM of the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_RecallEEPROMtoSRAM(EERAM47x04 *pComp, bool waitEndOfRecall)
{
#ifdef CHECK_NULL_PARAM
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error = ERR_OK;
  EERAM47x04_Status_Register Reg;

  //--- Send a recall operation ---
  uint8_t Data = EERAM47x04_RECALL_COMMAND;
  Error = EERAM47x04_WriteRegister(pComp, EERAM47x04_COMMAND_REGISTER_ADDR, Data); // Execute a recall
  if (Error != ERR_OK) return Error;                                               // If there is an error while reading register, then return the error
  //--- Wait the end of recall if asked ---
  if (waitEndOfRecall)
  {
    Reg.Status = EERAM47x04_ARRAY_MODIFIED;
    uint32_t Timeout = pComp->fnGetCurrentms() + EERAM47x04_RECALL_TIMEOUT + 1; // Wait at least RECALL_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = EERAM47x04_ReadRegister(pComp, &Reg.Status);                // Get the status register
      if ((Error != ERR_OK) && (Error != ERR__I2C_NACK)) return Error;    // If there is an error while calling EERAM47x04_ReadRegister() then return the error
      if ((Reg.Status & EERAM47x04_ARRAY_MODIFIED) == 0) break;           // The recall is finished, all went fine
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
  }
  return ERR_OK;
}



//=============================================================================
// Activate the Auto-Store of the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_ActivateAutoStore(EERAM47x04 *pComp)
{
  eERRORRESULT Error = ERR_OK;
  EERAM47x04_Status_Register Reg;

  //--- Get the status register ---
  Error = EERAM47x04_ReadRegister(pComp, &Reg.Status);                                 // Get the status register
  if (Error != ERR_OK) return Error;                                                   // If there is an error while sending data to I2C then stop the transfer
  //--- Set the status register ---
  Reg.Status |= EERAM47x04_ASE_ENABLE;                                                 // Set the Auto-Store flag
  return EERAM47x04_WriteRegister(pComp, EERAM47x04_STATUS_REGISTER_ADDR, Reg.Status); // Save the status register
}



//=============================================================================
// Deactivate the Auto-Store of the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_DeactivateAutoStore(EERAM47x04 *pComp)
{
  eERRORRESULT Error = ERR_OK;
  EERAM47x04_Status_Register Reg;

  //--- Get the status register ---
  Error = EERAM47x04_ReadRegister(pComp, &Reg.Status);                                 // Get the status register
  if (Error != ERR_OK) return Error;                                                   // If there is an error while sending data to I2C then stop the transfer
  //--- Set the status register ---
  Reg.Status &= ~EERAM47x04_ASE_ENABLE;                                                // Unset the Auto-Store flag
  return EERAM47x04_WriteRegister(pComp, EERAM47x04_STATUS_REGISTER_ADDR, Reg.Status); // Save the status register
}



//=============================================================================
// Set block write protect of the EERAM47x04 device
//=============================================================================
eERRORRESULT EERAM47x04_SetBlockWriteProtect(EERAM47x04 *pComp, eEERAM47x04_BlockProtect blockProtect)
{
  eERRORRESULT Error = ERR_OK;
  EERAM47x04_Status_Register Reg;

  //--- Get the status register ---
  Error = EERAM47x04_ReadRegister(pComp, &Reg.Status);                                 // Get the status register
  if (Error != ERR_OK) return Error;                                                   // If there is an error while sending data to I2C then stop the transfer
  //--- Set the status register ---
  Reg.Bits.BP = (uint8_t)blockProtect;                                                 // Set the block protect
  return EERAM47x04_WriteRegister(pComp, EERAM47x04_STATUS_REGISTER_ADDR, Reg.Status); // Save the status register
}





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------