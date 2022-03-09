//#include "FRM_Animate.h"
//#include <stdio.h>
//#include <cstdint>
//
//
//
//typedef uint8_t BYTE;
//typedef uint16_t DWORD;
//
//
//// Palette
//BYTE g_Palette[768];
//
//// ÐÐ°ÑÐ°Ð»ÑÐ½Ð¾Ðµ Ð·Ð½Ð°ÑÐµÐ½Ð¸Ðµ ÑÐ²ÐµÑÐ°
//BYTE g_nSlime[] =		{ 0, 108, 0, 11, 115, 7, 27, 123, 15, 43, 131, 27 };                         // Slime
//BYTE g_nMonitors[] =	{ 107, 107, 111, 99, 103, 127, 87, 107, 143, 0, 147, 163, 107, 187, 255 };   // Monitors
//BYTE g_nFireSlow[] =	{ 255, 0, 0, 215, 0, 0 , 147, 43, 11, 255, 119, 0, 255, 59, 0 };             // Slow fire
//BYTE g_nFireFast[] =	{ 71, 0, 0, 123, 0, 0, 179, 0, 0, 123, 0, 0, 71, 0, 0 };                     // Fast fire
//BYTE g_nShoreline[] =	{ 83, 63, 43, 75, 59, 43, 67, 55, 39, 63, 51, 39, 55, 47, 35, 51, 43, 35 };  // Shoreline
//BYTE g_nBlinkingRed =	252;																		 // Alarm
//
//// Current parameters of cycle
//DWORD g_dwSlimeCurrent = 0;
//DWORD g_dwMonitorsCurrent = 0;
//DWORD g_dwFireSlowCurrent = 0;
//DWORD g_dwFireFastCurrent = 0;
//DWORD g_dwShorelineCurrent = 0;
//BYTE  g_nBlinkingRedCurrent = 0;
//
//// Time of Last changinge
//DWORD g_dwLastCycleSlow = 0;
//DWORD g_dwLastCycleMedium = 0;
//DWORD g_dwLastCycleFast = 0;
//DWORD g_dwLastCycleVeryFast = 0;
//
//// Current speed factor
//DWORD g_dwCycleSpeedFactor = 1;
//
//void AnimatePalette()
//{
//	BOOL bPaletteChanged = FALSE;
//	DWORD dwCurrentTime = GetTickCount();
//
//	if (dwCurrentTime - g_dwLastCycleSlow >= 200 * g_dwCycleSpeedFactor) {
//		// Slime
//		DWORD dwSlimeCurrentWork = g_dwSlimeCurrent;
//
//		for (int i = 3; i >= 0; i--) {
//			g_Palette[687 + i * 3] = g_nSlime[dwSlimeCurrentWork * 3] >> 2;                       // Red  
//			g_Palette[687 + i * 3 + 1] = g_nSlime[dwSlimeCurrentWork * 3 + 1] >> 2;               // Green
//			g_Palette[687 + i * 3 + 2] = g_nSlime[dwSlimeCurrentWork * 3 + 2] >> 2;               // Blue
//
//			if (dwSlimeCurrentWork == 3)
//				dwSlimeCurrentWork = 0;
//			else
//				dwSlimeCurrentWork++;
//		}
//
//		if (g_dwSlimeCurrent == 3)
//			g_dwSlimeCurrent = 0;
//		else
//			g_dwSlimeCurrent++;