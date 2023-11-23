/**
 * @auth: Portland.HODL
 * An 24xx EEPROM library designed around DMA access
 */

#include <string.h>

#include "i2c.h"
#include "24xx_eeprom.h"


EepromReturn EepromInit(volatile Eeprom24xx* eeprom, bool a0, bool a1)   
{
    eeprom->a0 = a0;
    eeprom->a1 = a1;
    eeprom->tx_inflight = false;
    eeprom->rx_inflight = false;

    memset((void*)eeprom->tx_buf, 0, sizeof(eeprom->tx_buf));
    memset((void*)eeprom->rx_buf, 0, sizeof(eeprom->rx_buf));

    return EEPROM_SUCCESS;
}

EepromReturn EepromFlushTxBuff(volatile Eeprom24xx* eeprom)
{
    memset((void*)eeprom->tx_buf, 0, sizeof(eeprom->tx_buf));

    return EEPROM_SUCCESS;
}

uint8_t EepromCreateControlByte(volatile Eeprom24xx* eeprom, bool read, bool b0)
{
    uint8_t ctrl_byte = EEPROM_CTRL_MASK_CODE;

    if(read)
        ctrl_byte += EEPROM_CTRL_MASK_RW;
    if(eeprom->a0)
        ctrl_byte += EEPROM_CTRL_MASK_A0;
    if(eeprom->a1)
        ctrl_byte += EEPROM_CTRL_MASK_A1;
    if(b0)
        ctrl_byte += EEPROM_CTRL_MASK_B0;

    return ctrl_byte;
}

EepromReturn EepromWrite(volatile Eeprom24xx* eeprom, bool b0, short eeprom_address, size_t len)
{
    if(eeprom->tx_inflight)
        return EEPROM_FAILURE;
    else {
        // Place the TX inflight to prevent causing DMA collisions
        eeprom->tx_inflight = true;
        if (HAL_I2C_Mem_Write_DMA(&hi2c1,EepromCreateControlByte((Eeprom24xx*) eeprom, false, b0), eeprom_address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&eeprom->tx_buf[0], len) != HAL_OK)
            return EEPROM_FAILURE;
        // Reset the position since we just placed all of the data on the buffer to be sent.
    }

    return EEPROM_SUCCESS;
}

EepromReturn EepromRead(volatile Eeprom24xx* eeprom, bool b0, short eeprom_address, size_t len)
{
    if(eeprom->rx_inflight)
        return EEPROM_FAILURE;
    if(len >= EEPROM_DRIVER_RX_BUF_SZ)
        return EEPROM_FAILURE;
    else {
        eeprom->rx_inflight = true;
        if(HAL_I2C_Mem_Read_DMA(&hi2c1,EepromCreateControlByte((Eeprom24xx*)eeprom, true, b0), eeprom_address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)&eeprom->rx_buf[0], len) != HAL_OK)
            Error_Handler();
    }

    return EEPROM_SUCCESS;
}