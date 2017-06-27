/****************************************************************************
 *
 * Filename:    ps3000Wrap.h
 * 
 * Description:
 *  This header defines the interface to the wrapper library for the 
 * PicoScope 3000 series of PC Oscilloscopes using the PicoScope 3000 Series
 * (ps3000) API functions.
 *
 * Copyright (C) 2010-2017 Pico Technology Ltd. See LICENSE file for terms. 
 *
 ****************************************************************************/
#ifndef __PS3000WRAP_H__
#define __PS3000WRAP_H__

#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#include "ps3000.h"

#ifdef PREF0
#undef PREF0
#endif
#define PREF0 __declspec(dllexport)

#ifdef PREF1
#undef PREF1
#endif
#define PREF1 __stdcall
#else
#include <sys/types.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <libps3000-3.7/ps3000.h>

#define memcpy_s(a,b,c,d) memcpy(a,c,d)

#define PREF0
#define PREF1

typedef enum enBOOL
{
  FALSE, TRUE
} BOOL;
#endif

///////////////////////////////////////
//
//	Constant and variable definitions
//
///////////////////////////////////////

#define DUAL_SCOPE			2	// 2 channel oscilloscope
#define QUAD_SCOPE			4	// 4 channel oscilloscope
#define	MAX_CHANNELS		4
#define MAX_CHANNEL_BUFFERS 8

int16_t		g_overflow;
uint32_t	g_triggeredAt;
int16_t		g_triggered;
int16_t		g_auto_stop;
uint32_t	g_nValues;
int16_t		g_ready;
int16_t	*	g_overviewBuffers[MAX_CHANNEL_BUFFERS] = {0, 0, 0, 0, 0, 0, 0, 0};

int16_t		g_channelCount = 0; // Should be 2 (PicoScope 3224) or 4 (PicoScope 3424/3425)

int16_t		g_enabledChannels[MAX_CHANNELS]	= {0, 0, 0, 0};	// Set which channels are enabled
uint32_t	g_bufferLengths[MAX_CHANNELS]	= {0, 0, 0, 0}; // Max and Min buffer lengths must be equal

int16_t		g_bufferLengthsSet[MAX_CHANNELS] = {FALSE, FALSE, FALSE, FALSE};	// Indicate if the buffer was set using the setDataBuffers function

///////////////////////////////////////
//
//	Function declarations
//
///////////////////////////////////////

extern int16_t PREF0 PREF1 GetStreamingLastValues
(
	int16_t handle
);

extern int16_t PREF0 PREF1 IsReady
(
	int16_t handle
);

extern int16_t PREF0 PREF1 AvailableData
(
	int16_t handle, 
	int16_t *overflow, 
	uint32_t *triggeredAt, 
	int16_t *triggered, 
	int16_t *auto_stop, 
	uint32_t *nValues
);

extern int16_t PREF0 PREF1 ClearTriggerInfo
(
	int16_t handle
);

extern int16_t PREF0 PREF1 SetDataBuffer
(
	int16_t handle, 
	int16_t channel, 
	int16_t * buffer,
	uint32_t bufferLength
);

// Legacy function - use SetDataBuffers2 for new applications.
extern void PREF0 PREF1 SetDataBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * minBuffer, 
	int16_t * maxBuffer
);

extern int16_t PREF0 PREF1 SetDataBuffersV2
(
	int16_t handle, 
	int16_t channel, 
	int16_t * minBuffer, 
	int16_t * maxBuffer,
	uint32_t bufferLength
);

extern int16_t PREF0 PREF1 setChannelCount
(
	int16_t handle, 
	int16_t channelCount
);

extern int16_t PREF0 PREF1 setEnabledChannels
(
	int16_t handle, 
	int16_t * enabledChannels
);

extern int16_t PREF0 PREF1 clearFastStreamingParameters
(
	int16_t handle
);

#endif
