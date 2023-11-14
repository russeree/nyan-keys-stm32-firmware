#ifndef LATTICE_ICE_HX_H
#define LATTICE_ICE_HX_H

#include <stdint.h>
#include <main.h>
#include "24xx_eeprom.h"

// We need access to the eeprom to read it's contents (Bitstream and size)
extern Eeprom24xx nos_eeprom;

typedef enum {
    FPGA_FAILURE,
    FPGA_SUCCESS
} FPGAReturn;

typedef struct {
    bool configured;
    short bitstream_compressed_size;
} LatticeIceHX;

/**
 * @brief Perform an on boot configuration of the FPGA, this meant to run once. Currently uses 150KB ram for the Ice40HX4K
 */
FPGAReturn FPGAInit(LatticeIceHX* fpga);

/**
 * @brief Obtain the bitstream size from the EEPROM, should be less than 2^16-1 in size, though the register space used is much larger
 */
FPGAReturn FPGAGetBitstreamCompressedSize(LatticeIceHX* fpga);

#endif // NYAN_OS_H