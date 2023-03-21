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
const EEPROM_Conf AT24C01A_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .ArrayByteSize =  16/*Pages*/ * 8, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C01A_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .ArrayByteSize =  16/*Pages*/ * 8, .MaxI2CclockSpeed = 400000, };

// AT24C02 EEPROM configurations
const EEPROM_Conf AT24C02_1V8_Conf  = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .ArrayByteSize =  32/*Pages*/ * 8, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C02_Conf      = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0   , .AddressType = EEPROM_ADDRESS_1Byte            , .PageWriteTime = 5, .PageSize =  8, .ArrayByteSize =  32/*Pages*/ * 8, .MaxI2CclockSpeed = 400000, };

// AT24C04 EEPROM configurations
const EEPROM_Conf AT24C04_1V8_Conf  = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1     , .AddressType = EEPROM_ADDRESS_1Byte_plus_A0    , .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize =  32/*Pages*/ *16, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C04_Conf      = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1     , .AddressType = EEPROM_ADDRESS_1Byte_plus_A0    , .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize =  32/*Pages*/ *16, .MaxI2CclockSpeed = 400000, };

// AT24C08A EEPROM configurations
const EEPROM_Conf AT24C08A_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2       , .AddressType = EEPROM_ADDRESS_1Byte_plus_A1A0  , .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize =  64/*Pages*/ *16, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C08A_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2       , .AddressType = EEPROM_ADDRESS_1Byte_plus_A1A0  , .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize =  64/*Pages*/ *16, .MaxI2CclockSpeed = 400000, };

// AT24C16A EEPROM configurations
const EEPROM_Conf AT24C16A_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_NO_CHIP_ADDRESS_SELECT, .AddressType = EEPROM_ADDRESS_1Byte_plus_A2A1A0, .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize = 128/*Pages*/ *16, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf AT24C16A_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_NO_CHIP_ADDRESS_SELECT, .AddressType = EEPROM_ADDRESS_1Byte_plus_A2A1A0, .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize = 128/*Pages*/ *16, .MaxI2CclockSpeed = 400000, };


//=== 24XX256 devices =========================================================
// 24AA256 EEPROM configurations
const EEPROM_Conf _24AA256_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .ArrayByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 100000, };
const EEPROM_Conf _24AA256_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .ArrayByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 400000, };

// 24LC256 EEPROM configurations
const EEPROM_Conf _24LC256_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .ArrayByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 400000, };

// 24FC256 EEPROM configurations
const EEPROM_Conf _24FC256_1V8_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .ArrayByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf _24FC256_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 5, .PageSize = 64, .ArrayByteSize = 512/*Pages*/ *64, .MaxI2CclockSpeed = 1000000, };


//=== AT24CM02 devices =========================================================
const EEPROM_Conf AT24CM02_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2, .AddressType = EEPROM_ADDRESS_2Byte_plus_A1A0, .PageWriteTime = 10, .PageSize = 256, .ArrayByteSize = 1024/*Pages*/ *256, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24CM02_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2, .AddressType = EEPROM_ADDRESS_2Byte_plus_A1A0, .PageWriteTime = 10, .PageSize = 256, .ArrayByteSize = 1024/*Pages*/ *256, .MaxI2CclockSpeed = 1000000, };


//=== AT24MACX02 devices =========================================================
// AT24MAC402 EEPROM configurations
const EEPROM_Conf AT24MAC402_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24MAC402_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed = 1000000, };

// AT24MAC602 EEPROM configurations
const EEPROM_Conf AT24MAC602_1V7_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed =  400000, };
const EEPROM_Conf AT24MAC602_Conf     = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1A0, .AddressType = EEPROM_ADDRESS_1Byte, .PageWriteTime = 5, .PageSize = 16, .ArrayByteSize = 16/*Pages*/ *16, .MaxI2CclockSpeed = 1000000, };


//=== 47(L/C)04 devices =========================================================
const EEPROM_Conf EERAM47L04_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 8, .PageSize = 512, .ArrayByteSize = 512, .MaxI2CclockSpeed = 1000000, };
const EEPROM_Conf EERAM47C04_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 8, .PageSize = 512, .ArrayByteSize = 512, .MaxI2CclockSpeed = 1000000, };


