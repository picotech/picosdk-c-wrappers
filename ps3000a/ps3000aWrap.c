/**************************************************************************
 *
 * Filename: ps3000aWrap.c
 *
 * Description:
 *   The source code in this release is for use with Pico products when 
 *	interfaced with Microsoft Excel VBA, National Instruments LabVIEW and 
 *	MathWorks MATLAB or any third-party programming language or application 
 *	that is unable to support C-style callback functions or structures.
 *
 *	You may modify, copy and distribute the source code for the purpose of 
 *	developing programs to collect data using Pico products to add new 
 *	functionality. If you modify the standard functions provided, we cannot 
 *	guarantee that they will work with the above-listed programming 
 *	languages or third-party products.
 *
 *  Please refer to the PicoScope 3000 Series (A API) Programmer's Guide
 *  for descriptions of the underlying functions where stated.
 *
 * Copyright (C) 2011-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 **************************************************************************/

#include "ps3000aWrap.h"

/////////////////////////////////
//
//	Function declarations
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See ps3000aStreamingReady (callback)
*
****************************************************************************/
void PREF1 StreamingCallback(
    int16_t handle,
	int32_t noOfSamples,
	uint32_t startIndex,
	int16_t overflow,
	uint32_t triggerAt,
	int16_t triggered,
	int16_t autoStop,
	void * pParameter)
{
	int16_t channel = 0;
	int16_t digitalPort = 0;
	WRAP_UNIT_INFO * wrapUnitInfo = NULL;
	
	if (pParameter != NULL)
	{
		wrapUnitInfo = (WRAP_UNIT_INFO *) pParameter;
	}

	// Assign values to structure
	wrapUnitInfo->numSamples = noOfSamples;
	wrapUnitInfo->autoStop = autoStop;
	wrapUnitInfo->startIndex = startIndex;

	wrapUnitInfo->triggered = triggered;
	wrapUnitInfo->triggeredAt = triggerAt;
  
	wrapUnitInfo->overflow = overflow;

	// Verify if wrapper buffer info set and data received
	if (wrapUnitInfo != NULL && noOfSamples)
	{
		for (channel = (int16_t) PS3000A_CHANNEL_A; channel < wrapUnitInfo->channelCount; channel++)
		{
			// Analogue channels
			if (wrapUnitInfo->enabledChannels[channel])
			{
				// Copy data...
				if (wrapUnitInfo->appBuffers && wrapUnitInfo->driverBuffers)
				{
					// Max buffers
					if (wrapUnitInfo->appBuffers[channel * 2]  && wrapUnitInfo->driverBuffers[channel * 2])
					{
						memcpy_s (&wrapUnitInfo->appBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t),
							&wrapUnitInfo->driverBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t));
					}

					// Min buffers
					if (wrapUnitInfo->appBuffers[channel * 2 + 1] && wrapUnitInfo->driverBuffers[channel * 2 + 1])
					{
						memcpy_s (&wrapUnitInfo->appBuffers[channel * 2 + 1][startIndex],  noOfSamples * sizeof(int16_t),
							&wrapUnitInfo->driverBuffers[channel * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
					}
				}
			}
		}

		// Digital channels
		if (wrapUnitInfo->digitalPortCount > 0)
		{
			// Use index 0 to indicate Digital Port 0
			for (digitalPort = (int16_t) PS3000A_WRAP_DIGITAL_PORT0; digitalPort < wrapUnitInfo->digitalPortCount; digitalPort++)
			{
				if (wrapUnitInfo->enabledDigitalPorts[digitalPort])
				{
					// Copy data...
					if (wrapUnitInfo->appDigiBuffers && wrapUnitInfo->driverDigiBuffers)
					{
						// Max digital buffers
						if (wrapUnitInfo->appDigiBuffers[digitalPort * 2]  && wrapUnitInfo->driverDigiBuffers[digitalPort * 2])
						{	
							memcpy_s (&wrapUnitInfo->appDigiBuffers[digitalPort * 2][startIndex], noOfSamples * sizeof(int16_t),
								&wrapUnitInfo->driverDigiBuffers[digitalPort * 2][startIndex], noOfSamples * sizeof(int16_t));
						}

						// Min digital buffers
						if (wrapUnitInfo->appDigiBuffers[digitalPort * 2 + 1]  && wrapUnitInfo->driverDigiBuffers[digitalPort * 2 + 1])
						{
							memcpy_s (&wrapUnitInfo->appDigiBuffers[digitalPort * 2 + 1][startIndex],  noOfSamples * sizeof(int16_t),
								&wrapUnitInfo->driverDigiBuffers[digitalPort * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
						}
					}
				}
			}
		}

	}

	wrapUnitInfo->ready = 1;
}

/****************************************************************************
* BlockCallback
*
* See ps3000aBlockReady (callback)
*
****************************************************************************/
void PREF1 BlockCallback(int16_t handle, PICO_STATUS status, void * pParameter)
{
	int16_t device_index = (int16_t) pParameter;
	
	g_deviceInfo[device_index].ready = 1;
	status = PICO_OK;

}

/****************************************************************************
* AutoStopped
*
* Indicates if the device has stopped on collection of the number of samples 
* specified in the call to the ps3000aRunStreaming function (if the 
* ps3000aRunStreaming function's autostop flag is set).
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
*
* Returns:
*
* Zero - if streaming has not stopped or the deviceIndex is out of range.
* Non-zero - if streaming has stopped automoatically.
*
****************************************************************************/
extern int16_t PREF0 PREF1 AutoStopped(uint16_t deviceIndex)
{
	int16_t autoStop = 0;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if ( g_deviceInfo[deviceIndex].ready ) 
		{
			autoStop = g_deviceInfo[deviceIndex].autoStop;
		}

	}
  
	return autoStop;
}

/****************************************************************************
* Available Data
*
* Returns the number of samples returned from the driver and shows the start 
* index of the data in the application buffer when collecting data in 
* streaming mode.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* startIndex - on exit, an index to the first valid sample in the buffer 
*				(When data is available).
*
* Returns:
*
* 0 - Data is not yet available or the device index is invalid.
* Non-zero - the number of samples returned from the driver.
*
****************************************************************************/
extern uint32_t PREF0 PREF1 AvailableData(uint16_t deviceIndex, uint32_t *startIndex)
{
	uint32_t numSamples = 0;

	if (deviceIndex < 0 || deviceIndex >= g_nextDeviceIndex)
	{
		numSamples = 0;
	}
	else
	{
		if ( g_deviceInfo[deviceIndex].ready ) 
		{
			*startIndex = g_deviceInfo[deviceIndex].startIndex;
			numSamples = g_deviceInfo[deviceIndex].numSamples;
		}
	}
	
	return numSamples;
}

/****************************************************************************
* ClearTriggerReady
*
* Clears the triggered and triggeredAt flags in relation to streaming mode 
* capture.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 ClearTriggerReady(uint16_t deviceIndex)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		g_deviceInfo[deviceIndex].triggered = FALSE;
		g_deviceInfo[deviceIndex].triggeredAt = 0;
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* decrementDeviceCount
*
* Reduces the count of the number of PicoScope devices being controlled by
* the application.
*
* NOTE: This function does not close the connection to the device being 
*		controlled - use the ps3000aCloseUnit function for this.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
*
* Returns:
*
* PICO_OK, if successful.
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 decrementDeviceCount(uint16_t deviceIndex)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		//Reset parameters
		g_deviceInfo[deviceIndex].handle = 0;
		g_deviceInfo[deviceIndex].autoStop = 0;
		g_deviceInfo[deviceIndex].channelCount = 0;
		g_deviceInfo[deviceIndex].digitalPortCount = 0;
		g_deviceInfo[deviceIndex].numSamples = 0;
		g_deviceInfo[deviceIndex].overflow = 0;
		g_deviceInfo[deviceIndex].startIndex = 0;
		g_deviceInfo[deviceIndex].triggered = 0;
		g_deviceInfo[deviceIndex].triggeredAt = 0;

		g_deviceCount = g_deviceCount - 1;
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* getDeviceCount
*
* Returns the number of PicoScope 3000 devices being controlled by the 
* application.
*
* Input Arguments:
*
* None
*
* Returns:
*
* The number of PicoScope 3000 devices being controlled.
*
****************************************************************************/
extern uint16_t PREF0 PREF1 getDeviceCount(void)
{
	return g_deviceCount;
}

/****************************************************************************
* GetStreamingLatestValues
*
* Facilitates communication with the driver to return the next block of 
* values to your application when capturing data in streaming mode. Use with 
* programming languages that do not support callback functions.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
*
* Returns:
*
* PICO_INVALID_PARAMETER, if deviceIndex is invalid.
* See also ps3000aGetStreamingLatestValues return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues(uint16_t deviceIndex)
{
	PICO_STATUS status;

	if (deviceIndex < 0 || deviceIndex >= g_nextDeviceIndex)
	{
		status = PICO_INVALID_PARAMETER;
	}
	else
	{
		g_deviceInfo[deviceIndex].ready = 0;
		g_deviceInfo[deviceIndex].numSamples = 0;
		g_deviceInfo[deviceIndex].autoStop = 0;

		status = ps3000aGetStreamingLatestValues(g_deviceInfo[deviceIndex].handle, StreamingCallback, &g_deviceInfo[deviceIndex]);
	}

	return status;
}

/****************************************************************************
* initWrapUnitInfo
*
* This function initialises a WRAP_UNIT_INFO structure for a PicoScope 3000
* series device and places it in the g_deviceInfo array at the next 
* available index.
*
* A maximum of 4 devices can be supported by the wrapper.
*
* The user's main application should map the handle to the index starting
* with the first handle corresponding to index 0.
*
* Input Arguments:
*
* handle - the handle of the required device.
* deviceIndex - on exit, the index at which the WRAP_UNIT_INFO structure will 
*				be stored in the g_deviceInfo array.
*
* Returns:
*
* PICO_OK, if successful.
* PICO_INVALID_HANDLE, if the handle is less than or equal to 0.
* PICO_MAX_UNITS_OPENED, if the wrapper already has records for the maximum
*						number of devices that it will support.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 initWrapUnitInfo(int16_t handle, uint16_t * deviceIndex)
{
	PICO_STATUS status = PICO_OK;
	struct tWrapUnitInfo wrapUnitInfo;
	uint16_t devIndex = g_nextDeviceIndex;

	if (handle <= 0)
	{
		status = PICO_INVALID_HANDLE;
	}
	else if (devIndex == WRAP_MAX_PICO_DEVICES)
	{
		status = PICO_MAX_UNITS_OPENED;
	}
	else
	{
		// Initialise struct
		memset(&wrapUnitInfo, 0, sizeof(WRAP_UNIT_INFO));

		wrapUnitInfo.handle = handle;
		g_deviceInfo[devIndex] = wrapUnitInfo;

		*deviceIndex = devIndex;
		
		g_nextDeviceIndex = g_nextDeviceIndex + 1;
		g_deviceCount = g_deviceCount + 1;		
	}

	return status;
}

/****************************************************************************
* IsReady
*
* This function is used to poll the driver to verify that data is ready to be 
* received. The RunBlock or GetStreamingLatestValues function must have been 
* called prior to calling this function.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
*
* Returns:
*
* 0 - Data is not yet available or deviceIndex is out of range.
* Non-zero - Data is ready to be collected.
*
****************************************************************************/
extern int16_t PREF0 PREF1 IsReady(uint16_t deviceIndex)
{
	int16_t ready = 0;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		ready = g_deviceInfo[deviceIndex].ready;
	}

	return ready;
}

