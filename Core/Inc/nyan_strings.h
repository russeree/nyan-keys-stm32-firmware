// nyan_strings.h
#ifndef NYAN_STRINGS
#define NYAN_STRINGS

#include <stdint.h>

static uint8_t nyan_keys_welcome_text[] =
"Nyan Keys Operating System (NOS) V0.01\r\n"
"Made by Portland.HODL\r\n"
"\r\n";

static uint8_t nyan_keys_newline[] = "\r\n";
static uint8_t nyan_keys_path_text[] = "NyanOS \xF0\x9F\x90\xB1 > ";

// COMMAND: getinfo
static uint8_t nyan_keys_getinfo[] =
"Version: 0.01\r\n"
"Author: Portland.HODL\r\n"
"Built: 07/11/23\r\n";

static uint8_t nyan_keys_getinfo_owner[] = "Owner: "; // Make sure to newline after this is filled out from the EEPROM

//COMMAND: set-owner
static uint8_t nyan_keys_set_owner_success[] = "Nyan Keys owner has been successfully set\r\n";

static uint8_t nyan_keys_unknown_command[] = "Command not supported by NyanOS";

//COMMAND: write-bitstream
static uint8_t nyan_keys_write_bitstream_info_start[] = "ready\r\n";
static uint8_t nyan_keys_write_bitstream_info_success[] = "Nyan Keys FGPA bitstream has been written\r\n";
static uint8_t nyan_keys_write_bitstream_error_size[] = "Failed to parse bitstream length, size must be less than 65535 bytes.\r\n";
static uint8_t nyan_keys_write_bitstream_error_size_tx_busy[] = "Failed to write bitstream length, TX buffer is busy.\r\n";

#endif // NYAN_STRINGS_H