//=== 47(L/C)16 devices =========================================================
const EEPROM_Conf EERAM47L16_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 25, .PageSize = 2048, .ArrayByteSize = 2048, .MaxI2CclockSpeed = 1000000, };
const EEPROM_Conf EERAM47C16_Conf = { .ChipAddress = 0xA0, .ChipSelect = EEPROM_CHIP_ADDRESS_A2A1, .AddressType = EEPROM_ADDRESS_2Bytes, .PageWriteTime = 25, .PageSize = 2048, .ArrayByteSize = 2048, .MaxI2CclockSpeed = 1000000, };



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





//**********************************************************************************************************************************************************
//=============================================================================
// EEPROM initialization
//=============================================================================
eERRORRESULT Init_EEPROM(EEPROM *pComp)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  if (pComp->I2CclockSpeed > pComp->Conf->MaxI2CclockSpeed) return ERR__I2C_FREQUENCY_ERROR;
  Error = pI2C->fnI2C_Init(pI2C, pComp->I2CclockSpeed);
  if (Error != ERR_OK) return Error; // If there is an error while calling fnInterfaceInit() then return the error

  return (EEPROM_IsReady(pComp) ? ERR_OK : ERR__NO_DEVICE_DETECTED);
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
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return false;
#endif
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER),
    I2C_MEMBER(ChipAddr    ) ((pComp->Conf->ChipAddress | pComp->AddrA2A1A0) & I2C_WRITE_ANDMASK),
    I2C_MEMBER(Start       ) true,
    I2C_MEMBER(pBuffer     ) NULL,
    I2C_MEMBER(BufferSize  ) 0,
    I2C_MEMBER(Stop        ) true,
  };
  return (pI2C->fnI2C_Transfer(pI2C, &PacketDesc) == ERR_OK); // Send only the chip address and get the Ack flag
}





//**********************************************************************************************************************************************************
// Write EEPROM address to device (DO NOT USE DIRECTLY)
eERRORRESULT __EEPROM_WriteAddress(EEPROM *pComp, uint32_t address, const eI2C_TransferType transferType)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  const EEPROM_Conf* const pConf = pComp->Conf;
  const uint8_t AddrBytes  =  (pConf->AddressType & (uint8_t)EEPROM_ADDRESS_Bytes_MASK);
  const uint8_t AddrTypeAx = ((pConf->AddressType & (uint8_t)EEPROM_ADDRESS_plus_Ax_MASK) >> 4);

  //--- Create address ---
  uint8_t Address[EEPROM_ADDRESS_4Bytes];
  for (int_fast8_t z = AddrBytes; --z >=0;) Address[z] = (uint8_t)((address >> ((AddrBytes - z - 1) * 8)) & 0xFF);
  //--- Send the address ---
  I2CInterface_Packet PacketDesc =
  {
    I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(transferType),
    I2C_MEMBER(ChipAddr    ) (pConf->ChipAddress | (pComp->AddrA2A1A0 & ~AddrTypeAx) | ((address >> (8 * AddrBytes - 1)) & AddrTypeAx)) & I2C_WRITE_ANDMASK, // Generate chip address
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
// Read data from the EEPROM (DO NOT USE DIRECTLY, use EEPROM_ReadData() instead)
eERRORRESULT __EEPROM_ReadPage(EEPROM *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > pComp->Conf->PageSize) return ERR__OUT_OF_RANGE;
  eERRORRESULT Error;

  //--- Read the page ---
  Error = __EEPROM_WriteAddress(pComp, address, I2C_WRITE_THEN_READ_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_OK)                                                           // If there is no error while writing address then
  {
    I2CInterface_Packet PacketDesc =
    {
      I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_READ_SECOND_PART),
      I2C_MEMBER(ChipAddr    ) ((pComp->Conf->ChipAddress | pComp->AddrA2A1A0) | I2C_READ_ORMASK),
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
// Read EEPROM data from the EEPROM device
//=============================================================================
eERRORRESULT EEPROM_ReadData(EEPROM *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  const EEPROM_Conf* const pConf = pComp->Conf;
  if ((address + size) > pConf->ArrayByteSize) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;
  size_t PageRemData;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    PageRemData = pConf->PageSize - (address & (pConf->PageSize - 1));     // Get how many bytes remain in the current page
    PageRemData = (size < PageRemData ? size : PageRemData);               // Get the least remaining bytes to read between remain size and remain in page

    //--- Read with timeout ---
    uint32_t Timeout = pComp->fnGetCurrentms() + pConf->PageWriteTime + 1; // Wait at least PageWriteTime + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = __EEPROM_ReadPage(pComp, address, data, PageRemData);        // Read data from a page
      if (Error == ERR_OK) break;                                          // All went fine, continue the data sending
      if (Error != ERR__NOT_READY) return Error;                           // If there is an error while calling __EEPROM_WritePage() then return the error
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT;  // Timout ? return the error
    }
    address += PageRemData;
    data += PageRemData;
    size -= PageRemData;
  }
  return ERR_OK;
}





