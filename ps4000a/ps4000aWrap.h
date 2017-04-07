/****************************************************************************
 *
 * Filename:    ps4000aWrap.h
 * Copyright:   Pico Technology Limited 2014
 * Author:      HSM
 * Description:
 *
 * This header defines the interface to the wrapper dll for the 
 *	PicoScope 4000 series of PC Oscilloscopes using the PicoScope 4000 
 *  Series 'A' API.
 *
 ****************************************************************************/
#ifndef __PS4000AWRAP_H__
#define __PS4000AWRAP_H__

#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#include "ps4000aApi.h"

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
#include <libps4000a-1.0/ps4000aApi.h>
#ifndef PICO_STATUS
#include <libps4000a-1.0/PicoStatus.h>
#endif

#define memcpy_s(a,b,c,d) memcpy(a,c,d)

#define PREF0
#define PREF1

typedef enum enBOOL
{
  FALSE, TRUE
} BOOL;
#endif

////////////////////////////////////////
//
//	Variable and struct declarations
//
////////////////////////////////////////

int16_t		_ready;
int16_t		_autoStop;
uint32_t	_numSamples;
uint32_t	_triggeredAt = 0;
int16_t		_triggered = FALSE;
uint32_t	_startIndex;
int16_t		_overflow = 0;

int16_t		_channelCount = 0; // Should be set to 8 from the main application for the PicoScope 4824
int16_t		_enabledChannels[PS4000A_MAX_CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0}; // Keep a record of the channels that are enabled

typedef struct tWrapBufferInfo
{
	int16_t *driverBuffers[PS4000A_MAX_CHANNEL_BUFFERS];
	int16_t *appBuffers[PS4000A_MAX_CHANNEL_BUFFERS];
	int32_t bufferLengths[PS4000A_MAX_CHANNELS]; // In order of A max, A min, B max, ... G min.

} WRAP_BUFFER_INFO;

WRAP_BUFFER_INFO _wrapBufferInfo;

/////////////////////////////////
//
//	Function declarations
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

extern PICO_STATUS PREF0 PREF1 setChannelCount
(
	int16_t handle, 
	int16_t channelCount
);

extern PICO_STATUS PREF0 PREF1 setEnabledChannels
(
	int16_t handle, 
	int16_t * enabledChannels
);

extern PICO_STATUS PREF0 PREF1 setAppAndDriverBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * appBuffer, 
	int16_t * driverBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * appMaxBuffer,
	int16_t * appMinBuffer, 
	int16_t * driverMaxBuffer, 
	int16_t * driverMinBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setTriggerConditions
(
	int16_t handle,
	int32_t *conditionsArray,
	int16_t nConditions,
	int32_t info
);

extern PICO_STATUS PREF0 PREF1 setTriggerDirections
(
	int16_t handle,
	int32_t *directionsArray,
	int16_t nDirections
);

extern PICO_STATUS PREF0 PREF1 setTriggerProperties
(
	int16_t handle,
	int32_t *propertiesArray,
	int16_t nProperties,
	int32_t autoTrig
);



#endif
