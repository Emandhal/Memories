/*!*****************************************************************************
 * @file    48L512.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    26/12/2021
 * @brief   EERAM48L512 driver
 * @details SPI-Compatible 512-kbit SPI Serial EERAM
 * Follow datasheet DS20006008C Rev.C (Oct 2019)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "48L512.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
//-----------------------------------------------------------------------------

#ifdef USE_DYNAMIC_INTERFACE
#  define GET_SPI_INTERFACE  pComp->SPI
#else
#  define GET_SPI_INTERFACE  &pComp->SPI
#endif
#ifdef USE_EXTERNAL_CRC16
#  define ComputeCRC16IBM3740  pComp->fnComputeCRC16
#else
#  define ComputeCRC16IBM3740  __ComputeCRC16IBM3740
#endif

//-----------------------------------------------------------------------------





//=============================================================================
// Prototypes for private functions
//=============================================================================
#if !defined(USE_EXTERNAL_CRC16)
/*! @brief Function that compute CRC16-IBM3740 for the driver
 *
 * This function will be called when a CRC16-IBM3740 computation is needed
 * @param[in,out] *pCRC Is the current computation of the CRC. If this is the first data computed, set the value to 0xFFFF
 * @param[in] *data Is the byte steam of data to compute
 * @param[in] size Is the size of the byte stream
 */
static void __ComputeCRC16IBM3740(uint16_t* pCRC, const uint8_t* data, size_t size);
#endif

