/*!*****************************************************************************
 * @file    EEPROM.c
 * @author  FMA
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   Generic EEPROM driver
 * @details Generic I2C-Compatible (2-wire) Serial EEPROM
 * It can work with every memory with an address 1010xxx_ compatibility
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "EEPROM.h"
//-----------------------------------------------------------------------------
#ifdef USE_ERROR_CONTEXT
#  define UNIT_ERR_CONTEXT  ERRCONTEXT__EEPROM // Error context of this unit
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
#  define GET_I2C_INTERFACE  pComp->I2C
#else
#  define GET_I2C_INTERFACE  &pComp->I2C
#endif

//-----------------------------------------------------------------------------





//=== AT24CXX(A) devices ======================================================
// AT24C01A EEPROM configurations
const EEPROM_Conf AT24C01A_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .OffsetAddress = 0, .TotalByteSize =  16/*Pages*/ * 8, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C01A_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .OffsetAddress = 0, .TotalByteSize =  16/*Pages*/ * 8, .MaxI2CclockSpeed = 400000, };

// AT24C02 EEPROM configurations
const EEPROM_Conf AT24C02_1V8_Conf  = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .OffsetAddress = 0, .TotalByteSize =  32/*Pages*/ * 8, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C02_Conf      = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .OffsetAddress = 0, .TotalByteSize =  32/*Pages*/ * 8, .MaxI2CclockSpeed = 400000, };

// AT24C04 EEPROM configurations
const EEPROM_Conf AT24C04_1V8_Conf  = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1     , .AddressType = EEPROM_ADDRESS_1Byte_plus_A0    , .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize =  32/*Pages*/ *16, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C04_Conf      = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1     , .AddressType = EEPROM_ADDRESS_1Byte_plus_A0    , .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize =  32/*Pages*/ *16, .MaxI2CclockSpeed = 400000, };

// AT24C08A EEPROM configurations
const EEPROM_Conf AT24C08A_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2       , .AddressType = EEPROM_ADDRESS_1Byte_plus_A1A0  , .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize =  64/*Pages*/ *16, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C08A_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2       , .AddressType = EEPROM_ADDRESS_1Byte_plus_A1A0  , .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize =  64/*Pages*/ *16, .MaxI2CclockSpeed = 400000, };

// AT24C16A EEPROM configurations
const EEPROM_Conf AT24C16A_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_NO_CHIP_ADDRESS_SELECT, .AddressType = EEPROM_ADDRESS_1Byte_plus_A2A1A0, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 128/*Pages*/ *16, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C16A_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_NO_CHIP_ADDRESS_SELECT, .AddressType = EEPROM_ADDRESS_1Byte_plus_A2A1A0, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 128/*Pages*/ *16, .MaxI2CclockSpeed = 400000, };


//=== 24XX256 devices =========================================================
// 24AA256 EEPROM configurations
const EEPROM_Conf _24AA256_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .OffsetAddress = 0, .TotalByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf _24AA256_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .OffsetAddress = 0, .TotalByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 400000, };

// 24LC256 EEPROM configurations
const EEPROM_Conf _24LC256_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .OffsetAddress = 0, .TotalByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 400000, };

// 24FC256 EEPROM configurations
const EEPROM_Conf _24FC256_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .OffsetAddress = 0, .TotalByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf _24FC256_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .OffsetAddress = 0, .TotalByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 1000000, };


#if !defined(USE_EEPROM_GENERICNESS)

//=== AT24CM02 devices =========================================================
const EEPROM_Conf AT24CM02_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2, .AddressType = EEPROM_ADDRESS_2Byte_plus_A1A0, .PageWriteTime = 10, .PageSize = 256, .OffsetAddress = 0, .TotalByteSize = 1024/*Pages*/ *256, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24CM02_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2, .AddressType = EEPROM_ADDRESS_2Byte_plus_A1A0, .PageWriteTime = 10, .PageSize = 256, .OffsetAddress = 0, .TotalByteSize = 1024/*Pages*/ *256, .MaxI2CclockSpeed = 1000000, };


