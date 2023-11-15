#ifndef LATTICE_ICE_HX_H
#define LATTICE_ICE_HX_H

#include <stdint.h>
#include <main.h>
#include "24xx_eeprom.h"
#include "iceuncompr.h"

// We need access to the eeprom to read it's contents (Bitstream and size)
extern Eeprom24xx nos_eeprom;
extern Iceuncompr ice_uncompr;

typedef enum {
    FPGA_FAILURE,
    FPGA_SUCCESS
} FPGAReturn;

typedef struct {
    bool configured;
    uint16_t bitstream_compressed_size;
    uint8_t* p_bitstream_compressed;
} LatticeIceHX;

/**
 * @brief Perform an on boot configuration of the FPGA, this meant to run once. Currently uses 150KB ram for the Ice40HX4K
 */
FPGAReturn FPGAInit(LatticeIceHX* fpga);

/**
 * @brief Fetch data from the eeprom in chunks and write it to a locally allocated buffer 
 */
FPGAReturn FPGAGetBitstreamData(LatticeIceHX* fpga);

/**
 * @brief Obtain the bitstream size from the EEPROM, should be less than 2^16-1 in size, though the register space used is much larger
 */
FPGAReturn FPGAGetBitstreamCompressedSize(LatticeIceHX* fpga);

#endif // LATTICE_ICE_HX_H