/*! @brief Write address of the EERAM48L512 device
 *
 * This function asserts the device, write the OP code and send the address. The device remains asserted after this function
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] opCode Is the OP code to send
 * @param[in] address Is the address to send
 * @param[in,out] *pCRC If set value non NULL, it asks to calculate the CRC and will return the CRC of the address part
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __EERAM48L512_WriteAddress(EERAM48L512 *pComp, const uint8_t opCode, const uint32_t address, uint16_t* pCRC);

/*! @brief Read data from the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] opCode Is the OP code to send
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @param[in] useCRC If 'true' the function will compute the CRC and check with the one received with data
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __EERAM48L512_ReadData(EERAM48L512 *pComp, const uint8_t opCode, uint32_t address, uint8_t* data, size_t size, bool useCRC);

/*! @brief Write data to the EERAM48L512 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] opCode Is the OP code to send
 * @param[in] address Is the address to send
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @param[in] useCRC If 'true' the function will compute the CRC and send it the data
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __EERAM48L512_WriteData(EERAM48L512 *pComp, const uint8_t opCode, uint32_t address, const uint8_t* data, size_t size, bool useCRC);
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// EERAM48L512 initialization
//=============================================================================
eERRORRESULT Init_EERAM48L512(EERAM48L512 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pI2C == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pI2C->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  pComp->InternalConfig = 0;

  //--- Configure SPI ---
  if (pComp->SPIclockSpeed > EERAM48L512_SPICLOCK_MAX) return ERR__SPI_FREQUENCY_ERROR;
  return pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, STD_SPI_MODE0, pComp->SPIclockSpeed);
}





//**********************************************************************************************************************************************************
#if !defined(USE_EXTERNAL_CRC16)
//=============================================================================
// [STATIC] Compute the CRC16-IBM3740 from the byte stream in parameter
//=============================================================================
void __ComputeCRC16IBM3740(uint16_t* pCRC, const uint8_t* data, size_t size)
{
  if (data == NULL) return;
  const uint16_t Polynome = 0x1021;

  uint_fast16_t bit = 0;
  while (size-- != 0)
  {
    *pCRC ^= ((uint16_t)*data++) << 8;
    for (int_fast8_t i = 8; --i >= 0; )
    {
      bit = (*pCRC & 0x8000);
      *pCRC <<= 1;
      if (bit != 0) *pCRC ^= Polynome;
    }
  }
}
#endif





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Write address of the EERAM48L512 device
//=============================================================================
eERRORRESULT __EERAM48L512_WriteAddress(EERAM48L512 *pComp, const uint8_t opCode, const uint16_t address, uint16_t* pCRC)
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

  //--- Create address ---
  uint8_t Address[EERAM48L512_ADDRESS_BYTE_SIZE + 1] = { opCode, (uint8_t)(address >> 8), (uint8_t)(address & 0xFF) };
  if (pCRC != NULL) ComputeCRC16IBM3740(pCRC, &Address[1], sizeof(Address)-1); // Calculate CRC if ask
  //--- Send the address ---
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &Address[0],
    SPI_MEMBER(RxData      ) NULL,
    SPI_MEMBER(DataSize    ) ( EERAM48L512_IS_NV_USER_SPACE(opCode) ? 1 : sizeof(Address)),
    SPI_MEMBER(Terminate   ) false,
  };
  return pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Transfer the address
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Read data from the EERAM48L512 device
//=============================================================================
eERRORRESULT __EERAM48L512_ReadData(EERAM48L512 *pComp, const uint8_t opCode, uint16_t address, uint8_t* data, size_t size, bool useCRC)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > EERAM48L512_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;

  //--- Read data ---
  uint16_t CRC = 0xFFFF;                                            // Initial value of the CRC
  uint16_t* pCRC = (useCRC ? &CRC : NULL);
  Error = __EERAM48L512_WriteAddress(pComp, opCode, address, pCRC); // Start a write at address with the device
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET;    // Remove write enable flag
  if (Error == ERR_NONE)                                            // If there is no error while writing address then
  {
    SPIInterface_Packet PacketDesc =
    {
      SPI_MEMBER(Config.Value) SPI_BLOCKING | SPI_USE_DUMMYBYTE_FOR_RECEIVE | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
      SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
      SPI_MEMBER(DummyByte   ) 0x00,
      SPI_MEMBER(TxData      ) NULL,
      SPI_MEMBER(RxData      ) data,
      SPI_MEMBER(DataSize    ) size,
      SPI_MEMBER(Terminate   ) (useCRC == false),
    };
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc);   // Continue the transfer by reading the data and stop transfer if no CRC
    if (useCRC)                                        // If the CRC computation shall be retreived with the data...
    {
      if (Error != ERR_NONE) return Error;             // If there is an error while calling fnSPI_Transfer() then return the error
      ComputeCRC16IBM3740(pCRC, data, size);           // Calculate CRC of received data
      //--- Get the CRC ---
      uint8_t CRCdata[2];
      PacketDesc.RxData       = &CRCdata[0];
      PacketDesc.DataSize     = sizeof(CRCdata);
      PacketDesc.Terminate    = true;
      Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Continue the transfer by reading the CRC and stop transfer
      if (Error != ERR_NONE) return Error;             // If there is an error while calling fnSPI_Transfer() then return the error
      //--- Check the CRC ---
      const uint16_t ReceivedCRC = (((uint16_t)CRCdata[0] << 8) | (uint16_t)CRCdata[1]);
      if (ReceivedCRC != CRC) return ERR__CRC_ERROR;
    }
  }
  return Error;
}



//=============================================================================
// Read SRAM data from the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_ReadSRAMData(EERAM48L512 *pComp, uint16_t address, uint8_t* data, size_t size)
{
  return __EERAM48L512_ReadData(pComp, EERAM48L512_READ, address, data, size, false);
}



//=============================================================================
// Secure read SRAM data from the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_ReadSecure(EERAM48L512 *pComp, uint16_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  if ((address & EERAM48L512_PAGE_SIZE_MASK) != 0) return ERR__ADDRESS_ALIGNMENT; // Address shall be aligned with EERAM48L512_PAGE_SIZE
  if ((size % EERAM48L512_PAGE_SIZE) != 0 ) return ERR__BAD_DATA_SIZE;            // Data to write shall be EERAM48L512_PAGE_SIZE
  if ((address + (uint32_t)size) > EERAM48L512_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    //--- Read data ---
    Error = __EERAM48L512_ReadData(pComp, EERAM48L512_SREAD, address, data, EERAM48L512_PAGE_SIZE, true); // Write data to a page with CRC
    if (Error != ERR_NONE) return Error;                                                                  // If there is an error while calling __EERAM48L512_ReadData() then return the error
    //--- Update parameters ---
    address += EERAM48L512_PAGE_SIZE;
    data += EERAM48L512_PAGE_SIZE;
    size -= EERAM48L512_PAGE_SIZE;
  }
  return ERR_NONE;
}



//=============================================================================
// Read NonVolatile User Space data from the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_ReadNVUSData(EERAM48L512 *pComp, uint8_t* data)
{
  return __EERAM48L512_ReadData(pComp, EERAM48L512_RDNUR, 0, data, EERAM48L512_NONVOLATILE_SIZE, false);
}



//=============================================================================
// Read SRAM data with DMA from the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_ReadSRAMDataWithDMA(EERAM48L512 *pComp, uint16_t address, uint8_t* data, size_t size)
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
  if ((address + (uint32_t)size) > EERAM48L512_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  SPIInterface_Packet PacketDesc;
  eERRORRESULT Error;

  //--- Check DMA ---
  if (EERAM48L512_IS_DMA_TRANSFER_IN_PROGRESS(pComp->InternalConfig))
  {
    const uint16_t CurrTransactionNumber = EERAM48L512_TRANSACTION_NUMBER_GET(pComp->InternalConfig);
    PacketDesc.Config.Value = SPI_USE_NON_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE) | SPI_TRANSACTION_NUMBER_SET(CurrTransactionNumber);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = NULL;
    PacketDesc.RxData       = NULL;
    PacketDesc.DataSize     = 0;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Send only the chip address and get the Ack flag, to return the status of the current transfer
    if ((Error != ERR__SPI_BUSY) && (Error != ERR__SPI_OTHER_BUSY)) pComp->InternalConfig &= EERAM48L512_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    return Error;
  }

  //--- Read data ---
  Error = __EERAM48L512_WriteAddress(pComp, EERAM48L512_READ, address, NULL); // Start a read at address with the device
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET;              // Remove write enable flag
  if (Error == ERR_NONE)                                                      // If there is no error while writing address then
  {
    PacketDesc.Config.Value = SPI_USE_NON_BLOCKING | SPI_USE_DUMMYBYTE_FOR_RECEIVE | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = NULL;
    PacketDesc.RxData       = data;
    PacketDesc.DataSize     = size;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Restart at first data read transfer, get the data and stop transfer at last data
    if (Error != ERR__SPI_OTHER_BUSY) pComp->InternalConfig &= EERAM48L512_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    if (Error == ERR__SPI_BUSY) pComp->InternalConfig |= EERAM48L512_DMA_TRANSFER_IN_PROGRESS;
    EERAM48L512_TRANSACTION_NUMBER_CLEAR(pComp->InternalConfig);
    pComp->InternalConfig |= EERAM48L512_TRANSACTION_NUMBER_SET(SPI_TRANSACTION_NUMBER_GET(PacketDesc.Config.Value));
  }
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Write data to the EERAM48L512 device
//=============================================================================
eERRORRESULT __EERAM48L512_WriteData(EERAM48L512 *pComp, const uint8_t opCode, uint16_t address, const uint8_t* data, size_t size, bool useCRC)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > EERAM48L512_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Write data ---
  uint16_t CRC = 0xFFFF;                                            // Initial value of the CRC
  uint16_t* pCRC = (useCRC ? &CRC : NULL);
  Error = __EERAM48L512_WriteAddress(pComp, opCode, address, pCRC); // Start a write at address with the device
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET;    // Remove write enable flag
  if (Error == ERR_NONE)                                            // If there is no error while writing address then
  {
    SPIInterface_Packet PacketDesc =
    {
      SPI_MEMBER(Config.Value) SPI_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
      SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
      SPI_MEMBER(DummyByte   ) 0x00,
      SPI_MEMBER(TxData      ) pData,
      SPI_MEMBER(RxData      ) NULL,
      SPI_MEMBER(DataSize    ) size,
      SPI_MEMBER(Terminate   ) (useCRC == false),
    };
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc);   // Continue the transfer by sending the data and stop transfer if no CRC
    if (useCRC)                                        // If the CRC computation shall be sent with the data...
    {
      if (Error != ERR_NONE) return Error;             // If there is an error while calling fnSPI_Transfer() then return the error
      ComputeCRC16IBM3740(pCRC, pData, size);          // Calculate CRC
      //--- Send the CRC ---
      uint8_t CRCdata[2] = { (uint8_t)(*pCRC >> 8), (uint8_t)(*pCRC & 0xFF) };
      PacketDesc.TxData       = &CRCdata[0];
      PacketDesc.DataSize     = sizeof(CRCdata);
      PacketDesc.Terminate    = true;
      Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Continue the transfer by sending the CRC and stop transfer
    }
  }
  return Error;
}



//=============================================================================
// Write SRAM data to the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_WriteSRAMData(EERAM48L512 *pComp, uint16_t address, const uint8_t* data, size_t size)
{
  return __EERAM48L512_WriteData(pComp, EERAM48L512_WRITE, address, data, size, false);
}



//=============================================================================
// Secure write SRAM data to the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_WriteSecure(EERAM48L512 *pComp, uint16_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  if ((address & EERAM48L512_PAGE_SIZE_MASK) != 0) return ERR__ADDRESS_ALIGNMENT; // Address shall be aligned with EERAM48L512_PAGE_SIZE
  if ((size % EERAM48L512_PAGE_SIZE) != 0 ) return ERR__BAD_DATA_SIZE;            // Data to write shall be EERAM48L512_PAGE_SIZE
  if ((address + (uint32_t)size) > EERAM48L512_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  EERAM48L512_StatusRegister RegStatus;
  eERRORRESULT Error;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    //--- Write data ---
    Error = __EERAM48L512_WriteData(pComp, EERAM48L512_SWRITE, address, pData, EERAM48L512_PAGE_SIZE, true); // Write data to a page with CRC
    if (Error != ERR_NONE) return Error;                                                                     // If there is an error while calling __EERAM48L512_WriteData() then return the error
    //--- Check the write ---
    Error = EERAM48L512_GetStatus(pComp, &RegStatus);
    if (Error != ERR_NONE) return Error;                                                                     // If there is an error while calling EERAM48L512_GetStatus() then return the error
    if ((RegStatus.Status & EERAM48L512_WRITE_SECURE_FAILED) != 0) return ERR__CRC_ERROR;                    // Check CRC
    //--- Update parameters ---
    address += EERAM48L512_PAGE_SIZE;
    pData += EERAM48L512_PAGE_SIZE;
    size -= EERAM48L512_PAGE_SIZE;
    if ((size > 0) && EERAM48L512_IS_STATUS_WRITE_ENABLE(pComp->InternalConfig)) EERAM48L512_SetWriteEnable(pComp); // Set write enable for next page
  }
  return ERR_NONE;
}



//=============================================================================
// Write NonVolatile User Space data to the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_WriteNVUSData(EERAM48L512 *pComp, const uint8_t* data)
{
  return __EERAM48L512_WriteData(pComp, EERAM48L512_WRNUR, 0, data, EERAM48L512_NONVOLATILE_SIZE, false);
}



//=============================================================================
// Write a command to the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_WriteCommand(EERAM48L512 *pComp, const eEERAM48L512_OPcodes command)
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
  uint8_t RegData = (uint8_t)command;

  //--- Read data from SPI ---
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET; // Remove write enable flag
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &RegData,
    SPI_MEMBER(RxData      ) NULL,
    SPI_MEMBER(DataSize    ) sizeof(RegData),
    SPI_MEMBER(Terminate   ) true,
  };
  return pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Start a read transfer, get the data and stop transfer
}



//=============================================================================
// Write SRAM data with DMA to the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_WriteSRAMDataWithDMA(EERAM48L512 *pComp, uint16_t address, const uint8_t* data, size_t size)
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
  if ((address + (uint32_t)size) > EERAM48L512_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  SPIInterface_Packet PacketDesc;
  eERRORRESULT Error;

  //--- Check DMA ---
  if (EERAM48L512_IS_DMA_TRANSFER_IN_PROGRESS(pComp->InternalConfig))
  {
    const uint16_t CurrTransactionNumber = EERAM48L512_TRANSACTION_NUMBER_GET(pComp->InternalConfig);
    PacketDesc.Config.Value = SPI_USE_NON_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE) | SPI_TRANSACTION_NUMBER_SET(CurrTransactionNumber);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = NULL;
    PacketDesc.RxData       = NULL;
    PacketDesc.DataSize     = 0;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Send only the chip address and get the Ack flag, to return the status of the current transfer
    if ((Error != ERR__SPI_BUSY) && (Error != ERR__SPI_OTHER_BUSY)) pComp->InternalConfig &= EERAM48L512_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    return Error;
  }

  //--- Write data ---
  Error = __EERAM48L512_WriteAddress(pComp, EERAM48L512_WRITE, address, NULL); // Start a read at address with the device
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET;               // Remove write enable flag
  if (Error == ERR_NONE)                                                       // If there is no error while writing address then
  {
    PacketDesc.Config.Value = SPI_USE_NON_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = pData;
    PacketDesc.RxData       = NULL;
    PacketDesc.DataSize     = size;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Restart at first data read transfer, get the data and stop transfer at last data
    if (Error != ERR__SPI_OTHER_BUSY) pComp->InternalConfig &= EERAM48L512_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    if (Error == ERR__SPI_BUSY) pComp->InternalConfig |= EERAM48L512_DMA_TRANSFER_IN_PROGRESS;
    EERAM48L512_TRANSACTION_NUMBER_CLEAR(pComp->InternalConfig);
    pComp->InternalConfig |= EERAM48L512_TRANSACTION_NUMBER_SET(SPI_TRANSACTION_NUMBER_GET(PacketDesc.Config.Value));
  }
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read Status register from the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_GetStatus(EERAM48L512 *pComp, EERAM48L512_StatusRegister* status)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (status == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t RegData[2] = { EERAM48L512_RDSR, 0x00 };
  eERRORRESULT Error;

  //--- Read data from SPI ---
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET; // Remove write enable flag
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &RegData[0],
    SPI_MEMBER(RxData      ) &RegData[0],
    SPI_MEMBER(DataSize    ) sizeof(RegData),
    SPI_MEMBER(Terminate   ) true,
  };
  Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Start a read transfer, get the data and stop transfer
  status->Status = RegData[1];
  return Error;
}



//=============================================================================
// Write Status register to the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_SetStatus(EERAM48L512 *pComp, const EERAM48L512_StatusRegister status)
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
  uint8_t RegData[2] = { EERAM48L512_WRSR, status.Status };

  //--- Read data from SPI ---
  pComp->InternalConfig &= EERAM48L512_STATUS_WRITE_DISABLE_SET; // Remove write enable flag
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_BLOCKING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &RegData[0],
    SPI_MEMBER(RxData      ) NULL,
    SPI_MEMBER(DataSize    ) sizeof(RegData),
    SPI_MEMBER(Terminate   ) true,
  };
  return pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Start a read transfer, get the data and stop transfer
}





//**********************************************************************************************************************************************************
//=============================================================================
// Store all the SRAM to the EEPROM of the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_StoreSRAMtoEEPROM(EERAM48L512 *pComp, bool waitEndOfStore)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  EERAM48L512_StatusRegister Reg;
  eERRORRESULT Error = ERR_NONE;

  //--- Send the store operation ---
  Error = EERAM48L512_WriteCommand(pComp, EERAM48L512_STORE); // Start a store
  if (Error != ERR_NONE) return Error;                        // If there is an error while calling EERAM48L512_WriteCommand() then return the error

  //--- Wait the end of store if asked ---
  if (waitEndOfStore)
  {
    Reg.Status = 0x00;
    uint32_t Timeout = pComp->fnGetCurrentms() + EERAM48L512_STORE_TIMEOUT + 1; // Wait at least STORE_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = EERAM48L512_GetStatus(pComp, &Reg);                         // Get the status register
      if (Error != ERR_NONE) return Error;                                // If there is an error while calling EERAM48L512_GetStatus() then return the error
      if ((Reg.Status & EERAM48L512_IS_BUSY) == 0) break;                 // The store is finished, all went fine
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
  }
  return Error;
}



//=============================================================================
// Recall all data from EEPROM to SRAM of the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_RecallEEPROMtoSRAM(EERAM48L512 *pComp, bool waitEndOfRecall)
{
#ifdef CHECK_NULL_PARAM
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  EERAM48L512_StatusRegister Reg;
  eERRORRESULT Error = ERR_NONE;

  //--- Send a recall operation ---
  Error = EERAM48L512_WriteCommand(pComp, EERAM48L512_RECALL); // Start a store
  if (Error != ERR_NONE) return Error;                         // If there is an error while calling EERAM48L512_WriteCommand() then return the error
  //--- Wait the end of recall if asked ---
  if (waitEndOfRecall)
  {
    Reg.Status = 0x00;
    uint32_t Timeout = pComp->fnGetCurrentms() + EERAM48L512_RECALL_TIMEOUT + 1; // Wait at least RECALL_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = EERAM48L512_GetStatus(pComp, &Reg);                         // Get the status register
      if (Error != ERR_NONE) return Error;                                // If there is an error while calling EERAM48L512_GetStatus() then return the error
      if ((Reg.Status & EERAM48L512_IS_BUSY) == 0) break;                 // The recall is finished, all went fine
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
  }
  return ERR_NONE;
}



//=============================================================================
// Activate the Auto-Store of the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_ActivateAutoStore(EERAM48L512 *pComp)
{
  eERRORRESULT Error = ERR_NONE;
  EERAM48L512_StatusRegister Reg;

  //--- Get the status register ---
  Error = EERAM48L512_GetStatus(pComp, &Reg);   // Get the status register
  if (Error != ERR_NONE) return Error;          // If there is an error while calling EERAM48L512_GetStatus() then return the error
  //--- Set the status register ---
  Reg.Status &= ~EERAM48L512_AUTOSTORE_DISABLE; // Unset the Auto-Store flag
  return EERAM48L512_SetStatus(pComp, Reg);     // Save the status register
}



//=============================================================================
// Deactivate the Auto-Store of the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_DeactivateAutoStore(EERAM48L512 *pComp)
{
  eERRORRESULT Error = ERR_NONE;
  EERAM48L512_StatusRegister Reg;

  //--- Get the status register ---
  Error = EERAM48L512_GetStatus(pComp, &Reg);  // Get the status register
  if (Error != ERR_NONE) return Error;         // If there is an error while calling EERAM48L512_GetStatus() then return the error
  //--- Set the status register ---
  Reg.Status |= EERAM48L512_AUTOSTORE_DISABLE; // Set the Auto-Store flag
  return EERAM48L512_SetStatus(pComp, Reg);    // Save the status register
}



//=============================================================================
// Set block write protect of the EERAM48L512 device
//=============================================================================
eERRORRESULT EERAM48L512_SetBlockWriteProtect(EERAM48L512 *pComp, eEERAM48L512_BlockProtect blockProtect)
{
  eERRORRESULT Error = ERR_NONE;
  EERAM48L512_StatusRegister Reg;

  //--- Get the status register ---
  Error = EERAM48L512_GetStatus(pComp, &Reg); // Get the status register
  if (Error != ERR_NONE) return Error;        // If there is an error while calling EERAM48L512_GetStatus() then return the error
  //--- Set the status register ---
  Reg.Bits.BP = (uint8_t)blockProtect;        // Set the block protect
  return EERAM48L512_SetStatus(pComp, Reg);   // Save the status register
}

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------