/**
 * @file nyan_bitcoin.c
 * @brief opt-in Bitcoin mining functionality for the nayn keys keyboard.
 */

#include "nyan_bitcoin.h"

NyanBitcoinReturn NyanBitcoinInit(NyanBitcoin* bitcoin)
{
    bitcoin->enabled = true;
    bitcoin->active = false;
    bitcoin->current_nonce = 0; 
    
    return NYAN_BITCOIN_SUCCESS;
}