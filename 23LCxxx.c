/*!*****************************************************************************
 * @file    23LCxxx.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    09/01/2022
 * @brief   Generic SRAM 23LCxxx driver
 * @details Generic driver for Microchip (c) Serial SRAM 23LCxxx. Works with:
 *   23A640/23K640: 64K SPI Bus Low-Power Serial SRAM
 *   23A256/23K256: 256K SPI Bus Low-Power Serial SRAM
 *   23A512/23LC512: 512Kbit SPI Serial SRAM with SDI and SQI Interface
 *   23A1024/23LC1024: 1Mbit SPI Serial SRAM with SDI and SQI Interface
 *   23LCV512: 512-Kbit SPI Serial SRAM with Battery Backup and SDI Interface
 *   23LCV1024: 1Mbit SPI Serial SRAM with Battery Backup and SDI Interface
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "23LCxxx.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
//-----------------------------------------------------------------------------

#ifdef USE_DYNAMIC_INTERFACE
#  define GET_I2C_INTERFACE  pComp->SPI
#else
#  define GET_SPI_INTERFACE  &pComp->SPI
#endif

//-----------------------------------------------------------------------------





//=== 23xxx devices ======================================================
// 23x640 configurations
const SRAM23LCxxx_Conf SRAM23A640_Conf    = { .ModeSet = SRAM23LCxxx_SPI                                    , .UseHold = true , .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =   8192/*Bytes*/, .MaxSPIclockSpeed = 20000000, };
const SRAM23LCxxx_Conf SRAM23K640_Conf    = { .ModeSet = SRAM23LCxxx_SPI                                    , .UseHold = true , .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =   8192/*Bytes*/, .MaxSPIclockSpeed = 20000000, };

// 23x256 configurations
const SRAM23LCxxx_Conf SRAM23A256_Conf    = { .ModeSet = SRAM23LCxxx_SPI                                    , .UseHold = true , .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =  32768/*Bytes*/, .MaxSPIclockSpeed = 20000000, };
const SRAM23LCxxx_Conf SRAM23K256_Conf    = { .ModeSet = SRAM23LCxxx_SPI                                    , .UseHold = true , .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =  32768/*Bytes*/, .MaxSPIclockSpeed = 20000000, };

// 23x512 configurations
const SRAM23LCxxx_Conf SRAM23A512_Conf    = { .ModeSet = SRAM23LCxxx_SPI | SRAM23LCxxx_SDI | SRAM23LCxxx_SQI, .UseHold = false, .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =  65536/*Bytes*/, .MaxSPIclockSpeed = 20000000, };
const SRAM23LCxxx_Conf SRAM23LC512_Conf   = { .ModeSet = SRAM23LCxxx_SPI | SRAM23LCxxx_SDI | SRAM23LCxxx_SQI, .UseHold = false, .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =  65536/*Bytes*/, .MaxSPIclockSpeed = 20000000, };

// 23x1024 configurations
const SRAM23LCxxx_Conf SRAM23A1024_Conf   = { .ModeSet = SRAM23LCxxx_SPI | SRAM23LCxxx_SDI | SRAM23LCxxx_SQI, .UseHold = false, .AddressBytes = 3, .PageSize = 32, .ArrayByteSize = 131072/*Bytes*/, .MaxSPIclockSpeed = 20000000, };
const SRAM23LCxxx_Conf SRAM23LC1024_Conf  = { .ModeSet = SRAM23LCxxx_SPI | SRAM23LCxxx_SDI | SRAM23LCxxx_SQI, .UseHold = false, .AddressBytes = 3, .PageSize = 32, .ArrayByteSize = 131072/*Bytes*/, .MaxSPIclockSpeed = 20000000, };

// 23LCV512 configuration
const SRAM23LCxxx_Conf SRAM23LCV512_Conf  = { .ModeSet = SRAM23LCxxx_SPI | SRAM23LCxxx_SDI                  , .UseHold = false, .AddressBytes = 2, .PageSize = 32, .ArrayByteSize =  65536/*Bytes*/, .MaxSPIclockSpeed = 20000000, };

