/**
 * Lattice ICE40HX NyanOS - Nyan Keys Driver
 * @author Reese Russell
 */

#include "lattice_ice_hx.h"

FPGAReturn FPGAInit(LatticeIceHX* fpga)
{
    fpga->configured = false;
    FPGAGetBitstreamCompressedSize(fpga);
    
    return FPGA_SUCCESS;
}

FPGAReturn FPGAGetBitstreamCompressedSize(LatticeIceHX* fpga)
{
    // We need to fetch the owners name from the eeprom - !!!FIXME!!! Polled
    EepromRead(&nos_eeprom, false, ADDR_FPGA_BITSTREAM_LEN, SIZE_FPGA_BITSTREAM_LEN);
    while(nos_eeprom.rx_inflight){}
    // Cast the eeprom as a pointer of shorts
    uint16_t *rx_buf = (uint16_t *)nos_eeprom.rx_buf;
    fpga->bitstream_compressed_size = rx_buf[SIZE_FPGA_BITSTREAM_LEN/2 -2]; //Little Endian Cast? {0xFFxx}

    return FPGA_SUCCESS;
}