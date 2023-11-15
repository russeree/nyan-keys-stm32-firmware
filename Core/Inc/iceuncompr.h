/**
 * @file iceuncompr.h
 * @brief Header file for ICE decompression utility.
 *
 * This file provides the necessary declarations for the ICE decompression utility.
 */

#ifndef ICEUNCOMPR_H
#define ICEUNCOMPR_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "spi.h"

/**
 * @struct Iceuncompr
 * @brief Structure to hold all necessary data for ICE decompression.
 *
 * This structure contains pointers to input and output data buffers, their sizes,
 * and variables for tracking the state of the decompression process.
 */
typedef struct {
    FILE *input_data_fh;       ///< Input data file handle.
    uint8_t *output_data;      ///< Pointer to the output data buffer. Ensure to free after use.
    uint32_t input_data_size;  ///< Size of the input data buffer.
    uint32_t read_bitcounter;  ///< Counter for bits read from the input buffer.
    uint32_t read_buffer;      ///< Buffer for storing the currently read bits.
    uint32_t write_bitcounter; ///< Counter for bits written to the output buffer.
    uint8_t write_buffer;     ///< Buffer for storing the currently written bits.
    uint32_t write_position;   ///< Current position in the output buffer.
} Iceuncompr;

/**
 * Reads a bit from the input stream.
 * @param ice Pointer to Iceuncompr structure.
 * @return The read bit, or EOF on error.
 */
int read_bit(Iceuncompr *ice);

/**
 * Writes a bit to the output buffer.
 * @param ice Pointer to Iceuncompr structure.
 * @param value The bit value to write.
 */
void write_bit(Iceuncompr *ice, int value);

/**
 * Reads an integer of specified bit length from the input stream.
 * @param ice Pointer to Iceuncompr structure.
 * @param bits The number of bits to read.
 * @return The read integer, or EOF on error.
 */
int read_int(Iceuncompr *ice, int bits);

/**
 * Writes a specified number of zero bits to the output buffer.
 * @param ice Pointer to Iceuncompr structure.
 * @param bits The number of zero bits to write.
 */
void write_zeros(Iceuncompr *ice, int bits);

/**
 * Main function for decompressing ICE-compressed data.
 * @param ice Pointer to Iceuncompr structure.
 * @return Zero on success, non-zero on failure.
 */
int ice_uncompress(Iceuncompr *ice);

/**
 * Writes decompressed bitstream.
 * @param ice Pointer to Iceuncompr structure.
 * @param input_data Pointer to input data buffer.
 * @param size Size of input data.
 * @return True on success, false on failure.
 */
bool WriteUncomprBitstream(Iceuncompr *ice, uint8_t *input_data, uint32_t size);

#endif // ICEUNCOMPR_H