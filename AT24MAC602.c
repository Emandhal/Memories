/*******************************************************************************
 * @file    AT24MAC602.c
 * @author  FMA
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   AT24MAC602 driver
 *
 * I2C-Compatible (2-wire) 2-Kbit (256kB x 8) Serial EEPROM
 * with a Factory-Programmed EUI-64™ Address
 * plus an Embedded Unique 128-bit Serial Number
 * Follow datasheet AT24MAC602 Rev.8808E (Jan  2015)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "AT24MAC602.h"
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





//=============================================================================
// Prototypes for private functions
//=============================================================================
// Read data from the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_ReadData() instead)
static eERRORRESULT __AT24MAC602_ReadPage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, uint8_t* data, size_t size);
// Write data to the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_WriteData() instead)
static eERRORRESULT __AT24MAC602_WritePage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, const uint8_t* data, size_t size);





//**********************************************************************************************************************************************************
//=============================================================================
// AT24MAC602 initialization
//=============================================================================
eERRORRESULT Init_AT24MAC602(AT24MAC602 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  if (pComp->I2C_ClockSpeed > AT24MAC602_I2CCLOCK_MAXSUP2V5) return ERR__I2C_FREQUENCY_ERROR;
  Error = pComp->fnI2C_Init(pComp->InterfaceDevice, pComp->I2C_ClockSpeed);
  if (Error != ERR_OK) return Error; // If there is an error while calling fnI2C_Init() then return the Error

  return (AT24MAC602_IsReady(pComp) ? ERR_OK : ERR__NO_DEVICE_DETECTED);
}



//=============================================================================
// Is the AT24MAC602 device ready
//=============================================================================
bool AT24MAC602_IsReady(AT24MAC602 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return false;
  if (pComp->fnI2C_Transfer == NULL) return false;
#endif
  uint8_t ChipAddrW = ((AT24MAC602_EEPROM_CHIPADDRESS_BASE | pComp->AddrA2A1A0) & AT24MAC602_I2C_WRITE);
  return (pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, NULL, 0, true, true) == ERR_OK); // Send only the chip address and get the Ack flag
}





//**********************************************************************************************************************************************************
// Write the byte address the AT24MAC602 (DO NOT USE DIRECTLY)
static eERRORRESULT __AT24MAC602_WriteAddress(AT24MAC602 *pComp, const uint8_t chipAddr, uint8_t address)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  uint8_t ChipAddrW = chipAddr & AT24MAC602_I2C_WRITE;
  //--- Send the address ---
  Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, &address, 1, true, false); // Transfer the address
  if (Error == ERR__I2C_NACK) return ERR__NOT_READY;                                          // If the device receive a NAK, then the device is not ready
  if (Error == ERR__I2C_NACK_DATA) return ERR__I2C_INVALID_ADDRESS;                           // If the device receive a NAK while transferring data, then this is an invalid address
  return Error;
}





//**********************************************************************************************************************************************************
// Read data from the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_ReadData() instead)
eERRORRESULT __AT24MAC602_ReadPage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > AT24MAC602_PAGE_SIZE) return ERR__OUT_OF_RANGE;
  eERRORRESULT Error;
  uint8_t ChipAddrR = chipAddr | AT24MAC602_I2C_READ;

  //--- Read the page ---
  Error = __AT24MAC602_WriteAddress(pComp, chipAddr, address);                                // Start a write at address with the device
  if (Error == ERR_OK)                                                                        // If there is no error while writing address then
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrR, data, size, true, true); // Restart a read transfer, get the data and stop transfer
  return Error;
}


//=============================================================================
// Read EEPROM data from the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_ReadEEPROMData(AT24MAC602 *pComp, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + size) > AT24MAC602_EEPROM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  uint8_t PageRemData;
  uint8_t ChipAddr = AT24MAC602_EEPROM_CHIPADDRESS_BASE | pComp->AddrA2A1A0;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    PageRemData = AT24MAC602_PAGE_SIZE - (address & AT24MAC602_PAGE_SIZE_MASK);   // Get how many bytes are
    PageRemData = (size < PageRemData ? size : PageRemData);                      // Get the least remaining bytes to read between remain size and remain in page

    //--- Read with timeout ---
    uint32_t Timeout = pComp->fnGetCurrentms() + 6;                               // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = __AT24MAC602_ReadPage(pComp, ChipAddr, address, data, PageRemData); // Read data from a page
      if (Error == ERR_OK) break;                                                 // All went fine, continue the data sending
      if (Error != ERR__NOT_READY) return Error;                                  // If there is an error while calling __AT24MAC602_ReadPage() then return the error
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT;         // Timout ? return the error
    }
    address += PageRemData;
    data += PageRemData;
    size -= PageRemData;
  }
  return ERR_OK;
}





//**********************************************************************************************************************************************************
// Write data to the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_ReadData() instead)
eERRORRESULT __AT24MAC602_WritePage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > AT24MAC602_PAGE_SIZE) return ERR__OUT_OF_RANGE;
  eERRORRESULT Error;
  uint8_t ChipAddrW = (chipAddr & AT24MAC602_I2C_WRITE);
  uint8_t* pData = (uint8_t*)data;

  //--- Write the page ---
  Error = __AT24MAC602_WriteAddress(pComp, ChipAddrW, address);                         // Start a write at address with the device
  if (Error == ERR_OK)                                                                  // If there is no error while writing address then
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, 0, pData, size, false, true); // Continue the transfer by sending the data and stop transfer (chip address will not be used)
  return Error;
}


//=============================================================================
// Write EEPROM data to the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_WriteEEPROMData(AT24MAC602 *pComp, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + size) > AT24MAC602_EEPROM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  uint8_t PageRemData;
  uint8_t ChipAddr = AT24MAC602_EEPROM_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  uint32_t Timeout;
  uint8_t* pData = (uint8_t*)data;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    PageRemData = AT24MAC602_PAGE_SIZE - (address & AT24MAC602_PAGE_SIZE_MASK);     // Get how many bytes are
    PageRemData = (size < PageRemData ? size : PageRemData);                        // Get the least remaining bytes to write between remain size and remain in page

    //--- Write with timeout ---
    Timeout = pComp->fnGetCurrentms() + 6;                                          // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = __AT24MAC602_WritePage(pComp, ChipAddr, address, pData, PageRemData); // Write data to a page
      if (Error == ERR_OK) break;                                                   // All went fine, continue the data sending
      if (Error != ERR__NOT_READY) return Error;                                    // If there is an error while calling __AT24MAC602_WritePage() then return the error
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT;           // Timout ? return the error
    }
    address += PageRemData;
    pData += PageRemData;
    size  -= PageRemData;
  }
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read the EUI-64 register of the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_GetEUI64(AT24MAC602 *pComp, AT24MAC602_MAC_EUI64 *pEUI64)
{
  uint8_t ChipAddr = AT24MAC602_EUI_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pEUI64 == NULL)) return ERR__PARAMETER_ERROR;
#endif
  return __AT24MAC602_ReadPage(pComp, ChipAddr, AT24MAC602_EUI64_MEMORYADDR, &pEUI64->EUI64[0], EUI64_LEN);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read the Serial Number register of the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_Get128bitsSerialNumber(AT24MAC602 *pComp, uint8_t *dataSerialNum)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (dataSerialNum == NULL)) return ERR__PARAMETER_ERROR;
#endif
  uint8_t ChipAddr = AT24MAC602_SERIAL_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  return __AT24MAC602_ReadPage(pComp, ChipAddr, AT24MAC602_SERIAL_MEMORYADDR, dataSerialNum, AT24MAC602_SERIALNUMBER_LEN);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Set the Permanent Software Write Protection (PSWP) of the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_SetPermanentWriteProtection(AT24MAC602 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t ChipAddr = AT24MAC602_PSWP_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  return __AT24MAC602_WritePage(pComp, ChipAddr, 0x00, 0x00, 1);
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