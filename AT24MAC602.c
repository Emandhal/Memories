/*!*****************************************************************************
 * @file    AT24MAC602.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.2.0
 * @date    04/06/2023
 * @brief   AT24MAC602 driver
 * @details I2C-Compatible (2-wire) 2-Kbit (256kB x 8) Serial EEPROM with a
 * Factory-Programmed EUI-64™ Address plus an Embedded Unique 128-bit Serial Number
 * Follow datasheet AT24MAC602 Rev.8808E (Jan 2015)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "AT24MAC602.h"
//-----------------------------------------------------------------------------
#ifdef USE_ERROR_CONTEXT
#  define UNIT_ERR_CONTEXT  ERRCONTEXT__AT24MAC602 // Error context of this unit
#else
#  define ERR_GENERATE(error)   error
#  define ERR_ERROR_Get(error)  error
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
//-----------------------------------------------------------------------------

#ifdef USE_DYNAMIC_INTERFACE
#  define GET_I2C_INTERFACE  pComp->Eeprom.I2C
#else
#  define GET_I2C_INTERFACE  &pComp->Eeprom.I2C
#endif

//-----------------------------------------------------------------------------

#ifdef USE_EEPROM_GENERICNESS
// AT24MAC602 EEPROM configurations
const EEPROM_Conf AT24MAC602_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24MAC602_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed = 1000000, };
#endif

//-----------------------------------------------------------------------------





//=============================================================================
// Prototypes for private functions
//=============================================================================
// Read data from the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_ReadData() instead)
static eERRORRESULT __AT24MAC602_ReadPage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, uint8_t* data, size_t size);
// Write data to the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_WriteData() instead)
static eERRORRESULT __AT24MAC602_WritePage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, const uint8_t* data, size_t size);
//-----------------------------------------------------------------------------
#define AT24MAC602_TIME_DIFF(begin,end)  ( ((end) >= (begin)) ? ((end) - (begin)) : (UINT32_MAX - ((begin) - (end) - 1)) ) // Works only if time difference is strictly inferior to (UINT32_MAX/2) and call often
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// AT24MAC602 initialization
//=============================================================================
eERRORRESULT Init_AT24MAC602(AT24MAC602 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Init == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  eERRORRESULT Error;

  if (pComp->Eeprom.I2CclockSpeed > AT24MAC602_I2CCLOCK_MAXSUP2V5) return ERR_GENERATE(ERR__I2C_FREQUENCY_ERROR);
  Error = pI2C->fnI2C_Init(pI2C, pComp->Eeprom.I2CclockSpeed);
  if (Error != ERR_NONE) return Error; // If there is an error while calling fnI2C_Init() then return the Error

  return (AT24MAC602_IsReady(pComp) ? ERR_NONE : ERR_GENERATE(ERR__NO_DEVICE_DETECTED));
}



//=============================================================================
// Is the AT24MAC602 device ready
//=============================================================================
bool AT24MAC602_IsReady(AT24MAC602 *pComp)
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
  I2CInterface_Packet PacketDesc = I2C_INTERFACE8_NO_DATA_DESC((AT24MAC602_EEPROM_CHIPADDRESS_BASE | pComp->Eeprom.AddrA2A1A0) & I2C_WRITE_ANDMASK);
  return (pI2C->fnI2C_Transfer(pI2C, &PacketDesc) == ERR_NONE); // Send only the chip address and get the Ack flag
}

//-----------------------------------------------------------------------------



//=============================================================================
// [STATIC] Read data from the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_ReadData() instead)
//=============================================================================
eERRORRESULT __AT24MAC602_ReadPage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  if (size > AT24MAC602_PAGE_SIZE) return ERR_GENERATE(ERR__OUT_OF_RANGE);
  eERRORRESULT Error;

  //--- Send the address ---
  I2CInterface_Packet RegPacketDesc = I2C_INTERFACE8_TX_DATA_DESC(chipAddr, true, &address, sizeof(uint8_t), false, I2C_WRITE_THEN_READ_FIRST_PART);
  Error = pI2C->fnI2C_Transfer(pI2C, &RegPacketDesc);                                            // Transfer the register's address
  if (ERR_ERROR_Get(Error) == ERR__I2C_NACK) return ERR_GENERATE(ERR__NOT_READY);                // If the device receive a NAK, then the device is not ready
  if (ERR_ERROR_Get(Error) == ERR__I2C_NACK_DATA) return ERR_GENERATE(ERR__I2C_INVALID_ADDRESS); // If the device receive a NAK while transferring data, then this is an invalid address
  if (Error != ERR_NONE) return Error;                                                           // If there is an error while calling fnI2C_Transfer() then return the Error
  //--- Get the data ---
  I2CInterface_Packet DataPacketDesc = I2C_INTERFACE8_RX_DATA_DESC(chipAddr, true, data, size, true, I2C_WRITE_THEN_READ_SECOND_PART);
  return pI2C->fnI2C_Transfer(pI2C, &DataPacketDesc);                                            // Restart at first data read transfer, get the data and stop transfer at last byte
}


#ifdef USE_EEPROM_GENERICNESS
//=============================================================================
// Read EEPROM data from the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_ReadEEPROMData(AT24MAC602 *pComp, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  return EEPROM_ReadData(&pComp->Eeprom, address, data, size);
}


#else // !defined(USE_EEPROM_GENERICNESS)


//=============================================================================
// Read EEPROM data from the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_ReadEEPROMData(AT24MAC602 *pComp, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
  if (pComp->Eeprom.fnGetCurrentms == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  if ((address + size) > AT24MAC602_EEPROM_SIZE) return ERR_GENERATE(ERR__OUT_OF_MEMORY);
  const uint8_t ChipAddr = AT24MAC602_EEPROM_CHIPADDRESS_BASE | pComp->Eeprom.AddrA2A1A0;
  eERRORRESULT Error;
  uint8_t PageRemData;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    PageRemData = AT24MAC602_PAGE_SIZE - (address & AT24MAC602_PAGE_SIZE_MASK);   // Get how many bytes are
    PageRemData = (size < PageRemData ? size : PageRemData);                      // Get the least remaining bytes to read between remain size and remain in page

    //--- Read with timeout ---
    uint32_t StartTime = pComp->Eeprom.fnGetCurrentms();                          // Start the timeout
    while (true)
    {
      Error = __AT24MAC602_ReadPage(pComp, ChipAddr, address, data, PageRemData); // Read data from a page
      if (Error == ERR_NONE) break;                                               // All went fine, continue the data sending
      if (ERR_ERROR_Get(Error) != ERR__NOT_READY) return Error;                   // If there is an error while calling __AT24MAC602_ReadPage() then return the error
      if (AT24MAC602_TIME_DIFF(StartTime, pComp->Eeprom.fnGetCurrentms()) > 6)    // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
      {
        return ERR_GENERATE(ERR__DEVICE_TIMEOUT);                                 // Timeout? return the error
      }
    }
    address += PageRemData;
    data += PageRemData;
    size -= PageRemData;
  }
  return ERR_NONE;
}
#endif // USE_EEPROM_GENERICNESS

//-----------------------------------------------------------------------------



//=============================================================================
// [STATIC] Write data to the AT24MAC602 (DO NOT USE DIRECTLY, use AT24MAC602_ReadData() instead)
//=============================================================================
eERRORRESULT __AT24MAC602_WritePage(AT24MAC602 *pComp, uint8_t chipAddr, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  if (size > AT24MAC602_PAGE_SIZE) return ERR_GENERATE(ERR__OUT_OF_RANGE);
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Send the address ---
  I2CInterface_Packet RegPacketDesc = I2C_INTERFACE8_TX_DATA_DESC(chipAddr, true, &address, sizeof(uint8_t), false, I2C_WRITE_THEN_WRITE_FIRST_PART);
  Error = pI2C->fnI2C_Transfer(pI2C, &RegPacketDesc);                                            // Transfer the register's address
  if (ERR_ERROR_Get(Error) == ERR__I2C_NACK) return ERR_GENERATE(ERR__NOT_READY);                // If the device receive a NAK, then the device is not ready
  if (ERR_ERROR_Get(Error) == ERR__I2C_NACK_DATA) return ERR_GENERATE(ERR__I2C_INVALID_ADDRESS); // If the device receive a NAK while transferring data, then this is an invalid address
  if (Error != ERR_NONE) return Error;                                                           // If there is an error while calling fnI2C_Transfer() then return the Error
  //--- Send the data ---
  I2CInterface_Packet DataPacketDesc = I2C_INTERFACE8_TX_DATA_DESC(chipAddr, false, pData, size, true, I2C_WRITE_THEN_WRITE_SECOND_PART);
  return pI2C->fnI2C_Transfer(pI2C, &DataPacketDesc);                                            // Continue by transferring the data, and stop transfer at last byte
}


#ifdef USE_EEPROM_GENERICNESS
//=============================================================================
// Write EEPROM data to the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_WriteEEPROMData(AT24MAC602 *pComp, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  return EEPROM_WriteData(&pComp->Eeprom, address, data, size);
}


#else // !defined(USE_EEPROM_GENERICNESS)


//=============================================================================
// Write EEPROM data to the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_WriteEEPROMData(AT24MAC602 *pComp, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
  if (pComp->Eeprom.fnGetCurrentms == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  if ((address + size) > AT24MAC602_EEPROM_SIZE) return ERR_GENERATE(ERR__OUT_OF_MEMORY);
  const uint8_t ChipAddr = AT24MAC602_EEPROM_CHIPADDRESS_BASE | pComp->Eeprom.AddrA2A1A0;
  eERRORRESULT Error;
  uint8_t PageRemData;
  uint8_t* pData = (uint8_t*)data;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    PageRemData = AT24MAC602_PAGE_SIZE - (address & AT24MAC602_PAGE_SIZE_MASK);    // Get how many bytes are
    PageRemData = (size < PageRemData ? size : PageRemData);                       // Get the least remaining bytes to write between remain size and remain in page

    //--- Write with timeout ---
    uint32_t StartTime = pComp->Eeprom.fnGetCurrentms();                           // Start the timeout
    while (true)
    {
      Error = __AT24MAC602_WritePage(pComp, ChipAddr, address, data, PageRemData); // Read data from a page
      if (Error == ERR_NONE) break;                                                // All went fine, continue the data sending
      if (ERR_ERROR_Get(Error) != ERR__NOT_READY) return Error;                    // If there is an error while calling __AT24MAC602_WritePage() then return the error
      if (AT24MAC602_TIME_DIFF(StartTime, pComp->Eeprom.fnGetCurrentms()) > 6)     // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
      {
        return ERR_GENERATE(ERR__DEVICE_TIMEOUT);                                  // Timeout? return the error
      }
    }
    address += PageRemData;
    pData += PageRemData;
    size  -= PageRemData;
  }
  return ERR_NONE;
}
#endif // USE_EEPROM_GENERICNESS

//-----------------------------------------------------------------------------



//==============================================================================
// Wait the end of write to the AT24MAC602 device
//==============================================================================
eERRORRESULT AT24MAC602_WaitEndOfWrite(AT24MAC602 *pComp)
{
  //--- Write with timeout ---
  uint32_t Timeout = pComp->Eeprom.fnGetCurrentms() + 6; // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
  while (true)
  {
    if (AT24MAC602_IsReady(pComp)) break;                // Wait the end of write, and exit if all went fine
    if (pComp->Eeprom.fnGetCurrentms() >= Timeout)
    {
      return ERR_GENERATE(ERR__DEVICE_TIMEOUT);          // Timeout? return the error
    }      
  }
  return ERR_NONE;
}

//-----------------------------------------------------------------------------



//=============================================================================
// Read the EUI-64 register of the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_GetEUI64(AT24MAC602 *pComp, AT24MAC602_MAC_EUI64 *pEUI64)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pEUI64 == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  const uint8_t ChipAddr = AT24MAC602_EUI_CHIPADDRESS_BASE | pComp->Eeprom.AddrA2A1A0;
  return __AT24MAC602_ReadPage(pComp, ChipAddr, AT24MAC602_EUI64_MEMORYADDR, &pEUI64->EUI64[0], EUI64_LEN);
}

//-----------------------------------------------------------------------------



//=============================================================================
// Read the Serial Number register of the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_Get128bitsSerialNumber(AT24MAC602 *pComp, uint8_t *dataSerialNum)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (dataSerialNum == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  const uint8_t ChipAddr = AT24MAC602_SERIAL_CHIPADDRESS_BASE | pComp->Eeprom.AddrA2A1A0;
  return __AT24MAC602_ReadPage(pComp, ChipAddr, AT24MAC602_SERIAL_MEMORYADDR, dataSerialNum, AT24MAC602_SERIALNUMBER_LEN);
}

//-----------------------------------------------------------------------------



//=============================================================================
// Set the Permanent Software Write Protection (PSWP) of the AT24MAC602 device
//=============================================================================
eERRORRESULT AT24MAC602_SetPermanentWriteProtection(AT24MAC602 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  const uint8_t ChipAddr = AT24MAC602_PSWP_CHIPADDRESS_BASE | pComp->Eeprom.AddrA2A1A0;
  return __AT24MAC602_WritePage(pComp, ChipAddr, 0x00, 0x00, 1);
}

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------