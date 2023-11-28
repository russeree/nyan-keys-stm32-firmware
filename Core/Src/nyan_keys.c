/**
 * NyanKeys FPGA IP Driver (SPI2)
 * @author Reese Russell
 */

#include <stdlib.h>
#include <string.h>

#include "nyan_keys.h"
#include "spi.h"
#include "usb_hid_keys.h"

static uint8_t keys_registers_addresses[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x00}; // We need the last dummy byte to extract the last byte from the keys IP

bool NyanGetKeyState(NyanKeys *keys, int key)
{
    int byteIndex = key / 8;
    int bitIndex = key % 8;
    // We offset the byte index by 1 to account for the dummy first byte;
    return (keys->key_states[byteIndex + 1] & (1 << bitIndex)) != 0;
}

NyanKeysReturn NyanStuctAllocator(NyanKeys *keys, volatile NyanKeyBoardDescriptor *desc, uint8_t hid_scan_code)
{
    if(keys->boot_byte_cnt < NUM_BOOT_KEYS)
        desc->BOOTKEYCODE[keys->boot_byte_cnt++] = hid_scan_code;
    else if(keys->ext_byte_cnt < NUM_HYBRID_KEYS)
        desc->EXTKEYCODE[keys->boot_byte_cnt++] = hid_scan_code;
    else
        return NYAN_KEYS_FAILURE;
    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanKeysInit(NyanKeys *keys)
{
    HAL_GPIO_WritePin(Keys_Slave_Select_GPIO_Port, Keys_Slave_Select_Pin, GPIO_PIN_SET);

    keys->key_read_inflight = false;
    keys->warm_up_reads = 0;
    keys->warmed_up = false;

    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanGetKeys(NyanKeys *keys)
{
    //Send out the DMA and we will get the results back from the FPGA 
    if(!keys->key_read_inflight) {
        HAL_GPIO_WritePin(Keys_Slave_Select_GPIO_Port, Keys_Slave_Select_Pin, GPIO_PIN_RESET);
        keys->key_read_inflight = true;
        if (HAL_SPI_TransmitReceive_DMA(&hspi2, &keys_registers_addresses[0], &keys->key_states[0], sizeof(keys_registers_addresses)) != HAL_OK) {
            keys->key_read_inflight = false; 
        }
    }

    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanBuildHidReportFromKeyStates(NyanKeys *keys, volatile NyanKeyBoardDescriptor *desc)
{
    // Determine the warmup state of Nyan Keys FPGA outputs
    if (keys->warm_up_reads < KEYS_WARMUP_READS) {
        keys->warm_up_reads++;
        if (keys->warm_up_reads >= KEYS_WARMUP_READS) {
            keys->warmed_up = true;
        }
    }

    // Nullify the descriptor report
    memset((void*)desc, 0, sizeof(NyanKeyBoardDescriptor));

    // Set descriptor report counters to 0
    keys->boot_byte_cnt = 0;
    keys->ext_byte_cnt = 0;

    // Iterate through the keys and process their states - Perform actions on state
    for (Keyboard60PercentKeys key = ESC; key < NUM_KEYS; ++key) {
        if(!NyanGetKeyState(keys, key) && keys->warmed_up) {
            switch (key) {
                case ESC:
                    NyanStuctAllocator(keys, desc, KEY_ESC);
                    break;
                case TAB:
                    NyanStuctAllocator(keys, desc, KEY_TAB);
                    break;
                case CAPS:
                    NyanStuctAllocator(keys, desc, KEY_CAPSLOCK);
                    break;
                case L_SHIFT:
                    NyanStuctAllocator(keys, desc, KEY_LEFTSHIFT);
                    break;
                case LEFT_CTRL:
                    NyanStuctAllocator(keys, desc, KEY_LEFTCTRL);  
                    break;
                case NUM_1:
                    NyanStuctAllocator(keys, desc, KEY_1);
                    break;
                case L_WIN:
                    NyanStuctAllocator(keys, desc, KEY_LEFTMETA);
                    break;
                case L_ALT:
                    NyanStuctAllocator(keys, desc, KEY_LEFTALT);
                    break;
                case Q:
                    NyanStuctAllocator(keys, desc, KEY_Q);
                    break;
                case A:
                    NyanStuctAllocator(keys, desc, KEY_A);
                    break;
                case Z:
                    NyanStuctAllocator(keys, desc, KEY_Z);
                    break;
                case NUM_2:
                    NyanStuctAllocator(keys, desc, KEY_2);
                    break;
                case W:
                    NyanStuctAllocator(keys, desc, KEY_W);
                    break;
                case S:
                    NyanStuctAllocator(keys, desc, KEY_S);
                    break;
                case X:
                    NyanStuctAllocator(keys, desc, KEY_X);
                    break;
                case C:
                    NyanStuctAllocator(keys, desc, KEY_C);
                    break;
                case D:
                    NyanStuctAllocator(keys, desc, KEY_D);
                    break;
                case K:
                    NyanStuctAllocator(keys, desc, KEY_K);
                    break;
                case I:
                    NyanStuctAllocator(keys, desc, KEY_I);
                    break;
                case NUM_8:
                    NyanStuctAllocator(keys, desc, KEY_8);
                    break;
                case L_ANGLE_BRACKET:
                    NyanStuctAllocator(keys, desc, KEY_COMMA);
                    break;
                case L:
                    NyanStuctAllocator(keys, desc, KEY_L);
                    break;
                case O:
                    NyanStuctAllocator(keys, desc, KEY_O);
                    break;
                case NUM_9:
                    NyanStuctAllocator(keys, desc, KEY_9);
                    break;
                case R_ANGLE_BRACKET:
                    NyanStuctAllocator(keys, desc, KEY_DOT);
                    break;
                case COLON:
                    NyanStuctAllocator(keys, desc, KEY_SEMICOLON);
                    break;
                case P:
                    NyanStuctAllocator(keys, desc, KEY_P);
                    break;
                case NUM_0:
                    NyanStuctAllocator(keys, desc, KEY_0);
                    break;
                case QUESTION_MARK:
                    NyanStuctAllocator(keys, desc, KEY_SLASH);
                    break;
                case L_SQUARE_BRACKET:
                    NyanStuctAllocator(keys, desc, KEY_LEFTBRACE);
                    break;
                case R_WIN:
                    NyanStuctAllocator(keys, desc, KEY_RIGHTMETA);
                    break;
                case FN:
                    // This should never be called.
                    break;
                case MINUS:
                    NyanStuctAllocator(keys, desc, KEY_MINUS);
                    break;
                case QUOTE:
                    NyanStuctAllocator(keys, desc, KEY_APOSTROPHE);
                    break;
                case MENU:
                    NyanStuctAllocator(keys, desc, KEY_PROPS);
                    break;
                case R_SQUARE_BRACKET:
                    NyanStuctAllocator(keys, desc, KEY_RIGHTBRACE);
                    break;
                case PLUS:
                    NyanStuctAllocator(keys, desc, KEY_EQUAL);
                    break;
                case R_SHIFT:
                    NyanStuctAllocator(keys, desc, KEY_RIGHTSHIFT);
                    break;
                case ENTER:
                    NyanStuctAllocator(keys, desc, KEY_ENTER);
                    break;
                case SLASH:
                    NyanStuctAllocator(keys, desc, KEY_SLASH);
                    break;
                case BACKSPACE:
                    NyanStuctAllocator(keys, desc, KEY_BACKSPACE);
                    break;
                case R_CTRL:
                    NyanStuctAllocator(keys, desc, KEY_RIGHTCTRL);
                    break;
                case E:
                    NyanStuctAllocator(keys, desc, KEY_E);
                    break;
                case NUM_3:
                    NyanStuctAllocator(keys, desc, KEY_3);
                    break;
                case V:
                    NyanStuctAllocator(keys, desc, KEY_V);
                    break;
                case F:
                    NyanStuctAllocator(keys, desc, KEY_F);
                    break;
                case R:
                    NyanStuctAllocator(keys, desc, KEY_R);
                    break;
                case NUM_4:
                    NyanStuctAllocator(keys, desc, KEY_4);
                    break;
                case SPACE:
                    NyanStuctAllocator(keys, desc, KEY_SPACE);
                    break;
                case B:
                    NyanStuctAllocator(keys, desc, KEY_B);
                    break;
                case G:
                    NyanStuctAllocator(keys, desc, KEY_G);
                    break;
                case T:
                    NyanStuctAllocator(keys, desc, KEY_T);
                    break;
                case NUM_5:
                    NyanStuctAllocator(keys, desc, KEY_5);
                    break;
                case H:
                    NyanStuctAllocator(keys, desc, KEY_H);
                    break;
                case Y:
                    NyanStuctAllocator(keys, desc, KEY_Y);
                    break;
                case NUM_6:
                    NyanStuctAllocator(keys, desc, KEY_6);
                    break;
                case N:
                    NyanStuctAllocator(keys, desc, KEY_N);
                    break;
                case J:  
                    NyanStuctAllocator(keys, desc, KEY_J);
                    break;
                case U:
                    NyanStuctAllocator(keys, desc, KEY_U);
                    break;
                case NUM_7:
                    NyanStuctAllocator(keys, desc, KEY_7);
                    break;
                case M:
                    NyanStuctAllocator(keys, desc, KEY_M);
                    break;
                default:
                    // Handle any other case
                    break;
            }
        }
    }

    return NYAN_KEYS_SUCCESS;
}