/****************************************************************************
* IsTriggerReady
*
* Indicates whether a trigger has occurred when collecting data in streaming 
* mode, and the location of the trigger point in the overview buffer.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* triggeredAt - on exit, the index of the sample in the buffer where the 
*				trigger occurred, relative to the first valid sample index.
*				This value will be set to 0, when triggered is returned as 0.
*
* Returns:
*
* 0 - The device has not triggered, or deviceIndex is invalid.
* Non-zero - The device has been triggered.
*
****************************************************************************/
extern int16_t PREF0 PREF1 IsTriggerReady(uint16_t deviceIndex, uint32_t *triggeredAt)
{
	int16_t triggered = 0;
	*triggeredAt = 0;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (g_deviceInfo[deviceIndex].triggered)
		{
			triggered = g_deviceInfo[deviceIndex].triggered;
			*triggeredAt = g_deviceInfo[deviceIndex].triggeredAt;
		}
	}

	return triggered;
}

/****************************************************************************
* RunBlock
*
* This function starts collecting data in block mode without the requirement 
* for specifying callback functions. Use the IsReady function in conjunction 
* to poll the driver once this function has been called.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* preTriggerSamples - see noOfPreTriggerSamples in ps3000aRunBlock.
* postTriggerSamples - see noOfPreTriggerSamples in ps3000aRunBlock.
* timebase - see ps3000aRunBlock.
* segmentIndex - see ps3000aRunBlock.
*
* Returns:
*
* PICO_INVALID_PARAMETER, if deviceIndex or other paramters are invalid.
* See also ps3000aRunBlock return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 RunBlock(uint16_t deviceIndex, int32_t preTriggerSamples, int32_t postTriggerSamples,
            uint32_t timebase, uint32_t segmentIndex)
{
	PICO_STATUS status = PICO_OK;
	int16_t oversample = 1;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		g_deviceInfo[deviceIndex].ready = 0;
		g_deviceInfo[deviceIndex].numSamples = preTriggerSamples + postTriggerSamples;

		status = ps3000aRunBlock(g_deviceInfo[deviceIndex].handle, preTriggerSamples, postTriggerSamples, timebase, oversample, 
						NULL, segmentIndex, BlockCallback, (void *) deviceIndex);
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setAppAndDriverBuffers
*
* Set the application and corresponding driver buffer in order for the 
* streaming callback to copy the data for the analogue channel from the 
* driver buffer to the application buffer.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* channel - the channel number (should be a numerical value corresponding to
			a PS3000A_CHANNEL enumeration value).
* appBuffer - the application buffer.
* driverBuffer - the buffer set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*				 equal).
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds.
* PICO_INVALID_CHANNEL, if channel is not valid.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setAppAndDriverBuffers(uint16_t deviceIndex, int16_t channel, int16_t * appBuffer, int16_t * driverBuffer, int32_t bufferLength)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (channel >= PS3000A_CHANNEL_A && channel < g_deviceInfo[deviceIndex].channelCount)
		{
			g_deviceInfo[deviceIndex].appBuffers[channel * 2] = appBuffer;
			g_deviceInfo[deviceIndex].driverBuffers[channel * 2] = driverBuffer;
				
			g_deviceInfo[deviceIndex].bufferLengths[channel] = bufferLength;
		}
		else
		{
			status =  PICO_INVALID_CHANNEL;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setMaxMinAppAndDriverBuffers
*
* Set the application and corresponding driver buffers in order for the 
* streaming callback to copy the data for the analogue channel from the 
* driver max and min buffers to the respective application buffers for 
* aggregated data collection.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* channel - the channel number (should be a numerical value corresponding to
			a PS3000A_CHANNEL enumeration value).
* appMaxBuffer - the application max buffer.
* appMinBuffer - the application min buffer.
* driverMaxBuffer - the max buffer set by the driver.
* driverMinBuffer - the min buffer set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*				 equal).
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds.
* PICO_INVALID_CHANNEL, if channel is not valid.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers(uint16_t deviceIndex, int16_t channel, int16_t * appMaxBuffer, int16_t * appMinBuffer, int16_t * driverMaxBuffer, int16_t * driverMinBuffer, int32_t bufferLength)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (channel >= PS3000A_CHANNEL_A && channel < g_deviceInfo[deviceIndex].channelCount)
		{
			g_deviceInfo[deviceIndex].appBuffers[channel * 2] = appMaxBuffer;
			g_deviceInfo[deviceIndex].driverBuffers[channel * 2] = driverMaxBuffer;

			g_deviceInfo[deviceIndex].appBuffers[channel * 2 + 1] = appMinBuffer;
			g_deviceInfo[deviceIndex].driverBuffers[channel * 2 + 1] = driverMinBuffer;

			g_deviceInfo[deviceIndex].bufferLengths[channel] = bufferLength;
		}
		else
		{
			status = PICO_INVALID_CHANNEL;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setAppAndDriverDigiBuffers
*
* Set the application and corresponding driver buffer in order for the 
* streaming callback to copy the data for the digital port from the 
* driver buffer to the application buffer.
*
* This function applies to the PicoScope 3000 MSO models only.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* digiPort - the digital port number (0 or 1).
* appDigiBuffer - the application buffer for the digital port.
* driverDigiBuffer - the buffer for the digital port set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*				 equal).
*
* Returns:
*
* PICO_OK, if successful.
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds.
* PICO_INVALID_DIGITAL_PORT, if digiPort is not 0 (Port 0) or 1 (Port 1).
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setAppAndDriverDigiBuffers(uint16_t deviceIndex, int16_t digiPort, int16_t * appDigiBuffer, int16_t * driverDigiBuffer, int32_t bufferLength)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (digiPort == PS3000A_WRAP_DIGITAL_PORT0 || digiPort == PS3000A_WRAP_DIGITAL_PORT1)
		{
			g_deviceInfo[deviceIndex].appDigiBuffers[digiPort * 2] = appDigiBuffer;
			g_deviceInfo[deviceIndex].driverDigiBuffers[digiPort * 2] = driverDigiBuffer;
				
			g_deviceInfo[deviceIndex].digiBufferLengths[digiPort] = bufferLength;
		}
		else
		{
			status = PICO_INVALID_DIGITAL_PORT;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setMaxMinAppAndDriverDigiBuffers
*
* Set the application and corresponding driver buffers in order for the 
* streaming callback to copy the data for the digital port from the 
* driver max and min buffers to the respective application buffers for 
* aggregated data collection.
*
* This function applies to the PicoScope 3000 MSO models only.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* digiPort - the digital port number (0 or 1).
* appMaxDigiBuffer - the application max buffer for the digital port.
* appMinDigiBuffer - the application min buffer for the digital port.
* driverMaxDigiBuffer - the max buffer set by the driver for the digital port.
* driverMinDigiBuffer - the min buffer set by the driver for the digital port.
* bufferLength - the length of the buffers (the length of the buffers must be
*				 equal).
*
* Returns:
*
* PICO_OK, if successful.
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds.
* PICO_INVALID_DIGITAL_PORT, if digiPort is not 0 (Port 0) or 1 (Port 1).
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverDigiBuffers(uint16_t deviceIndex, int16_t digiPort, int16_t * appMaxDigiBuffer, int16_t * appMinDigiBuffer, int16_t * driverMaxDigiBuffer, int16_t * driverMinDigiBuffer, int32_t bufferLength)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (digiPort == PS3000A_WRAP_DIGITAL_PORT0 || digiPort == PS3000A_WRAP_DIGITAL_PORT1)
		{
			g_deviceInfo[deviceIndex].appDigiBuffers[digiPort * 2] = appMaxDigiBuffer;
			g_deviceInfo[deviceIndex].driverDigiBuffers[digiPort * 2] = driverMaxDigiBuffer;

			g_deviceInfo[deviceIndex].appDigiBuffers[digiPort * 2 + 1] = appMinDigiBuffer;
			g_deviceInfo[deviceIndex].driverDigiBuffers[digiPort * 2 + 1] = driverMinDigiBuffer;

			g_deviceInfo[deviceIndex].digiBufferLengths[digiPort] = bufferLength;
		}
		else
		{
			status = PICO_INVALID_DIGITAL_PORT;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setChannelCount
*
* Set the number of analogue channels on the device. This is used to assist 
* with copying data in the streaming callback.
*
* The initWrapUnitInfo must have been called before this function is called.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* channelCount - the number of channels on the device.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds or channelCount
*							is not 2 or 4.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setChannelCount(uint16_t deviceIndex, int16_t channelCount)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (channelCount == DUAL_SCOPE || channelCount == PS3000A_MAX_CHANNELS)
		{
			g_deviceInfo[deviceIndex].channelCount = channelCount;

			status = PICO_OK;
		}
		else
		{
			status = PICO_INVALID_PARAMETER;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setEnabledChannels
*
* Set the number of enabled analogue channels on the device. This is used to 
* assist with copying data in the streaming callback.
*
* The setChannelCount function must be called prior to calling this function.
*
* Input Arguments:
*
* deviceIndex - the index in the g_deviceInfo array corresponding to the
*				required device.
* enabledChannels - an array representing the channel states. This must be 
*					4 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, deviceIndex is out of bounds or channelCount
*							is not 2 or 4.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledChannels(uint16_t deviceIndex, int16_t * enabledChannels)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (g_deviceInfo[deviceIndex].channelCount == DUAL_SCOPE || g_deviceInfo[deviceIndex].channelCount == PS3000A_MAX_CHANNELS)
		{
			memcpy_s((int16_t *) g_deviceInfo[deviceIndex].enabledChannels, PS3000A_MAX_CHANNELS * sizeof(int16_t), 
				(int16_t *) enabledChannels, PS3000A_MAX_CHANNELS * sizeof(int16_t));

			status = PICO_OK;
		}
		else
		{
			status = PICO_INVALID_PARAMETER;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setDigitalPortCount
*
* Set the number of digital ports on the device. This is used to assist 
* with copying data in the streaming callback.
*
* The initWrapUnitInfo must have been called before this function is called.
*
* Input Arguments:
*
* deviceIndex - the index assigned by the wrapper corresponding to the 
*				required device.
* digitalPortCount - the number of digital ports on the device. This should
*					be set to 2 for the PicoScope 3000 MSO devices and 0 for 
*					other models.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, deviceIndex is out of bounds or digitalPortCount
							is invalid.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setDigitalPortCount(uint16_t deviceIndex, int16_t digitalPortCount)
{
	PICO_STATUS status = PICO_OK;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (digitalPortCount == 0 || digitalPortCount == DUAL_PORT_MSO || digitalPortCount == PS3000A_MAX_DIGITAL_PORTS)
		{
			g_deviceInfo[deviceIndex].digitalPortCount = digitalPortCount;

			status = PICO_OK;
		}
		else
		{
			status = PICO_INVALID_PARAMETER;
		}
	}
	else
	{
		status = PICO_INVALID_PARAMETER;
	}

	return status;
}

/****************************************************************************
* setEnabledDigitalPorts
*
* Set the number of enabled digital ports on the device. This is used to 
* assist with copying data in the streaming callback.
*
* For PicoScope 3000 MSO models, the setDigitalPortCount function must have
* been called, otherwise this function does not need to be called.
*
* Input Arguments:
*
* deviceIndex - the index in the g_deviceInfo array corresponding to the
*				required device.
* enabledDigitalPorts - an array representing the digital port states. This 
*						must be 4 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_PARAMETER, if deviceIndex is out of bounds, or 
*							digitalPortCount is invalid.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledDigitalPorts(uint16_t deviceIndex, int16_t * enabledDigitalPorts)
{
	PICO_STATUS status = PICO_OK;

	int16_t digiPortCount = g_deviceInfo[deviceIndex].digitalPortCount;

	if (deviceIndex >= 0 && deviceIndex < g_nextDeviceIndex)
	{
		if (digiPortCount == 0 || digiPortCount == DUAL_PORT_MSO || digiPortCount == PS3000A_MAX_DIGITAL_PORTS)
		{
			memcpy_s((int16_t *) g_deviceInfo[deviceIndex].enabledDigitalPorts, PS3000A_MAX_DIGITAL_PORTS * sizeof(int16_t), 
				(int16_t *) enabledDigitalPorts, PS3000A_MAX_DIGITAL_PORTS * sizeof(int16_t));

		}
		else
		{
			status = PICO_INVALID_PARAMETER;
		}
	}
	else
	{
		status = PICO_INVALID_HANDLE;
	}

	return status;
}

/****************************************************************************
* SetPulseWidthQualifier
*
* This function sets up pulse-width qualification, which can be used on its 
* own for pulse-width triggering or combined with level triggering or window 
* triggering to produce more complex triggers. The pulse-width qualifier is 
* defined by one or more sets of integers corresponding to 
* PS3000A_PWQ_CONDITIONS structures which are then converted and passed 
* to the ps3000aSetPulseWidthQualifier function.
*
* Use this function with programming languages that do not support structs.
* Use the SetPulseWidthQualifierV2 function for PicoScope 3000 MSO models.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqConditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of 
*				pwqConditionsArray elements / 6)
* direction - the direction of the signal required for the pulse width
*				trigger to fire (see PS3000A_THRESHOLD_DIRECTION enumerations).
* lower - the lower limit of the pulse-width counter, measured in samples.
* upper - the upper limit of the pulse-width counter, measured in samples.
* type - the pulse-width type (see PS3000A_PULSE_WIDTH_TYPE enumerations).
*
* Returns:
*
* See ps3000aSetPulseWidthQualifier return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifier(int16_t handle, uint32_t *pwqConditionsArray, int16_t nConditions,
																			uint32_t direction, uint32_t lower, uint32_t upper, uint32_t type)
{
	// Allocate memory
	PS3000A_PWQ_CONDITIONS *pwqConditions = (PS3000A_PWQ_CONDITIONS *) calloc(nConditions, sizeof(PS3000A_PWQ_CONDITIONS));

	int16_t i;
	int16_t j = 0;

	PICO_STATUS status;

	for (i = 0; i < nConditions; i++)
	{
		pwqConditions[i].channelA = (PS3000A_TRIGGER_STATE) pwqConditionsArray[j];
		pwqConditions[i].channelB = (PS3000A_TRIGGER_STATE) pwqConditionsArray[j + 1];
		pwqConditions[i].channelC = (PS3000A_TRIGGER_STATE) pwqConditionsArray[j + 2];
		pwqConditions[i].channelD = (PS3000A_TRIGGER_STATE) pwqConditionsArray[j + 3];
		pwqConditions[i].external = (PS3000A_TRIGGER_STATE) pwqConditionsArray[j + 4];
		pwqConditions[i].aux      = (PS3000A_TRIGGER_STATE) pwqConditionsArray[j + 5];

		j = j + 6;
	}

	status = ps3000aSetPulseWidthQualifier(handle, pwqConditions, nConditions, (PS3000A_THRESHOLD_DIRECTION) direction, lower, upper, (PS3000A_PULSE_WIDTH_TYPE) type);
	free(pwqConditions);
	return status;
}

/****************************************************************************
* SetPulseWidthQualifierV2
*
* This function sets up pulse-width qualification, which can be used on its 
* own for pulse-width triggering or combined with level triggering or window 
* triggering to produce more complex triggers. 
* The pulse-width qualifier is defined by one or more sets of integers 
* corresponding to PS3000A_PWQ_CONDITIONS_V2 structures which are then 
* converted and passed to the ps3000aSetPulseWidthQualifierV2 function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqConditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of 
*				pwqConditionsArray elements / 7)
* direction - the direction of the signal required for the pulse width
*				trigger to fire (see PS3000A_THRESHOLD_DIRECTION enumerations).
* lower - the lower limit of the pulse-width counter, measured in samples.
* upper - the upper limit of the pulse-width counter, measured in samples.
* type - the pulse-width type (see PS3000A_PULSE_WIDTH_TYPE enumerations).
*
* Returns:
*
* See ps3000aSetPulseWidthQualifier return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifierV2( int16_t handle, uint32_t *pwqConditionsArrayV2,
																				int16_t nConditions, uint32_t direction, uint32_t lower,
																				uint32_t upper, uint32_t type)
{
	// Allocate memory
	PS3000A_PWQ_CONDITIONS_V2 *pwqConditionsV2 = (PS3000A_PWQ_CONDITIONS_V2 *) calloc(nConditions, sizeof(PS3000A_PWQ_CONDITIONS_V2));

	int16_t i;
	int16_t j = 0;

	PICO_STATUS status;

	for (i = 0; i < nConditions; i++)
	{
		pwqConditionsV2[i].channelA = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j];
		pwqConditionsV2[i].channelB = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j + 1];
		pwqConditionsV2[i].channelC = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j + 2];
		pwqConditionsV2[i].channelD = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j + 3];
		pwqConditionsV2[i].external = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j + 4];
		pwqConditionsV2[i].aux      = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j + 5];
		pwqConditionsV2[i].digital  = (PS3000A_TRIGGER_STATE) pwqConditionsArrayV2[j + 6];

		j = j + 7;
	}

	status = ps3000aSetPulseWidthQualifierV2(handle, pwqConditionsV2, nConditions, (PS3000A_THRESHOLD_DIRECTION) direction, lower, upper, (PS3000A_PULSE_WIDTH_TYPE) type);
	free(pwqConditionsV2);
	return status;
}

/****************************************************************************
* SetTriggerConditions
*
* This function sets up trigger conditions on the scope's inputs. The 
* trigger is defined by one or more sets of integers corresponding to 
* PS3000A_TRIGGER_CONDITIONS structures which are then converted and passed 
* to the ps3000aSetTriggerChannelConditions function.
*
* Use this function with programming languages that do not support structs.
* Use the SetTriggerConditionsV2 function for PicoScope 3000 MSO models.
*
* Input Arguments:
*
* handle - the handle of the required device.
* conditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of conditionsArray 
*				elements / 7)
*
* Returns:
*
* See ps3000aSetTriggerChannelConditions return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditions(int16_t handle, uint32_t *conditionsArray, int16_t nConditions)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS3000A_TRIGGER_CONDITIONS *conditions = (PS3000A_TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(PS3000A_TRIGGER_CONDITIONS));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA				= (PS3000A_TRIGGER_STATE) conditionsArray[j];
		conditions[i].channelB				= (PS3000A_TRIGGER_STATE) conditionsArray[j + 1];
		conditions[i].channelC				= (PS3000A_TRIGGER_STATE) conditionsArray[j + 2];
		conditions[i].channelD				= (PS3000A_TRIGGER_STATE) conditionsArray[j + 3];
		conditions[i].external				= (PS3000A_TRIGGER_STATE) conditionsArray[j + 4];
		conditions[i].aux					= (PS3000A_TRIGGER_STATE) conditionsArray[j + 5];
		conditions[i].pulseWidthQualifier	= (PS3000A_TRIGGER_STATE) conditionsArray[j + 6];

		j = j + 7;
	}
	status = ps3000aSetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return status;
}

/****************************************************************************
* SetTriggerConditionsV2
*
* This function sets up trigger conditions on the scope's inputs. The 
* trigger is defined by one or more sets of integers corresponding to 
* PS3000A_TRIGGER_CONDITIONS_V2 structures which are then converted and passed 
* to the ps3000aSetTriggerChannelConditionsV2 function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* conditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of conditionsArray 
*				elements / 8)
*
* Returns:
*
* See ps3000aSetTriggerChannelConditionsV2 return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditionsV2(int16_t handle, uint32_t *conditionsArrayV2, int16_t nConditions)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS3000A_TRIGGER_CONDITIONS_V2 *conditions = (PS3000A_TRIGGER_CONDITIONS_V2 *) calloc (nConditions, sizeof(PS3000A_TRIGGER_CONDITIONS_V2));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA				= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j];
		conditions[i].channelB				= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 1];
		conditions[i].channelC				= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 2];
		conditions[i].channelD				= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 3];
		conditions[i].external				= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 4];
		conditions[i].aux					= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 5];
		conditions[i].pulseWidthQualifier	= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 6];
		conditions[i].digital				= (PS3000A_TRIGGER_STATE) conditionsArrayV2[j + 7];

		j = j + 8;
	}
	status = ps3000aSetTriggerChannelConditionsV2(handle, conditions, nConditions);
	free (conditions);

	return status;
}

/****************************************************************************
* SetTriggerProperties
*
* This function is used to enable or disable triggering and set its 
* parameters by means of assigning the values from the propertiesArray to an 
* array of PS3000A_TRIGGER_CHANNEL_PROPERTIES structures which are then 
* passed to the ps3000aSetTriggerChannelProperties function with the other 
* parameters.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to 
*					PS3000A_TRIGGER_CHANNEL_PROPERTIES structures describing 
*					the required properties to be set. See also 
*					channelProperties in ps3000aSetTriggerChannelProperties.
*
* nProperties - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of propertiesArray 
*				elements / 6)
* autoTrig - see autoTriggerMilliseconds in ps3000aSetTriggerChannelProperties.
*
*
* Returns:
*
* See ps3000aSetTriggerChannelProperties return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerProperties(int16_t handle, uint32_t *propertiesArray, int16_t nProperties, int32_t autoTrig)
{
	PS3000A_TRIGGER_CHANNEL_PROPERTIES *channelProperties = (PS3000A_TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(PS3000A_TRIGGER_CHANNEL_PROPERTIES));
	int16_t i;
	int16_t j = 0;
	PICO_STATUS status;
	int16_t auxEnable = 0;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper				= propertiesArray[j];
		channelProperties[i].thresholdUpperHysteresis	= propertiesArray[j + 1];
		channelProperties[i].thresholdLower				= propertiesArray[j + 2];
		channelProperties[i].thresholdLowerHysteresis	= propertiesArray[j + 3];
		channelProperties[i].channel					= (PS3000A_CHANNEL) propertiesArray[j + 4];
		channelProperties[i].thresholdMode				= (PS3000A_THRESHOLD_MODE) propertiesArray[j + 5];

		j = j + 6;
	}
	status = ps3000aSetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	return status;
}

/****************************************************************************
* resetNextDeviceIndex
*
* This function is used to reset the index used to determine the next point 
* at which to store a WRAP_UNIT_INFO structure.
*
* This function should only be called once the devices have been disconnected.
*
* Input Arguments:
*
* None
*
* Returns:
*
* PICO_OK
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 resetNextDeviceIndex(void)
{
	g_nextDeviceIndex = 0;

	return PICO_OK;
}
