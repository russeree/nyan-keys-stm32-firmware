/**
 * NyanOS (NOS) v0.01
 * Reese Russell
 * MIT License
 */

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "nyan_os.h"
#include "nyan_strings.h"
#include "usbd_cdc_acm_if.h"

NyanReturn NyanOsInit(volatile NyanOS* nos, Eeprom24xx* eeprom)
{
    // Set the operational state
    nos->state = READY;
    nos->next_state = READY;
    nos->exe = NYAN_EXE_IDLE;
    nos->exe_char = _NYAN_EXE_CHAR;
    nos->command_buffer_pos = 0;
    nos->data_buffer_pos = 0;
    nos->cdc_ch = 0;
    nos->send_welcome_screen = 1;
    nos->tx_inflight = 0;
    nos->eeprom = eeprom;

    // Manual Setting of the memory because of the volatile qualifier.
    memset((void*)nos->command_buffer, 0, sizeof(nos->command_buffer));

    // Output Buffer Creation
    nos->tx_buffer.p_array = NULL;
    nos->tx_buffer.size = 0;
    
    return NOS_SUCCESS;
}

NyanReturn NyanWelcomeDisplay(volatile NyanOS *nos) {
    if(nos->send_welcome_screen) {
        nos->send_welcome_screen = 0x00;
        NyanPrint(nos, (char*)&nyan_keys_welcome_text[0], sizeof(nyan_keys_welcome_text));
        NyanPrint(nos, (char*)&nyan_keys_path_text[0], sizeof(nyan_keys_path_text));
    }

    return NOS_SUCCESS;
};

NyanReturn NyanAddInputBuffer(volatile NyanOS *nos, uint8_t *pbuf, uint32_t *Len)
{
    const char del_char = 0x7F;
    const char backspace_char = 0x08;
    const char carriage_return = '\r';
    const char line_feed = '\n';
    // 0. Check which state we are in.
    switch(nos->state){
        case READY: {
            for(uint32_t idx = 0; idx < *Len; ++idx) {
                if((pbuf[idx] == backspace_char ||  pbuf[idx] == del_char) && nos->command_buffer_pos > 0) {
                    // Handle backspace
                    uint8_t backspace_seq[3] = {backspace_char, ' ', backspace_char};
                    NyanPrint(nos, (char*)&backspace_seq[0], sizeof(backspace_seq));
                    --nos->command_buffer_pos;
                } else if(pbuf[idx] == line_feed || pbuf[idx] == carriage_return) {
                    // Handle the action of executing a command by pressing enter
                    nos->command_buffer_pos = 0;
                    NyanDecode(nos);
                    memset((void*)nos->command_buffer, 0, sizeof(nos->command_buffer));
                    NyanPrint(nos, (char*)&nyan_keys_newline[0], sizeof(nyan_keys_newline));                    
                    break;
                } else if(nos->command_buffer_pos >= _NYAN_CMD_BUF_LEN - 1) {
                    // Handle out of command buffer space on next char
                } else if(pbuf[idx] >= 0x20 && pbuf[idx] <= 0x7E) {
                    nos->command_buffer[nos->command_buffer_pos++] = pbuf[idx];
                    NyanPrint(nos, (char*)pbuf, (size_t)*Len);
                }
            }
            break;
        }
        default: 
            break;
    }
    return NOS_SUCCESS;
}

NyanReturn NyanPrint(volatile NyanOS *nos, char* data, size_t len)
{
    if (!nos || !data) {
        // Handle null pointers being passed in
        return NOS_FAILURE;
    }

    if (nos->tx_buffer.p_array == NULL) {
        // Since the pointer is null we need to create a new one to hold our new data!
        nos->tx_buffer.p_array = (uint8_t *)malloc(len);
        if (nos->tx_buffer.p_array == NULL) {
            // Handle failed memory allocation
            return NOS_FAILURE;
        }
        nos->tx_buffer.size = len;
        memcpy(nos->tx_buffer.p_array, data, len); // Copy the data into the buffer
    } else {
        // The pointer is not null, so we realloc and then add the contents of data to it
        uint8_t *new_buffer = (uint8_t *)realloc(nos->tx_buffer.p_array, nos->tx_buffer.size + len);
        if (new_buffer == NULL) {
            // Handle failed memory reallocation
            return NOS_FAILURE;
        }
        nos->tx_buffer.p_array = new_buffer;
        memcpy(nos->tx_buffer.p_array + nos->tx_buffer.size, data, len); // Append the new data
        nos->tx_buffer.size += len; // Increase the size to reflect the new total size
    }

    return NOS_SUCCESS;
}

void FreeNyanString(NyanString* nyanString) {
    // Clear the memory contents
    for (uint32_t i = 0; i < nyanString->size; ++i) {
        nyanString->p_array[i] = 0x00;
    }
    // Free up the pointer and memory
    free(nyanString->p_array);
    nyanString->p_array = NULL;
    nyanString->size = 0;
}

NyanReturn NyanDecode(volatile NyanOS* nos) {
    uint8_t decode_position = 0;
    // First set the nos state to idle
    nos->exe = NYAN_EXE_IDLE;
    // Iterate over the available commands for a match to the input buffer
    for (uint8_t cmd_idx = 0; cmd_idx < _NYAN_NUM_COMMANDS; ++cmd_idx) {
        size_t command_len = strlen(nyan_commands[cmd_idx]);
        // Make sure we compare only the relevant part of the buffer1
        if (_NYAN_CMD_BUF_LEN >= command_len && memcmp((const char*)nos->command_buffer, nyan_commands[cmd_idx], command_len) == 0) {
            // We have found a match, set the current evaluation cursor to the command length, args will get parsed next.
            decode_position = command_len;
            nos->exe = (NyanExe)cmd_idx;
            break;
        } else {
            nos->exe = NYAN_EXE_COMMAND_NOT_SUPPORTED;
        }
    }

    // If no command is matched, return some indication (e.g., NULL or a specific error string)
    return NOS_SUCCESS;
}

NyanReturn NyanExecute(volatile NyanOS* nos) {
    switch(nos->exe) {
        case NYAN_EXE_GET_INFO :
            NyanExeGetinfo(nos);
            NyanPrint(nos, (char*)&nyan_keys_newline[0], sizeof(nyan_keys_newline));
            NyanPrint(nos, (char*)&nyan_keys_path_text[0], sizeof(nyan_keys_path_text));
            nos->exe = NYAN_EXE_IDLE;
            return NOS_SUCCESS;
        case NYAN_EXE_FLASH_BITSTREAM :
            nos->exe = NYAN_EXE_IDLE;
            return NOS_SUCCESS;
        case NYAN_EXE_IDLE :
            return NOS_SUCCESS;
        case NYAN_EXE_COMMAND_NOT_SUPPORTED :
            NyanPrint(nos, (char*)&nyan_keys_unknown_command[0], sizeof(nyan_keys_unknown_command));
            NyanPrint(nos, (char*)&nyan_keys_newline[0], sizeof(nyan_keys_newline));
            NyanPrint(nos, (char*)&nyan_keys_path_text[0], sizeof(nyan_keys_path_text));
            nos->exe = NYAN_EXE_IDLE;
            return NOS_SUCCESS;
        default:
            // The execution state is out of bounds correct this.
            nos->exe = NYAN_EXE_IDLE;
            return NOS_FAILURE;
    }
}

NyanReturn NyanExeGetinfo(volatile NyanOS* nos) {
    NyanPrint(nos, (char*)&nyan_keys_getinfo[0], sizeof(nyan_keys_getinfo));
}