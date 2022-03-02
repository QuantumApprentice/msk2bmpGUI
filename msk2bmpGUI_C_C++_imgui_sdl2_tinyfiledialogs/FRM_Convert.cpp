#include "FRM_Convert.h"
#include <cstdint>

#pragma pack(push, 1)
typedef struct {
	uint32_t version = 0;					// 0x0000
	uint16_t FPS = 0;						// 0x0004
	uint16_t Action_Frame = 0;				// 0x0006
	uint16_t Frames_Per_Orientation = 0;	// 0x0008
	int16_t  Shift_Orient_x[6];				// 0x000A
	int16_t  Shift_Orient_y[6];				// 0x0016
	uint32_t Frame_0_Offset[6];				// 0x0022
	uint32_t Frame_Area;					// 0x003A
	uint16_t Frame_0_Width;					// 0x003E
	uint16_t Frame_0_Height;				// 0x0040
	uint32_t Frame_0_Size;					// 0x0042
	uint16_t Shift_Offset_x;				// 0x0046
	uint16_t Shift_Offset_y;				// 0x0048

} FRM_Header;
#pragma pack(pop)

void FRM_Convert()
{

}