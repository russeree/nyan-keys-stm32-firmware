/**
 * @file iceuncompr.c
 * @brief Implementation file for ICE decompression utility.
 *
 * This file contains the implementation of functions required for the ICE decompression utility.
 */
#include "iceuncompr.h"

// Function to read a bit from the input stream
int read_bit(Iceuncompr *ice) {
    if (ice->read_bitcounter == 0) {
        ice->read_bitcounter = 8;
        if (ice->input_data_fh != NULL) {
            int ch = fgetc(ice->input_data_fh);
            if (ch != EOF) {
                ice->read_buffer = ch;
            } else {
                // Handle end of file or error
                return EOF;
            }
        } else {
            // Handle error if file handle is NULL
            return EOF;
        }
    }

    ice->read_bitcounter--;
    return (ice->read_buffer >> ice->read_bitcounter) & 1;
}

// Function to write a bit to the output buffer
void write_bit(Iceuncompr *ice, int value) {
    ice->write_bitcounter--;

    if (value)
        ice->write_buffer |= 1 << ice->write_bitcounter;

    if (ice->write_bitcounter == 0) {
        // Write the byte to the SPI channel
        HAL_SPI_Transmit(&hspi4, (uint8_t*)&ice->write_buffer, 1, 100);
        ice->write_bitcounter = 8;
        ice->write_buffer = 0;
    }
}

// Function to read an integer from the input stream
int read_int(Iceuncompr *ice, int bits) {
    int ret = 0;
    while (bits-- > 0) {
        int bit = read_bit(ice);
        if (bit == EOF) {
            return EOF;  // Handle EOF or error
        }
        if (bit) {
            ret |= 1 << bits;
        }
    }
    return ret;
}

// Function to write zeros to the output buffer
void write_zeros(Iceuncompr *ice, int bits) {
    while (bits-- > 0) {
        write_bit(ice, 0);
    }
}

// Main decompression function
int ice_uncompress(Iceuncompr *ice) {
    ice->read_bitcounter = 0;
    ice->write_bitcounter = 8;
    ice->write_buffer = 0;

    int magic1_ok = read_int(ice, 32) == 0x49434543;
    int magic2_ok = read_int(ice, 32) == 0x4f4d5052;

    if (!magic1_ok || !magic2_ok) {
        return 1;
    }

    while (1) {
        if (read_bit(ice)) {
            write_zeros(ice, read_int(ice, 2));
            write_bit(ice, 1);
        } else if (read_bit(ice)) {
            write_zeros(ice, read_int(ice, 5));
            write_bit(ice, 1);
        } else if (read_bit(ice)) {
            write_zeros(ice, read_int(ice, 8));
            write_bit(ice, 1);
        } else if (read_bit(ice)) {
            int n = read_int(ice, 6);
            while (n--) {
                write_bit(ice, read_bit(ice));
            }
            write_bit(ice, 1);
        } else if (read_bit(ice)) {
            write_zeros(ice, read_int(ice, 23));
            write_bit(ice, 1);
        } else {
            write_zeros(ice, read_int(ice, 23));
            break;
        }
    }

    return 0;
}

bool WriteUncomprBitstream(Iceuncompr *ice, uint8_t *input_data, uint32_t size)
{
    // Create a filehandle to work with for bitstream decompression.
    ice->input_data_fh = fmemopen(input_data, size, "rb");
    // Flush the contents of the buffer
    if(!ice->input_data_fh)
        return false;
    
    ice_uncompress(ice);

    return true;
}