// 23LCV512 configuration
const SRAM23LCxxx_Conf SRAM23LCV1024_Conf = { .ModeSet = SRAM23LCxxx_SPI | SRAM23LCxxx_SDI                  , .UseHold = false, .AddressBytes = 3, .PageSize = 32, .ArrayByteSize = 131072/*Bytes*/, .MaxSPIclockSpeed = 20000000, };

//**********************************************************************************************************************************************************





//=============================================================================
// Prototypes for private functions
//=============================================================================
/*! @brief Write address of the SRAM23LCxxx device
 *
 * This function asserts the device, write the instruction and send the address. The function takes care of dummy byte on read on SDI and SQI modes
 * The device remains asserted after this function
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] instruction Is the instruction to send
 * @param[in] address Is the address to send
 * @param[in] onlyInstruction Indicate if only the instruction have to be sent
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __SRAM23LCxxx_WriteAddress(SRAM23LCxxx *pComp, const uint8_t instruction, const uint32_t address, const bool onlyInstruction);

/*! @brief Read data from the SRAM23LCxxx device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] instruction Is the instruction to send
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __SRAM23LCxxx_ReadData(SRAM23LCxxx *pComp, const eSRAM23LCxxx_InstructionSet instruction, uint32_t address, uint8_t* data, size_t size);

/*! @brief Write data to the SRAM23LCxxx device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] instruction Is the instruction to send
 * @param[in] address Is the address to send
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __SRAM23LCxxx_WriteData(SRAM23LCxxx *pComp, const eSRAM23LCxxx_InstructionSet instruction, uint32_t address, const uint8_t* data, size_t size);
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// SRAM23LCxxx initialization
//=============================================================================
eERRORRESULT Init_SRAM23LCxxx(SRAM23LCxxx *pComp, const SRAM23LCxxx_Config* pConf)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pConf == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  pComp->InternalConfig = SRAM23LCxxx_IO_MODE_SET(SRAM23LCxxx_SPI);
  eERRORRESULT Error;

  //--- Recover I/O access mode ---
  if (pConf->RecoverSPIbus)
  {
    if ((pComp->Conf->ModeSet & SRAM23LCxxx_SDI) > 0)                 // Device supports SDI?
    {
      Error = pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, DUAL_SPI_MODE0, pComp->SPIclockSpeed); // Configure interface in SDI mode
      if (Error != ERR_OK) return Error;                              // If there is an error while calling fnSPI_Init() then return the error
      Error = SRAM23LCxxx_WriteInstruction(pComp, SRAM23LCxxx_RSTIO); // Return to SPI mode
      if (Error != ERR_OK) return Error;                              // If there is an error while calling SRAM23LCxxx_WriteInstruction() then return the error
    }
    if ((pComp->Conf->ModeSet & SRAM23LCxxx_SQI) > 0)                 // Device supports SQI?
    {
      Error = pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, QUAD_SPI_MODE0, pComp->SPIclockSpeed); // Configure interface in SQI mode
      if (Error != ERR_OK) return Error;                              // If there is an error while calling fnSPI_Init() then return the error
      Error = SRAM23LCxxx_WriteInstruction(pComp, SRAM23LCxxx_RSTIO); // Return to SPI mode
      if (Error != ERR_OK) return Error;                              // If there is an error while calling SRAM23LCxxx_WriteInstruction() then return the error
    }
    Error = pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, SPI_MODE0, pComp->SPIclockSpeed); // Configure interface in SDI mode
    if (Error != ERR_OK) return Error;                                // If there is an error while calling fnSPI_Init() then return the error
  }

  //--- Configure SPI interface ---
  Error = SRAM23LCxxx_SetIOmode(pComp, pConf->IOmode);
  if (Error != ERR_OK) return Error;                                  // If there is an error while calling SRAM23LCxxx_SetIOmode() then return the error

  //--- Configure memory mode ---
  return SRAM23LCxxx_SetOperationMode(pComp, pConf->OperationMode, pConf->DisableHold);
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Write address of the SRAM23LCxxx device
//=============================================================================
eERRORRESULT __SRAM23LCxxx_WriteAddress(SRAM23LCxxx *pComp, const uint8_t instruction, const uint32_t address, const bool onlyInstruction)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif

  //--- Create address ---
  uint8_t Address[1/*Instruction*/ + 4/*Bytes address*/] = { instruction, 0, 0, 0, 0 }; // Up to 32-bits address
  const uint8_t AddrBytes = (onlyInstruction ? 0 : pComp->Conf->AddressBytes);
  for (int_fast8_t z = AddrBytes; --z >=0;) Address[z + 1] = (uint8_t)((address >> ((AddrBytes - z - 1) * 8)) & 0xFF);
  //--- Send the address ---
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &Address[0],
    SPI_MEMBER(RxData      ) NULL,
    SPI_MEMBER(DataSize    ) 1 + AddrBytes,
    SPI_MEMBER(Terminate   ) false,
  };
  return pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Transfer the address
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Read data from the SRAM23LCxxx device
//=============================================================================
eERRORRESULT __SRAM23LCxxx_ReadData(SRAM23LCxxx *pComp, const eSRAM23LCxxx_InstructionSet instruction, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > pComp->Conf->ArrayByteSize) return ERR__OUT_OF_MEMORY;
  const eSRAM23LCxxx_IOmodes IOmode = SRAM23LCxxx_IO_MODE_GET(pComp->InternalConfig);
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_USE_DUMMYBYTE_FOR_RECEIVE | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) NULL,
    SPI_MEMBER(RxData      ) data,
    SPI_MEMBER(DataSize    ) size,
    SPI_MEMBER(Terminate   ) true,
  };
  eERRORRESULT Error;

  //--- Read data ---
  Error = __SRAM23LCxxx_WriteAddress(pComp, instruction, address, (instruction == SRAM23LCxxx_RDSR)); // Start a write at address with the device
  if (Error == ERR_OK)                                 // If there is no error while writing address then
  {
    if ((IOmode != SRAM23LCxxx_SPI) && (instruction != SRAM23LCxxx_RDSR)) // In SDI or SQI?
    {
      PacketDesc.DataSize  = 1;
      PacketDesc.Terminate = false;
      Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Continue the transfer by reading the dummy byte
      if (Error != ERR_OK) return Error;               // If there is an error while calling fnSPI_Transfer() then return the error
      PacketDesc.DataSize  = size;
      PacketDesc.Terminate = true;
    }
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc);   // Continue the transfer by reading the data and stop transfer
  }
  return Error;
}



