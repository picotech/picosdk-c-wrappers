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
 * Copyright (C) 2013-2018 Pico Technology Ltd. See LICENSE file for terms.
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

#define PS5000A_WRAP_MAX_CHANNEL_BUFFERS		(2 * PS5000A_MAX_CHANNELS)
#define PS5000A_WRAP_MAX_DIGITAL_PORTS			2
#define PS5000A_WRAP_MAX_DIGITAL_BUFFERS		4  // 4 - Port 0 Max/Min and Port 1 Max/Min

/////////////////////////////////
//
//	Variable declarations
//
/////////////////////////////////

extern int16_t		_ready;
extern int16_t		_autoStop;
extern uint32_t		_numSamples;
extern uint32_t		_triggeredAt;
extern int16_t		_triggered;
extern uint32_t		_startIndex;				// Start index in driver data buffer
extern int16_t		_overflow;

extern int16_t		_channelCount; // Should be set to 2 or 4 from the main application
extern int16_t		_enabledChannels[PS5000A_MAX_CHANNELS]; // Keep a record of the channels that are enabled

extern int16_t		_digitalPortCount;																			// Should be set to 2 from the main application
extern int16_t		_enabledDigitalPorts[PS5000A_WRAP_MAX_DIGITAL_PORTS];		// Keep a record of the channels that are enabled

typedef struct tWrapBufferInfo
{
	int16_t *driverBuffers[PS5000A_WRAP_MAX_CHANNEL_BUFFERS];					// The buffers registered with the driver
	int16_t *appBuffers[PS5000A_WRAP_MAX_CHANNEL_BUFFERS];						// Application buffers to copy the driver data into
	uint32_t bufferLengths[PS5000A_MAX_CHANNELS];											// Buffer lengths

																																		// Digital ports
	int16_t *driverDigiBuffers[PS5000A_WRAP_MAX_DIGITAL_BUFFERS];			// The buffers registered with the driver for the digital ports
	int16_t *appDigiBuffers[PS5000A_WRAP_MAX_DIGITAL_BUFFERS];				// Application buffers to copy the driver digital data into
	uint32_t digiBufferLengths[PS5000A_WRAP_MAX_DIGITAL_BUFFERS];			// Buffer lengths for digital ports - only 2 ports.

} WRAP_BUFFER_INFO;

extern WRAP_BUFFER_INFO _wrapBufferInfo;

// Enum to define Digital Port indices
typedef enum enPS5000AWrapDigitalPortIndex
{
		PS5000A_WRAP_DIGITAL_PORT0,
		PS5000A_WRAP_DIGITAL_PORT1
} PS5000A_WRAP_DIGITAL_PORT_INDEX;

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

extern PICO_STATUS PREF0 PREF1 ClearTriggerReady
(
	int16_t handle
);

// This function is deprecated - use SetTriggerConditionsV2
extern PICO_STATUS PREF0 PREF1 SetTriggerConditions
(
	int16_t handle, 
	int32_t *conditionsArray, 
	int16_t nConditions
);

// This function is deprecated - use SetTriggerPropertiesV2
extern PICO_STATUS PREF0 PREF1 SetTriggerProperties
(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties,  
	int32_t autoTrig
);

// This function is deprecated - use SetPulseWidthQualifierV2
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
	PS5000A_CHANNEL channel, 
	int16_t * appBuffer, 
	int16_t * driverBuffer,
	uint32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers
(
	int16_t handle, 
	PS5000A_CHANNEL channel, 
	int16_t * appMaxBuffer,
	int16_t * appMinBuffer, 
	int16_t * driverMaxBuffer, 
	int16_t * driverMinBuffer,
	uint32_t bufferLength
);

extern PICO_STATUS PREF0 PREF1 setEnabledDigitalPorts
(
		int16_t handle,
		int16_t * enabledDigitalPorts
);

extern PICO_STATUS PREF0 PREF1 getOverflow
(
		int16_t handle,
		int16_t * overflow
);

extern PICO_STATUS PREF0 PREF1 SetTriggerConditionsV2
(
	int16_t handle,
	int32_t *conditionsArray,
	int16_t nConditions,
	PS5000A_CONDITIONS_INFO info
);

extern PICO_STATUS PREF0 PREF1 SetTriggerDirectionsV2
(
	int16_t handle,
	int32_t * directions,
	int16_t nDirections
);

extern PICO_STATUS PREF0 PREF1 SetTriggerPropertiesV2
(
	int16_t handle,
	int32_t *propertiesArray,
	int16_t nProperties
);

extern PICO_STATUS PREF0 PREF1 SetTriggerDigitalPortProperties
(
	int16_t handle,
	int32_t *digitalDirections,
	int16_t nDirections
);

extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifierConditions
(
	int16_t handle,
	int32_t *pwqConditionsArray,
	int16_t nConditions,
	PS5000A_CONDITIONS_INFO info
);

extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifierDirections
(
	int16_t handle,
	int32_t * pwqDirectionsArray,
	int16_t nDirections
);

extern PICO_STATUS PREF0 PREF1 SetPulseWidthDigitalPortProperties
(
	int16_t handle,
	int32_t * pwqDigitalDirections,
	int16_t nDirections
);
#endif
