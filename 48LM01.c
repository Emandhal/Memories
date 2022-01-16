/*******************************************************************************
 * @file    48LM01.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    26/12/2021
 * @brief   EERAM48LM01 driver
 *
 * SPI-Compatible 1-Mbit SPI Serial EERAM
 * Follow datasheet DS20006008C Rev.C (Oct 2019)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "48LM01.h"
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

/*! @brief Write address of the EERAM48LM01 device
 *
 * This function asserts the device, write the OP code and send the address. The device remains asserted after this function
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] opCode Is the OP code to send
 * @param[in] address Is the address to send
 * @param[in,out] *pCRC If set value non NULL, it asks to calculate the CRC and will return the CRC of the address part
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __EERAM48LM01_WriteAddress(EERAM48LM01 *pComp, const uint8_t opCode, const uint32_t address, uint16_t* pCRC);

/*! @brief Read data from the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] opCode Is the OP code to send
 * @param[in] address Is the address to read
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @param[in] useCRC If 'true' the function will compute the CRC and check with the one received with data
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __EERAM48LM01_ReadData(EERAM48LM01 *pComp, const uint8_t opCode, uint32_t address, uint8_t* data, size_t size, bool useCRC);

/*! @brief Write data to the EERAM48LM01 device
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] opCode Is the OP code to send
 * @param[in] address Is the address to send
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @param[in] useCRC If 'true' the function will compute the CRC and send it the data
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __EERAM48LM01_WriteData(EERAM48LM01 *pComp, const uint8_t opCode, uint32_t address, const uint8_t* data, size_t size, bool useCRC);
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// EERAM48LM01 initialization
//=============================================================================
eERRORRESULT Init_EERAM48LM01(EERAM48LM01 *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Init == NULL) return ERR__PARAMETER_ERROR;
#endif
  pComp->InternalConfig = 0;

  //--- Configure SPI ---
  if (pComp->SPIclockSpeed > EERAM48LM01_SPICLOCK_MAX) return ERR__SPI_FREQUENCY_ERROR;
  return pSPI->fnSPI_Init(pSPI, pComp->SPIchipSelect, SPI_MODE0, pComp->SPIclockSpeed);
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
// [STATIC] Write address of the EERAM48LM01 device
//=============================================================================
eERRORRESULT __EERAM48LM01_WriteAddress(EERAM48LM01 *pComp, const uint8_t opCode, const uint32_t address, uint16_t* pCRC)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif

  //--- Create address ---
  uint8_t Address[EERAM48LM01_ADDRESS_BYTE_SIZE + 1] = { opCode, (uint8_t)(address >> 16), (uint8_t)(address >> 8), (uint8_t)(address & 0xFF) };
  if (pCRC != NULL)                                            // Calculate CRC if ask
  {
    // If 17th bit of address is 0 then: ((0xFFFF ^ 0x0000) << 1) ^ 0x1021 = 0xEFDF
    // If 17th bit of address is 1 then: ((0xFFFF ^ 0x8000) << 1)          = 0xFFFE
    *pCRC = ((address & 0x10000) == 0 ? 0xEFDF : 0xFFFE);      // Pre-computed CRC start value + 17th bit of address
    ComputeCRC16IBM3740(pCRC, &Address[2], sizeof(Address)-2); // Compute the rest of address
  }
  //--- Send the address ---
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) &Address[0],
    SPI_MEMBER(RxData      ) NULL,
    SPI_MEMBER(DataSize    ) ( EERAM48LM01_IS_NV_USER_SPACE(opCode) ? 1 : sizeof(Address)),
    SPI_MEMBER(Terminate   ) false,
  };
  return pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Transfer the address
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Read data from the EERAM48LM01 device
//=============================================================================
eERRORRESULT __EERAM48LM01_ReadData(EERAM48LM01 *pComp, const uint8_t opCode, uint32_t address, uint8_t* data, size_t size, bool useCRC)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > EERAM48LM01_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;

  //--- Read data ---
  uint16_t CRC = 0xFFFF;                                            // Initial value of the CRC
  uint16_t* pCRC = (useCRC ? &CRC : NULL);
  Error = __EERAM48LM01_WriteAddress(pComp, opCode, address, pCRC); // Start a write at address with the device
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET;    // Remove write enable flag
  if (Error == ERR_OK)                                              // If there is no error while writing address then
  {
    SPIInterface_Packet PacketDesc =
    {
      SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_USE_DUMMYBYTE_FOR_RECEIVE | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
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
      if (Error != ERR_OK) return Error;               // If there is an error while calling fnSPI_Transfer() then return the error
      ComputeCRC16IBM3740(pCRC, data, size);           // Calculate CRC of received data
      //--- Get the CRC ---
      uint8_t CRCdata[2];
      PacketDesc.RxData       = &CRCdata[0];
      PacketDesc.DataSize     = sizeof(CRCdata);
      PacketDesc.Terminate    = true;
      Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Continue the transfer by reading the CRC and stop transfer
      if (Error != ERR_OK) return Error;               // If there is an error while calling fnSPI_Transfer() then return the error
      //--- Check the CRC ---
      const uint16_t ReceivedCRC = (((uint16_t)CRCdata[0] << 8) | (uint16_t)CRCdata[1]);
      if (ReceivedCRC != CRC) return ERR__CRC_ERROR;
    }
  }
  return Error;
}



//=============================================================================
// Read SRAM data from the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_ReadSRAMData(EERAM48LM01 *pComp, uint32_t address, uint8_t* data, size_t size)
{
  return __EERAM48LM01_ReadData(pComp, EERAM48LM01_READ, address, data, size, false);
}



//=============================================================================
// Secure read SRAM data from the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_ReadSecure(EERAM48LM01 *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  if ((address & EERAM48LM01_PAGE_SIZE_MASK) != 0) return ERR__ADDRESS_ALIGNMENT; // Address shall be aligned with EERAM48LM01_PAGE_SIZE
  if ((size % EERAM48LM01_PAGE_SIZE) != 0 ) return ERR__BAD_DATA_SIZE;            // Data to write shall be EERAM48LM01_PAGE_SIZE
  if ((address + (uint32_t)size) > EERAM48LM01_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  eERRORRESULT Error;

  //--- Cut data to read into pages ---
  while (size > 0)
  {
    //--- Read data ---
    Error = __EERAM48LM01_ReadData(pComp, EERAM48LM01_SREAD, address, data, EERAM48LM01_PAGE_SIZE, true); // Write data to a page with CRC
    if (Error != ERR_OK) return Error;                                                                    // If there is an error while calling __EERAM48LM01_ReadData() then return the error
    //--- Update parameters ---
    address += EERAM48LM01_PAGE_SIZE;
    data += EERAM48LM01_PAGE_SIZE;
    size -= EERAM48LM01_PAGE_SIZE;
  }
  return ERR_OK;
}



//=============================================================================
// Read NonVolatile User Space data from the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_ReadNVUSData(EERAM48LM01 *pComp, uint8_t* data)
{
  return __EERAM48LM01_ReadData(pComp, EERAM48LM01_RDNUR, 0, data, EERAM48LM01_NONVOLATILE_SIZE, false);
}



//=============================================================================
// Read SRAM data with DMA from the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_ReadSRAMDataWithDMA(EERAM48LM01 *pComp, uint32_t address, uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > EERAM48LM01_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  SPIInterface_Packet PacketDesc;
  eERRORRESULT Error;

  //--- Check DMA ---
  if (EERAM48LM01_IS_DMA_TRANSFER_IN_PROGRESS(pComp->InternalConfig))
  {
    const uint16_t CurrTransactionNumber = EERAM48LM01_TRANSACTION_NUMBER_GET(pComp->InternalConfig);
    PacketDesc.Config.Value = SPI_USE_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE) | SPI_TRANSACTION_NUMBER_SET(CurrTransactionNumber);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = NULL;
    PacketDesc.RxData       = NULL;
    PacketDesc.DataSize     = 0;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Send only the chip address and get the Ack flag, to return the status of the current transfer
    if ((Error != ERR__SPI_BUSY) && (Error != ERR__SPI_OTHER_BUSY)) pComp->InternalConfig &= EERAM48LM01_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    return Error;
  }

  //--- Read data ---
  Error = __EERAM48LM01_WriteAddress(pComp, EERAM48LM01_READ, address, NULL); // Start a read at address with the device
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET;              // Remove write enable flag
  if (Error == ERR_OK)// If there is no error while writing address then
  {
    PacketDesc.Config.Value = SPI_USE_POLLING | SPI_USE_DUMMYBYTE_FOR_RECEIVE | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = NULL;
    PacketDesc.RxData       = data;
    PacketDesc.DataSize     = size;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Restart at first data read transfer, get the data and stop transfer at last data
    if (Error != ERR__SPI_OTHER_BUSY) pComp->InternalConfig &= EERAM48LM01_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    if (Error == ERR__SPI_BUSY) pComp->InternalConfig |= EERAM48LM01_DMA_TRANSFER_IN_PROGRESS;
    EERAM48LM01_TRANSACTION_NUMBER_CLEAR(pComp->InternalConfig);
    pComp->InternalConfig |= EERAM48LM01_TRANSACTION_NUMBER_SET(SPI_TRANSACTION_NUMBER_GET(PacketDesc.Config.Value));
  }
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// [STATIC] Write data to the EERAM48LM01 device
//=============================================================================
eERRORRESULT __EERAM48LM01_WriteData(EERAM48LM01 *pComp, const uint8_t opCode, uint32_t address, const uint8_t* data, size_t size, bool useCRC)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > EERAM48LM01_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  eERRORRESULT Error;

  //--- Write data ---
  uint16_t CRC = 0xFFFF;                                            // Initial value of the CRC
  uint16_t* pCRC = (useCRC ? &CRC : NULL);
  Error = __EERAM48LM01_WriteAddress(pComp, opCode, address, pCRC); // Start a write at address with the device
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET;    // Remove write enable flag
  if (Error == ERR_OK)                                              // If there is no error while writing address then
  {
    SPIInterface_Packet PacketDesc =
    {
      SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
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
      if (Error != ERR_OK) return Error;               // If there is an error while calling fnSPI_Transfer() then return the error
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
// Write SRAM data to the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_WriteSRAMData(EERAM48LM01 *pComp, uint32_t address, const uint8_t* data, size_t size)
{
  return __EERAM48LM01_WriteData(pComp, EERAM48LM01_WRITE, address, data, size, false);
}



//=============================================================================
// Secure write SRAM data to the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_WriteSecure(EERAM48LM01 *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (data == NULL)) return ERR__PARAMETER_ERROR;
#endif
  if ((address & EERAM48LM01_PAGE_SIZE_MASK) != 0) return ERR__ADDRESS_ALIGNMENT; // Address shall be aligned with EERAM48LM01_PAGE_SIZE
  if ((size % EERAM48LM01_PAGE_SIZE) != 0 ) return ERR__BAD_DATA_SIZE;            // Data to write shall be EERAM48LM01_PAGE_SIZE
  if ((address + (uint32_t)size) > EERAM48LM01_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  EERAM48LM01_StatusRegister RegStatus;
  eERRORRESULT Error;

  //--- Cut data to write into pages ---
  while (size > 0)
  {
    //--- Write data ---
    Error = __EERAM48LM01_WriteData(pComp, EERAM48LM01_SWRITE, address, pData, EERAM48LM01_PAGE_SIZE, true); // Write data to a page with CRC
    if (Error != ERR_OK) return Error;                                                                       // If there is an error while calling __EERAM48LM01_WriteData() then return the error
    //--- Check the write ---
    Error = EERAM48LM01_GetStatus(pComp, &RegStatus);
    if (Error != ERR_OK) return Error;                                                                       // If there is an error while calling EERAM48LM01_GetStatus() then return the error
    if ((RegStatus.Status & EERAM48LM01_WRITE_SECURE_FAILED) != 0) return ERR__CRC_ERROR;                    // Check CRC
    //--- Update parameters ---
    address += EERAM48LM01_PAGE_SIZE;
    pData += EERAM48LM01_PAGE_SIZE;
    size -= EERAM48LM01_PAGE_SIZE;
    if ((size > 0) && EERAM48LM01_IS_STATUS_WRITE_ENABLE(pComp->InternalConfig)) EERAM48LM01_SetWriteEnable(pComp); // Set write enable for next page
  }
  return ERR_OK;
}



//=============================================================================
// Write NonVolatile User Space data to the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_WriteNVUSData(EERAM48LM01 *pComp, const uint8_t* data)
{
  return __EERAM48LM01_WriteData(pComp, EERAM48LM01_WRNUR, 0, data, EERAM48LM01_NONVOLATILE_SIZE, false);
}



//=============================================================================
// Write a command to the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_WriteCommand(EERAM48LM01 *pComp, const eEERAM48LM01_OPcodes command)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t RegData = (uint8_t)command;

  //--- Read data from SPI ---
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET; // Remove write enable flag
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



//=============================================================================
// Write SRAM data with DMA to the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_WriteSRAMDataWithDMA(EERAM48LM01 *pComp, uint32_t address, const uint8_t* data, size_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  if ((address + (uint32_t)size) > EERAM48LM01_EERAM_SIZE) return ERR__OUT_OF_MEMORY;
  uint8_t* pData = (uint8_t*)data;
  SPIInterface_Packet PacketDesc;
  eERRORRESULT Error;

  //--- Check DMA ---
  if (EERAM48LM01_IS_DMA_TRANSFER_IN_PROGRESS(pComp->InternalConfig))
  {
    const uint16_t CurrTransactionNumber = EERAM48LM01_TRANSACTION_NUMBER_GET(pComp->InternalConfig);
    PacketDesc.Config.Value = SPI_USE_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE) | SPI_TRANSACTION_NUMBER_SET(CurrTransactionNumber);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = NULL;
    PacketDesc.RxData       = NULL;
    PacketDesc.DataSize     = 0;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Send only the chip address and get the Ack flag, to return the status of the current transfer
    if ((Error != ERR__SPI_BUSY) && (Error != ERR__SPI_OTHER_BUSY)) pComp->InternalConfig &= EERAM48LM01_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    return Error;
  }

  //--- Write data ---
  Error = __EERAM48LM01_WriteAddress(pComp, EERAM48LM01_WRITE, address, NULL); // Start a read at address with the device
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET;               // Remove write enable flag
  if (Error == ERR_OK)                                                         // If there is no error while writing address then
  {
    PacketDesc.Config.Value = SPI_USE_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE);
    PacketDesc.ChipSelect   = pComp->SPIchipSelect;
    PacketDesc.DummyByte    = 0x00;
    PacketDesc.TxData       = pData;
    PacketDesc.RxData       = NULL;
    PacketDesc.DataSize     = size;
    PacketDesc.Terminate    = true;
    Error = pSPI->fnSPI_Transfer(pSPI, &PacketDesc); // Restart at first data read transfer, get the data and stop transfer at last data
    if (Error != ERR__SPI_OTHER_BUSY) pComp->InternalConfig &= EERAM48LM01_NO_DMA_TRANSFER_IN_PROGRESS_SET;
    if (Error == ERR__SPI_BUSY) pComp->InternalConfig |= EERAM48LM01_DMA_TRANSFER_IN_PROGRESS;
    EERAM48LM01_TRANSACTION_NUMBER_CLEAR(pComp->InternalConfig);
    pComp->InternalConfig |= EERAM48LM01_TRANSACTION_NUMBER_SET(SPI_TRANSACTION_NUMBER_GET(PacketDesc.Config.Value));
  }
  return Error;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Read Status register from the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_GetStatus(EERAM48LM01 *pComp, EERAM48LM01_StatusRegister* status)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (status == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t RegData[2] = { EERAM48LM01_RDSR, 0x00 };
  eERRORRESULT Error;

  //--- Read data from SPI ---
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET; // Remove write enable flag
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
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
// Write Status register to the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_SetStatus(EERAM48LM01 *pComp, const EERAM48LM01_StatusRegister status)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  SPI_Interface* pSPI = GET_SPI_INTERFACE;
#if defined(CHECK_NULL_PARAM) && defined(USE_DYNAMIC_INTERFACE)
  if (pSPI->fnSPI_Transfer == NULL) return ERR__PARAMETER_ERROR;
#endif
  uint8_t RegData[2] = { EERAM48LM01_WRSR, status.Status };

  //--- Write data to SPI ---
  pComp->InternalConfig &= EERAM48LM01_STATUS_WRITE_DISABLE_SET; // Remove write enable flag
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
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
// Store all the SRAM to the EEPROM of the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_StoreSRAMtoEEPROM(EERAM48LM01 *pComp, bool waitEndOfStore)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  EERAM48LM01_StatusRegister Reg;
  eERRORRESULT Error = ERR_OK;

  //--- Send the store operation ---
  Error = EERAM48LM01_WriteCommand(pComp, EERAM48LM01_STORE); // Start a store
  if (Error != ERR_OK) return Error;                          // If there is an error while calling EERAM48LM01_WriteCommand() then return the error

  //--- Wait the end of store if asked ---
  if (waitEndOfStore)
  {
    Reg.Status = 0x00;
    uint32_t Timeout = pComp->fnGetCurrentms() + EERAM48LM01_STORE_TIMEOUT + 1; // Wait at least STORE_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = EERAM48LM01_GetStatus(pComp, &Reg);                         // Get the status register
      if (Error != ERR_OK) return Error;                                  // If there is an error while calling EERAM48LM01_GetStatus() then return the error
      if ((Reg.Status & EERAM48LM01_IS_BUSY) == 0) break;                 // The store is finished, all went fine
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
  }
  return Error;
}



//=============================================================================
// Recall all data from EEPROM to SRAM of the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_RecallEEPROMtoSRAM(EERAM48LM01 *pComp, bool waitEndOfRecall)
{
#ifdef CHECK_NULL_PARAM
  if (pComp->fnGetCurrentms == NULL) return ERR__PARAMETER_ERROR;
#endif
  EERAM48LM01_StatusRegister Reg;
  eERRORRESULT Error = ERR_OK;

  //--- Send a recall operation ---
  Error = EERAM48LM01_WriteCommand(pComp, EERAM48LM01_RECALL); // Start a store
  if (Error != ERR_OK) return Error;                           // If there is an error while calling EERAM48LM01_WriteCommand() then return the error
  //--- Wait the end of recall if asked ---
  if (waitEndOfRecall)
  {
    Reg.Status = 0x00;
    uint32_t Timeout = pComp->fnGetCurrentms() + EERAM48LM01_RECALL_TIMEOUT + 1; // Wait at least RECALL_TIMEOUT + 1ms because GetCurrentms can be 1 cycle before the new ms
    while (true)
    {
      Error = EERAM48LM01_GetStatus(pComp, &Reg);                         // Get the status register
      if (Error != ERR_OK) return Error;                                  // If there is an error while calling EERAM48LM01_GetStatus() then return the error
      if ((Reg.Status & EERAM48LM01_IS_BUSY) == 0) break;                 // The recall is finished, all went fine
      if (pComp->fnGetCurrentms() >= Timeout) return ERR__DEVICE_TIMEOUT; // Timout ? return the error
    }
  }
  return ERR_OK;
}



//=============================================================================
// Activate the Auto-Store of the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_ActivateAutoStore(EERAM48LM01 *pComp)
{
  eERRORRESULT Error = ERR_OK;
  EERAM48LM01_StatusRegister Reg;

  //--- Get the status register ---
  Error = EERAM48LM01_GetStatus(pComp, &Reg);   // Get the status register
  if (Error != ERR_OK) return Error;            // If there is an error while calling EERAM48LM01_GetStatus() then return the error
  //--- Set the status register ---
  Reg.Status &= ~EERAM48LM01_AUTOSTORE_DISABLE; // Unset the Auto-Store flag
  return EERAM48LM01_SetStatus(pComp, Reg);     // Save the status register
}



//=============================================================================
// Deactivate the Auto-Store of the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_DeactivateAutoStore(EERAM48LM01 *pComp)
{
  eERRORRESULT Error = ERR_OK;
  EERAM48LM01_StatusRegister Reg;

  //--- Get the status register ---
  Error = EERAM48LM01_GetStatus(pComp, &Reg);  // Get the status register
  if (Error != ERR_OK) return Error;           // If there is an error while calling EERAM48LM01_GetStatus() then return the error
  //--- Set the status register ---
  Reg.Status |= EERAM48LM01_AUTOSTORE_DISABLE; // Set the Auto-Store flag
  return EERAM48LM01_SetStatus(pComp, Reg);    // Save the status register
}



//=============================================================================
// Set block write protect of the EERAM48LM01 device
//=============================================================================
eERRORRESULT EERAM48LM01_SetBlockWriteProtect(EERAM48LM01 *pComp, eEERAM48LM01_BlockProtect blockProtect)
{
  eERRORRESULT Error = ERR_OK;
  EERAM48LM01_StatusRegister Reg;

  //--- Get the status register ---
  Error = EERAM48LM01_GetStatus(pComp, &Reg); // Get the status register
  if (Error != ERR_OK) return Error;          // If there is an error while calling EERAM48LM01_GetStatus() then return the error
  //--- Set the status register ---
  Reg.Bits.BP = (uint8_t)blockProtect;        // Set the block protect
  return EERAM48LM01_SetStatus(pComp, Reg);   // Save the status register
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