#ifndef NYAN_BITCOIN_H
#define NYAN_BITCOIN_H

#include <stdint.h>
#include <main.h>

#include "nyan_sha256.h"

#define _BITCOIN_BLOCK_HEADER_SIZE 80
#define _SHA

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
 * Merkle root hash, timestamp, bits, and nonce. Nyan GUI can modify these values. We pack this to enable
 * a contigious hash through all members.
 */
typedef struct __attribute__((packed)) {
    uint8_t version[4];                /**< Version of the block. */
    uint8_t prv_block_header_hash[32]; /**< Hash of the previous block header. */
    uint8_t merkle_root_hash[32];      /**< Merkle root hash. */
    uint8_t timestamp[4];              /**< Timestamp of the block. */
    uint8_t n_bits[4];                 /**< Difficulty target of the block. */
    uint8_t nonce[4];                  /**< Nonce used for mining. */
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
    bool active;                      /**< Actively mining, block header components submitted*/
    NyanBitcoinHeader block_header;   /**< Current block header being mined. */
    uint8_t* p_block_header;          /**< A uint8_t pointer to the block header data to be hashed */ 
    uint32_t current_nonce;           /**< Current nonce value in mining. */
    BYTE buf[SHA256_BLOCK_SIZE];      /**< SHA256 Output Buffer */
    SHA256_CTX ctx;                   /**< Bitcoin Miner SHA256 Context */
} NyanBitcoin;

/**
 * @brief Initializes and enables bitcoin mining on the Nyan Keys OS.
 * 
 * @param bitcoin Pointer to a NyanBitcoin structure.
 * @return NyanBitcoinReturn Indicates whether the initialization was successful.
 */
NyanBitcoinReturn NyanBitcoinInit(NyanBitcoin* bitcoin);

NyanBitcoinReturn NyanBitcoinHashHeader(NyanBitcoin* bitcoin);

#endif // NYAN_BITCOIN_H