//=== AT24MACX02 devices =========================================================
// AT24MAC402 EEPROM configurations
const EEPROM_Conf AT24MAC402_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24MAC402_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed = 1000000, };

// AT24MAC602 EEPROM configurations
const EEPROM_Conf AT24MAC602_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24MAC602_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .OffsetAddress = 0, .TotalByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed = 1000000, };


//=== 47(L/C)04 devices =========================================================
const EEPROM_Conf EERAM47L04_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 8, .PageSize = 512, .OffsetAddress = 0, .TotalByteSize = 512, .MaxI2CclockSpeed = 1000000, };
const EEPROM_Conf EERAM47C04_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 8, .PageSize = 512, .OffsetAddress = 0, .TotalByteSize = 512, .MaxI2CclockSpeed = 1000000, };


//=== 47(L/C)16 devices =========================================================
const EEPROM_Conf EERAM47L16_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 25, .PageSize = 2048, .OffsetAddress = 0, .TotalByteSize = 2048, .MaxI2CclockSpeed = 1000000, };
const EEPROM_Conf EERAM47C16_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 25, .PageSize = 2048, .OffsetAddress = 0, .TotalByteSize = 2048, .MaxI2CclockSpeed = 1000000, };

#endif // !defined(USE_EEPROM_GENERICNESS)

//**********************************************************************************************************************************************************





//=============================================================================
// Prototypes for private functions
//=============================================================================
// Write EEPROM address to device (DO NOT USE DIRECTLY)
static eERRORRESULT __EEPROM_WriteAddress(EEPROM *pComp, uint32_t address, const eI2C_TransferType transferType);
// Read data from the EEPROM (DO NOT USE DIRECTLY, use EEPROM_ReadData() instead)
static eERRORRESULT __EEPROM_ReadPage(EEPROM *pComp, uint32_t address, uint8_t* data, size_t size);
// Write data to the EEPROM (DO NOT USE DIRECTLY, use EEPROM_WriteData() instead)
static eERRORRESULT __EEPROM_WritePage(EEPROM *pComp, uint32_t address, const uint8_t* data, size_t size);
//-----------------------------------------------------------------------------
#define EEPROM_TIME_DIFF(begin,end)  ( ((end) >= (begin)) ? ((end) - (begin)) : (UINT32_MAX - ((begin) - (end) - 1)) ) // Works only if time difference is strictly inferior to (UINT32_MAX/2) and call often
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// EEPROM initialization
//=============================================================================
eERRORRESULT Init_EEPROM(EEPROM *pComp)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Init == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  eERRORRESULT Error;

  if (pComp->I2CclockSpeed > pComp->Conf->MaxI2CclockSpeed) return ERR_GENERATE(ERR__I2C_FREQUENCY_ERROR);
  Error = pI2C->fnI2C_Init(pI2C, pComp->I2CclockSpeed);
  if (Error != ERR_NONE) return Error; // If there is an error while calling fnInterfaceInit() then return the error

  return (EEPROM_IsReady(pComp) ? ERR_NONE : ERR_GENERATE(ERR__NO_DEVICE_DETECTED));
}


//=============================================================================
// Is the EEPROM device ready
//=============================================================================
bool EEPROM_IsReady(EEPROM *pComp)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return false;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return false;
# endif
  if (pI2C->fnI2C_Transfer == NULL) return false;
