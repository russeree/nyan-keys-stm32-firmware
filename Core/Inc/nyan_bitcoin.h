#ifndef NYAN_BITCOIN_H
#define NYAN_BITCOIN_H

#include <stdint.h>
#include <main.h>

/** 
 * @enum NyanBitcoinReturn
 * @brief Return values for NyanBitcoin functions.
 */
typedef enum {
    NYAN_BITCOIN_FAILURE, /**< Indicates a failure in a NyanBitcoin function. */
    NYAN_BITCOIN_SUCCESS  /**< Indicates success. */
} NyanBitcoinReturn;

/** 
 * @struct NyanBitcoinHeader
 * @brief Structure representing the header of a Nyan Bitcoin block.
 *
 * This structure contains details about the block template such as version, previous block header hash, 
 * Merkle root hash, timestamp, bits, and nonce. Nyan GUI can modify these values
 */
typedef struct {
    uint8_t version[4];              /**< Version of the block. */
    uint8_t prv_block_header_hash[32]; /**< Hash of the previous block header. */
    uint8_t merkle_root_hash[32];     /**< Merkle root hash. */
    uint8_t timestamp[4];             /**< Timestamp of the block. */
    uint8_t n_bits[4];                /**< Difficulty target of the block. */
    uint8_t nonce[4];                 /**< Nonce used for mining. */
} NyanBitcoinHeader;

/** 
 * @struct NyanBitcoin
 * @brief Structure representing a Nyan Bitcoin mining context.
 *
 * This structure contains the current state of a mining operation, including whether mining 
 * is enabled, the current block header, and the current nonce value.
 */
typedef struct {
    bool enabled;                     /**< Flag indicating if mining is enabled. */
    NyanBitcoinHeader block_header;   /**< Current block header being mined. */
    uint32_t current_nonce;           /**< Current nonce value in mining. */
} NyanBitcoin;

/**
 * @brief Initializes and enables bitcoin mining on the Nyan Keys OS.
 * 
 * @param bitcoin Pointer to a NyanBitcoin structure.
 * @return NyanBitcoinReturn Indicates whether the initialization was successful.
 */
NyanBitcoinReturn NyanBitcoinInit(NyanBitcoin* bitcoin);

#endif // NYAN_BITCOIN_H