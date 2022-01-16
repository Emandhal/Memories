/*******************************************************************************
 * @file    SPI_Interface.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    02/10/2021
 * @brief   SPI interface for driver
 *
 * This SPI interface definitions for all the https://github.com/Emandhal drivers
 * and developments
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020-2022 Fabien MAILLY
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

/* Revision history:
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef __SPI_INTERFACE_H_INC
#define __SPI_INTERFACE_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#  define SPI_MEMBER(name)
#else
#  define SPI_MEMBER(name)  .name =
#endif
//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SPI Interface definitions
//********************************************************************************************************************

//! SPI configuration structure
typedef union SPI_Conf
{
  uint16_t Value;
  struct
  {
    uint16_t UseDummyByte   : 1; //!<  0    - Use dummy byte for receiving: 'true' = use the DummyByte member for all bytes to receive ; 'false' = Use TxData for all bytes to receive
    uint16_t                : 2; //!<  1- 2
    uint16_t IsPolling      : 1; //!<  3    - Polling indication for DMA use: 'true' = The driver uses polling for this transfer and the SPI peripheral can use DMA (non-blocking transfer) ; 'false' = No polling therefore do not use DMA (blocking transfer)
    uint16_t EndianResult   : 3; //!<  4- 6 - If the transfer changes the endianness, the peripheral that do the transfer will say it here
    uint16_t EndianTransform: 3; //!<  7- 9 - The driver that asks for the transfer needs an endian change from little to big-endian or big to little-endian
    uint16_t TransactionInc : 6; //!< 10-15 - Current the transaction number (managed by the SPI+DMA driver). When a new DMA transaction is initiate, set this value to '0', the SPI+DMA driver will return an incremental number. This is for knowing that the transaction has been accepted or the bus is busy with another transaction
  } Bits;
} SPI_Conf;

#define SPI_USE_DUMMYBYTE_FOR_RECEIVE  (0x1u << 0) //!< Use dummy byte for receiving
#define SPI_USE_TXDATA_FOR_RECEIVE     (0x0u << 0) //!< Use TxData for receiving
#define SPI_USE_POLLING                (0x1u << 3) //!< Use the polling (can use DMA, non blocking transfer)
#define SPI_NO_POLLING                 (0x0u << 3) //!< Do not use polling (no DMA, blocking transfer)

//! Endianness transform enum
typedef enum
{
  SPI_NO_ENDIAN_CHANGE     = 0x0u, //!< Do not change endianness therefore read/write byte at the same order as received/sent
  SPI_SWITCH_ENDIAN_16BITS = 0x2u, //!< Switch endianness per read/write 16-bits data received/sent
  SPI_SWITCH_ENDIAN_24BITS = 0x3u, //!< Switch endianness per read/write 24-bits data received/sent
  SPI_SWITCH_ENDIAN_32BITS = 0x4u, //!< Switch endianness per read/write 32-bits data received/sent
} eSPI_EndianTransform;

#define SPI_ENDIAN_RESULT_Pos              4
#define SPI_ENDIAN_RESULT_Mask             (0x7u << SPI_ENDIAN_RESULT_Pos)
#define SPI_ENDIAN_RESULT_SET(value)       (((uint16_t)(value) << SPI_ENDIAN_RESULT_Pos) & SPI_ENDIAN_RESULT_Mask) //!< Set endian result
#define SPI_ENDIAN_RESULT_GET(value)       (((uint16_t)(value) & SPI_ENDIAN_RESULT_Mask) >> SPI_ENDIAN_RESULT_Pos) //!< Get endian result
#define SPI_ENDIAN_TRANSFORM_Pos           7
#define SPI_ENDIAN_TRANSFORM_Mask          (0x7u << SPI_ENDIAN_TRANSFORM_Pos)
#define SPI_ENDIAN_TRANSFORM_SET(value)    (((uint16_t)(value) << SPI_ENDIAN_TRANSFORM_Pos) & SPI_ENDIAN_TRANSFORM_Mask) //!< Set endian transform
#define SPI_ENDIAN_TRANSFORM_GET(value)    (((uint16_t)(value) & SPI_ENDIAN_TRANSFORM_Mask) >> SPI_ENDIAN_TRANSFORM_Pos) //!< Get endian transform
#define SPI_TRANSACTION_NUMBER_Pos         10
#define SPI_TRANSACTION_NUMBER_Mask        (0x3Fu)
#define SPI_TRANSACTION_NUMBER_SET(value)  (((uint16_t)(value) & SPI_TRANSACTION_NUMBER_Mask) << SPI_TRANSACTION_NUMBER_Pos) //!< Set transaction number
#define SPI_TRANSACTION_NUMBER_GET(value)  (((uint16_t)(value) >> SPI_TRANSACTION_NUMBER_Pos) & SPI_TRANSACTION_NUMBER_Mask) //!< Get transaction number

//-----------------------------------------------------------------------------

//! @brief Description of the data transmission
typedef struct
{
  SPI_Conf Config;    //! Configuration of the SPI transfer
  uint8_t ChipSelect; //! Is the Chip Select index to use for the SPI/Dual-SPI/Quad-SPI transfer
  uint8_t DummyByte;  //! Is the byte to use for receiving data (used with flag SPI_USE_DUMMYBYTE_FOR_RECEIVE in SPIInterface_Packet.Config when receiving data or SPIInterface_Packet.TxData is NULL)
  uint8_t *TxData;    //! Is the data to send through the interface (used with flag SPI_USE_TXDATA_FOR_RECEIVE in SPIInterface_Packet.Config when receiving data)
  uint8_t *RxData;    //! Is where the data received through the interface will be stored. This parameter can be nulled by the driver if no received data is expected
  size_t DataSize;    //! Is the size of the data to send and receive through the interface
  bool Terminate;     //! Ask to terminate the current transfer. If 'true', deassert the ChipSelect pin at the end of transfer else leave the pin asserted
} SPIInterface_Packet;

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// Fill packet description helpers
//********************************************************************************************************************

//! Prepare SPI packet description to transmit bytes
#define SPI_INTERFACE_TX_DATA_DESC(txData,size,terminate)                                     \
  {                                                                                           \
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE), \
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,                                            \
    SPI_MEMBER(DummyByte   ) 0x00,                                                            \
    SPI_MEMBER(TxData      ) txData,                                                          \
    SPI_MEMBER(RxData      ) NULL,                                                            \
    SPI_MEMBER(DataSize    ) size,                                                            \
    SPI_MEMBER(Terminate   ) terminate,                                                       \
  }

//! Prepare SPI packet description to transmit bytes (TxData = RxData)
#define SPI_INTERFACE_RX_DATA_DESC(data,size,terminate)                                       \
  {                                                                                           \
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE), \
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,                                            \
    SPI_MEMBER(DummyByte   ) 0x00,                                                            \
    SPI_MEMBER(TxData      ) data,                                                            \
    SPI_MEMBER(RxData      ) data,                                                            \
    SPI_MEMBER(DataSize    ) size,                                                            \
    SPI_MEMBER(Terminate   ) terminate,                                                       \
  }

//! Prepare SPI packet description to receive data using dummy byte
#define SPI_INTERFACE_RX_DATA_WITH_DUMMYBYTE_DESC(dummyByte,rxData,size,terminate)                                            \
  {                                                                                                                           \
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE) | SPI_USE_DUMMYBYTE_FOR_RECEIVE, \
    SPI_MEMBER(ChipSelect  ) pComp->SPIchipSelect,                                                                            \
    SPI_MEMBER(DummyByte   ) dummyByte,                                                                                       \
    SPI_MEMBER(TxData      ) NULL,                                                                                            \
    SPI_MEMBER(RxData      ) rxData,                                                                                          \
    SPI_MEMBER(DataSize    ) size,                                                                                            \
    SPI_MEMBER(Terminate   ) terminate,                                                                                       \
  }

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SPI Interface functions definitions
//********************************************************************************************************************

#define SPI_PIN_COUNT_Pos         0
#define SPI_PIN_COUNT_Mask        (0x1Fu << SPI_PIN_COUNT_Pos)
#define SPI_PIN_COUNT_SET(value)  (((uint16_t)(value) << SPI_PIN_COUNT_Pos) & SPI_PIN_COUNT_Mask) //!< Set data pin count of the SPI communication
#define SPI_PIN_COUNT_GET(value)  (((uint16_t)(value) & SPI_PIN_COUNT_Mask) >> SPI_PIN_COUNT_Pos) //!< Get data pin count of the SPI communication

#define SPI_CPOL_Pos         7
#define SPI_CPOL_Mask        (0x1u << SPI_CPOL_Pos)
#define SPI_CPOL_SET(value)  (((uint16_t)(value) << SPI_CPOL_Pos) & SPI_CPOL_Mask) //!< Set clock polarity
#define SPI_CPOL_GET(value)  (((uint16_t)(value) & SPI_CPOL_Mask) >> SPI_CPOL_Pos) //!< Get clock polarity
#define SPI_CPHA_Pos         6
#define SPI_CPHA_Mask        (0x1u << SPI_CPHA_Pos)
#define SPI_CPHA_SET(value)  (((uint16_t)(value) << SPI_CPHA_Pos) & SPI_CPHA_Mask) //!< Set clock phase
#define SPI_CPHA_GET(value)  (((uint16_t)(value) & SPI_CPHA_Mask) >> SPI_CPHA_Pos) //!< Get clock phase

#define SPI_COMM_MODE_Mask  ( SPI_CPOL_Mask | SPI_CPHA_Mask )
#define SPI_COMM_MODE0      ( SPI_CPOL_SET(0) | SPI_CPHA_SET(0) ) //! SPI mode 0: Clock polarity (CPOL/CKP) = 0 ; Clock phase (CPHA) = 0 ; Clock edge (CKE/NCPHA) = 1
#define SPI_COMM_MODE1      ( SPI_CPOL_SET(0) | SPI_CPHA_SET(1) ) //! SPI mode 0: Clock polarity (CPOL/CKP) = 0 ; Clock phase (CPHA) = 1 ; Clock edge (CKE/NCPHA) = 0
#define SPI_COMM_MODE2      ( SPI_CPOL_SET(1) | SPI_CPHA_SET(0) ) //! SPI mode 0: Clock polarity (CPOL/CKP) = 1 ; Clock phase (CPHA) = 0 ; Clock edge (CKE/NCPHA) = 1
#define SPI_COMM_MODE3      ( SPI_CPOL_SET(1) | SPI_CPHA_SET(1) ) //! SPI mode 0: Clock polarity (CPOL/CKP) = 1 ; Clock phase (CPHA) = 1 ; Clock edge (CKE/NCPHA) = 0

//! SPI bit width and mode enumerator
typedef enum
{
  SPI_MODE0      = SPI_COMM_MODE0 | SPI_PIN_COUNT_SET(1), //!< Communication with device with 1 bit per clock (Standard SPI mode 0)
  SPI_MODE1      = SPI_COMM_MODE1 | SPI_PIN_COUNT_SET(1), //!< Communication with device with 1 bit per clock (Standard SPI mode 1)
  SPI_MODE2      = SPI_COMM_MODE2 | SPI_PIN_COUNT_SET(1), //!< Communication with device with 1 bit per clock (Standard SPI mode 2)
  SPI_MODE3      = SPI_COMM_MODE3 | SPI_PIN_COUNT_SET(1), //!< Communication with device with 1 bit per clock (Standard SPI mode 3)
  DUAL_SPI_MODE0 = SPI_COMM_MODE0 | SPI_PIN_COUNT_SET(2), //!< Communication with device with 2 bits per clock (Dual-SPI mode 0)
  DUAL_SPI_MODE1 = SPI_COMM_MODE1 | SPI_PIN_COUNT_SET(2), //!< Communication with device with 2 bits per clock (Dual-SPI mode 1)
  DUAL_SPI_MODE2 = SPI_COMM_MODE2 | SPI_PIN_COUNT_SET(2), //!< Communication with device with 2 bits per clock (Dual-SPI mode 2)
  DUAL_SPI_MODE3 = SPI_COMM_MODE3 | SPI_PIN_COUNT_SET(2), //!< Communication with device with 2 bits per clock (Dual-SPI mode 3)
  QUAD_SPI_MODE0 = SPI_COMM_MODE0 | SPI_PIN_COUNT_SET(4), //!< Communication with device with 4 bits per clock (Quad-SPI mode 0)
  QUAD_SPI_MODE1 = SPI_COMM_MODE1 | SPI_PIN_COUNT_SET(4), //!< Communication with device with 4 bits per clock (Quad-SPI mode 1)
  QUAD_SPI_MODE2 = SPI_COMM_MODE2 | SPI_PIN_COUNT_SET(4), //!< Communication with device with 4 bits per clock (Quad-SPI mode 2)
  QUAD_SPI_MODE3 = SPI_COMM_MODE3 | SPI_PIN_COUNT_SET(4), //!< Communication with device with 4 bits per clock (Quad-SPI mode 3)
} eSPIInterface_Mode;

//-----------------------------------------------------------------------------


typedef struct SPI_Interface SPI_Interface; //! Typedef of SPI_Interface device object structure


/*! @brief Interface function for SPI peripheral initialization
 *
 * This function will be called at driver initialization to configure the interface driver
 * @param[in] *pIntDev Is the SPI interface container structure used for the interface initialization
 * @param[in] chipSelect Is the Chip Select index to use for the SPI/Dual-SPI/Quad-SPI initialization
 * @param[in] mode Is the mode of the SPI to configure
 * @param[in] sckFreq Is the SCK frequency in Hz to set at the interface initialization
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*SPIInit_Func)(SPI_Interface *pIntDev, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq);


/*! @brief Interface packet function for SPI peripheral transfer
 *
 * This function will be called when the driver needs to transfer data over the SPI communication with the device
 * @note Concerning the endianness change, at the #SPITransferPacket_Func call, the driver ask for a transform if possible. If the SPI/DMA drivers can perform this in the interface function, it need to set the transform result at the same transform value.
 * If it can't perform the transformation, set the endian result to SPI_NO_ENDIAN_CHANGE and the driver will do it. This can discharge the driver and CPU to perform the transformation. Some DMA can perform it with data/block stride.
 * @warning A SPIInit_Func() must be called before using this function
 * @param[in] *pIntDev Is the SPI interface container structure used for the communication
 * @param[in] *pPacketConf Is the packet description to transfer through SPI
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*SPITransferPacket_Func)(SPI_Interface *pIntDev, SPIInterface_Packet* const pPacketDesc);

//-----------------------------------------------------------------------------



//! @brief SPI interface container structure
struct SPI_Interface
{
  void *InterfaceDevice;                 //!< This is the pointer that will be in the first parameter of all interface call functions
  SPIInit_Func fnSPI_Init;               //!< This function will be called at driver initialization to configure the interface driver
  SPITransferPacket_Func fnSPI_Transfer; //!< This function will be called when the driver needs to transfer data over the SPI communication with the device
  uint8_t Channel;                       //!< SPI channel of the interface device in case of multiple virtual SPI channels (This is not the ChipSelect)
};

//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* __SPI_INTERFACE_H_INC */