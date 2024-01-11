/**
 * NyanKeys FPGA IP Driver (SPI2)
 * @author Reese Russell
 */

#include <stdlib.h>
#include <string.h>

#include "24xx_eeprom.h"
#include "nyan_eeprom_map.h"
#include "nyan_keys.h"
#include "spi.h"
#include "usb_hid_keys.h"

extern Eeprom24xx nos_eeprom;

static uint8_t keys_registers_addresses[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x00}; // We need the last dummy byte to extract the last byte from the keys IP

inline bool NyanGetKeyState(NyanKeys *keys, int key)
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
    // We only have one device on the bus so we will just leave SS Low
    HAL_GPIO_WritePin(Keys_Slave_Select_GPIO_Port, Keys_Slave_Select_Pin, GPIO_PIN_RESET);

    keys->warm_up_reads = 0;
    keys->warmed_up = false;
    keys->super_key_disabled = NyanKeysReadSuperDisableEEPROM(&nos_eeprom);

    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanGetKeys(NyanKeys *keys)
{
    // Send out the DMA and we will get the results back from the FPGA 
    if(HAL_SPI_TransmitReceive_DMA(&hspi2, &keys_registers_addresses[0], (uint8_t*)&keys->key_states[0], sizeof(keys_registers_addresses)) != HAL_OK) {
        return NYAN_KEYS_FAILURE;
    }
    
    return NYAN_KEYS_SUCCESS;
}

NyanKeysReturn NyanKeysWriteSuperDisableEEPROM(Eeprom24xx* eeprom, bool disabled)
{   
    // First lets clear out the TX buff
    if(EepromFlushTxBuff(eeprom) != EEPROM_SUCCESS){
        return NYAN_KEYS_FAILURE;
    }
    // Second lets copy our new buffer over to the EEPROM driver
    eeprom->tx_buf[0] = (uint8_t)disabled;
    // Write the state to the eeprom. Don't overwrite the full 16 bytes of slot one because we will use it for other things.
    EepromWrite(eeprom, false, ADDR_RESERVED_0, 1);

    return NYAN_KEYS_SUCCESS;
}

