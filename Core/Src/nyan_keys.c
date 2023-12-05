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
        if (HAL_SPI_TransmitReceive_DMA(&hspi2, &keys_registers_addresses[0], (uint8_t*)&keys->key_states[0], sizeof(keys_registers_addresses)) != HAL_OK) {
            keys->key_read_inflight = false; 
        }
    }

    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanBuildHidReportFromKeyStates(NyanKeys *keys, volatile NyanKeyBoardDescriptor *desc)
{
    // Nullify the descriptor report
    if(keys->warmed_up)
        memset((void*)desc, 0, sizeof(NyanKeyBoardDescriptor));

    // Set descriptor report counters to 0
    keys->boot_byte_cnt = 0;
    keys->ext_byte_cnt = 0;

    // Get the state of the alternate function key
    bool alt_fn = !NyanGetKeyState(keys, FN);

    // Iterate through the keys and process their states - Perform actions on state
    for (Keyboard60PercentKeys key = ESC; key < NUM_KEYS; ++key) {
         if(!NyanGetKeyState(keys, key) && keys->warmed_up) {
            switch (key) {
                case ESC:
                    if(alt_fn) {
                        NyanStuctAllocator(keys, desc, KEY_GRAVE);
                    }
                    else
                        NyanStuctAllocator(keys, desc, KEY_ESC);
                    break;
                case TAB:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_TAB);
                    break;
                case CAPS:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_CAPSLOCK);
                    break;
                case L_SHIFT:
                    desc->MODIFIER |= KEY_MOD_LSHIFT;
                    if(alt_fn) {

                    }
                    else {
                        NyanStuctAllocator(keys, desc, KEY_LEFTSHIFT);
                    }
                    break;
                case LEFT_CTRL:
                    desc->MODIFIER |= KEY_MOD_LCTRL;
                    if(alt_fn) {}
                    else {
                        NyanStuctAllocator(keys, desc, KEY_LEFTCTRL);  
                    }
                    break;
                case NUM_1:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F1);
                    else
                        NyanStuctAllocator(keys, desc, KEY_1);
                    break;
                case L_WIN:
                    desc->MODIFIER |= KEY_MOD_LMETA;
                    if(alt_fn) {}
                    else {
                        NyanStuctAllocator(keys, desc, KEY_LEFTMETA);
                    }
                    break;
                case L_ALT:
                    desc->MODIFIER |= KEY_MOD_LALT;
                    if(alt_fn) {}
                    else {
                        NyanStuctAllocator(keys, desc, KEY_LEFTALT);
                    }
                    break;
                case Q:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_Q);
                    break;
                case A:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_LEFT);
                    else
                        NyanStuctAllocator(keys, desc, KEY_A);
                    break;
                case Z:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_Z);
                    break;
                case NUM_2:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F2);
                    else
                        NyanStuctAllocator(keys, desc, KEY_2);
                    break;
                case W:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_UP);
                    else
                        NyanStuctAllocator(keys, desc, KEY_W);
                    break;
                case S:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_DOWN);
                    else
                        NyanStuctAllocator(keys, desc, KEY_S);
                    break;
                case X:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_X);
                    break;
                case C:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_C);
                    break;
                case D:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_RIGHT);
                    else
                        NyanStuctAllocator(keys, desc, KEY_D);
                    break;
                case K:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_HOME);
                    else
                        NyanStuctAllocator(keys, desc, KEY_K);
                    break;
                case I:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_SYSRQ);
                    else
                        NyanStuctAllocator(keys, desc, KEY_I);
                    break;
                case NUM_8:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F8);
                    else
                        NyanStuctAllocator(keys, desc, KEY_8);
                    break;
                case L_ANGLE_BRACKET:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_END);
                    else
                        NyanStuctAllocator(keys, desc, KEY_COMMA);
                    break;
                case L:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_PAGEUP);
                    else
                        NyanStuctAllocator(keys, desc, KEY_L);
                    break;
                case O:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_SCROLLLOCK);
                    else
                        NyanStuctAllocator(keys, desc, KEY_O);
                    break;
                case NUM_9:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F9);
                    else
                        NyanStuctAllocator(keys, desc, KEY_9);
                    break;
                case R_ANGLE_BRACKET:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_PAGEDOWN);
                    else
                        NyanStuctAllocator(keys, desc, KEY_DOT);
                    break;
                case COLON:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_LEFT);
                    else
                        NyanStuctAllocator(keys, desc, KEY_SEMICOLON);
                    break;
                case P:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_PAUSE);
                    else 
                        NyanStuctAllocator(keys, desc, KEY_P);
                    break;
                case NUM_0:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F10);
                    else
                        NyanStuctAllocator(keys, desc, KEY_0);
                    break;
                case QUESTION_MARK:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_DOWN);
                    else
                        NyanStuctAllocator(keys, desc, KEY_SLASH);
                    break;
                case L_SQUARE_BRACKET:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_UP);
                    else
                        NyanStuctAllocator(keys, desc, KEY_LEFTBRACE);
                    break;
                case R_WIN:
                    desc->MODIFIER |= KEY_MOD_LMETA;
                    if(alt_fn) {}
                    else {
                        NyanStuctAllocator(keys, desc, KEY_RIGHTMETA);
                    }
                    break;
                case FN:
                    // This should never be called.
                    break;
                case MINUS:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F11);
                    else
                        NyanStuctAllocator(keys, desc, KEY_MINUS);
                    break;
                case QUOTE:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_RIGHT);
                    else
                        NyanStuctAllocator(keys, desc, KEY_APOSTROPHE);
                    break;
                case MENU:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_PROPS);
                    break;
                case R_SQUARE_BRACKET:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_RIGHTBRACE);
                    break;
                case PLUS:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F12);
                    else
                        NyanStuctAllocator(keys, desc, KEY_EQUAL);
                    break;
                case R_SHIFT:
                    desc->MODIFIER |= KEY_MOD_RSHIFT;
                    if(alt_fn) {}
                    else {
                        NyanStuctAllocator(keys, desc, KEY_RIGHTSHIFT);
                    }
                    break;
                case ENTER:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_ENTER);
                    break;
                case SLASH:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_INSERT);
                    else
                        NyanStuctAllocator(keys, desc, KEY_BACKSLASH);
                    break;
                case BACKSPACE:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_DELETE);
                    else
                        NyanStuctAllocator(keys, desc, KEY_BACKSPACE);
                    break;
                case R_CTRL:
                    desc->MODIFIER |= KEY_MOD_RCTRL;
                    if(alt_fn) {}
                    else {
                        NyanStuctAllocator(keys, desc, KEY_RIGHTCTRL);
                    }
                    break;
                case E:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_E);
                    break;
                case NUM_3:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F3);
                    else
                        NyanStuctAllocator(keys, desc, KEY_3);
                    break;
                case V:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_V);
                    break;
                case F:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_F);
                    break;
                case R:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_R);
                    break;
                case NUM_4:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F4);
                    else
                        NyanStuctAllocator(keys, desc, KEY_4);
                    break;
                case SPACE:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_SPACE);
                    break;
                case B:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_MEDIA_VOLUMEDOWN);
                    else
                        NyanStuctAllocator(keys, desc, KEY_B);
                    break;
                case G:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_G);
                    break;
                case T:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_T);
                    break;
                case NUM_5:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F5);
                    else
                        NyanStuctAllocator(keys, desc, KEY_5);
                    break;
                case H:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_HOME);
                    else
                        NyanStuctAllocator(keys, desc, KEY_H);
                    break;
                case Y:
                    if(alt_fn) {}
                    else
                        NyanStuctAllocator(keys, desc, KEY_Y);
                    break;
                case NUM_6:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F6);
                    else
                        NyanStuctAllocator(keys, desc, KEY_6);
                    break;
                case N:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_MEDIA_VOLUMEUP);
                    else
                        NyanStuctAllocator(keys, desc, KEY_N);
                    break;
                case J:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_LEFT);
                    else
                        NyanStuctAllocator(keys, desc, KEY_J);
                    break;
                case U:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_PAGEUP);
                    else
                        NyanStuctAllocator(keys, desc, KEY_U);
                    break;
                case NUM_7:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_F7);
                    else
                        NyanStuctAllocator(keys, desc, KEY_7);
                    break;
                case M:
                    if(alt_fn)
                        NyanStuctAllocator(keys, desc, KEY_MEDIA_MUTE);
                    else
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

void NyanWarmupIncrementor(NyanKeys *keys)
{
    // Determine the warmup state of Nyan Keys FPGA outputs
    if (keys->warm_up_reads < KEYS_WARMUP_READS) {
        keys->warm_up_reads++;
        if (keys->warm_up_reads >= KEYS_WARMUP_READS) {
            keys->warmed_up = true;
        }
    }
}