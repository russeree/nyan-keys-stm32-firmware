// nyan_strings.c

#include <nyan_strings.h>

uint8_t nyan_keys_welcome_text[] =
"Nyan Keys Operating System (NOS) V0.01\r\n"
"Made by Portland.HODL\r\n"
"\r\n";

uint8_t nyan_keys_newline[] = "\r\n";
uint8_t nyan_keys_path_text[] = "NyanOS \xF0\x9F\x90\xB1 > ";

// COMMAND: getinfo
uint8_t nyan_keys_getinfo[] =
"Version: 0.01\r\n"
"Author: Portland.HODL\r\n"
"Built: 07/11/23\r\n";

uint8_t nyan_keys_getinfo_owner[] = "Owner: "; // Make sure to newline after this is filled out from the EEPROM

//COMMAND: set-owner
uint8_t nyan_keys_set_owner_success[] = "Nyan Keys owner has been successfully set\r\n";

uint8_t nyan_keys_unknown_command[] = "Command not supported by NyanOS";

//COMMAND: write-bitstream
uint8_t nyan_keys_write_bitstream_info_start[] = "ready\r\n";
uint8_t nyan_keys_write_bitstream_info_success[] = "Nyan Keys FGPA bitstream has been written\r\n";
uint8_t nyan_keys_write_bitstream_error_size[] = "Failed to parse bitstream length, size must be less than 65535 bytes.\r\n";
uint8_t nyan_keys_write_bitstream_error_size_tx_busy[] = "Failed to write bitstream length, TX buffer is busy.\r\n";

//COMMAND: bitcoin-miner-set-version
uint8_t nyan_keys_write_bitcoin_miner_block_version_success[] = "Nyan BitCoin Miner block template version set successfully.\r\n";