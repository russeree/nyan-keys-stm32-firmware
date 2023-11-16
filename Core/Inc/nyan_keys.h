#ifndef NYAN_KEYS_H
#define NYAN_KEYS_H

#include <stdint.h>
#include <main.h>

typedef enum {
    NYAN_KEYS_FAILURE,
    NYAN_KEYS_SUCCESS
} NyanKeysReturn;

typedef struct {
    bool key_read_inflight;
    uint8_t key_states[9];
} NyanKeys;

/**
 * @brief Init and selftest the Nyan Keys FPGA IP
 */
NyanKeysReturn NyanKeysInit(NyanKeys* keys);

/**
 * @brief Get the value of the keys stats on the nyan keys board.
 */
NyanKeysReturn NyanGetKeys(NyanKeys *keys);

#endif // NYAN_KEYS_H