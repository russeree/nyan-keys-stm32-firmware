/**
 * @file nyan_keys.h
 * @brief Header file for Nyan Keys FPGA IP driver.
 */

#ifndef NYAN_KEYS_H
#define NYAN_KEYS_H

#include <stdint.h>
#include <main.h>

#define NUM_KEYS 61 /**< Number of key state bits to be read from FPGA over SPI */
#define NUM_HID_KEYS 60 /**< Number of keys that could have any impact on the HID descriptor - We remove the FN Keys */
#define NUM_BOOT_KEYS 6 /**< Number of keys that can occupy the boot bytes compatible section of nyan keys*/
#define NUM_HYBRID_KEYS (NUM_HID_KEYS - NUM_BOOT_KEYS) /**< Number of keys that can occupy the extended scancodes bytes section of nyan keys for NRKO*/
#define KEYS_WARMUP_READS 10000 /**< Number of spi read to perform to allow key states post init to settle */

/**
 * @enum NyanKeysReturn
 * @brief Return types for Nyan Keys functions.
 */
typedef enum {
    NYAN_KEYS_FAILURE, /**< Indicates a failure in the operation */
    NYAN_KEYS_SUCCESS  /**< Indicates success in the operation */
} NyanKeysReturn;

/**
 * @struct NyanKeyBoardDescriptor
 * @brief Structure for USB Report.
 */
typedef struct __attribute__((packed)) {
    uint8_t MODIFIER;                     /**< Modifier keys state */
    uint8_t RESERVED;                     /**< Reserved byte */
    uint8_t BOOTKEYCODE[NUM_BOOT_KEYS];   /**< Boot key codes */
    uint8_t EXTKEYCODE[NUM_HYBRID_KEYS];  /**< Extended key codes */
} NyanKeyBoardDescriptor;

typedef enum {
    ESC, TAB, CAPS, L_SHIFT, LEFT_CTRL, NUM_1, L_WIN, L_ALT, Q, A, Z,
    NUM_2, W, S, X, C, D, K, I, NUM_8, L_ANGLE_BRACKET, L, O, NUM_9, R_ANGLE_BRACKET,
    COLON, P, NUM_0, QUESTION_MARK, L_SQUARE_BRACKET, R_WIN, FN, MINUS, QUOTE, MENU, R_SQUARE_BRACKET, PLUS, R_SHIFT,
    ENTER, SLASH, BACKSPACE, R_CTRL, E, NUM_3, V, F, R, NUM_4, SPACE, B, G, T, NUM_5,
    H, Y, NUM_6, N, J, U, NUM_7, M
} Keyboard60PercentKeys;


/**
 * @struct NyanKeys
 * @brief Structure to hold the state of keys.
 */
typedef struct {
    volatile bool warmed_up;                                   /**< We allow for KEYS_WARMUP_READS before allowing the processing of keys */
    volatile uint32_t warm_up_reads;                           /**< A count of the number of reads to determine if the warmup flag can go true */
    volatile uint8_t key_states[((NUM_KEYS + 7) / 8) + 1];     /**< Array to hold the state of each key */
    volatile uint8_t key_states_prv[((NUM_KEYS + 7) / 8) + 1]; /**< Previous state of each key*/
    volatile bool super_key_disabled;                          /**< Disable Super Key (Win) key */
    uint8_t boot_byte_cnt;                                     /**< Track the number of boot compatible bytes used */
    uint8_t ext_byte_cnt;                                      /**M Track the number of extended report bytes used */
} NyanKeys;

/**
 * @brief Initializes and performs self-test on the Nyan Keys FPGA IP.
 * @param keys Pointer to NyanKeys structure.
 * @return NyanKeysReturn success or failure.
 */
NyanKeysReturn NyanKeysInit(NyanKeys* keys);

/**
 * @brief Retrieves the value of the key states from the Nyan Keys board.
 * @param keys Pointer to NyanKeys structure.
 * @return NyanKeysReturn success or failure.
 */
NyanKeysReturn NyanGetKeys(NyanKeys *keys);

/**
 * @brief Retrieves the state of a specific key.
 * @param keys Pointer to NyanKeys structure.
 * @param key The key index to check.
 * @return True if the key is pressed, false otherwise.
 */
bool NyanGetKeyState(NyanKeys *keys, int key);

/**
 * @brief Builds and publishes the key states to the USB Extended Descriptor.
 * @param keys Pointer to NyanKeys structure.
 * @param desc Pointer to NyanKeyBoardDescriptor structure.
 * @return NyanKeysReturn success or failure.
 */
NyanKeysReturn NyanBuildHidReportFromKeyStates(NyanKeys *keys, volatile NyanKeyBoardDescriptor *desc);

/**
 * @brief Saves the state of the Super Key disablement to the onboard eeprom
 * @param eeprom pointer to the EEPROM driver (extern)
 * @param disabled boolean representing if the super key is disabled or not. 
 * @return NyanKeysReturn success or failure. 
 */
NyanKeysReturn NyanKeysWriteSuperDisableEEPROM(Eeprom24xx* eeprom, bool disabled);

/**
 * @brief Reads the state of the Super Key disablement to the onboard eeprom
 * @param eeprom pointer to the EEPROM driver (extern)
 * @return Super key enabled or disabled
 */
bool NyanKeysReadSuperDisableEEPROM(Eeprom24xx* eeprom);

/**
 * @brief Performs the warmup tasks for the Nyan Keys keyboard FPGA key input.
 * @param keys Pointer to NyanKeys structure.
 * @return NyanKeysReturn success or failure.
 * @return NyanKeysReturn success or failure.
 */
void NyanWarmupIncrementor(NyanKeys *keys);

#endif // NYAN_KEYS_H