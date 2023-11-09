/**
 * @file 24xx_eeprom.h
 * @brief EEPROM control byte utility for 24XX EEPROM series.
 *
 * Provides utility functions for working with 24XX series EEPROM devices.
 * This includes initialization, read and write operations using DMA.
 */

#ifndef _24XX_EEPROM_H
#define _24XX_EEPROM_H

#include <stdint.h>
#include <stdbool.h>

// Definitions
#define EEPROM_DRIVER_TX_BUF_SZ  128     /**< Size of the transmit buffer. */
#define EEPROM_DRIVER_RX_BUF_SZ  1024    /**< Size of the receive buffer. */
#define EEPROM_CTRL_MASK_RW   0x01       /**< Read/Write control bit mask. */
#define EEPROM_CTRL_MASK_A0   0x02       /**< A0 address bit mask. */
#define EEPROM_CTRL_MASK_A1   0x04       /**< A1 address bit mask. */
#define EEPROM_CTRL_MASK_B0   0x08       /**< B0 address bit mask. */
#define EEPROM_CTRL_MASK_CODE 0xA0       /**< Control byte mask code for EEPROM. */
#define EEPROM_PAGE_SIZE      0x7F       /**< Maximum number of bytes in a single TX. */
#define EEPROM_MAX_ADDR_SIZE  0xFFFF     /**< Max address value for a single block (2^16-1). */

// Enumerations
typedef enum {
    EEPROM_FAILURE, /**< Indicates a failure in EEPROM operation. */
    EEPROM_SUCCESS  /**< Indicates a successful EEPROM operation. */
} EepromReturn;

// Structures
typedef struct {
    bool a0;
    bool a1;
    bool tx_inflight;
    bool rx_inflight; 
    uint8_t tx_buf[EEPROM_DRIVER_TX_BUF_SZ]; /**< Transmit buffer. */
    uint8_t rx_buf[EEPROM_DRIVER_RX_BUF_SZ]; /**< Receive buffer. */
} Eeprom24xx;

// Function declarations

/**
 * @brief Initializes an EEPROM instance.
 *
 * Sets up the EEPROM structure and initializes internal states.
 *
 * @param eeprom Pointer to the Eeprom24xx structure.
 * @param a0 State of address line A0.
 * @param a1 State of address line A1.
 * @return EepromReturn Status of the initialization (success or failure).
 */
EepromReturn EepromInit(Eeprom24xx* eeprom, bool a0, bool a1);

/**
 * @brief Flushes the transmit buffer of the EEPROM.
 *
 * Clears the contents of the transmit buffer in preparation for new data.
 *
 * @param eeprom Pointer to the Eeprom24xx structure.
 * @return EepromReturn Status of the flush operation (success or failure).
 */
EepromReturn EepromFlushTxBuff(Eeprom24xx* eeprom);

/**
 * @brief Writes data to the EEPROM using the DMA interface.
 *
 * This function initiates a write operation to the EEPROM at a specified address.
 *
 * @param eeprom Pointer to the Eeprom24xx structure.
 * @param b0 State of address bit B0 for addressing.
 * @param eeprom_address The EEPROM address where data writing should begin.
 * @param len Number of bytes to read.
 * @return EepromReturn Status of the write operation (success or failure).
 */
EepromReturn EepromWrite(Eeprom24xx* eeprom, bool b0, short eeprom_address, size_t len);

/**
 * @brief Reads data from the EEPROM using the DMA interface.
 *
 * Reads a specified number of bytes from the EEPROM starting from a given address.
 *
 * @param eeprom Pointer to the Eeprom24xx structure.
 * @param b0 State of address bit B0 for addressing.
 * @param eeprom_address The EEPROM address from where to start reading.
 * @param len Number of bytes to read.
 * @return EepromReturn Status of the read operation (success or failure).
 */
EepromReturn EepromRead(Eeprom24xx* eeprom, bool b0, short eeprom_address, size_t len);

#endif // _24XX_EEPROM_H