#endif
  I2CInterface_Packet PacketDesc = I2C_INTERFACE8_NO_DATA_DESC((pComp->Conf->ChipAddress | pComp->AddrA2A1A0) & I2C_WRITE_ANDMASK);
  return (pI2C->fnI2C_Transfer(pI2C, &PacketDesc) == ERR_NONE); // Send only the chip address and get the Ack flag
}

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Write EEPROM address to device (DO NOT USE DIRECTLY)
//=============================================================================
eERRORRESULT __EEPROM_WriteAddress(EEPROM *pComp, uint32_t address, const eI2C_TransferType transferType)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  eERRORRESULT Error;
  const EEPROM_Conf* const pConf = pComp->Conf;
  const uint8_t AddrBytes  =  (pConf->AddressType & (uint8_t)EEPROM_ADDRESS_Bytes_MASK);
  const uint8_t AddrTypeAx = ((pConf->AddressType & (uint8_t)EEPROM_ADDRESS_plus_Ax_MASK) >> 4);
  address += pConf->OffsetAddress;

  //--- Create address ---
  uint8_t Address[EEPROM_ADDRESS_4Bytes];
  for (int_fast8_t z = AddrBytes; --z >=0;) Address[z] = (uint8_t)((address >> ((AddrBytes - z - 1) * 8)) & 0xFF);
  //--- Send the address ---
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_BLOCKING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(transferType),
    I2C_MEMBER(ChipAddr    ) (pConf->ChipAddress | (pComp->AddrA2A1A0 & ~AddrTypeAx) | ((address >> (8 * AddrBytes - 1)) & AddrTypeAx)) & I2C_WRITE_ANDMASK, // Generate chip address
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) &Address[0],
    I2C_MEMBER(BufferSize  ) AddrBytes,
    I2C_MEMBER(Stop        ) false,
  };
  Error = pI2C->fnI2C_Transfer(pI2C, &PacketDesc);                                               // Transfer the address
  if (ERR_ERROR_Get(Error) == ERR__I2C_NACK) return ERR_GENERATE(ERR__NOT_READY);                // If the device receive a NAK, then the device is not ready
  if (ERR_ERROR_Get(Error) == ERR__I2C_NACK_DATA) return ERR_GENERATE(ERR__I2C_INVALID_ADDRESS); // If the device receive a NAK while transferring data, then this is an invalid address
  return Error;
}


//=============================================================================
// [STATIC] Read data from the EEPROM (DO NOT USE DIRECTLY, use EEPROM_ReadData() instead)
//=============================================================================
eERRORRESULT __EEPROM_ReadPage(EEPROM *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  if (size > pComp->Conf->PageSize) return ERR_GENERATE(ERR__OUT_OF_RANGE);
  const uint8_t ChipAddrR = ((pComp->Conf->ChipAddress | pComp->AddrA2A1A0) | I2C_READ_ORMASK);
  eERRORRESULT Error;

  //--- Read the page ---
  Error = __EEPROM_WriteAddress(pComp, address, I2C_WRITE_THEN_READ_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_NONE)                                                         // If there is no error while writing address then
  {
    I2CInterface_Packet DataPacketDesc = I2C_INTERFACE8_RX_DATA_DESC(ChipAddrR, true, data, size, true, I2C_WRITE_THEN_READ_SECOND_PART);
    Error = pI2C->fnI2C_Transfer(pI2C, &DataPacketDesc);                         // Restart a read transfer, get the data and stop transfer
  }
  return Error;
}


