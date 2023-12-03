/**
 * @file nyan_bitcoin.c
 * @brief opt-in Bitcoin mining functionality for the nayn keys keyboard.
 */

#include "nyan_bitcoin.h"

NyanBitcoinReturn NyanBitcoinInit(NyanBitcoin* bitcoin)
{
    // Set the config variables
    bitcoin->enabled = true;
    bitcoin->active = false;
    bitcoin->current_nonce = 0;
    bitcoin->p_block_header = (uint8_t*)&bitcoin->block_header;

    // SHA256 Context Init
    sha256_init(&bitcoin->ctx);
    
    return NYAN_BITCOIN_SUCCESS;
}

NyanBitcoinReturn NyanBitcoinHashHeader(NyanBitcoin* bitcoin)
{
    sha256_init(&bitcoin->ctx);
    // 1st round of hashing of the header
    sha256_update(&bitcoin->ctx, bitcoin->p_block_header, _BITCOIN_BLOCK_HEADER_SIZE);
    sha256_final(&bitcoin->ctx, bitcoin->buf);
    // 2nd round of hashing the hash of the header
    sha256_update(&bitcoin->ctx, bitcoin->buf, SHA256_BLOCK_SIZE);
    sha256_final(&bitcoin->ctx, bitcoin->buf);

    if(bitcoin->buf[0] == 0xFF){
        bitcoin->current_nonce++;
    }
    bitcoin->current_nonce++;

    return NYAN_BITCOIN_SUCCESS;
}
