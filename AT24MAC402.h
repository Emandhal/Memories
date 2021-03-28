/*******************************************************************************
 * @file    AT24MAC402.h
 * @author  FMA
 * @version 1.0.0
 * @date    24/08/2020
 * @brief   AT24MAC402 driver
 *
 * I2C-Compatible (2-wire) 2-Kbit (256kB x 8) Serial EEPROM
 * with a Factory-Programmed EUI-48™ Address
 * plus an Embedded Unique 128-bit Serial Number
 * Follow datasheet AT24MAC402 Rev.8808E (Jan 2015)
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020 Fabien MAILLY
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
#ifndef AT24MAC402_H_INC
#define AT24MAC402_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------

#ifndef __PACKED__
# ifndef __cplusplus
#   define __PACKED__  __attribute__((packed))
# else
#   define __PACKED__
# endif
#endif

#ifndef PACKITEM
# ifndef __cplusplus
#   define PACKITEM
# else
#   define PACKITEM  __pragma(pack(push, 1))
# endif
#endif

#ifndef UNPACKITEM
# ifndef __cplusplus
#   define UNPACKITEM
# else
#   define UNPACKITEM  __pragma(pack(pop))
# endif
#endif

//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#ifndef ControlItemSize
#  define ControlItemSize(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }
#endif

//-----------------------------------------------------------------------------



// Limits definitions
#define AT24MAC402_I2CCLOCK_MAX1V7      (  400000u ) //!< Max I2C clock frequency at 1.7V
#define AT24MAC402_I2CCLOCK_MAXSUP2V5   ( 1000000u ) //!< Max I2C clock frequency at 2.5V or more



// Device definitions
#define AT24MAC402_I2C_READ                 ( 0x01 ) //!< Standard I2C LSB bit to set
#define AT24MAC402_I2C_WRITE                ( 0xFE ) //!< Standard I2C bit mask which clear the LSB

#define AT24MAC402_EEPROM_CHIPADDRESS_BASE  ( 0xA0 ) //!< EEPROM chip base address
#define AT24MAC402_PSWP_CHIPADDRESS_BASE    ( 0x60 ) //!< Permanent Software Write Protection (PSWP) chip base address
#define AT24MAC402_SERIAL_CHIPADDRESS_BASE  ( 0xB0 ) //!< Unique Serial Number chip base address
#define AT24MAC402_EUI_CHIPADDRESS_BASE     ( 0xB0 ) //!< EUI-48™ chip base address
#define AT24MAC402_CHIPADDRESS_MASK         ( 0xFE ) //!< Chip address mask

#define AT24MAC402_RSWP_SET_CHIPADDRESS     ( 0x62 ) //!< Set Reversible Software Write Protection (RSWP) chip base address (See datasheet for hardware configuration)
#define AT24MAC402_RSWP_CLEAR_CHIPADDRESS   ( 0x66 ) //!< Clear Reversible Software Write Protection (RSWP) chip base address (See datasheet for hardware configuration)

#define AT24MAC402_SERIAL_MEMORYADDR        ( 0x80 ) //!< Memory address where the Serial Number is
#define AT24MAC402_EUI48_MEMORYADDR         ( 0x9A ) //!< Memory address where the EUI48 is

#define AT24MAC402_ADDRESS_SIZE_MAX         ( 16 ) //!< The AT24MAC402 has 256 page maximum

#define AT24MAC402_PAGE_SIZE                ( 16 ) //!< The AT24MAC402 is 16 bytes page size
#define AT24MAC402_PAGE_SIZE_MASK           ( AT24MAC402_PAGE_SIZE - 1 ) //!< The AT24MAC402 page mask is 0x0F
#define AT24MAC402_EEPROM_SIZE              ( AT24MAC402_ADDRESS_SIZE_MAX * AT24MAC402_PAGE_SIZE ) //!< The AT24MAC402 total EEPROM size

#define AT24MAC402_SERIAL_SIZE              ( 16 ) //!< AT24MAC402 Unique Serial Number size



/*! @brief Generate the AT24MAC402 chip configurable address following the state of A0, A1, and A2
 * You shall set '1' (when corresponding pin is connected to +V) or '0' (when corresponding pin is connected to Ground) on each parameter
 */
#define AT24MAC402_ADDR(A2, A1, A0)  ( (uint8_t)((((A2) & 0x01) << 3) | (((A1) & 0x01) << 2) | (((A0) & 0x01) << 1)) )

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// AT24MAC402 Specific Controller Registers
//********************************************************************************************************************