bool NyanKeysReadSuperDisableEEPROM(Eeprom24xx* eeprom)
{   // Fetch the state of the super key disablement from the eeprom
    EepromRead(eeprom, false, ADDR_RESERVED_0, 1);
    while(eeprom->rx_inflight){}
    return (bool)(eeprom->rx_buf[0] == 0x00 ? false : true);
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
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_GRAVE : KEY_ESC);
                    break;
                case TAB:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_TAB : KEY_TAB);
                    break;
                case CAPS:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_CAPSLOCK : KEY_CAPSLOCK);
                    break;
                case L_SHIFT:
                    desc->MODIFIER |= KEY_MOD_LSHIFT;
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFTSHIFT : KEY_LEFTSHIFT);
                    break;
                case LEFT_CTRL:
                    desc->MODIFIER |= KEY_MOD_LCTRL;
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFTCTRL : KEY_LEFTCTRL);
                    break;
                case NUM_1:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F1 : KEY_1);
                    break;
                case L_WIN:
                    /*** Handle the disablement of the windows logo (super) for gaming ***/
                    if(alt_fn) {
                        keys->super_key_disabled = !keys->super_key_disabled;
                        NyanKeysWriteSuperDisableEEPROM(&nos_eeprom, keys->super_key_disabled);
                    } if (keys->super_key_disabled) {
                        //If the super key is disabled we do nothing on press
                    } else {
                        desc->MODIFIER |= KEY_MOD_LMETA;
                        NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFTMETA : KEY_LEFTMETA);
                    }  
                    break;
                case L_ALT:
                    desc->MODIFIER |= KEY_MOD_LALT;
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFTALT : KEY_LEFTALT);
                    break;
                case Q:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_Q : KEY_Q);
                    break;
                case A:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFT : KEY_A);
                    break;
                case Z:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_Z : KEY_Z);
                    break;
                case NUM_2:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F2 : KEY_2);
                    break;
                case W:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_UP : KEY_W);
                    break;
                case S:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_DOWN : KEY_S);
                    break;
                case X:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_X : KEY_X);
                    break;
                case C:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_C : KEY_C);
                    break;
                case D:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_RIGHT : KEY_D);
                    break;
                case K:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_HOME : KEY_K);
                    break;
                case I:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_SYSRQ : KEY_I);
                    break;
                case NUM_8:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F8 : KEY_8);
                    break;
                case L_ANGLE_BRACKET:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_END : KEY_COMMA);
                    break;
                case L:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_PAGEUP : KEY_L);
                    break;
                case O:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_SCROLLLOCK : KEY_O);
                    break;
                case NUM_9:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F9 : KEY_9);
                    break;
                case R_ANGLE_BRACKET:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_PAGEDOWN : KEY_DOT);
                    break;
                case COLON:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFT : KEY_SEMICOLON);
                    break;
                case P:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_PAUSE : KEY_P);
                    break;
                case NUM_0:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F10 : KEY_0);
                    break;
                case QUESTION_MARK:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_DOWN : KEY_SLASH);
                    break;
                case L_SQUARE_BRACKET:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_UP : KEY_LEFTBRACE);
                    break;
                case R_WIN:
                    /*** Handle the disablement of the windows logo (super) for gaming ***/
                    if(alt_fn) {
                        keys->super_key_disabled = !keys->super_key_disabled;
                        NyanKeysWriteSuperDisableEEPROM(&nos_eeprom, keys->super_key_disabled);
                    } if (keys->super_key_disabled) {
                        //If the super key is disabled we do nothing on press
                    } else {
                        desc->MODIFIER |= KEY_MOD_LMETA;
                        NyanStuctAllocator(keys, desc, alt_fn ? KEY_RIGHTMETA : KEY_RIGHTMETA);
                    }
                    break;
                case FN:
                    // This should never be called.
                    break;
                case MINUS:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F11 : KEY_MINUS);
                    break;
                case QUOTE:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_RIGHT : KEY_APOSTROPHE);
                    break;
                case MENU:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_COMPOSE : KEY_COMPOSE);;
                    break;
                case R_SQUARE_BRACKET:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_RIGHTBRACE : KEY_RIGHTBRACE);
                    break;
                case PLUS:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F12 : KEY_EQUAL);
                    break;
                case R_SHIFT:
                    desc->MODIFIER |= KEY_MOD_RSHIFT;
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_RIGHTSHIFT : KEY_RIGHTSHIFT);
                    break;
                case ENTER:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_ENTER : KEY_ENTER);
                    break;
                case SLASH:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_INSERT : KEY_BACKSLASH);
                    break;
                case BACKSPACE:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_DELETE : KEY_BACKSPACE);
                    break;
                case R_CTRL:
                    desc->MODIFIER |= KEY_MOD_RCTRL;
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_RIGHTCTRL : KEY_RIGHTCTRL);
                    break;
                case E:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_E : KEY_E);
                    break;
                case NUM_3:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F3 : KEY_3);
                    break;
                case V:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_V : KEY_V);
                    break;
                case F:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F : KEY_F);
                    break;
                case R:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_R : KEY_R);
                    break;
                case NUM_4:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F4 : KEY_4);
                    break;
                case SPACE:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_SPACE : KEY_SPACE);
                    break;
                case G:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_G : KEY_G);
                    break;
                case B:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_B : KEY_B);
                    break;
                case T:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_T : KEY_T);
                    break;
                case NUM_5:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F5 : KEY_5);
                    break;
                case H:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_HOME : KEY_H);
                    break;
                case Y:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_Y : KEY_Y);
                    break;
                case NUM_6:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F6 : KEY_6);
                    break;
                case N:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_VOLUMEUP : KEY_N);
                    break;
                case J:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_LEFT : KEY_J);
                    break;
                case U:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_PAGEUP : KEY_U);
                    break;
                case NUM_7:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_F7 : KEY_7);
                    break;
                case M:
                    NyanStuctAllocator(keys, desc, alt_fn ? KEY_MUTE : KEY_M);
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