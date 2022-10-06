#pragma once
#ifndef B_ENDIAN_HPP
#define B_ENDIAN_HPP

#include <cstdint>
#include <fstream>

namespace B_Endian {
    // Convert signed values
    //int32_t read_32(std::istream& f);
    //int16_t read_16(std::istream& f);
    //int8_t  read_8 (std::istream& f);

    // Convert unsigned values
    uint32_t read_u32(std::istream& f);
    uint32_t write_u32(int f);
    uint16_t read_u16(std::istream& f);
    uint16_t write_u16(int f);
    uint8_t  read_u8 (std::istream& f);
}

#endif