/*******************************************************************************
 * @file    47x16.c
 * @author  FMA
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   EERAM47x16 driver
 *
 * I2C-Compatible (2-wire) 16-Kbit (2kB x 8) Serial EERAM
 * Follow datasheet 47L04/47C04/47L16/47C16 Rev.C (Jun  2016)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "47x16.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// EERAM47x16 initialization
//=============================================================================
eERRORRESULT Init_EERAM47x16(EERAM47x16 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  if (pComp->I2C_ClockSpeed > EERAM47x16_I2CCLOCK_MAX) return ERR__I2C_FREQUENCY_ERROR;
  Error = pComp->fnI2C_Init(pComp->InterfaceDevice, pComp->I2C_ClockSpeed);
  if (Error != ERR_OK) return Error; // If there is an error while calling fnI2C_Init() then return the Error

  return (EERAM47x16_IsReady(pComp) ? ERR_OK : ERR__NO_DEVICE_DETECTED);
}



//=============================================================================
// Is the EERAM47x16 device ready
//=============================================================================
bool EERAM47x16_IsReady(EERAM47x16 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return false;
  if (pComp->fnI2C_Transfer == NULL) return false;
#endif
  uint8_t ChipAddrW = (EERAM47x16_SRAM_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x16_I2C_WRITE;
  return (pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, NULL, 0, true, true) == ERR_OK); // Send only the chip address and get the Ack flag
}





//**********************************************************************************************************************************************************
// Write 2-bytes address the EERAM47x16 (DO NOT USE DIRECTLY)
static eERRORRESULT __EERAM47x16_WriteAddress(EERAM47x16 *pComp, const uint8_t chipAddr, uint16_t address)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  uint8_t ChipAddrW = chipAddr & EERAM47x16_I2C_WRITE;
  uint8_t AddrBytes = ((chipAddr & EERAM47x16_CHIPADDRESS_BASE_MASK) == EERAM47x16_SRAM_CHIPADDRESS_BASE ? 2 : 1); // If the base chip address is the SRAM then the address is 2 bytes else 1 byte (control registers)

  //--- Create address ---
  uint8_t Address[sizeof(uint16_t)];
  for (int_fast8_t z = AddrBytes; --z >= 0;) Address[z] = (uint8_t)((address >> ((AddrBytes - z - 1) * 8)) & 0xFF);
  //--- Send the address ---
  Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, &Address[0], AddrBytes, true, false); // Transfer the address
  if (Error == ERR__I2C_NACK) return ERR__NOT_READY;                                                     // If the device receive a NAK, then the device is not ready
  if (Error == ERR__I2C_NACK_DATA) return ERR__I2C_INVALID_ADDRESS;                                      // If the device receive a NAK while transferring data, then this is an invalid address
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read SRAM data from the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_ReadSRAMData(EERAM47x16 *pComp, uint16_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > EERAM47x16_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  uint8_t ChipAddrW = ((EERAM47x16_SRAM_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x16_CHIPADDRESS_MASK);
  uint8_t ChipAddrR = (ChipAddrW | EERAM47x16_I2C_READ);

  //--- Read data ---
  Error = __EERAM47x16_WriteAddress(pComp, ChipAddrW, address);                               // Start a write at address with the device
  if (Error == ERR_OK)                                                                        // If there is no error while writing address then
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrR, data, size, true, true); // Restart a read transfer, get the data and stop transfer
  return Error;
}



//=============================================================================
// Read Control register from the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_ReadRegister(EERAM47x16 *pComp, uint8_t* data)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t ChipAddrR = (((EERAM47x16_REG_CHIPADDRESS_BASE | pComp->AddrA2A1) & EERAM47x16_CHIPADDRESS_MASK) | EERAM47x16_I2C_READ);
  //--- Read data from I2C ---
  return pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrR, data, 1, true, true); // Start a read transfer, get the data and stop transfer
}





//**********************************************************************************************************************************************************
// Write data to the EERAM47x16 (DO NOT USE DIRECTLY, use EERAM47x16_WriteSRAMData() or EERAM47x16_WriteRegister() instead)
static eERRORRESULT __EERAM47x16_WriteData(EERAM47x16 *pComp, const uint8_t chipAddr, uint16_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > EERAM47x16_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  uint8_t* pData = (uint8_t*)data;
  uint8_t ChipAddr = (chipAddr | pComp->AddrA2A1) & EERAM47x16_CHIPADDRESS_MASK;

  //--- Write data ---
  Error = __EERAM47x16_WriteAddress(pComp, ChipAddr, address);                                 // Start a write at address with the device
  if (Error == ERR_OK)                                                                         // If there is no error while writing address then
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddr, pData, size, false, true); // Continue the transfer by sending the data and stop transfer (chip address will not be used)
  return Error;
}



//=============================================================================
// Write SRAM data to the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_WriteSRAMData(EERAM47x16 *pComp, uint16_t address, const uint8_t* data, size_t size)
{
  return __EERAM47x16_WriteData(pComp, EERAM47x16_SRAM_CHIPADDRESS_BASE, address, data, size);
}



//=============================================================================
// Write Control register to the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_WriteRegister(EERAM47x16 *pComp, uint8_t address, const uint8_t* data)
{
  eERRORRESULT Error = __EERAM47x16_WriteData(pComp, EERAM47x16_REG_CHIPADDRESS_BASE, address, data, 1);
  if (Error == ERR__I2C_NACK_DATA) Error = ERR__I2C_INVALID_COMMAND; // Here a NACK indicate that the command (aka 'data') is invalid. It cannot be anything else
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Store all the SRAM to the EEPROM of the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_StoreSRAMtoEEPROM(EERAM47x16 *pComp, bool forceStore, bool waitEndOfStore)
{
#ifdef CHECK_NULL_PARAM
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error = ERR_OK;
  EERAM47x16_Status_Register Reg;
  bool Store = forceStore;

  //--- Check the need of store operation ---
  if (forceStore == false)                                                            // Check register Status.AM if the store is not forced
  {
    Error = EERAM47x16_ReadRegister(pComp, &Reg.Status);                              // Get the status register
    if (Error != ERR_OK) return Error;                                                // If there is an error while reading register, then return the error
    Store = ((Reg.Status & EERAM47x16_ARRAY_MODIFIED) > 0);                           // If the array has been modified, flag the store
  }
  //--- Send the store operation if necessary or asked ---
  if (Store)
  {
    uint8_t Data = EERAM47x16_STORE_COMMAND;
    Error = EERAM47x16_WriteRegister(pComp, EERAM47x16_COMMAND_REGISTER_ADDR, &Data); // Execute a store
    if (Error != ERR_OK) return Error;                                                // If there is an error while reading register, then return the error
    //--- Wait the end of store if asked ---
    if (waitEndOfStore)
    {
      Reg.Status = EERAM47x16_ARRAY_MODIFIED;
      uint32_t Timeout = pComp->fnGetCurrentms() + EERAM47x16_STORE_TIMEOUT + 1; // Wait at least STORE_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
      while (true)
      {
        Error = EERAM47x16_ReadRegister(pComp, &Reg.Status);                // Get the status register
        if ((Error != ERR_OK) && (Error != ERR__I2C_NACK)) return Error;    // If there is an error while calling EERAM47x16_ReadRegister() then return the error
        if ((Reg.Status & EERAM47x16_ARRAY_MODIFIED) == 0) break;           // The store is finished, all went fine
        if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
      }
    }
  }
  return Error;
}



//=============================================================================
// Recall all data from EEPROM to SRAM of the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_RecallEEPROMtoSRAM(EERAM47x16 *pComp, bool waitEndOfRecall)
{
#ifdef CHECK_NULL_PARAM
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error = ERR_OK;
  EERAM47x16_Status_Register Reg;

  //--- Send a recall operation ---
  uint8_t Data = EERAM47x16_RECALL_COMMAND;
  Error = EERAM47x16_WriteRegister(pComp, EERAM47x16_COMMAND_REGISTER_ADDR, &Data); // Execute a recall
  if (Error != ERR_OK) return Error;                                                // If there is an error while reading register, then return the error
  //--- Wait the end of recall if asked ---
  if (waitEndOfRecall)
  {
    Reg.Status = EERAM47x16_ARRAY_MODIFIED;
    uint32_t Timeout = pComp->fnGetCurrentms() + EERAM47x16_RECALL_TIMEOUT + 1; // Wait at least RECALL_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = EERAM47x16_ReadRegister(pComp, &Reg.Status);                // Get the status register
      if ((Error != ERR_OK) && (Error != ERR__I2C_NACK)) return Error;    // If there is an error while calling EERAM47x16_ReadRegister() then return the error
      if ((Reg.Status & EERAM47x16_ARRAY_MODIFIED) == 0) break;           // The recall is finished, all went fine
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
  }
  return ERR_OK;
}



//=============================================================================
// Activate the Auto-Store of the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_ActivateAutoStore(EERAM47x16 *pComp)
{
  eERRORRESULT Error = ERR_OK;
  EERAM47x16_Status_Register Reg;

  //--- Get the status register ---
  Error = EERAM47x16_ReadRegister(pComp, &Reg.Status);                                  // Get the status register
  if (Error != ERR_OK) return Error;                                                    // If there is an error while sending data to I2C then stop the transfer
  //--- Set the status register ---
  Reg.Status |= EERAM47x16_ASE_ENABLE;                                                  // Set the Auto-Store flag
  return EERAM47x16_WriteRegister(pComp, EERAM47x16_STATUS_REGISTER_ADDR, &Reg.Status); // Save the status register
}



//=============================================================================
// Deactivate the Auto-Store of the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_DeactivateAutoStore(EERAM47x16 *pComp)
{
  eERRORRESULT Error = ERR_OK;
  EERAM47x16_Status_Register Reg;

  //--- Get the status register ---
  Error = EERAM47x16_ReadRegister(pComp, &Reg.Status);                                  // Get the status register
  if (Error != ERR_OK) return Error;                                                    // If there is an error while sending data to I2C then stop the transfer
  //--- Set the status register ---
  Reg.Status &= ~EERAM47x16_ASE_ENABLE;                                                 // Unset the Auto-Store flag
  return EERAM47x16_WriteRegister(pComp, EERAM47x16_STATUS_REGISTER_ADDR, &Reg.Status); // Save the status register
}



//=============================================================================
// Set block write protect of the EERAM47x16 device
//=============================================================================
eERRORRESULT EERAM47x16_SetBlockWriteProtect(EERAM47x16 *pComp, eEERAM47x16_BlockProtect blockProtect)
{
  eERRORRESULT Error = ERR_OK;
  EERAM47x16_Status_Register Reg;

  //--- Get the status register ---
  Error = EERAM47x16_ReadRegister(pComp, &Reg.Status);                                  // Get the status register
  if (Error != ERR_OK) return Error;                                                    // If there is an error while sending data to I2C then stop the transfer
  //--- Set the status register ---
  Reg.Bits.BP = (uint8_t)blockProtect;                                                  // Set the block protect
  return EERAM47x16_WriteRegister(pComp, EERAM47x16_STATUS_REGISTER_ADDR, &Reg.Status); // Save the status register
}





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------