#define EUI48_OUI_LEN  ( 3 ) //!< Organizationally Unique Identifier (OUI) size is 3 bytes
#define EUI48_NIC_LEN  ( 3 ) //!< Network Interface Controller (NIC) size is 3 bytes
#define EUI48_LEN      ( EUI48_OUI_LEN + EUI48_NIC_LEN )

//! 48-bits Extended Unique Identifier
PACKITEM
typedef union __PACKED__ AT24MAC402_MAC_EUI48
{
    uint8_t EUI48[EUI48_LEN];
    struct
    {
      uint8_t OUI[EUI48_OUI_LEN]; //!< Organizationally Unique Identifier (OUI) data
      uint8_t NIC[EUI48_NIC_LEN]; //!< Network Interface Controller (NIC) data
    };
    struct
    {
        uint8_t IGcast: 1; //!< 0   - '0' => Unicast ; '1' => Multicast
        uint8_t ULaddr: 1; //!< 1   - '0' => Globally unique ; '1' => Locally Administered
        uint8_t       : 6; //!< 2-7
    } Bits;
} AT24MAC402_MAC_EUI48;
UNPACKITEM
ControlItemSize(AT24MAC402_MAC_EUI48, EUI48_LEN);

//-----------------------------------------------------------------------------



#define EUI64_OUI_LEN  ( 3 ) //!< Organizationally Unique Identifier (OUI) size is 3 bytes
#define EUI64_NIC_LEN  ( 5 ) //!< Network Interface Controller (NIC) size is 5 bytes
#define EUI64_LEN      ( EUI64_OUI_LEN + EUI64_NIC_LEN )

//! 64-bits Extended Unique Identifier
PACKITEM
typedef union __PACKED__ AT24MAC402_MAC_EUI64
{
    uint8_t EUI64[EUI64_LEN];
    struct
    {
      uint8_t OUI[EUI64_OUI_LEN]; //!< Organizationally Unique Identifier (OUI) data
      uint8_t NIC[EUI64_NIC_LEN]; //!< Network Interface Controller (NIC) data
    };
    struct
    {
        uint8_t IGcast: 1; //!< 0   - '0' => Unicast ; '1' => Multicast
        uint8_t ULaddr: 1; //!< 1   - '0' => Globally unique ; '1' => Locally Administered
        uint8_t       : 6; //!< 2-7
    } Bits;
} AT24MAC402_MAC_EUI64;
UNPACKITEM
ControlItemSize(AT24MAC402_MAC_EUI64, EUI64_LEN);

//-----------------------------------------------------------------------------

#define AT24MAC402_SERIALNUMBER_LEN  ( 128 / 8 ) //!< The Serial Number length is 128 bits thus 16 bytes

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// AT24MAC402 Driver API
//********************************************************************************************************************
typedef struct AT24MAC402 AT24MAC402; //! Typedef of AT24MAC402 device object structure



/*! @brief Interface function for driver initialization of the AT24MAC402
 *
 * This function will be called at driver initialization to configure the interface driver
 * @param[in] *pIntDev Is the AT24MAC402.InterfaceDevice of the device that call the interface initialization
 * @param[in] sclFreq Is the SCL frequency in Hz to set at the interface initialization
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*AT24MAC402_I2CInit_Func)(void *pIntDev, const uint32_t sclFreq);


/*! @brief Interface function for I2C transfer of the AT24MAC402
 *
 * This function will be called when the driver needs to transfer data over the I2C communication with the device
 * Can be a read of data or a transmit of data. It also indicate if it needs a start and/or a stop
 * @warning A I2CInit_Func() must be called before using this function
 * @param[in] *pIntDev Is the AT24MAC402.InterfaceDevice of the device that call the I2C transfer
 * @param[in] deviceAddress Is the device address on the bus (8-bits only). The LSB bit indicate if it is a I2C Read (bit at '1') or a I2C Write (bit at '0')
 * @param[in,out] *data Is a pointer to memory data to write in case of I2C Write, or where the data received will be stored in case of I2C Read (can be NULL if no data transfer other than chip address)
 * @param[in] byteCount Is the byte count to write over the I2C bus or the count of byte to read over the bus
 * @param[in] start Indicate if the transfer needs a start (in case of a new transfer) or restart (if the previous transfer have not been stopped)
 * @param[in] stop Indicate if the transfer needs a stop after the last byte sent
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*AT24MAC402_I2CTranfert_Func)(void *pIntDev, const uint8_t deviceAddress, uint8_t *data, size_t byteCount, bool start, bool stop);


/*! @brief Function that gives the current millisecond of the system to the driver
 *
 * This function will be called when the driver needs to get current millisecond
 * @return Returns the current millisecond of the system
 */