//=============================================================================
// Read EEPROM data from the EEPROM device
//=============================================================================
eERRORRESULT EEPROM_ReadData(EEPROM *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
  if (pComp->Conf == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
  if (pComp->fnGetCurrentms == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  const EEPROM_Conf* const pConf = pComp->Conf;
  if ((address + size) > pConf->TotalByteSize) return ERR_GENERATE(ERR__OUT_OF_MEMORY);
  eERRORRESULT Error;
  size_t PageRemData;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    PageRemData = pConf->PageSize - (address & (pConf->PageSize - 1));                        // Get how many bytes remain in the current page
    PageRemData = (size < PageRemData ? size : PageRemData);                                  // Get the least remaining bytes to read between remain size and remain in page

    //--- Read with timeout ---
    uint32_t StartTime = pComp->fnGetCurrentms();                                             // Start the timeout
    while (true)
    {
      Error = __EEPROM_ReadPage(pComp, address, data, PageRemData);                           // Read data from a page
      if (Error == ERR_NONE) break;                                                           // All went fine, continue the data sending
      if (ERR_ERROR_Get(Error) != ERR__NOT_READY) return Error;                               // If there is an error while calling __EEPROM_WritePage() then return the error
      if (EEPROM_TIME_DIFF(StartTime, pComp->fnGetCurrentms()) > (pConf->PageWriteTime + 1u)) // Wait at least PageWriteTime + 1ms because GetCurrentms can be 1 cycle before the new ms
      {
        return ERR_GENERATE(ERR__DEVICE_TIMEOUT);                                             // Timeout? return the error
      }
    }
    address += PageRemData;
    data += PageRemData;
    size -= PageRemData;
  }
  return ERR_NONE;
}

//-----------------------------------------------------------------------------



//=============================================================================
// [STATIC] Write data to the EEPROM (DO NOT USE DIRECTLY, use EEPROM_WriteData() instead)
//=============================================================================
eERRORRESULT __EEPROM_WritePage(EEPROM *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
# endif
  if (pI2C->fnI2C_Transfer == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  if (size > pComp->Conf->PageSize) return ERR_GENERATE(ERR__OUT_OF_RANGE);
  const uint8_t ChipAddrW = ((pComp->Conf->ChipAddress | pComp->AddrA2A1A0) & I2C_WRITE_ANDMASK);
  eERRORRESULT Error;

  //--- Write the page ---
  Error = __EEPROM_WriteAddress(pComp, address, I2C_WRITE_THEN_WRITE_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_NONE)                                                          // If there is no error while writing address then
  {
    I2CInterface_Packet DataPacketDesc = I2C_INTERFACE8_TX_DATA_DESC(ChipAddrW, false, data, size, true, I2C_WRITE_THEN_WRITE_SECOND_PART);
    Error = pI2C->fnI2C_Transfer(pI2C, &DataPacketDesc);                          // Continue the transfer by sending the data and stop transfer
  }
  return Error;
}


//=============================================================================
// Write EEPROM data to the EEPROM device
//=============================================================================
eERRORRESULT EEPROM_WriteData(EEPROM *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR_GENERATE(ERR__PARAMETER_ERROR);
  if (pComp->Conf == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
  if (pComp->fnGetCurrentms == NULL) return ERR_GENERATE(ERR__PARAMETER_ERROR);
#endif
  const EEPROM_Conf* const pConf = pComp->Conf;
  if ((address + size) > pConf->TotalByteSize) return ERR_GENERATE(ERR__OUT_OF_MEMORY);
  eERRORRESULT Error;
  size_t PageRemData;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    PageRemData = pConf->PageSize - (address & (pConf->PageSize - 1));                        // Get how many bytes remain in the current page
    PageRemData = (size < PageRemData ? size : PageRemData);                                  // Get the least remaining bytes to write between remain size and remain in page

    //--- Write with timeout ---
    uint32_t StartTime = pComp->fnGetCurrentms();                                             // Start the timeout
    while (true)
    {
      Error = __EEPROM_WritePage(pComp, address, data, PageRemData);                          // Write data to a page
      if (Error == ERR_NONE) break;                                                           // All went fine, continue the data sending
      if (ERR_ERROR_Get(Error) != ERR__NOT_READY) return Error;                               // If there is an error while calling __EEPROM_WritePage() then return the error
      if (EEPROM_TIME_DIFF(StartTime, pComp->fnGetCurrentms()) > (pConf->PageWriteTime + 1u)) // Wait at least PageWriteTime + 1ms because GetCurrentms can be 1 cycle before the new ms
      {
        return ERR_GENERATE(ERR__DEVICE_TIMEOUT);                                             // Timeout? return the error
      }
    }
    address += PageRemData;
    data += PageRemData;
    size -= PageRemData;
  }
  return ERR_NONE;
}


//==============================================================================
// Wait the end of write to the EEPROM device
//==============================================================================
eERRORRESULT EEPROM_WaitEndOfWrite(EEPROM *pComp)
{
  //--- Write with timeout ---
  const EEPROM_Conf* const pConf = pComp->Conf;
  uint32_t StartTime = pComp->fnGetCurrentms();                                             // Start the timeout
  while (true)
  {
    if (EEPROM_IsReady(pComp)) break;                                                       // Wait the end of write, and exit if all went fine
    if (EEPROM_TIME_DIFF(StartTime, pComp->fnGetCurrentms()) > (pConf->PageWriteTime + 1u)) // Wait at least PageWriteTime + 1ms because GetCurrentms can be 1 cycle before the new ms
    {
      return ERR_GENERATE(ERR__DEVICE_TIMEOUT);                                             // Timeout? return the error
    }
  }
  return ERR_NONE;
}

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------