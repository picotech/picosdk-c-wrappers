/****************************************************************************
 *
 * Filename:    ps2000Wrap.h
 *
 * Description:
 *	This header defines the interface to the wrapper library for the 
 *	PicoScope 2000 series of PC Oscilloscopes.
 *
 * Copyright (c) 2009-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 ****************************************************************************/
#ifndef __PS2000WRAP_H__
#define __PS2000WRAP_H__

#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#include "ps2000.h"

#ifdef PREF0
#undef PREF0
#endif
#define PREF0 __declspec(dllexport)

#ifdef PREF1
#undef PREF1
#endif
#define PREF1 __stdcall

#elif _WIN64
#include "windows.h"
#include <stdio.h>
#include "ps2000.h"

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
#include <libps2000-2.1/ps2000.h>

#define PREF0
#define PREF1

#define memcpy_s(a,b,c,d) memcpy(a,c,d)

typedef enum enBOOL
{
  FALSE, TRUE
} BOOL;
#endif

#define DUAL_SCOPE 2      // Dual analogue channel scope 

volatile int16_t	_ready = 0;

int16_t		_overflow;
uint32_t	_triggeredAt;
int16_t		_triggered;
int16_t		_auto_stop;
uint32_t	_nValues;

uint16_t	_channelCount = DUAL_SCOPE;	// Set the number of channels for the device
uint16_t	_enabledChannels[DUAL_SCOPE] = {0, 0}; // Set the number of channels enabled here

uint32_t	g_totalValues = 0;
uint32_t	g_startIndex = 0;
uint32_t	g_prevStartIndex = 0;	// Keep track of previous index into application buffer in streaming mode collection
int16_t		g_appBufferFull = 0;	// Use this in the callback to indicate if it is going to copy past the end of the buffer

uint32_t	g_overview_buffer_size	= 0;
uint32_t	g_collection_size		= 0;

// Struct to help with retrieving data into application buffers in streaming data capture
typedef struct
{
	int16_t *appBuffers[DUAL_SCOPE * 2];
	uint32_t bufferSizes[DUAL_SCOPE * 2];
} WRAP_BUFFER_INFO;

WRAP_BUFFER_INFO g_wrapBufferInfo;

// Function definitions

extern int16_t PREF0 PREF1 PollFastStreaming
(
	int16_t handle
);

extern void PREF0 PREF1 SetBuffer
(
	int16_t handle, 
	int16_t channel, 
	int16_t * buffer, 
	uint32_t bufferSize
);

extern void PREF0 PREF1 SetAggregateBuffer
(
	int16_t handle, 
	int16_t channel, 
	int16_t * bufferMax, 
	int16_t * bufferMin, 
	uint32_t bufferSize
);

extern int16_t PREF0 PREF1 FastStreamingReady
(
	int16_t handle
);

extern uint32_t PREF0 PREF1 GetFastStreamingDetails
(
	int16_t handle, 
	int16_t * overflow, 
	uint32_t * triggeredAt, 
	int16_t * triggered, 
	int16_t * auto_stop,
	int16_t * appBufferFull,
	uint32_t * startIndex
);

extern void PREF0 PREF1 setEnabledChannels
(
	int16_t handle, 
	int16_t * enabledChannels
);

extern void PREF0 PREF1 clearFastStreamingParameters
(
	int16_t handle
);

extern int16_t PREF0 PREF1 setCollectionInfo
(
	int16_t handle,
	uint32_t collectionSize, 
	uint32_t overviewBufferSize
);

#endif