typedef uint32_t (*GetCurrentms_Func)(void);



//! AT24MAC402 device object structure
struct AT24MAC402
{
  void *UserDriverData;                       //!< Optional, can be used to store driver data or NULL

  //--- Interface clocks ---
  uint32_t I2C_ClockSpeed;                    //!< Clock frequency of the I2C interface in Hertz

  //--- Interface driver call functions ---
  void *InterfaceDevice;                      //!< This is the pointer that will be in the first parameter of all interface call functions
  AT24MAC402_I2CInit_Func fnI2C_Init;         //!< This function will be called at driver initialization to configure the interface driver
  AT24MAC402_I2CTranfert_Func fnI2C_Transfer; //!< This function will be called when the driver needs to transfer data over the I2C communication with the device

  //--- Time call function ---
  GetCurrentms_Func fnGetCurrentms;           //!< This function will be called when the driver need to get current millisecond

  //--- Device address ---
  uint8_t AddrA2A1A0;                         //!< Device configurable address A2, A1, and A0. You can use the macro AT24MAC402_ADDR() to help filling this parameter. Only these 3 lower bits are used: ....210. where 2 is A2, 1 is A1, 0 is A0, and '.' are fixed by device
};
//-----------------------------------------------------------------------------





/*! @brief AT24MAC402 initialization
 *
 * This function initializes the AT24MAC402 driver and call the initialization of the interface driver (SPI).
 * Next it checks parameters and configures the AT24MAC402
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_AT24MAC402(AT24MAC402 *pComp);



/*! @brief Is the AT24MAC402 device ready
 *
 * Poll the acknowledge from the AT24MAC402
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns 'true' if ready else 'false'
 */
bool AT24MAC402_IsReady(AT24MAC402 *pComp);

//********************************************************************************************************************



/*! @brief Read EEPROM data from the AT24MAC402 device
 *
 * This function reads data from the EEPROM area of a AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read (can be inside a page)
 * @param[in] *data Is where the data will be stored
 * @param[in] size Is the size of the data array to read
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_ReadEEPROMData(AT24MAC402 *pComp, uint8_t address, uint8_t* data, size_t size);



/*! @brief Write EEPROM data to the AT24MAC402 device
 *
 * This function writes data to the EEPROM area of a AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address where data will be written (can be inside a page)
 * @param[in] *data Is the data array to store
 * @param[in] size Is the size of the data array to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_WriteEEPROMData(AT24MAC402 *pComp, uint8_t address, const uint8_t* data, size_t size);

//********************************************************************************************************************



/*! @brief Read the EUI-48 register of the AT24MAC402 device
 *
 * This function get a EUI-48 data from the AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] *pEUI48 Is the pointed structure of the EUI-48 structure
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_GetEUI48(AT24MAC402 *pComp, AT24MAC402_MAC_EUI48 *pEUI48);


/*! @brief Generate a EUI-64 value from the EUI-48 of the AT24MAC402 device
 *
 * This function get a EUI-48 data from the AT24MAC402 device and convert it to a EUI-64
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] *pEUI64 Is the pointed structure of the EUI-64 structure
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_GenerateEUI64(AT24MAC402 *pComp, AT24MAC402_MAC_EUI64 *pEUI64);

//********************************************************************************************************************



/*! @brief Read the 128-bits Serial Number register of the AT24MAC402 device
 *
 * This function get the 16 bytes Serial Number from the AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] *dataSerialNum Is the pointed array where the Serial Numbre will be stored. This array have to be 16 bytes in length
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_Get128bitsSerialNumber(AT24MAC402 *pComp, uint8_t *dataSerialNum);

//********************************************************************************************************************



/*! @brief Set the Permanent Software Write Protection (PSWP) of the AT24MAC402 device
 *
 * The Software Write Protection write protects only the first-half of the array (00h - 7Fh) while the hardware write protection, via the WP pin, is used to protect the entire array
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_SetPermanentWriteProtection(AT24MAC402 *pComp);

//********************************************************************************************************************





//-----------------------------------------------------------------------------
#undef __PACKED__
#undef PACKITEM
#undef UNPACKITEM
#undef ControlItemSize
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------
#endif /* AT24MAC402_H_INC */