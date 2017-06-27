/****************************************************************************
 *
 * Filename:    ps5000aWrap.h
 *
 * Description:
 *
 * This header defines the interface to the wrapper library for the 
 * PicoScope 5000 series of PC Oscilloscopes using the ps5000a API 
 * functions.
 *
 * Copyright (C) 2013-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 ****************************************************************************/

#ifndef __PS5000AWRAP_H__
#define __PS5000AWRAP_H__

#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#include "ps5000aApi.h"

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
#include "ps5000aApi.h"

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
#include <libps5000a-1.1/ps5000aApi.h>
#ifndef PICO_STATUS
#include <libps5000a-1.1/PicoStatus.h>
#endif

#define memcpy_s(a,b,c,d) memcpy(a,c,d)

#define PREF0
#define PREF1

typedef enum enBOOL
{
  FALSE, TRUE
} BOOL;
#endif

int16_t		_ready;
int16_t		_autoStop;
uint32_t	_numSamples;
uint32_t	_triggeredAt = 0;
int16_t		_triggered = FALSE;
uint32_t	_startIndex;
int16_t		_overflow = 0;

int16_t		_channelCount = 0; // Should be set to 2 or 4 from the main application
int16_t		_enabledChannels[PS5000A_MAX_CHANNELS] = {0, 0, 0, 0}; // Keep a record of the channels that are enabled

typedef struct tWrapBufferInfo
{
	int16_t *driverBuffers[PS5000A_MAX_CHANNEL_BUFFERS];
	int16_t *appBuffers[PS5000A_MAX_CHANNEL_BUFFERS];
	uint32_t bufferLengths[PS5000A_MAX_CHANNELS];

} WRAP_BUFFER_INFO;

WRAP_BUFFER_INFO _wrapBufferInfo;

/////////////////////////////////
//
//	Function definitions
//
/////////////////////////////////

extern PICO_STATUS PREF0 PREF1 RunBlock
(
	int16_t handle, 
	int32_t preTriggerSamples, 
	int32_t postTriggerSamples, 
	uint32_t timebase,
	uint32_t segmentIndex
);

extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues
(
	int16_t handle
);

extern uint32_t PREF0 PREF1 AvailableData
(
	int16_t handle, 
	uint32_t *startIndex
);

extern int16_t PREF0 PREF1 AutoStopped
(
	int16_t handle
);

extern int16_t PREF0 PREF1 IsReady
(
	int16_t handle
);

extern int16_t PREF0 PREF1 IsTriggerReady
(
	int16_t handle, 
	uint32_t *triggeredAt
);

extern int16_t PREF0 PREF1 ClearTriggerReady
(
	int16_t handle
);

extern PICO_STATUS PREF0 PREF1 SetTriggerConditions
(
	int16_t handle, 
	int32_t *conditionsArray, 
	int16_t nConditions
);

extern PICO_STATUS PREF0 PREF1 SetTriggerProperties
(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties,  
	int32_t autoTrig
);

extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifier
(
	int16_t handle,
	int32_t *pwqConditionsArray,
	int16_t nConditions,
	int32_t direction,
	uint32_t lower,
	uint32_t upper,
	int32_t type
);

extern void PREF0 PREF1 setChannelCount
(
	int16_t handle, 
	int16_t channelCount
);

extern int16_t PREF0 PREF1 setEnabledChannels
(
	int16_t handle, 
	int16_t * enabledChannels
);

extern int16_t PREF0 PREF1 setAppAndDriverBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * appBuffer, 
	int16_t * driverBuffer,
	uint32_t bufferLength
);

extern int16_t PREF0 PREF1 setMaxMinAppAndDriverBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * appMaxBuffer,
	int16_t * appMinBuffer, 
	int16_t * driverMaxBuffer, 
	int16_t * driverMinBuffer,
	uint32_t bufferLength
);

#endif
