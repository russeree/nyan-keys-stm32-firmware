/**
 * NyanKeys FPGA IP Driver (SPI2)
 * @author Reese Russell
 */

#include <stdlib.h>

#include "spi.h"
#include "nyan_keys.h"

static uint8_t keys_registers_addresses[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x00}; // We need the last dummy byte to extract the last byte from the keys IP

NyanKeysReturn NyanKeysInit(NyanKeys *keys)
{
    HAL_GPIO_WritePin(Keys_Slave_Select_GPIO_Port, Keys_Slave_Select_Pin, GPIO_PIN_SET);
    keys->key_read_inflight = false;
    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanGetKeys(NyanKeys *keys)
{
    //Send out the DMA and we will get the results back from the FPGA 
    if(!keys->key_read_inflight) {
        HAL_GPIO_WritePin(Keys_Slave_Select_GPIO_Port, Keys_Slave_Select_Pin, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive_DMA(&hspi2, &keys_registers_addresses[0], &keys->key_states[0], sizeof(keys_registers_addresses));
        keys->key_read_inflight = true;
    }

    return NYAN_KEYS_SUCCESS;
}