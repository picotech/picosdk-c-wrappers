/****************************************************************************
 *
 * Filename:    ps3000aWrap.h
 *
 * Description:
 *  This header defines the interface to the wrapper library for the 
 *	PicoScope 3000 series of PC Oscilloscopes using the PicoScope 3000 
 *	Series (ps3000a) API functions.
 *
 * Copyright (C) 2011 - 2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 ****************************************************************************/
#ifndef __PS3000AWRAP_H__
#define __PS3000AWRAP_H__

#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#include "ps3000aApi.h"

#ifdef PREF0
#undef PREF0
#endif
#define PREF0 __declspec(dllexport)

#ifdef PREF1
#undef PREF1
#endif
#define PREF1 __stdcall

#else
#include <libps3000a-1.1/ps3000aApi.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#ifndef PICO_STATUS
#include <libps3000a-1.1/PicoStatus.h>
#endif

#define memcpy_s(a,b,c,d) memcpy(a,c,d)

#define PREF0
#define PREF1

typedef enum enBOOL
{
  FALSE, TRUE
} BOOL;
#endif

#define MAX_PICO_DEVICES 64
#define WRAP_MAX_PICO_DEVICES 4

#define DUAL_SCOPE	2
#define DUAL_PORT_MSO 2

// 320X MSO has 2 digital ports
#define MAX_DIGITAL_BUFFERS		(PS3000A_MAX_DIGITAL_PORTS * 2) // First 4 correspond to Port 0 Max/Min and Port 1 Max/Min

// Enum to define Digital Port indices
typedef enum enPS3000AWrapDigitalPortIndex
{
	PS3000A_WRAP_DIGITAL_PORT0,
	PS3000A_WRAP_DIGITAL_PORT1
} PS3000A_WRAP_DIGITAL_PORT_INDEX;

/****************************************************************************
* tWrapUnitInfo
*
* This structure is used to store streaming information for a device.
*
****************************************************************************/
typedef struct tWrapUnitInfo
{
	int16_t handle;

	// Analogue and Digital Channel Information
	int16_t channelCount;										// Set to 2 (320XA/B/D/D MSO) or 4 (340XA/B/D/D MSO). 
	int16_t enabledChannels[PS3000A_MAX_CHANNELS];				// Keep a record of the channels that are enabled.

	int16_t digitalPortCount;									// Should be set to 2 from the main application for 320X MSO devices.
	int16_t enabledDigitalPorts[PS3000A_MAX_DIGITAL_PORTS];		// Keep a record of the ports that are enabled.

	// Streaming Parameters
	int16_t		ready;
	int32_t		numSamples;
	uint32_t	startIndex;
	int16_t		overflow;
	uint32_t	triggeredAt;
	int16_t		triggered;
	int16_t		autoStop;

	// Data Buffers

	// Analogue channels
	int16_t *driverBuffers[PS3000A_MAX_CHANNEL_BUFFERS];	// The buffers registered with the driver.
	int16_t *appBuffers[PS3000A_MAX_CHANNEL_BUFFERS];		// Application buffers to copy the driver data into.
	int32_t bufferLengths[PS3000A_MAX_CHANNELS];			// Buffer lengths for analogue channels.

	// Digital ports
	int16_t *driverDigiBuffers[MAX_DIGITAL_BUFFERS];		// The buffers registered with the driver for the digital ports.
	int16_t *appDigiBuffers[MAX_DIGITAL_BUFFERS];			// Application buffers to copy the driver digital data into.
	int32_t digiBufferLengths[PS3000A_MAX_DIGITAL_PORTS];	// Buffer lengths for digital ports.
	
} WRAP_UNIT_INFO;

WRAP_UNIT_INFO	g_deviceInfo[WRAP_MAX_PICO_DEVICES];	// Hold structures for each device

// Global parameters

uint16_t	g_deviceCount = 0;			// Keep a record of the number of devices
uint16_t	g_nextDeviceIndex = 0;		// Keep track of the next index to use (only from 0 to 3)

// Function declarations

extern int16_t PREF0 PREF1 AutoStopped
(
	uint16_t deviceIndex
);

extern uint32_t PREF0 PREF1 AvailableData
(
	uint16_t deviceIndex, 
	uint32_t *startIndex
);

extern PICO_STATUS PREF0 PREF1 ClearTriggerReady
(
	uint16_t deviceIndex
);

extern PICO_STATUS PREF0 PREF1 decrementDeviceCount
(
	uint16_t deviceIndex
);

extern uint16_t PREF0 PREF1 getDeviceCount
(
	void
);

extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues
(
	uint16_t deviceIndex
);

extern PICO_STATUS PREF0 PREF1 initWrapUnitInfo
(
	int16_t handle, 
	uint16_t * deviceIndex
);

extern int16_t PREF0 PREF1 IsReady
(
	uint16_t deviceIndex
);

extern int16_t PREF0 PREF1 IsTriggerReady
(
	uint16_t deviceIndex, 
	uint32_t *triggeredAt
);

extern PICO_STATUS PREF0 PREF1 RunBlock
(
	uint16_t deviceIndex, 
	int32_t preTriggerSamples, 
	int32_t postTriggerSamples, 
	uint32_t timebase,
	uint32_t segmentIndex
);

extern PICO_STATUS PREF0 PREF1 setAppAndDriverBuffers
(
	uint16_t deviceIndex, 
	int16_t channel, 
	int16_t * appBuffer, 
	int16_t * driverBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers
(
	uint16_t deviceIndex, 
	int16_t channel, 
	int16_t * appMaxBuffer,
	int16_t * appMinBuffer, 
	int16_t * driverMaxBuffer, 
	int16_t * driverMinBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setAppAndDriverDigiBuffers
(
	uint16_t deviceIndex, 
	int16_t digiPort, 
	int16_t * appDigiBuffer, 
	int16_t * driverDigiBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverDigiBuffers
(
	uint16_t deviceIndex, 
	int16_t digiPort, 
	int16_t * appMaxDigiBuffer,
	int16_t * appMinDigiBuffer, 
	int16_t * driverMaxDigiBuffer, 
	int16_t * driverMinDigiBuffer,
	int32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setChannelCount
(
	uint16_t deviceIndex, 
	int16_t channelCount
);

extern PICO_STATUS PREF0 PREF1 setEnabledChannels
(
	uint16_t deviceIndex, 
	int16_t * enabledChannels
);

extern PICO_STATUS PREF0 PREF1 setDigitalPortCount
(
	uint16_t deviceIndex, 
	int16_t digitalPortCount
);

extern PICO_STATUS PREF0 PREF1 setEnabledDigitalPorts
(
	uint16_t deviceIndex, 
	int16_t * enabledDigitalPorts
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

extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifierV2
(
	int16_t handle,
	uint32_t * pwqConditionsArrayV2,
	int16_t nConditions,
	uint32_t direction,
	uint32_t lower,
	uint32_t upper,
	uint32_t type
);

extern PICO_STATUS PREF0 PREF1 SetTriggerConditions
(
	int16_t handle, 
	uint32_t * conditionsArray, 
	int16_t nConditions
);

extern PICO_STATUS PREF0 PREF1 SetTriggerConditionsV2
(
	int16_t handle, 
	uint32_t * conditionsArray, 
	int16_t nConditions
);

extern PICO_STATUS PREF0 PREF1 SetTriggerProperties
(
	int16_t handle, 
	uint32_t * propertiesArray, 
	int16_t nProperties,  
	int32_t autoTrig
);

extern PICO_STATUS PREF0 PREF1 resetNextDeviceIndex
(
	void
);

#endif
