#ifndef NYAN_OS_H
#define NYAN_OS_H

#include <stdint.h>
#include <main.h>
#include "24xx_eeprom.h"
#include "lattice_ice_hx.h"
#include "nyan_bitcoin.h"
#include "nyan_eeprom_map.h"

#define _NYAN_WELCOME_GUARD_TIME 30 // Currently a multiple of TIM7 Period (.777 seconds)
#define _NYAN_CDC_CHANNEL 0
#define _NYAN_CDC_RX_BUF_SZ 512
#define _NYAN_CDC_TX_MAX_LEN 128
#define _NYAN_CMD_MAX_ARGS 10
#define _NYAN_CMD_BUF_LEN 128

#define _NYAN_EXE_CHAR '\n'

#define _NYAN_NUM_COMMANDS (sizeof(nyan_commands) / sizeof(nyan_commands[0]))

extern Eeprom24xx nos_eeprom;    // 24xx Based EEPROM
extern LatticeIceHX nos_fpga;    // Lattice ICE40HX4k FPGA driver access
extern NyanBitcoin nyan_bitcoin; // Nyan Keys Background Bitcoin Miner

static const char* const nyan_commands[] = {
    "getinfo",
    "write-bitstream",
    "set-owner",
    "bitcoin-miner-set-version"
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
    NYAN_EXE_BITCOIN_MINER_SET_BLOCK_VERSION,
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
    // Configuration
    bool send_welcome_screen; // This inits to false; Don't reset in init sequence.
    uint8_t send_welcome_screen_guard; // Guards against double welcome screens with a longer timer value.
    char exe_char;
    // Drivers
    Eeprom24xx *eeprom;
    NyanBitcoin *nyan_bitcoin;
    // State
    NyanStates state;
    NyanExe exe;
    bool exe_in_progress;
    uint8_t command_buffer[_NYAN_CMD_BUF_LEN + 1];
    uint8_t command_buffer_pos;
    uint8_t command_buffer_num_args;
    uint8_t cdc_ch;
    // USB CDC Transmission Buffer
    bool tx_inflight; // This inits to false; Don't reset in init sequence.
    bool tx_bulk_transfer_in_progress;
    uint8_t tx_chunks_solid; // Complete _NYAN_CDC_TX_MAX_LEN sized chunks
    uint8_t tx_chunks_partial_bytes; // Number of bytes of a partial chunk that exist. 
    uint8_t tx_chunk;
    NyanString tx_buffer;
    // DIRECT_BUFFER_ACCESS USB CDC Rx Buffer - Some of these vars need to be renamed
    uint8_t rx_buffer[_NYAN_CDC_RX_BUF_SZ];
    uint8_t rx_buffer_sz;
    uint32_t bytes_received;
    uint32_t bytes_array_size;
    uint8_t* bytes_array;
    // Allocated buffers
    uint8_t* command_arg_buffer[_NYAN_CMD_MAX_ARGS];
    uint8_t* data_buffer;
} NyanOS;

/**
 * @brief Initializes the NyanOS system.
 * @param nos Pointer to the NyanOS struct.
 * @param eeprom Pointer to the EEPROM driver struct.
 * @return NyanReturn indicating success or failure.
 */
NyanReturn NyanOsInit(volatile NyanOS* nos);

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
 * NyanOS Long 128+ character buffer printing support.
 * @param nos Pointer to the NyanOS struct.
 * @return NyanReturn indicating success or failure.
 */

NyanReturn NyanCdcTX(volatile NyanOS* nos);
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
 * @brief Writes the Bitcoin block header version into the NyanOS structure.
 *
 * This function is responsible for handling the write operation of the Bitcoin block header version
 * into the NyanOS structure. It checks the current state of the NyanOS and if it is in
 * DIRECT_BUFFER_ACCESS state, the function fails. Otherwise, it allocates a 4-byte buffer to store
 * the block version, waits for the buffer to be filled with data, and then copies this data into
 * the Bitcoin block header version field in the NyanOS structure.
 *
 * @param nos A pointer to the volatile NyanOS structure.
 *
 * @return NyanReturn An enum value indicating the success or failure of the operation.
 *         Returns NOS_SUCCESS on successful write operation, and NOS_FAILURE on failure
 *         (e.g., if already in DIRECT_BUFFER_ACCESS mode or memory allocation fails).
 *
 * @note This function puts the NyanOS into DIRECT_BUFFER_ACCESS state during operation and
 *       back to READY state upon completion.
 */
NyanReturn NyanExeWriteBitcoinBlockHeaderVersion(volatile NyanOS* nos);

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