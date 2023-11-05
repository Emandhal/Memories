/*!*****************************************************************************
 * @file    I2C_Interface.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    02/10/2021
 * @brief   I2C interface for driver
 * @details This I2C interface definitions for all the https://github.com/Emandhal
 * drivers and developments
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020-2023 Fabien MAILLY
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
#ifndef __I2C_INTERFACE_H_INC
#define __I2C_INTERFACE_H_INC
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
#  define I2C_MEMBER(name)
#else
#  define I2C_MEMBER(name)  .name =
#endif
//-----------------------------------------------------------------------------

#define I2C_READ_ORMASK    ( 0x01u ) //!< Standard I2C LSB bit to set
#define I2C_WRITE_ANDMASK  ( 0xFEu ) //!< Standard I2C bit mask which clear the LSB

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// I2C Interface definitions
//********************************************************************************************************************

//! I2C configuration structure
typedef union I2C_Conf
{
  uint32_t Value;
  struct
  {
    uint32_t TransferType   :  3; //!<  0- 2 - This is the transfer type of the packet
    uint32_t IsNonBlocking  :  1; //!<  3    - Non blocking use for the I2C: '1' = The driver ask for a non-blocking tansfer (with DMA or interrupt transfer) ; '0' = The driver ask for a blocking transfer
    uint32_t EndianResult   :  3; //!<  4- 6 - If the transfer changes the endianness, the peripheral that do the transfer will say it here
    uint32_t EndianTransform:  3; //!<  7- 9 - The driver that asks for the transfer needs an endian change from little to big-endian or big to little-endian
    uint32_t TransactionInc :  6; //!< 10-15 - Current transaction number (managed by the I2C+DMA driver). When a new DMA transaction is initiate, set this value to '0', the I2C+DMA driver will return an incremental number. This is for knowing that the transaction has been accepted or the bus is busy with another transaction
    uint32_t                : 15; //!< 16-30
    uint32_t Addr10bits     :  1; //!< 31    - Chip address length: '1' = 10-bits address ; '0' = 8-bits address
  } Bits;
} I2C_Conf;

//! I2C Transfer type enum
typedef enum
{
  I2C_SIMPLE_TRANSFER              = 0b000u, //!< This packet is a simple transfer
  I2C_WRITE_THEN_READ_FIRST_PART   = 0b001u, //!< This packet is the first part of a dual transfer (with restart). Transfer will be a write then read
  I2C_WRITE_THEN_READ_SECOND_PART  = 0b010u, //!< This packet is the second part of a dual transfer (with restart). Transfer will be a write then read
  I2C_WRITE_THEN_WRITE_FIRST_PART  = 0b101u, //!< This packet is the first part of a dual transfer (with restart). Transfer will be a write then write
  I2C_WRITE_THEN_WRITE_SECOND_PART = 0b110u, //!< This packet is the second part of a dual transfer (with restart). Transfer will be a write then write
} eI2C_TransferType;

#define I2C_TRANSFER_TYPE_Pos                0
#define I2C_TRANSFER_TYPE_Mask               (0x7u << I2C_TRANSFER_TYPE_Pos)
#define I2C_TRANSFER_TYPE_SET(value)         (((uint16_t)(value) << I2C_TRANSFER_TYPE_Pos) & I2C_TRANSFER_TYPE_Mask) //!< Set transfer type
#define I2C_TRANSFER_TYPE_GET(value)         (((uint16_t)(value) & I2C_TRANSFER_TYPE_Mask) >> I2C_TRANSFER_TYPE_Pos) //!< Get transfer type
#define I2C_FIRST_TRANSFER                   (0x1u << 0)                                             //!< First part of a dual transfer
#define I2C_IS_FIRST_TRANSFER(value)         ( ((uint16_t)(value) & I2C_FIRST_TRANSFER) > 0 )        //!< Is first part of a dual transfer?
#define I2C_SECOND_TRANSFER                  (0x1u << 1)                                             //!< Second part of a dual transfer
#define I2C_IS_SECOND_TRANSFER(value)        ( ((uint16_t)(value) & I2C_SECOND_TRANSFER) > 0 )       //!< Is second part of a dual transfer?
#define I2C_SECOND_TRANSFER_WRITE            (0x1u << 2)                                             //!< Second part of a dual transfer will be a write
#define I2C_IS_SECOND_TRANSFER_WRITE(value)  ( ((uint16_t)(value) & I2C_SECOND_TRANSFER_WRITE) > 0 ) //!< Is second part of a dual transfer a write?

#define I2C_USE_NON_BLOCKING                 (0x1u << 3) //!< Use a non-blocking transfer (with DMA or interrupt transfer)
#define I2C_BLOCKING                         (0x0u << 3) //!< Use a blocking transfer

//! Endianness transform enum
typedef enum
{
  I2C_NO_ENDIAN_CHANGE     = 0x0u, //!< Do not change endianness therefore read/write byte at the same order as received/sent
  I2C_SWITCH_ENDIAN_16BITS = 0x2u, //!< Switch endianness per read/write 16-bits data received/sent
  I2C_SWITCH_ENDIAN_24BITS = 0x3u, //!< Switch endianness per read/write 24-bits data received/sent
  I2C_SWITCH_ENDIAN_32BITS = 0x4u, //!< Switch endianness per read/write 32-bits data received/sent
} eI2C_EndianTransform;

#define I2C_ENDIAN_RESULT_Pos              4
#define I2C_ENDIAN_RESULT_Mask             (0x7u << I2C_ENDIAN_RESULT_Pos)
#define I2C_ENDIAN_RESULT_SET(value)       (((uint16_t)(value) << I2C_ENDIAN_RESULT_Pos) & I2C_ENDIAN_RESULT_Mask) //!< Set endian result
#define I2C_ENDIAN_RESULT_GET(value)       (((uint16_t)(value) & I2C_ENDIAN_RESULT_Mask) >> I2C_ENDIAN_RESULT_Pos) //!< Get endian result
#define I2C_ENDIAN_TRANSFORM_Pos           7
#define I2C_ENDIAN_TRANSFORM_Mask          (0x7u << I2C_ENDIAN_TRANSFORM_Pos)
#define I2C_ENDIAN_TRANSFORM_SET(value)    (((uint16_t)(value) << I2C_ENDIAN_TRANSFORM_Pos) & I2C_ENDIAN_TRANSFORM_Mask) //!< Set endian transform
#define I2C_ENDIAN_TRANSFORM_GET(value)    (((uint16_t)(value) & I2C_ENDIAN_TRANSFORM_Mask) >> I2C_ENDIAN_TRANSFORM_Pos) //!< Get endian transform
#define I2C_TRANSACTION_NUMBER_Pos         10
#define I2C_TRANSACTION_NUMBER_Mask        (0x3Fu)
#define I2C_TRANSACTION_NUMBER_SET(value)  (((uint16_t)(value) & I2C_TRANSACTION_NUMBER_Mask) << I2C_TRANSACTION_NUMBER_Pos) //!< Set transaction number
#define I2C_TRANSACTION_NUMBER_GET(value)  (((uint16_t)(value) >> I2C_TRANSACTION_NUMBER_Pos) & I2C_TRANSACTION_NUMBER_Mask) //!< Get transaction number

#define I2C_USE_10BITS_ADDRESS             (0x1u << 31) //!< Use a 10-bits chip address
#define I2C_USE_8BITS_ADDRESS              (0x0u << 31) //!< Use a 8-bits chip address

//-----------------------------------------------------------------------------

//! @brief Description of the data transmission
typedef struct
{
  I2C_Conf Config;    //! Configuration of the I2C transfer
  uint16_t ChipAddr;  //! I2C slave chip address to communicate with. Can be 8 or 10-bits
  bool Start;         //! Indicate if the transfer needs a start (in case of a new transfer) or restart (if the previous transfer has not been stopped)
  uint8_t* pBuffer;   //! In case of write, this is the bytes to send to slave chip, in case of read, this is where data read will be stored
  size_t BufferSize;  //! Buffer size in bytes
  bool Stop;          //! Indicate if the transfer needs a stop after the last byte sent by this function call
} I2CInterface_Packet;

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// Fill packet description helpers
//********************************************************************************************************************

//! Prepare I2C packet description to check component with a 8-bits device address
#define I2C_INTERFACE8_NO_DATA_DESC(chipAddr)                                                                             \
  {                                                                                                                       \
    I2C_MEMBER(Config.Value) I2C_BLOCKING | I2C_USE_8BITS_ADDRESS                                                         \
                           | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER), \
    I2C_MEMBER(ChipAddr    ) (chipAddr),                                                                                  \
    I2C_MEMBER(Start       ) true,                                                                                        \
    I2C_MEMBER(pBuffer     ) NULL,                                                                                        \
    I2C_MEMBER(BufferSize  ) 0,                                                                                           \
    I2C_MEMBER(Stop        ) true,                                                                                        \
  }

//! Prepare I2C packet description to check the DMA status with a 8-bits device address
#define I2C_INTERFACE8_CHECK_DMA_DESC(chipAddr,transactionNumber)                                                            \
  {                                                                                                                          \
    I2C_MEMBER(Config.Value) I2C_USE_NON_BLOCKING | I2C_USE_8BITS_ADDRESS | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE)   \
                           | I2C_TRANSFER_TYPE_SET(I2C_SIMPLE_TRANSFER) | I2C_TRANSACTION_NUMBER_SET(CurrTransactionNumber), \
    I2C_MEMBER(ChipAddr    ) (chipAddr) | I2C_READ_ORMASK,                                                                   \
    I2C_MEMBER(Start       ) true,                                                                                           \
    I2C_MEMBER(pBuffer     ) NULL,                                                                                           \
    I2C_MEMBER(BufferSize  ) 0,                                                                                              \
    I2C_MEMBER(Stop        ) true,                                                                                           \
  }

//! Prepare I2C packet description to transmit bytes with a 8-bits device address
#define I2C_INTERFACE8_TX_DATA_DESC(chipAddr,start,txData,size,stop,transferType)                                  \
  {                                                                                                                \
    I2C_MEMBER(Config.Value) I2C_BLOCKING | I2C_USE_8BITS_ADDRESS                                                  \
                           | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(transferType), \
    I2C_MEMBER(ChipAddr    ) (chipAddr) & I2C_WRITE_ANDMASK,                                                       \
    I2C_MEMBER(Start       ) (start),                                                                              \
    I2C_MEMBER(pBuffer     ) (uint8_t*)(txData),                                                                             \
    I2C_MEMBER(BufferSize  ) (size),                                                                               \
    I2C_MEMBER(Stop        ) (stop),                                                                               \
  }

//! Prepare I2C packet description to receive bytes with a 8-bits device address
#define I2C_INTERFACE8_RX_DATA_DESC(chipAddr,start,rxData,size,stop,transferType)                                  \
  {                                                                                                                \
    I2C_MEMBER(Config.Value) I2C_BLOCKING | I2C_USE_8BITS_ADDRESS                                                  \
                           | I2C_ENDIAN_TRANSFORM_SET(I2C_NO_ENDIAN_CHANGE) | I2C_TRANSFER_TYPE_SET(transferType), \
    I2C_MEMBER(ChipAddr    ) (chipAddr) | I2C_READ_ORMASK,                                                         \
    I2C_MEMBER(Start       ) (start),                                                                              \
    I2C_MEMBER(pBuffer     ) (uint8_t*)(rxData),                                                                             \
    I2C_MEMBER(BufferSize  ) (size),                                                                               \
    I2C_MEMBER(Stop        ) (stop),                                                                               \
  }

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// I2C Interface functions definitions
//********************************************************************************************************************

typedef struct I2C_Interface I2C_Interface; //! Typedef of I2C_Interface device object structure

//-----------------------------------------------------------------------------


/*! @brief Interface function for I2C peripheral initialization
 *
 * This function will be called at driver initialization to configure the interface driver
 * @param[in] *pIntDev Is the I2C interface container structure used for the interface initialization
 * @param[in] sclFreq Is the SCL frequency in Hz to set at the interface initialization
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*I2CInit_Func)(I2C_Interface *pIntDev, const uint32_t sclFreq);


/*! @brief Interface packet function for I2C peripheral transfer
 *
 * This function will be called when the driver needs to transfer data over the I2C communication with the device
 * The packet gives the possibility to set the address/command where the data will be stored/recall.
 * @note Concerning the endianness change, at the #I2CTransferPacket_Func call, the driver ask for a transform if possible. If the I2C/DMA drivers can perform this in the interface function, it need to set the transform result at the same transform value.
 * If it can't perform the transformation, set the endian result to I2C_NO_ENDIAN_CHANGE and the driver will do it. This can discharge the driver and CPU to perform the transformation. Some DMA can perform it with data/block stride.
 * @warning A I2CInit_Func() must be called before using this function
 * @param[in] *pIntDev Is the I2C interface container structure used for the communication
 * @param[in] *pPacketConf Is the packet description to transfer through I2C
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*I2CTransferPacket_Func)(I2C_Interface *pIntDev, I2CInterface_Packet* const pPacketDesc);

//-----------------------------------------------------------------------------

//! @brief I2C interface container structure
struct I2C_Interface
{
  void *InterfaceDevice;                 //!< This is the pointer that will be in the first parameter of all interface call functions
  uint32_t UniqueID;                     //!< This is a protection for the #InterfaceDevice pointer. This value will be check when using the struct I2C_Interface in the driver which use the generic I2C interface
  I2CInit_Func fnI2C_Init;               //!< This function will be called at driver initialization to configure the interface driver
  I2CTransferPacket_Func fnI2C_Transfer; //!< This function will be called when the driver needs to transfer data over the I2C communication with the device
  uint8_t Channel;                       //!< I2C channel of the interface device
};

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* __I2C_INTERFACE_H_INC */