//=============================================================================
// Read SRAM data from the SRAM23LCxxx device
//=============================================================================
eERRORRESULT SRAM23LCxxx_ReadSRAMData(SRAM23LCxxx *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  const SRAM23LCxxx_Conf* const pConf = pComp->Conf;
  const eSRAM23LCxxx_Modes SRAMmode = SRAM23LCxxx_MODE_GET(pComp->InternalConfig);
  if ((address + (uint32_t)size) > pConf->ArrayByteSize) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Cut data to write into pages/bytes ---
  size_t PageRemData = (SRAMmode == SRAM23LCxxx_BYTE_MODE ? 1 : size);
  while (size > 0)
  {
    if (SRAMmode == SRAM23LCxxx_PAGE_MODE)                               // Only in page mode
    {
      PageRemData = pConf->PageSize - (address & (pConf->PageSize - 1)); // Get how many bytes remain in the current page
      PageRemData = (size < PageRemData ? size : PageRemData);           // Get the least remaining bytes to read between remain size and remain in page
    }

    //--- Write data ---
    Error = __SRAM23LCxxx_ReadData(pComp, SRAM23LCxxx_READ, address, pData, PageRemData); // Read data from a page/bytes
    if (Error != ERR_OK) return Error;                                   // If there is an error while calling __SRAM23LCxxx_ReadData() then return the error
    address += PageRemData;
    pData += PageRemData;
    size -= PageRemData;
  }
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Write data to the SRAM23LCxxx device
//=============================================================================
eERRORRESULT __SRAM23LCxxx_WriteData(SRAM23LCxxx *pComp, const eSRAM23LCxxx_InstructionSet instruction, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > pComp->Conf->ArrayByteSize) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Write data ---
  Error = __SRAM23LCxxx_WriteAddress(pComp, instruction, address, (instruction == SRAM23LCxxx_WRSR)); // Start a write at address with the device
  if (Error == ERR_OK)                               // If there is no error while writing address then
  {
    SPIInterface_Packet PacketDesc =
    {
      SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
      SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
      SPI_MEMBER(DummyByte   ) 0x00,
      SPI_MEMBER(TxData      ) pData,
      SPI_MEMBER(RxData      ) NULL,
      SPI_MEMBER(DataSize    ) size,
      SPI_MEMBER(Terminate   ) true,
    };
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Continue the transfer by sending the data and stop transfer
  }
  return Error;
}



