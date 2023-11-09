// nyan_os.h
#ifndef NYAN_OS_H
#define NYAN_OS_H

#include <stdint.h>
#include "24xx_eeprom.h"

#define _NYAN_CMD_BUF_LEN 128
#define _NYAN_EXE_CHAR '\n'

#define _NYAN_NUM_COMMANDS (sizeof(nyan_commands) / sizeof(nyan_commands[0]))

static const char* const nyan_commands[] = {
    "getinfo",
    "flash"
};

typedef enum {
    NOS_FAILURE,
    NOS_SUCCESS
} NyanReturn;

typedef enum {
    NOT_READY,
    READY,
    FPGA_BITSTREAM_LOAD
} NyanStates;

typedef enum {
    NYAN_EXE_GET_INFO,
    NYAN_EXE_FLASH_BITSTREAM,
    NYAN_EXE_COMMAND_NOT_SUPPORTED,
    NYAN_EXE_IDLE
} NyanExe;

typedef struct {
    uint8_t* p_array;
    size_t size;
} NyanString;

typedef struct {
    // pointers to drivers
    Eeprom24xx* eeprom; 
    // state management Inputs
    NyanStates state;
    NyanStates next_state;
    // state management Execution
    NyanExe exe;
    // os buffers
    uint8_t command_buffer[_NYAN_CMD_BUF_LEN];
    uint8_t command_buffer_pos;
    uint8_t* data_buffer;
    uint8_t data_buffer_pos;
    // constant to load
    char exe_char;
    // usb cdc interface to use - Currently only supports a single endpoint
    uint8_t cdc_ch;
    //Settings and preferences
    uint8_t send_welcome_screen;
    //OS Buffer
    uint8_t tx_inflight;
    NyanString tx_buffer;
} NyanOS;

/**
 * Initializes the NyanOS state structure, NyanOS will not function without this call.
 */
NyanReturn NyanOsInit(volatile NyanOS* nos, Eeprom24xx* eeprom);

/**
 * Handle needed state changes to Nyan
 */
NyanReturn NyanOsDecodeCommand(volatile NyanOS* nos);

/**
 * Checks the input buffer for validity when applied to the command_buffer 
 */
NyanReturn NyanAddInputBuffer(volatile NyanOS* nos, uint8_t *pbuf, uint32_t *Len);

/**
 * Displays the welcome message to the user on connection;
 */
NyanReturn NyanWelcomeDisplay(volatile NyanOS* nos);

/**
 * printf() NyanOS Edition 
 */
NyanReturn NyanPrint(volatile NyanOS* nos, char* data, size_t len);

/**
 * Free the contents of a NyanString
 */
void FreeNyanString(NyanString* nyanString);

/**
 * @brief Decodes, stages, and resets the current input buffer.
 * 
 * This function can be invoked at any time but is ideally called when the user presses the return carriage key.
 * It decodes the current input buffer into a command to be executed, with parameters stored locally. The
 * actual processing occurs in the main loop to ensure that the receive buffer is not occupied for an extended
 * period while processing the command, allowing interrupts and callbacks to take precedence.
 * 
 * @param nos A pointer to the volatile NyanOS struct instance which contains the input buffer and state
 *            information for decoding and processing the command.
 * @return NOS_SUCCESS if the command and parameters are successfully decoded; NOS_FAILURE otherwise.
 */
NyanReturn NyanDecode(volatile NyanOS* nos);

/**
 * @brief Executes, cleans up, and presents a new prompt line to the end user.
 * 
 * This function should be called when NOS has a non NOS_EXE_IDLE state inside of the exe var.
 * 
 * @param nos A pointer to the volatile NyanOS struct instance which contains the exe request and state
 *            information for executing and processing the command.
 * @return NOS_SUCCESS if the command and parameters are successfully decoded; NOS_FAILURE otherwise.
 */
NyanReturn NyanExecute(volatile NyanOS* nos);

/**
 * @brief Prints to the CDC terminal the current info and stats of the NyanOS and Keyboard.
 * 
 * @return  NOS_SUCCESS regardless. 
 */
NyanReturn NyanExeGetinfo(volatile NyanOS* nos);

#endif // NYAN_OS_H