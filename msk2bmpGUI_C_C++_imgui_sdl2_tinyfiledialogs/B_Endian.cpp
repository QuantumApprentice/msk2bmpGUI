#include <iostream>
#include "B_Endian.h"

// Signed conversions
void B_Endian::swap_16(uint16_t& in)
{
    in = (in << 8) | (in >> 8);
}
void B_Endian::swap_16(int16_t& in)
{
    in = (in << 8) | (in >> 8);
}
void B_Endian::swap_32(uint32_t& in)
{
    uint32_t v = 0;
    v |= (in & 0x000000ff) << 24;
    v |= (in & 0x0000ff00) << 8;
    v |= (in & 0x00ff0000) >> 8;
    v |= (in & 0xff000000) >> 24;

    in = v;
}
void byte_swap_16x4(uint64_t *p) {
    uint64_t q = 0;
    q |= (*p & 0xff00ff00ff00ff00ULL) >> 8;
    q |= (*p & 0x00ff00ff00ff00ffULL) << 8;
    *p = q;
}



// Unsigned conversions
uint32_t B_Endian::read_u32(std::istream& f)
{
    uint32_t val;
    uint8_t bytes[4];
    f.read((char*)bytes, 4);

    val = bytes[3] 
        | (bytes[2] << 8) 
        | (bytes[1] << 16) 
        | (bytes[0] << 24);
    return val;
}
uint32_t B_Endian::write_u32(int f)
{
    uint32_t val = f;
    std::reverse((char*)&val, ((char*)&val) + 4);
    return val;
}
uint16_t B_Endian::read_u16(std::istream& f)
{
    uint16_t val;
    uint8_t bytes[2];
    f.read((char*)bytes, 2);
    val = bytes[1] | (bytes[0] << 8);
    return val;
}
uint16_t B_Endian::write_u16(int f)
{
    uint16_t val = f;
    std::reverse((char*)&val, ((char*)&val) + 2);
    return val;

    //uint8_t bytes[2];
    //for (int i = 0; i < 2; i++)
    //{
    //  bytes[i] = ((uint8_t)(f) + i);
    //}

    //val = bytes[1]
    //  | (bytes[0] << 8);
}
uint8_t B_Endian::read_u8(std::istream& f)
{
    uint8_t val;
    uint8_t bytes[1];
    f.read((char*)bytes, 1);
    val = bytes[0];
    return val;
}