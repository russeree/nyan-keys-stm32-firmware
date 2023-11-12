/**
 * NyanOS (NOS) v0.01
 * Reese Russell
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "main.h"
#include "24xx_eeprom.h"
#include "nyan_os.h"
#include "nyan_strings.h"
#include "usbd_cdc_acm_if.h"

NyanReturn NyanOsInit(volatile NyanOS* nos, Eeprom24xx* eeprom)
{
    // Set the operational state
    nos->state = READY;
    nos->exe = NYAN_EXE_IDLE;

    // Default init the OS vars
    nos->command_buffer_num_args = 0;
    nos->command_buffer_pos = 0;
    nos->cdc_ch = _NYAN_CDC_CHANNEL;

    // EEPROM Driver access
    nos->eeprom = eeprom;

    // Manual Setting of the memory because of the volatile qualifier.
    ClearNyanCommandBuffer(nos);

    // Set the arg pointer to a zero value
    for (int i = 0; i < _NYAN_CMD_MAX_ARGS; ++i) {
        nos->command_arg_buffer[i] = NULL;
    }

    // Output Buffer Creation
    nos->tx_buffer.p_array = NULL;
    nos->tx_buffer.size = 0;
    
    return NOS_SUCCESS;
}

NyanReturn NyanWelcomeDisplay(volatile NyanOS *nos)
{
    if(nos->send_welcome_screen && nos->send_welcome_screen_guard == 0) {
        nos->send_welcome_screen = 0x00;
        nos->send_welcome_screen_guard++;
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
                    ClearNyanCommandBuffer(nos);
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
        case DIRECT_BUFFER_ACCESS: {
            // In this state all signals are written directly to the buffer until the buffer is full
            for(uint32_t idx = 0; idx < *Len; ++idx) {
                if(nos->bytes_received < nos->bytes_array_size)
                    nos->bytes_array[nos->bytes_received++] = pbuf[idx];
            }
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

NyanReturn NyanDecode(volatile NyanOS* nos)
{
    // First set the nos state to idle
    nos->exe = NYAN_EXE_IDLE;
    // Iterate over the available commands for a match to the input buffer
    for (uint8_t cmd_idx = 0; cmd_idx < _NYAN_NUM_COMMANDS; ++cmd_idx) {
        size_t command_len = strlen(nyan_commands[cmd_idx]);
        // Make sure we compare only the relevant part of the buffer1
        if (_NYAN_CMD_BUF_LEN >= command_len && memcmp((const char*)nos->command_buffer, nyan_commands[cmd_idx], command_len) == 0) {
            NyanDecodeArgs(nos);
            // We have found a match, set the current evaluation cursor to the command length, args will get parsed next.
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
        case NYAN_EXE_SET_OWNER:
            NyanExeSetOwner(nos);
            NyanPrint(nos, (char*)&nyan_keys_set_owner_success[0], sizeof(nyan_keys_set_owner_success));
            NyanPrint(nos, (char*)&nyan_keys_path_text[0], sizeof(nyan_keys_path_text));
            nos->exe = NYAN_EXE_IDLE;
            return NOS_SUCCESS;
        case NYAN_EXE_WRITE_BITSTREAM :
            NyanExeWriteFpgaBitstream(nos);
            NyanPrint(nos, (char*)&nyan_keys_path_text[0], sizeof(nyan_keys_path_text));
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

NyanReturn NyanDecodeArgs(volatile NyanOS* nos)
{
    if (!nos) {
        return NOS_FAILURE;
    }

    // Destroy any previous allocated arguments
    FreeNyanCommandArgs(nos);

    nos->command_buffer[_NYAN_CMD_BUF_LEN] = '\0';
    const char *delimiter = " ";
    char *token = strtok((char *)nos->command_buffer, delimiter);

    int arg_count = 0;
    while (token != NULL) {
        if (arg_count < _NYAN_CMD_MAX_ARGS) {
            size_t tokenLength = strlen(token);
            nos->command_arg_buffer[arg_count] = malloc(tokenLength + 1); // Allocate memory for the argument
            if (nos->command_arg_buffer[arg_count] == NULL) {
                // Free any previously allocated memory
                for (int i = 0; i < arg_count; ++i) {
                    free(nos->command_arg_buffer[i]);
                }
                return NOS_FAILURE;
            }
            strcpy((char *)nos->command_arg_buffer[arg_count], token);
            arg_count++;
        }
        token = strtok(NULL, delimiter);
    }

    nos->command_buffer_num_args = arg_count;

    // Nullify the command buffer
    memset((void*)nos->command_buffer, 0, sizeof(nos->command_buffer));

    return NOS_SUCCESS;
}

NyanReturn NyanExeGetinfo(volatile NyanOS* nos)
{
    // We need to fetch the owners name from the eeprom
    EepromRead(nos->eeprom, false, ADDR_BOARD_OWNER, SIZE_BOARD_OWNER);

    // This has to be polling until callbacks are improved
    while(nos->eeprom->rx_inflight){}

    // Ensure data from EEPROM is null-terminated
    nos->eeprom->rx_buf[SIZE_BOARD_OWNER - 1] = '\0';

    char owner[SIZE_BOARD_OWNER];
    strncpy(owner, (const char *)nos->eeprom->rx_buf, SIZE_BOARD_OWNER);

    NyanPrint(nos, (char*)&nyan_keys_getinfo[0], sizeof(nyan_keys_getinfo));
    NyanPrint(nos, (char*)&nyan_keys_getinfo_owner[0], sizeof(nyan_keys_getinfo_owner));
    NyanPrint(nos, owner, strlen(owner));
    NyanPrint(nos, (char*)&nyan_keys_newline[0], sizeof(nyan_keys_newline));

    return NOS_SUCCESS;
}

NyanReturn NyanExeSetOwner(volatile NyanOS* nos) 
{
    if (!nos) {
        return NOS_FAILURE; // Handle null pointer
    }

    if (nos->command_buffer_num_args < 2) {
        return NOS_FAILURE; // Not enough args
    }

    size_t total_chars = 0;

    // Calculate total length needed, including spaces between arguments
    for (int i = 1; i < nos->command_buffer_num_args && nos->command_arg_buffer[i] != NULL; i++) {
        total_chars += strlen((char *)nos->command_arg_buffer[i]) + 1; // +1 for space or null terminator
    }

    // Since the size cant exceed 63 chars with null terminator 
    if (total_chars > SIZE_BOARD_OWNER - nos->command_buffer_num_args - 1  || total_chars == 0) {
        return NOS_FAILURE; // Would overflow memory boundaries 
    }

    // Allocate memory for the new owner name
    char* owners_name = (char*)malloc(SIZE_BOARD_OWNER);
    if (!owners_name) {
        return NOS_FAILURE; // Handle allocation failure
    }

    // Zero out the SIZE_BOARD_OWNER bytes
    for (int i = 0; i < SIZE_BOARD_OWNER; ++i) {
        owners_name[i] = '\0';
    }

    // Concatenate arguments with spaces
    char* current_pos = owners_name;
    for (int i = 1; i < nos->command_buffer_num_args && nos->command_arg_buffer[i] != NULL; i++) {
        strcpy(current_pos, (char *)nos->command_arg_buffer[i]);
        current_pos += strlen((char *)nos->command_arg_buffer[i]);

        // Add a space after each argument, except the last one
        if (i < nos->command_buffer_num_args - 1 && nos->command_arg_buffer[i + 1] != NULL) {
            *current_pos = ' ';
            current_pos++;
        }
    }

    // First lets clear out the TX buff
    if(EepromFlushTxBuff(nos->eeprom) != EEPROM_SUCCESS){
        return NOS_FAILURE;
    }

    // Second lets copy our new buffer over to the EEPROM driver
    for (int i = 0; i < SIZE_BOARD_OWNER; ++i){
        nos->eeprom->tx_buf[i]  = owners_name[i];
    }

    // Free up the allocated memory
    free(owners_name);

    // Write the name to the eeprom, the delay exists to ensure the write, later the callback can be used to free
    EepromWrite(nos->eeprom, false, ADDR_BOARD_OWNER, SIZE_BOARD_OWNER);

    return NOS_SUCCESS;
}

NyanReturn NyanExeWriteFpgaBitstream(volatile NyanOS* nos) {
    nos->bytes_array_size = 0;
    // Now we need to convert the arg 1 into an int - skip arg 0 because that is the command.
    nos->bytes_array_size = atoi((char *)nos->command_arg_buffer[1]);
    // Safety the size of the buffer to ensure that it doesn't exceed the size of a block 
    if(nos->bytes_array_size  > 0xFFFF) {
        //Print Error, Clear buffer, Set ready state. 
        nos->bytes_array_size = 0;
        NyanPrint(nos, (char*)&nyan_keys_write_bitstream_error_size[0], sizeof(nyan_keys_write_bitstream_error_size));
        return NOS_FAILURE;
    }
    // Write the length of the bitstream we are accepting to the EEPROM - 16 bytes
    unsigned int size_array[4] = { 0x00, 0x00, 0x00, nos->bytes_array_size };
    if(nos->eeprom->tx_inflight) {
        //Print Error, Clear buffer, Set ready state. 
        NyanPrint(nos, (char*)&nyan_keys_write_bitstream_error_size_tx_busy[0], sizeof(nyan_keys_write_bitstream_error_size_tx_busy));
        return NOS_FAILURE;
    }
    // Copy the data to the EEPROM buffer for writing
    for(short i = 0; i < sizeof(size_array); ++i) {
        nos->eeprom->tx_buf[i] = ((uint8_t*)size_array)[i];
    }
    // Write the data to the eeprom - wait for the write to complete since this is DMA and order matters
    EepromWrite(nos->eeprom, true, ADDR_BOARD_OWNER, SIZE_BOARD_OWNER);
    // Print the ready to accept bytes confirmation message
    NyanPrint(nos, (char*)&nyan_keys_write_bitstream_info_start[0], sizeof(nyan_keys_write_bitstream_info_start));
    // Lets allocate some memory to save this bitstream we are importing
    nos->bytes_array = (uint8_t*)malloc(nos->bytes_array_size * sizeof(uint8_t));
    if(nos->bytes_array_size == 0)
        return NOS_FAILURE;
    // Flush the input buffer to prepare for the FPGA bitstream - This is safe because we have create a buffer
    nos->state = DIRECT_BUFFER_ACCESS;

    while(nos->bytes_received != nos->bytes_array_size){
        // During this period we just loop until the byte array is full
        // The user can exit this loop by just filling the buffer up for now.
        // Enabling am abort sequence would be a next step
    }

    // Calculate the number iterations 
    unsigned int r = nos->bytes_array_size%EEPROM_DRIVER_TX_BUF_SZ;
    unsigned int q = nos->bytes_array_size/EEPROM_DRIVER_TX_BUF_SZ;
    if(r > 0)
        ++q;
    if (q == 0)
        return NOS_FAILURE;
    // Fill and iterate over pages in the eeprom, write, wait ...
    for(unsigned short page = 0; page < q; ++page) {
        // Flush the transmit buffer
        EepromFlushTxBuff(nos->eeprom);
        for(int byte = 0; byte > EEPROM_DRIVER_TX_BUF_SZ; byte++){
            nos->eeprom->tx_buf[byte] = nos->bytes_array[EEPROM_DRIVER_TX_BUF_SZ * page + byte];
        }
        EepromWrite(nos->eeprom, false, ADDR_FPGA_BITSTREAM + EEPROM_DRIVER_TX_BUF_SZ * page, 128);
        while(nos->eeprom->tx_inflight){
            // Wait while the TX is in flight as to avoid bogus writes;
        }
    }
    // Perform function cleanup maintenance
    nos->bytes_array_size = 0;
    nos->bytes_received = 0;
    free(nos->bytes_array);
    nos->bytes_array = NULL;
    nos->state = READY;
    //NyanPrint(nos, (char*)&nyan_keys_write_bitstream_info_success[0], sizeof(nyan_keys_write_bitstream_info_success));
    return NOS_SUCCESS;
}

void FreeNyanCommandArgs(volatile NyanOS* nos)
{
    if (!nos) {
        return;
    }

    for (int i = 0; i < _NYAN_CMD_MAX_ARGS; i++) {
        if (nos->command_arg_buffer[i] != NULL) {
            free(nos->command_arg_buffer[i]); 
            nos->command_arg_buffer[i] = NULL;
        }
    }
}

void FreeNyanString(NyanString* nyanString)
{
    // Clear the memory contents
    for (uint32_t i = 0; i < nyanString->size; ++i) {
        nyanString->p_array[i] = 0x00;
    }
    // Free up the pointer and memory
    free(nyanString->p_array);
    nyanString->p_array = NULL;
    nyanString->size = 0;
}

void ClearNyanCommandBuffer(volatile NyanOS* nos){
    nos->command_buffer_pos = 0;
    memset((void*)nos->command_buffer, 0, sizeof(nos->command_buffer));
};