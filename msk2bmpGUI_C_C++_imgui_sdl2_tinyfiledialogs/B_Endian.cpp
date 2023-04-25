#include "B_Endian.h"
#include <iostream>

// Signed conversions
//int32_t bendian::read_32(std::istream& f)
//{
//	return -1;
//}
//int16_t bendian::read_16(std::istream& f)
//{
//	return -1;
//}
//int8_t bendian::read_8(std::istream& f)
//{
//	return -1;
//}

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
    std::reverse((char*)&val,
        ((char*)&val) + 4);
    return val;
}
uint16_t B_Endian::read_u16(std::istream& f)
{
    uint16_t val;
    uint8_t bytes[2];
    f.read((char*)bytes, 2);
    val = bytes[1]
        | (bytes[0] << 8);
    return val;
}
uint16_t B_Endian::write_u16(int f)
{
    uint16_t val = f;
    std::reverse((char*)&val,
        ((char*)&val) + 2);
    return val;

    //uint8_t bytes[2];
    //for (int i = 0; i < 2; i++)
    //{
    //	bytes[i] = ((uint8_t)(f) + i);
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