//**********************************************************************************************************************************************************
// Write data to the EEPROM (DO NOT USE DIRECTLY, use EEPROM_ReadData() instead)
eERRORRESULT __EEPROM_WritePage(EEPROM *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pComp->Conf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  I2C_Interface* pI2C = GET_I2C_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pI2C->fnI2C_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size > pComp->Conf->PageSize) return ERR__OUT_OF_RANGE;
  eERRORRESULT Error;
  uint8_t* pData = (uint8_t*)data;

  //--- Write the page ---
  Error = __EEPROM_WriteAddress(pComp, address, I2C_WRITE_THEN_WRITE_FIRST_PART); // Start a write at address with the device
  if (Error == ERR_OK)                                                            // If there is no error while writing address then
  {
    I2CInterface_Packet PacketDesc =
    {
      I2C_MEMBER(Config.Value) I2C_NO_POLLING | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_WRITE_THEN_WRITE_SECOND_PART),
      I2C_MEMBER(ChipAddr    ) 0,     // Chip address will not be used
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
// Write EEPROM data to the EEPROM device
//=============================================================================
eERRORRESULT EEPROM_WriteData(EEPROM *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  size_t PageRemData;
  const EEPROM_Conf* const pConf = pComp->Conf;
  if ((address + size) > pConf->ArrayByteSize) return ERR__OUT_OF_MEMORY;
  uint32_t Timeout;
  uint8_t* pData = (uint8_t*)data;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    PageRemData = pConf->PageSize - (address & (pConf->PageSize - 1));    // Get how many bytes remain in the current page
    PageRemData = (size < PageRemData ? size : PageRemData);              // Get the least remaining bytes to write between remain size and remain in page

    //--- Write with timeout ---
    Timeout = pComp->fnGetCurrentms() + pConf->PageWriteTime + 1;         // Wait at least PageWriteTime + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = __EEPROM_WritePage(pComp, address, pData, PageRemData);     // Write data to a page
      if (Error == ERR_OK) break;                                         // All went fine, continue the data sending
      if (Error != ERR__NOT_READY) return Error;                          // If there is an error while calling __EEPROM_WritePage() then return the error
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
    address += PageRemData;
    pData += PageRemData;
    size  -= PageRemData;
  }
  return ERR_OK;
}


//==============================================================================
// Wait the end of write to the EEPROM device
//==============================================================================
eERRORRESULT EEPROM_WaitEndOfWrite(EEPROM *pComp)
{
  //--- Write with timeout ---
  const EEPROM_Conf* const pConf = pComp->Conf;
  uint32_t Timeout = pComp->fnGetCurrentms() + pConf->PageWriteTime + 1; // Wait at least PageWriteTime + 1ms because GetCurrentms can be 1 cycle before the new ms
  while (true)
  {
    if (EEPROM_IsReady(pComp)) break;                                    // Wait the end of write, and exit if all went fine
    if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT;  // Timout? return the error
  }
  return ERR_OK;
}







//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------