//=============================================================================
// Write SRAM data to the SRAM23LCxxx device
//=============================================================================
eERRORRESULT SRAM23LCxxx_WriteSRAMData(SRAM23LCxxx *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  const SRAM23LCxxx_Conf* const pConf = pComp->Conf;
  const eSRAM23LCxxx_Modes SRAMmode = SRAM23LCxxx_MODE_GET(pComp->InternalConfig);
  if ((address + (uint32_t)size) > pConf->ArrayByteSize) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Cut data to write into pages/bytes ---
  size_t PageRemData = (SRAMmode == SRAM23LCxxx_BYTE_MODE ? 1 : size);
  while (size > 0)
  {
    if (SRAMmode == SRAM23LCxxx_PAGE_MODE)                               // Only in page mode
    {
      PageRemData = pConf->PageSize - (address & (pConf->PageSize - 1)); // Get how many bytes remain in the current page
      PageRemData = (size < PageRemData ? size : PageRemData);           // Get the least remaining bytes to read between remain size and remain in page
    }

    //--- Write data ---
    Error = __SRAM23LCxxx_WriteData(pComp, SRAM23LCxxx_WRITE, address, pData, PageRemData); // Write data to a page/bytes
    if (Error != ERR_OK) return Error;                                   // If there is an error while calling __SRAM23LCxxx_WriteData() then return the error
    address += PageRemData;
    pData += PageRemData;
    size -= PageRemData;
  }
  return ERR_OK;
}



//=============================================================================
// Write an instruction to the SRAM23LCxxx device
//=============================================================================
eERRORRESULT SRAM23LCxxx_WriteInstruction(SRAM23LCxxx *pComp, const eSRAM23LCxxx_InstructionSet instruction)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t RegData = (uint8_t)instruction;

  //--- Read data from SPI ---
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &RegData,
    SPI_MEMBER(RxData      ) NULL,
    SPI_MEMBER(DataSize    ) sizeof(RegData),
    SPI_MEMBER(Terminate   ) true,
  };
  return pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Start a read transfer, get the data and stop transfer
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read Status register from the SRAM23LCxxx device
//=============================================================================
eERRORRESULT SRAM23LCxxx_GetStatus(SRAM23LCxxx *pComp, SRAM23LCxxx_StatusRegister* status)
{
#ifdef CHECK_NULL_PARAM
  if (status == NULL) return ERR__PARAMETER_ERROR;
#endif
  return __SRAM23LCxxx_ReadData(pComp, SRAM23LCxxx_RDSR, 0, &status->Status, sizeof(SRAM23LCxxx_StatusRegister));
}



//=============================================================================
// Write Status register to the SRAM23LCxxx device
//=============================================================================
eERRORRESULT SRAM23LCxxx_SetStatus(SRAM23LCxxx *pComp, const SRAM23LCxxx_StatusRegister status)
{
  return __SRAM23LCxxx_WriteData(pComp, SRAM23LCxxx_WRSR, 0, &status.Status, sizeof(SRAM23LCxxx_StatusRegister));
}





