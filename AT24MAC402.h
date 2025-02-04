/*!*****************************************************************************
 * @file    AT24MAC402.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.2.1
 * @date    04/01/2025
 * @brief   AT24MAC402 driver
 * @details I2C-Compatible (2-wire) 2-Kbit (256kB x 8) Serial EEPROM with a
 * Factory-Programmed EUI-48™ Address plus an Embedded Unique 128-bit Serial Number
 * Follow datasheet AT24MAC402 Rev.8808E (Jan 2015)
 ******************************************************************************/
 /* @page License
 *
 * Copyright (c) 2020-2025 Fabien MAILLY
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
 * 1.2.1    Update error management to add context
 * 1.2.0    Add EEPROM genericness
 * 1.1.0    I2C interface rework
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef AT24MAC402_H_INC
#define AT24MAC402_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "I2C_Interface.h"
//-----------------------------------------------------------------------------
#ifdef USE_EEPROM_GENERICNESS
#  include "EEPROM.h"
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#  define __AT24MAC402_PACKED__
#  define AT24MAC402_PACKITEM    __pragma(pack(push, 1))
#  define AT24MAC402_UNPACKITEM  __pragma(pack(pop))
#else
#  define __AT24MAC402_PACKED__  __attribute__((packed))
#  define AT24MAC402_PACKITEM
#  define AT24MAC402_UNPACKITEM
#endif
//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#define AT24MAC402_CONTROL_ITEM_SIZE(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }

//-----------------------------------------------------------------------------

// Limits definitions
#define AT24MAC402_I2CCLOCK_MAX1V7      (  400000u ) //!< Max I2C clock frequency at 1.7V
#define AT24MAC402_I2CCLOCK_MAXSUP2V5   ( 1000000u ) //!< Max I2C clock frequency at 2.5V or more

// Device definitions
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

#ifdef USE_EEPROM_GENERICNESS
// AT24MAC402 EEPROM configurations
extern const EEPROM_Conf AT24MAC402_1V7_Conf, AT24MAC402_Conf;
#endif

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// AT24MAC402 Specific Controller Registers
//********************************************************************************************************************

#define EUI48_OUI_LEN  ( 3 ) //!< Organizationally Unique Identifier (OUI) size is 3 bytes
#define EUI48_NIC_LEN  ( 3 ) //!< Network Interface Controller (NIC) size is 3 bytes
#define EUI48_LEN      ( EUI48_OUI_LEN + EUI48_NIC_LEN )

//! 48-bits Extended Unique Identifier
AT24MAC402_PACKITEM
typedef union __AT24MAC402_PACKED__ AT24MAC402_MAC_EUI48
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
AT24MAC402_UNPACKITEM;
AT24MAC402_CONTROL_ITEM_SIZE(AT24MAC402_MAC_EUI48, EUI48_LEN);

//-----------------------------------------------------------------------------

#define EUI64_OUI_LEN  ( 3 ) //!< Organizationally Unique Identifier (OUI) size is 3 bytes
#define EUI64_NIC_LEN  ( 5 ) //!< Network Interface Controller (NIC) size is 5 bytes
#define EUI64_LEN      ( EUI64_OUI_LEN + EUI64_NIC_LEN )

//! 64-bits Extended Unique Identifier
AT24MAC402_PACKITEM
typedef union __AT24MAC402_PACKED__ AT24MAC402_MAC_EUI64
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
AT24MAC402_UNPACKITEM;
AT24MAC402_CONTROL_ITEM_SIZE(AT24MAC402_MAC_EUI64, EUI64_LEN);

//-----------------------------------------------------------------------------

#define AT24MAC402_SERIALNUMBER_LEN  ( 128 / 8 ) //!< The Serial Number length is 128 bits thus 16 bytes

//! 128-bits Serial Number
AT24MAC402_PACKITEM
typedef union __AT24MAC402_PACKED__ AT24MAC402_SN
{
  unsigned int Uint32[AT24MAC402_SERIALNUMBER_LEN / sizeof(unsigned int)];
  uint8_t      Bytes[AT24MAC402_SERIALNUMBER_LEN  / sizeof(uint8_t)];
} AT24MAC402_SN;
AT24MAC402_UNPACKITEM;
AT24MAC402_CONTROL_ITEM_SIZE(AT24MAC402_SN, AT24MAC402_SERIALNUMBER_LEN);

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// AT24MAC402 Driver API
//********************************************************************************************************************

typedef struct AT24MAC402 AT24MAC402; //! Typedef of AT24MAC402 device object structure

//-----------------------------------------------------------------------------

#if !defined(USE_EEPROM_GENERICNESS)
/*! @brief Function that gives the current millisecond of the system to the driver
 *
 * This function will be called when the driver needs to get current millisecond
 * @return Returns the current millisecond of the system
 */
