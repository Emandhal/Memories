/*******************************************************************************
 * @file    AT24MAC402.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.1.0
 * @date    24/08/2020
 * @brief   AT24MAC402 driver
 *
 * I2C-Compatible (2-wire) 2-Kbit (256kB x 8) Serial EEPROM
 * with a Factory-Programmed EUI-48™ Address
 * plus an Embedded Unique 128-bit Serial Number
 * Follow datasheet AT24MAC402 Rev.8808E (Jan 2015)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "AT24MAC402.h"
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





//=============================================================================
// Prototypes for private functions
//=============================================================================
// Read data from the AT24MAC402 (DO NOT USE DIRECTLY, use AT24MAC402_ReadData() instead)
static eERRORRESULT __AT24MAC402_ReadPage(AT24MAC402 *pComp, uint8_t chipAddr, uint8_t address, uint8_t* data, size_t size);
// Write data to the AT24MAC402 (DO NOT USE DIRECTLY, use AT24MAC402_WriteData() instead)
static eERRORRESULT __AT24MAC402_WritePage(AT24MAC402 *pComp, uint8_t chipAddr, uint8_t address, const uint8_t* data, size_t size);





//**********************************************************************************************************************************************************
//=============================================================================
// AT24MAC402 initialization
//=============================================================================
eERRORRESULT Init_AT24MAC402(AT24MAC402 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  if (pComp->I2CclockSpeed > AT24MAC402_I2CCLOCK_MAXSUP2V5) return ERR__I2C_FREQUENCY_ERROR;
  Error = pI2C->fnI2C_Init(pI2C, pComp->I2CclockSpeed);
  if (Error != ERR_OK) return Error; // If there is an error while calling fnI2C_Init() then return the Error

  return (AT24MAC402_IsReady(pComp) ? ERR_OK : ERR__NO_DEVICE_DETECTED);
}



//=============================================================================
// Is the AT24MAC402 device ready
//=============================================================================
bool AT24MAC402_IsReady(AT24MAC402 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return false;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return false;
#endif
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER),
    I2C_MEMBER(ChipAddr    ) (AT24MAC402_EEPROM_CHIPADDRESS_BASE | pComp->AddrA2A1A0) & I2C_WRITE_ANDMASK,
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) NULL,
    I2C_MEMBER(BufferSize  ) 0,
    I2C_MEMBER(Stop        ) true,
  };
  return (pI2C->fnI2C_Transfer(pI2C, &PacketDesc) == ERR_OK); // Send only the chip address and get the Ack flag
}





//**********************************************************************************************************************************************************
// Write the byte address the AT24MAC402 (DO NOT USE DIRECTLY)
static eERRORRESULT __AT24MAC402_WriteAddress(AT24MAC402 *pComp, const uint8_t chipAddr, uint8_t address, const eI2C_TransferType transferType)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  //--- Send the address ---
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(transferType),
    I2C_MEMBER(ChipAddr    ) chipAddr & I2C_WRITE_ANDMASK,
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) &address,
    I2C_MEMBER(BufferSize  ) 1,
    I2C_MEMBER(Stop        ) false,
  };
  Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc);                  // Transfer the address
  if (Error == ERR__I2C_NACK) return ERR__NOT_READY;                // If the device receive a NAK, then the device is not ready
  if (Error == ERR__I2C_NACK_DATA) return ERR__I2C_INVALID_ADDRESS; // If the device receive a NAK while transferring data, then this is an invalid address
  return Error;
}





//**********************************************************************************************************************************************************
// Read data from the AT24MAC402 (DO NOT USE DIRECTLY, use AT24MAC402_ReadData() instead)
eERRORRESULT __AT24MAC402_ReadPage(AT24MAC402 *pComp, uint8_t chipAddr, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > AT24MAC402_PAGE_SIZE) return ERR__OUT_OF_RANGE;
  eERRORRESULT Error;

  //--- Read the page ---
  Error = __AT24MAC402_WriteAddress(pComp, chipAddr, address, I2C_WRITE_THEN_READ_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_OK)                                                                         // If there is no error while writing address then
  {
    I2CInterface_Packet PacketDesc =
    {
      I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_READ_SECOND_PART),
      I2C_MEMBER(ChipAddr    ) (chipAddr | I2C_READ_ORMASK),
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
// Read EEPROM data from the AT24MAC402 device
//=============================================================================
eERRORRESULT AT24MAC402_ReadEEPROMData(AT24MAC402 *pComp, uint8_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + size) > AT24MAC402_EEPROM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  uint8_t PageRemData;
  const uint8_t ChipAddr = AT24MAC402_EEPROM_CHIPADDRESS_BASE | pComp->AddrA2A1A0;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    PageRemData = AT24MAC402_PAGE_SIZE - (address & AT24MAC402_PAGE_SIZE_MASK);   // Get how many bytes are
    PageRemData = (size < PageRemData ? size : PageRemData);                      // Get the least remaining bytes to read between remain size and remain in page

    //--- Read with timeout ---
    uint32_t Timeout = pComp->fnGetCurrentms() + 6;                               // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = __AT24MAC402_ReadPage(pComp, ChipAddr, address, data, PageRemData); // Read data from a page
      if (Error == ERR_OK) break;                                                 // All went fine, continue the data sending
      if (Error != ERR__NOT_READY) return Error;                                  // If there is an error while calling __AT24MAC402_ReadPage() then return the error
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT;         // Timout ? return the error
    }
    address += PageRemData;
    data += PageRemData;
    size -= PageRemData;
  }
  return ERR_OK;
}





//**********************************************************************************************************************************************************
// Write data to the AT24MAC402 (DO NOT USE DIRECTLY, use AT24MAC402_ReadData() instead)
eERRORRESULT __AT24MAC402_WritePage(AT24MAC402 *pComp, uint8_t chipAddr, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > AT24MAC402_PAGE_SIZE) return ERR__OUT_OF_RANGE;
  eERRORRESULT Error;
  uint8_t* pData = (uint8_t*)data;
  const uint8_t ChipAddr = (chipAddr & I2C_WRITE_ANDMASK);

  //--- Write the page ---
  Error = __AT24MAC402_WriteAddress(pComp, ChipAddr, address, I2C_WRITE_THEN_WRITE_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_OK)                                                                          // If there is no error while writing address then
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
// Write EEPROM data to the AT24MAC402 device
//=============================================================================
eERRORRESULT AT24MAC402_WriteEEPROMData(AT24MAC402 *pComp, uint8_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + size) > AT24MAC402_EEPROM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  uint8_t PageRemData;
  const uint8_t ChipAddr = AT24MAC402_EEPROM_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  uint32_t Timeout;
  uint8_t* pData = (uint8_t*)data;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    PageRemData = AT24MAC402_PAGE_SIZE - (address & AT24MAC402_PAGE_SIZE_MASK);     // Get how many bytes are
    PageRemData = (size < PageRemData ? size : PageRemData);                        // Get the least remaining bytes to write between remain size and remain in page

    //--- Write with timeout ---
    Timeout = pComp->fnGetCurrentms() + 6;                                          // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = __AT24MAC402_WritePage(pComp, ChipAddr, address, pData, PageRemData); // Write data to a page
      if (Error == ERR_OK) break;                                                   // All went fine, continue the data sending
      if (Error != ERR__NOT_READY) return Error;                                    // If there is an error while calling __AT24MAC402_WritePage() then return the error
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT;           // Timout ? return the error
    }
    address += PageRemData;
    pData += PageRemData;
    size  -= PageRemData;
  }
  return ERR_OK;
}


//==============================================================================
// Wait the end of write to the AT24MAC402 device
//==============================================================================
eERRORRESULT AT24MAC402_WaitEndOfWrite(AT24MAC402 *pComp)
{
  //--- Write with timeout ---
  uint32_t Timeout = pComp->fnGetCurrentms() + 6;                       // Wait at least 5ms (see tWR in Table 6-3 from datasheet AC Characteristics) + 1ms because GetCurrentms can be 1 cycle before the new ms
  while (true)
  {
    if (AT24MAC402_IsReady(pComp)) break;                               // Wait the end of write, and exit if all went fine
    if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout? return the error
  }
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read the EUI-48 register of the AT24MAC402 device
//=============================================================================
eERRORRESULT AT24MAC402_GetEUI48(AT24MAC402 *pComp, AT24MAC402_MAC_EUI48 *pEUI48)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pEUI48 == NULL)) return ERR__PARAMETER_ERROR;
#endif
  const uint8_t ChipAddr = AT24MAC402_EUI_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  return __AT24MAC402_ReadPage(pComp, ChipAddr, AT24MAC402_EUI48_MEMORYADDR, &pEUI48->EUI48[0], EUI48_LEN);
}


//=============================================================================
// Generate a EUI-64 value from the EUI-48 of the AT24MAC402 device
//=============================================================================
eERRORRESULT AT24MAC402_GenerateEUI64(AT24MAC402 *pComp, AT24MAC402_MAC_EUI64 *pEUI64)
{
#ifdef CHECK_NULL_PARAM
  if (pEUI64 == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  AT24MAC402_MAC_EUI48 EUI48;

  Error = AT24MAC402_GetEUI48(pComp, &EUI48); // Get the EUI-48
  if (Error != ERR_OK) return Error;          // If there is an error while calling AT24MAC402_GetEUI48() then return the error

  //--- Generate the EUI-64 from the EUI-48 ---
  // See https://fr.wikipedia.org/wiki/Adresse_MAC and datasheet p9 §7.1
  pEUI64->OUI[0] = EUI48.OUI[0] | 0x02;       // Set the Locally Administered bit
  pEUI64->OUI[1] = EUI48.OUI[1];
  pEUI64->OUI[2] = EUI48.OUI[2];
  pEUI64->NIC[0] = 0xFF;
  pEUI64->NIC[1] = 0xFE;
  pEUI64->NIC[2] = EUI48.NIC[0];
  pEUI64->NIC[3] = EUI48.NIC[1];
  pEUI64->NIC[4] = EUI48.NIC[2];
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read the Serial Number register of the AT24MAC402 device
//=============================================================================
eERRORRESULT AT24MAC402_Get128bitsSerialNumber(AT24MAC402 *pComp, uint8_t *dataSerialNum)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (dataSerialNum == NULL)) return ERR__PARAMETER_ERROR;
#endif
  const uint8_t ChipAddr = AT24MAC402_SERIAL_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  return __AT24MAC402_ReadPage(pComp, ChipAddr, AT24MAC402_SERIAL_MEMORYADDR, dataSerialNum, AT24MAC402_SERIALNUMBER_LEN);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Set the Permanent Software Write Protection (PSWP) of the AT24MAC402 device
//=============================================================================
eERRORRESULT AT24MAC402_SetPermanentWriteProtection(AT24MAC402 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  const uint8_t ChipAddr = AT24MAC402_PSWP_CHIPADDRESS_BASE | pComp->AddrA2A1A0;
  return __AT24MAC402_WritePage(pComp, ChipAddr, 0x00, 0x00, 1);
}







//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------