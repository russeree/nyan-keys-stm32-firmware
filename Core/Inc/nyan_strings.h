// nyan_strings.h
#ifndef NYAN_STRINGS
#define NYAN_STRINGS

#include <stdint.h>

extern const uint8_t nyan_keys_welcome_text[];
extern const uint8_t nyan_keys_newline[];
extern const uint8_t nyan_keys_path_text[];

// COMMAND: getinfo
extern const uint8_t nyan_keys_getinfo[];

extern const uint8_t nyan_keys_getinfo_owner[]; // Make sure to newline after this is filled out from the EEPROM

// COMMAND: set-owner
extern const uint8_t nyan_keys_set_owner_success[];

extern const uint8_t nyan_keys_unknown_command[];

// COMMAND: write-bitstream
extern const uint8_t nyan_keys_write_bitstream_info_start[];
extern const uint8_t nyan_keys_write_bitstream_info_success[];
extern const uint8_t nyan_keys_write_bitstream_error_size[];
extern const uint8_t nyan_keys_write_bitstream_error_size_tx_busy[];

// COMMAND: bitcoin-miner-set-version
extern const uint8_t nyan_keys_write_bitcoin_miner_block_version_success[];

//COMMAND: bitcoin-miner-set-prv-block-hash
extern const uint8_t nyan_keys_write_bitcoin_miner_prv_block_hash_success[];

#endif // NYAN_STRINGS