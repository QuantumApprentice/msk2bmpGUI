/*
//https://falloutmods.fandom.com/wiki/Pal_animations#Animated_colors
#include "FRM_Animate.h"
#include <stdio.h>
#include <cstdint>



typedef uint8_t BYTE;
typedef uint16_t DWORD;


// Palette
BYTE g_Palette[768];

// ÐÐ°ÑÐ°Ð»ÑÐ½Ð¾Ðµ Ð·Ð½Ð°ÑÐµÐ½Ð¸Ðµ ÑÐ²ÐµÑÐ°
BYTE g_nSlime[] =		{ 0, 108, 0, 11, 115, 7, 27, 123, 15, 43, 131, 27 };                         // Slime
BYTE g_nMonitors[] =	{ 107, 107, 111, 99, 103, 127, 87, 107, 143, 0, 147, 163, 107, 187, 255 };   // Monitors
BYTE g_nFireSlow[] =	{ 255, 0, 0, 215, 0, 0 , 147, 43, 11, 255, 119, 0, 255, 59, 0 };             // Slow fire
BYTE g_nFireFast[] =	{ 71, 0, 0, 123, 0, 0, 179, 0, 0, 123, 0, 0, 71, 0, 0 };                     // Fast fire
BYTE g_nShoreline[] =	{ 83, 63, 43, 75, 59, 43, 67, 55, 39, 63, 51, 39, 55, 47, 35, 51, 43, 35 };  // Shoreline
BYTE g_nBlinkingRed =	252;																		 // Alarm

// Current parameters of cycle
DWORD g_dwSlimeCurrent = 0;
DWORD g_dwMonitorsCurrent = 0;
DWORD g_dwFireSlowCurrent = 0;
DWORD g_dwFireFastCurrent = 0;
DWORD g_dwShorelineCurrent = 0;
BYTE  g_nBlinkingRedCurrent = 0;

// Time of Last changinge
DWORD g_dwLastCycleSlow = 0;
DWORD g_dwLastCycleMedium = 0;
DWORD g_dwLastCycleFast = 0;
DWORD g_dwLastCycleVeryFast = 0;

// Current speed factor
DWORD g_dwCycleSpeedFactor = 1;

void AnimatePalette()
{
	BOOL bPaletteChanged = FALSE;
	DWORD dwCurrentTime = GetTickCount();

	if (dwCurrentTime - g_dwLastCycleSlow >= 200 * g_dwCycleSpeedFactor) {
		// Slime    ///////////////////////////////////////////////////////
		DWORD dwSlimeCurrentWork = g_dwSlimeCurrent;

		for (int i = 3; i >= 0; i--) {
			g_Palette[687 + i * 3] = g_nSlime[dwSlimeCurrentWork * 3] >> 2;                       // Red  
			g_Palette[687 + i * 3 + 1] = g_nSlime[dwSlimeCurrentWork * 3 + 1] >> 2;               // Green
			g_Palette[687 + i * 3 + 2] = g_nSlime[dwSlimeCurrentWork * 3 + 2] >> 2;               // Blue

			if (dwSlimeCurrentWork == 3)
				dwSlimeCurrentWork = 0;
			else
				dwSlimeCurrentWork++;
		}

		if (g_dwSlimeCurrent == 3)
			g_dwSlimeCurrent = 0;
		else
			g_dwSlimeCurrent++;

        // Shoreline    ///////////////////////////////////////////////////////
        DWORD dwShorelineCurrentWork = g_dwShorelineCurrent;

        for(int i = 5; i >= 0; i--) {
            g_Palette[744 + i * 3] = g_nShoreline[dwShorelineCurrentWork * 3] >> 2;               // Red
            g_Palette[744 + i * 3 + 1] = g_nShoreline[dwShorelineCurrentWork * 3 + 1] >> 2;       // Green
            g_Palette[744 + i * 3 + 2] = g_nShoreline[dwShorelineCurrentWork * 3 + 2] >> 2;       // Blue

            if (dwShorelineCurrentWork == 5)
                dwShorelineCurrentWork = 0;
            else
                dwShorelineCurrentWork++;
        }

        if (g_dwShorelineCurrent == 5)
            g_dwShorelineCurrent = 0;
        else
            g_dwShorelineCurrent++;

        // Fire_slow    ///////////////////////////////////////////////////////
        DWORD dwFireSlowCurrentWork = g_dwFireSlowCurrent;

        for(int i = 4; i >= 0; i--) {
            g_Palette[714 + i * 3] = g_nFireSlow[dwFireSlowCurrentWork * 3] >> 2;                 // Red
            g_Palette[714 + i * 3 + 1] = g_nFireSlow[dwFireSlowCurrentWork * 3 + 1] >> 2;         // Green
            g_Palette[714 + i * 3 + 2] = g_nFireSlow[dwFireSlowCurrentWork * 3 + 2] >> 2;         // Blue

            if (dwFireSlowCurrentWork == 4)
                dwFireSlowCurrentWork = 0;
            else
                dwFireSlowCurrentWork++;
        }

        if (g_dwFireSlowCurrent == 4)
            g_dwFireSlowCurrent = 0;
        else
            g_dwFireSlowCurrent++;

        g_dwLastCycleSlow = dwCurrentTime;
        bPaletteChanged = TRUE;
    }

    dwCurrentTime = GetTickCount();

    if (dwCurrentTime - g_dwLastCycleMedium >= 142 * g_dwCycleSpeedFactor) {
        // Fire_fast    ///////////////////////////////////////////////////////
        DWORD dwFireFastCurrentWork = g_dwFireFastCurrent;

        for(int i = 4; i >= 0; i--) {
            g_Palette[729 + i * 3] = g_nFireFast[dwFireFastCurrentWork * 3] >> 2;                 // Red
            g_Palette[729 + i * 3 + 1] = g_nFireFast[dwFireFastCurrentWork * 3 + 1] >> 2;         // Green
            g_Palette[729 + i * 3 + 2] = g_nFireFast[dwFireFastCurrentWork * 3 + 2] >> 2;         // Blue

            if (dwFireFastCurrentWork == 4)
                dwFireFastCurrentWork = 0;
            else
                dwFireFastCurrentWork++;
        }

        if (g_dwFireFastCurrent == 4)
            g_dwFireFastCurrent = 0;
        else
            g_dwFireFastCurrent++;

        g_dwLastCycleMedium = dwCurrentTime;
        bPaletteChanged = TRUE;
    }

    dwCurrentTime = GetTickCount();

    if (dwCurrentTime - g_dwLastCycleFast >= 100 * g_dwCycleSpeedFactor) {
        // Monitors     ///////////////////////////////////////////////////////
        DWORD dwMonitorsCurrentWork = g_dwMonitorsCurrent;

        for(int i = 4; i >= 0; i--) {
            g_Palette[699 + i * 3] = g_nMonitors[dwMonitorsCurrentWork * 3] >> 2;                 // Red
            g_Palette[699 + i * 3 + 1] = g_nMonitors[dwMonitorsCurrentWork * 3 + 1] >> 2;         // Green
            g_Palette[699 + i * 3 + 2] = g_nMonitors[dwMonitorsCurrentWork * 3 + 2] >> 2;         // Blue

            if (dwMonitorsCurrentWork == 4)
                dwMonitorsCurrentWork = 0;
            else
                dwMonitorsCurrentWork++;
        }

        if (g_dwMonitorsCurrent == 4)
            g_dwMonitorsCurrent = 0;
        else
            g_dwMonitorsCurrent++;

        g_dwLastCycleFast = dwCurrentTime;
        bPaletteChanged = TRUE;
    }

    dwCurrentTime = GetTickCount();

    if (dwCurrentTime - g_dwLastCycleVeryFast >= 33 * g_dwCycleSpeedFactor) {
        // Blinking red     ///////////////////////////////////////////////////////
        if ((g_nBlinkingRedCurrent == 0) ||(g_nBlinkingRedCurrent == 60))
            g_nBlinkingRed = BYTE(-g_nBlinkingRed);

        g_Palette[762] = g_nBlinkingRed + g_nBlinkingRedCurrent;                                  // Red
        g_Palette[763] = 0;                                                                       // Green
        g_Palette[764] = 0;                                                                       // Blue

        g_nBlinkingRedCurrent = g_nBlinkingRed + g_nBlinkingRedCurrent;

        g_dwLastCycleVeryFast = dwCurrentTime;
        bPaletteChanged = TRUE;
    }

    if (bPaletteChanged)
        UpdatePalette();
}
*/