//**********************************************************************************************************************************************************
//=============================================================================
// Set the I/O mode of the SRAM23LCxxx
//=============================================================================
eERRORRESULT SRAM23LCxxx_SetIOmode(SRAM23LCxxx *pComp, const eSRAM23LCxxx_IOmodes mode)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  //--- Check SPI configuration ---
  if (pComp->SPIclockSpeed > pComp->Conf->MaxSPIclockSpeed) return ERR__SPI_FREQUENCY_ERROR;
  if ((mode == SRAM23LCxxx_SDI) && ((pComp->Conf->ModeSet & SRAM23LCxxx_SDI) == 0)) return ERR__SPI_CONFIG_ERROR;
  if ((mode == SRAM23LCxxx_SQI) && ((pComp->Conf->ModeSet & SRAM23LCxxx_SQI) == 0)) return ERR__SPI_CONFIG_ERROR;

  //--- Reset interface to SPI ---
  if (SRAM23LCxxx_IO_MODE_GET(pComp->InternalConfig) != SRAM23LCxxx_SPI)
  {
    Error = SRAM23LCxxx_WriteInstruction(pComp, SRAM23LCxxx_RSTIO); // Return to SPI mode
    if (Error != ERR_OK) return Error;                              // If there is an error while calling SRAM23LCxxx_WriteInstruction() then return the error
    if (mode != SRAM23LCxxx_SPI)                                    // Reset interface to SPI only if the new mode will be other than SPI
    {
      Error = pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, SPI_MODE0, pComp->SPIclockSpeed); // Configure interface in SPI mode
      if (Error != ERR_OK) return Error;                            // If there is an error while calling fnSPI_Init() then return the error
    }
  }

  //--- Configure SPI interface ---
  eSPIInterface_Mode SxImode = SPI_MODE0;                           // Default mode SPI
  if (mode == SRAM23LCxxx_SDI)                                      // SDI mode asked?
  {
    Error = SRAM23LCxxx_WriteInstruction(pComp, SRAM23LCxxx_EDIO);  // Set SDI mode
    if (Error != ERR_OK) return Error;                              // If there is an error while calling SRAM23LCxxx_WriteInstruction() then return the error
    SxImode = DUAL_SPI_MODE0;
  }
  if (mode == SRAM23LCxxx_SQI)                                      // SQI mode asked?
  {
    Error = SRAM23LCxxx_WriteInstruction(pComp, SRAM23LCxxx_EQIO);  // Set SQI mode
    if (Error != ERR_OK) return Error;                              // If there is an error while calling SRAM23LCxxx_WriteInstruction() then return the error
    SxImode = QUAD_SPI_MODE0;
  }
  Error = pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, SxImode, pComp->SPIclockSpeed);
  if (Error != ERR_OK) return Error;                                // If there is an error while calling fnSPI_Init() then return the error

  pComp->InternalConfig &= ~SRAM23LCxxx_IO_MODE_Mask;
  pComp->InternalConfig |= SRAM23LCxxx_IO_MODE_SET(mode);           // Set the new I/O mode
  return ERR_OK;
}



//=============================================================================
// Set the SRAM operation mode of the SRAM23LCxxx
//=============================================================================
eERRORRESULT SRAM23LCxxx_SetOperationMode(SRAM23LCxxx *pComp, const eSRAM23LCxxx_Modes mode, const bool disableHold)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->Conf == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  //--- Configure the SRAM ---
  SRAM23LCxxx_StatusRegister Reg;
  Reg.Status = SRAM23LCxxx_HOLD_FEATURE_ENABLE | SRAM23LCxxx_MODE_SET(mode);
  if (pComp->Conf->UseHold && disableHold) Reg.Status |= SRAM23LCxxx_HOLD_FEATURE_DISABLE;

  //--- Set the new status ---
  Error = SRAM23LCxxx_SetStatus(pComp, Reg);
  if (Error != ERR_OK) return Error;   // If there is an error while calling SRAM23LCxxx_SetStatus() then return the error
  pComp->InternalConfig &= 0xFF3E;     // Clear status into the internal config
  pComp->InternalConfig |= Reg.Status; // Set the new status into the internal config
  return ERR_OK;
}







//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------