typedef uint32_t (*GetCurrentms_Func)(void);
#endif

//-----------------------------------------------------------------------------

//! AT24MAC402 device object structure
struct AT24MAC402
{
#ifdef USE_EEPROM_GENERICNESS
  struct EEPROM Eeprom;
#else
  struct EEPROM_AT24MAC402
  {
    void *UserDriverData;             //!< Optional, can be used to store driver data or NULL

    //--- Interface driver call functions ---
# ifdef USE_DYNAMIC_INTERFACE
    I2C_Interface* I2C;               //!< This is the I2C_Interface descriptor pointer that will be used to communicate with the device
# else
    I2C_Interface I2C;                //!< This is the I2C_Interface descriptor that will be used to communicate with the device
# endif
    uint32_t I2CclockSpeed;           //!< Clock frequency of the I2C interface in Hertz

    //--- Time call function ---
    GetCurrentms_Func fnGetCurrentms; //!< This function will be called when the driver need to get current millisecond

    //--- Device address ---
    uint8_t AddrA2A1A0;               //!< Device configurable address A2, A1, and A0. You can use the macro AT24MAC402_ADDR() to help filling this parameter. Only these 3 lower bits are used: ....210. where 2 is A2, 1 is A1, 0 is A0, and '.' are fixed by device
  } Eeprom;
#endif // USE_EEPROM_GENERICNESS
};

//-----------------------------------------------------------------------------


/*! @brief AT24MAC402 initialization
 *
 * This function initializes the AT24MAC402 driver and call the initialization of the interface driver (I2C).
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

//-----------------------------------------------------------------------------


/*! @brief Read EEPROM data from the AT24MAC402 device
 *
 * This function reads data from the EEPROM area of a AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] address Is the address to read (can be inside a page)
 * @param[out] *data Is where the data will be stored
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

/*! @brief Wait the end of write to the AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_WaitEndOfWrite(AT24MAC402 *pComp);

//-----------------------------------------------------------------------------


/*! @brief Read the EUI-48 register of the AT24MAC402 device
 *
 * This function get a EUI-48 data from the AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *pEUI48 Is the pointed structure of the EUI-48 structure
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_GetEUI48(AT24MAC402 *pComp, AT24MAC402_MAC_EUI48 *pEUI48);

/*! @brief Generate a EUI-64 value from the EUI-48 of the AT24MAC402 device
 *
 * This function get a EUI-48 data from the AT24MAC402 device and convert it to a EUI-64
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *pEUI64 Is the pointed structure of the EUI-64 structure
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_GenerateEUI64(AT24MAC402 *pComp, AT24MAC402_MAC_EUI64 *pEUI64);

//-----------------------------------------------------------------------------


/*! @brief Read the 128-bits Serial Number register of the AT24MAC402 device
 *
 * This function get the 16 bytes Serial Number from the AT24MAC402 device
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *dataSerialNum Is the pointed array where the Serial Numbre will be stored. This array have to be 16 bytes in length
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_Get128bitsSerialNumber(AT24MAC402 *pComp, uint8_t *dataSerialNum);

//-----------------------------------------------------------------------------


/*! @brief Set the Permanent Software Write Protection (PSWP) of the AT24MAC402 device
 *
 * The Software Write Protection write protects only the first-half of the array (00h - 7Fh) while the hardware write protection, via the WP pin, is used to protect the entire array
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT AT24MAC402_SetPermanentWriteProtection(AT24MAC402 *pComp);

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* AT24MAC402_H_INC */