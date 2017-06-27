/****************************************************************************
 *
 * Filename:    ps2000aWrap.h
 *
 * Description:
 *  This header defines the interface to the wrapper dll for the 
 * PicoScope 2000 series of PC Oscilloscopes using the PicoScope 2000 Series
 * A API.
 *
 * Copyright (C) 2011-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 ****************************************************************************/

#ifndef __PS2000AWRAP_H__
#define __PS2000AWRAP_H__

#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#include "ps2000aApi.h"

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
#include "ps2000aApi.h"

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
#include <libps2000a-1.1/ps2000aApi.h>
#ifndef PICO_STATUS
#include <libps2000a-1.1/PicoStatus.h>
#endif

#define memcpy_s(a,b,c,d) memcpy(a,c,d)

#define PREF0
#define PREF1

typedef enum enBOOL
{
  FALSE, TRUE
} BOOL;
#endif

// 2205 MSO also has 2 digital ports
#define MAX_DIGITAL_PORTS			(PS2000A_MAX_DIGITAL_PORTS / 2)		// 2
#define MAX_DIGITAL_BUFFERS			4									// 4 - Port 0 Max/Min and Port 1 Max/Min

int16_t		g_ready;
int16_t		g_autoStop;
int32_t		g_numSamples;
uint32_t	g_triggeredAt = 0;
int16_t		g_triggered = FALSE;
uint32_t	g_startIndex;		// Start index in driver data buffer
int16_t		g_overflow = 0;

// Parameters relating to channel and digital ports

int16_t g_channelCount = 2;											// Default set to 2
int16_t g_enabledChannels[PS2000A_MAX_CHANNELS] = {0, 0, 0, 0};		// Keep a record of the channels that are enabled

int16_t g_digitalPortCount = 0;										// Should be set to 2 from the main application for the 2205 MSO
int16_t g_enabledDigitalPorts[MAX_DIGITAL_PORTS] = {0, 0};			// Keep a record of the channels that are enabled

typedef struct tWrapBufferInfo
{
	// Analogue channels
	int16_t *driverBuffers[PS2000A_MAX_CHANNEL_BUFFERS];			// The buffers registered with the driver
	int16_t *appBuffers[PS2000A_MAX_CHANNEL_BUFFERS];				// Application buffers to copy the driver data into
	int32_t bufferLengths[PS2000A_MAX_CHANNELS];					// Buffer lengths

	// Digital ports
	int16_t *driverDigiBuffers[MAX_DIGITAL_BUFFERS];				// The buffers registered with the driver for the digital ports
	int16_t *appDigiBuffers[MAX_DIGITAL_BUFFERS];					// Application buffers to copy the driver digital data into
	int32_t digiBufferLengths[MAX_DIGITAL_PORTS];					// Buffer lengths for digital ports - only 2 ports.

} WRAP_BUFFER_INFO;

WRAP_BUFFER_INFO g_wrapBufferInfo;

// Enum to define Digital Port indices
typedef enum enPS2000AWrapDigitalPortIndex
{
	PS2000A_WRAP_DIGITAL_PORT0,
	PS2000A_WRAP_DIGITAL_PORT1
} PS2000A_WRAP_DIGITAL_PORT_INDEX;

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

extern int32_t PREF0 PREF1 AvailableData
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

extern PICO_STATUS PREF0 PREF1 ClearTriggerReady
(
	int16_t handle
);

extern PICO_STATUS PREF0 PREF1 SetTriggerConditions
(
	int16_t handle, 
	uint32_t * conditionsArray, 
	int16_t nConditions
);

extern PICO_STATUS PREF0 PREF1 SetTriggerProperties
(
	int16_t handle, 
	int32_t * propertiesArray, 
	int16_t nProperties,  
	int32_t autoTrig
);

extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifier
(
	int16_t handle,
	uint32_t * pwqConditionsArray,
	int16_t nConditions,
	uint32_t direction,
	uint32_t lower,
	uint32_t upper,
	uint32_t type
);

extern PICO_STATUS PREF0 PREF1 setChannelCount
(
	int16_t handle
);

extern PICO_STATUS PREF0 PREF1 setEnabledChannels
(
	int16_t handle, 
	int16_t * enabledChannels
);

// Merged with setChannelCount
//extern PICO_STATUS PREF0 PREF1 setDigitalPortCount
//(
//	int16_t handle, 
//	int16_t digitalPortCount
//);

extern PICO_STATUS PREF0 PREF1 setEnabledDigitalPorts
(
	int16_t handle, 
	int16_t * enabledDigitalPorts
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

extern PICO_STATUS PREF0 PREF1 setAppAndDriverDigiBuffers
(
	int16_t handle, 
	int16_t digiPort, 
	int16_t * appDigiBuffer, 
	int16_t * driverDigiBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverDigiBuffers
(
	int16_t handle, 
	int16_t digiPort, 
	int16_t * appMaxDigiBuffer,
	int16_t * appMinDigiBuffer, 
	int16_t * driverMaxDigiBuffer, 
	int16_t * driverMinDigiBuffer,
	int32_t bufferLength
);
#endif
