#ifndef NYAN_OS_H
#define NYAN_OS_H

#include <stdint.h>
#include <main.h>
#include "24xx_eeprom.h"
#include "nyan_eeprom_map.h"

#define _NYAN_CMD_MAX_ARGS 10
#define _NYAN_CMD_BUF_LEN 128
#define _NYAN_EXE_CHAR '\n'

#define _NYAN_NUM_COMMANDS (sizeof(nyan_commands) / sizeof(nyan_commands[0]))

static const char* const nyan_commands[] = {
    "getinfo",
    "write-bitstream",
    "set-owner"
};

typedef enum {
    NOS_FAILURE,
    NOS_SUCCESS
} NyanReturn;

typedef enum {
    NOT_READY,
    READY,
    DIRECT_BUFFER_ACCESS
} NyanStates;

typedef enum {
    NYAN_EXE_GET_INFO,
    NYAN_EXE_WRITE_BITSTREAM,
    NYAN_EXE_SET_OWNER,
    NYAN_EXE_COMMAND_NOT_SUPPORTED,
    NYAN_EXE_IDLE
} NyanExe;

typedef struct {
    uint8_t* p_array;
    size_t size;
} NyanString;

/**
 * NyanOS has a pretty decent sized bug where the buffer to send over USB-CDC greater than 129 TX gets disabled 
 */
typedef struct {
    Eeprom24xx* eeprom;
    NyanStates state;
    NyanStates next_state;
    NyanExe exe;
    uint8_t command_buffer[_NYAN_CMD_BUF_LEN + 1];
    uint8_t command_buffer_pos;
    uint8_t command_buffer_num_args;
    uint8_t* command_arg_buffer[_NYAN_CMD_MAX_ARGS];
    uint8_t* data_buffer;
    uint8_t data_buffer_pos;
    char exe_char;
    uint8_t cdc_ch;
    bool tx_inflight;
    NyanString tx_buffer;
    uint8_t send_welcome_screen;
    // Write
    uint32_t bytes_array_size;
    uint32_t bytes_received;
    uint8_t* bytes_array;
} NyanOS;

/**
 * @brief Initializes the NyanOS system.
 * @param nos Pointer to the NyanOS struct.
 * @param eeprom Pointer to the EEPROM driver struct.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanOsInit(volatile NyanOS* nos, Eeprom24xx* eeprom);

/**
 * @brief Decodes the currently buffered command in NyanOS.
 * @param nos Pointer to the NyanOS struct.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanOsDecodeCommand(volatile NyanOS* nos);

/**
 * @brief Adds data to the NyanOS input buffer.
 * @param nos Pointer to the NyanOS struct.
 * @param pbuf Pointer to the data buffer.
 * @param Len Length of the data to be added.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanAddInputBuffer(volatile NyanOS* nos, uint8_t *pbuf, uint32_t *Len);

/**
 * @brief Displays a welcome message to the user.
 * @param nos Pointer to the NyanOS struct.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanWelcomeDisplay(volatile NyanOS* nos);

/**
 * @brief Print function for NyanOS, similar to printf.
 * @param nos Pointer to the NyanOS struct.
 * @param data Pointer to the data to be printed.
 * @param len Length of the data to be printed.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanPrint(volatile NyanOS* nos, char* data, size_t len);

/**
 * @brief Decodes, stages, and resets the current input buffer in NyanOS.
 * @param nos Pointer to the NyanOS struct.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanDecode(volatile NyanOS* nos);

/**
 * @brief Executes the current command in the NyanOS and resets the execution state.
 * @param nos Pointer to the NyanOS struct.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanExecute(volatile NyanOS* nos);

/**
 * @brief Takes the first parameter and sets that as the owner of the Nyan Keys board.
 */
NyanReturn NyanExeSetOwner(volatile NyanOS* nos);

/**
 * @brief Store up to _NYAN_CMD_MAX_ARGS in the NyanOS class
 */
NyanReturn NyanDecodeArgs(volatile NyanOS* nos);

/**
 * @brief Prints current information and stats of the NyanOS and associated hardware.
 * @param nos Pointer to the NyanOS struct.
 * @return NyanReturn always returns NOS_SUCCESS.
 */
NyanReturn NyanExeGetinfo(volatile NyanOS* nos);

/**
 * @brief Writes the name of argument 1 to the owners name address slot.
 */
NyanReturn NyanExeSetOwner(volatile NyanOS* nos);

/**
 * @brief Write an FPGA Bitstream to the EEPROM in 128 byte chunks 
 */
NyanReturn NyanExeWriteFpgaBitstream(volatile NyanOS* nos);

/**
 * Clear and nullify the NyanOS command buffer
 */
void ClearNyanCommandBuffer(volatile NyanOS* nos);

/**
 * @brief Free the command args before creating new pointers
 */
void FreeNyanCommandArgs(volatile NyanOS* nos);

/**
 * @brief Frees the contents of a NyanString struct.
 * @param nyanString Pointer to the NyanString to be freed.
 */
void FreeNyanString(NyanString* nyanString);

#endif // NYAN_OS_H