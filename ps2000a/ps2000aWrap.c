/**************************************************************************
 *
 * Filename: ps2000aWrap.c
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
 *  Please refer to the PicoScope 2000 Series (A API) Programmer's Guide
 *  for descriptions of the underlying functions where stated.
 *
 * Copyright (C) 2011-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 **************************************************************************/


#include "ps2000aWrap.h"


/////////////////////////////////
//
//	Function declarations
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See ps2000aStreamingReady (callback)
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
	WRAP_BUFFER_INFO * wrapBufferInfo = NULL;
	
	if (pParameter != NULL)
	{
		wrapBufferInfo = (WRAP_BUFFER_INFO *) pParameter;
	}

	g_numSamples = noOfSamples;
	g_autoStop = autoStop;
	g_startIndex = startIndex;

	g_triggered = triggered;
	g_triggeredAt = triggerAt;
  
	g_overflow = overflow;

	// Verify if wrapper buffer info set and data received
	if (wrapBufferInfo != NULL && noOfSamples)
	{
		for (channel = (int16_t) PS2000A_CHANNEL_A; channel < g_channelCount; channel++)
		{
			// Analogue channels
			if (g_enabledChannels[channel])
			{

				// Copy data...
				if (wrapBufferInfo->appBuffers && wrapBufferInfo->driverBuffers)
				{
					// Max buffers
					if (wrapBufferInfo->appBuffers[channel * 2]  && wrapBufferInfo->driverBuffers[channel * 2])
					{
						memcpy_s (&wrapBufferInfo->appBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t),
							&wrapBufferInfo->driverBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t));
					}

					// Min buffers
					if (wrapBufferInfo->appBuffers[channel * 2 + 1] && wrapBufferInfo->driverBuffers[channel * 2 + 1])
					{
						memcpy_s (&wrapBufferInfo->appBuffers[channel * 2 + 1][startIndex],  noOfSamples * sizeof(int16_t),
							&wrapBufferInfo->driverBuffers[channel * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
					}
				}
			}
		}

		// Digital channels
		if (g_digitalPortCount > 0)
		{
			// Use index 0 to indicate Digital Port 0
			for(digitalPort = (int16_t) PS2000A_WRAP_DIGITAL_PORT0; digitalPort < g_digitalPortCount; digitalPort++)
			{
				if (g_enabledDigitalPorts[digitalPort])
				{
					// Copy data...
					if (wrapBufferInfo->appDigiBuffers && wrapBufferInfo->driverDigiBuffers)
					{
						// Max digital buffers
						if (wrapBufferInfo->appDigiBuffers[digitalPort * 2]  && wrapBufferInfo->driverDigiBuffers[digitalPort * 2])
						{
							
							memcpy_s (&wrapBufferInfo->appDigiBuffers[digitalPort * 2][startIndex], noOfSamples * sizeof(int16_t),
								&wrapBufferInfo->driverDigiBuffers[digitalPort * 2][startIndex], noOfSamples * sizeof(int16_t));
						}

						// Min digital buffers
						if (wrapBufferInfo->appDigiBuffers[digitalPort * 2 + 1]  && wrapBufferInfo->driverDigiBuffers[digitalPort * 2 + 1])
						{
							// Min digital buffers
							memcpy_s (&wrapBufferInfo->appDigiBuffers[digitalPort * 2 + 1][startIndex],  noOfSamples * sizeof(int16_t),
								&wrapBufferInfo->driverDigiBuffers[digitalPort * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
						}
					}
				}
			}
		}

	}

	g_ready = 1;
}

/****************************************************************************
* BlockCallback
*
* See ps2000aBlockReady (callback)
*
****************************************************************************/
void PREF1 BlockCallback(int16_t handle, PICO_STATUS status, void * pParameter)
{
	g_ready = 1;
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
* handle - the handle of the required device.
* preTriggerSamples - see noOfPreTriggerSamples in ps2000aRunBlock.
* postTriggerSamples - see noOfPreTriggerSamples in ps2000aRunBlock.
* timebase - see ps2000aRunBlock.
* segmentIndex - see ps2000aRunBlock.
*
* Returns:
*
* See ps2000aRunBlock return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 RunBlock(int16_t handle, int32_t preTriggerSamples, int32_t postTriggerSamples,
            uint32_t timebase, uint32_t segmentIndex)
{
	int16_t oversample = 0;
	g_ready = 0;
	g_numSamples = preTriggerSamples + postTriggerSamples;

	return ps2000aRunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, oversample, 
    NULL, segmentIndex, BlockCallback, NULL);
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
* handle - the handle of the required device.
*
* Returns:
*
* See ps2000aGetStreamingLatestValues return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues(int16_t handle)
{
	g_ready = 0;
	g_numSamples = 0;
	g_autoStop = 0;

	return ps2000aGetStreamingLatestValues(handle, StreamingCallback, &g_wrapBufferInfo);
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
* handle - the handle of the required device.
* startIndex - on exit, an index to the first valid sample in the buffer 
*				(When data is available).
*
* Returns:
*
* 0 - Data is not yet available.
* Non-zero - the number of samples returned from the driver.
*
****************************************************************************/
extern int32_t PREF0 PREF1 AvailableData(int16_t handle, uint32_t *startIndex)
{
	if( g_ready ) 
	{
		*startIndex = g_startIndex;
		return g_numSamples;
	}
	
	return (int32_t) 0;
}

/****************************************************************************
* AutoStopped
*
* Indicates if the device has stopped on collection of the number of samples 
* specified in the call to the ps2000aRunStreaming function (if the 
* ps2000aRunStreaming function's autostop flag is set).
*
* Input Arguments:
*
* handle - the handle of the required device.
*
* Returns:
*
* Non-zero - if streaming has autostopped.
*
****************************************************************************/
extern int16_t PREF0 PREF1 AutoStopped(int16_t handle)
{
	if( g_ready) 
	{
		return g_autoStop;
	}
	
	return 0;
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
* handle - the handle of the required device.
*
* Returns:
*
* 0 - Data is not yet available.
* Non-zero - Data is ready to be collected.
*
****************************************************************************/
extern int16_t PREF0 PREF1 IsReady(int16_t handle)
{
	return g_ready;
}

/****************************************************************************
* IsTriggerReady
*
* Indicates whether a trigger has occurred when collecting data in streaming 
* mode, and the location of the trigger point in the buffer.
*
* Input Arguments:
*
* handle - the handle of the required device.
* triggeredAt - on exit, the index of the sample in the buffer where the 
* trigger occurred.
*
* Returns:
*
* 0 - The device has not triggered.
* Non-zero - The device has been triggered.
*
****************************************************************************/
extern int16_t PREF0 PREF1 IsTriggerReady(int16_t handle, uint32_t *triggeredAt)
{
	if (g_triggered)
	{
		*triggeredAt = g_triggeredAt;
	}
	
	return g_triggered;
}

/****************************************************************************
* ClearTriggerReady
*
* Clears the triggered and triggeredAt flags in relation to streaming mode 
* capture.
*
* Input Arguments:
*
* None
*
* Returns:
*
* 1
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 ClearTriggerReady(int16_t handle)
{
	g_triggeredAt = 0;
	g_triggered = FALSE;
	return PICO_OK;
}

/****************************************************************************
* SetTriggerConditions
*
* This function sets up trigger conditions on the scope's inputs. The 
* trigger is defined by one or more sets of integers corresponding to 
* PS2000A_TRIGGER_CONDITIONS structures which are then converted and passed 
* to the ps2000aSetTriggerChannelConditions function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* conditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
* created its structures. (i.e. the number of conditionsArray elements / 8)
*
* Returns:
*
* See ps2000aSetTriggerChannelConditions return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditions(int16_t handle, uint32_t *conditionsArray, int16_t nConditions)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS2000A_TRIGGER_CONDITIONS *conditions = (PS2000A_TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(PS2000A_TRIGGER_CONDITIONS));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA	= (PS2000A_TRIGGER_STATE) conditionsArray[j];
		conditions[i].channelB	= (PS2000A_TRIGGER_STATE) conditionsArray[j + 1];
		conditions[i].channelC	= (PS2000A_TRIGGER_STATE) conditionsArray[j + 2];
		conditions[i].channelD	= (PS2000A_TRIGGER_STATE) conditionsArray[j + 3];
		conditions[i].external	= (PS2000A_TRIGGER_STATE) conditionsArray[j + 4];
		conditions[i].aux		= (PS2000A_TRIGGER_STATE) conditionsArray[j + 5];
		conditions[i].pulseWidthQualifier = (PS2000A_TRIGGER_STATE) conditionsArray[j + 6];
		conditions[i].digital = (PS2000A_TRIGGER_STATE) conditionsArray[j + 7];

		j = j + 8;
	}
	status = ps2000aSetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return status;
}

/****************************************************************************
* SetTriggerProperties
*
* This function is used to enable or disable triggering and set its 
* parameters by means of assigning the values from the propertiesArray to an 
* array of PS2000A_TRIGGER_CHANNEL_PROPERTIES structures which are then 
* passed to the ps2000aSetTriggerChannelProperties function with the other 
* parameters.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to 
*					PS2000A_TRIGGER_CHANNEL_PROPERTIES structures describing 
*					the required properties to be set. See also 
*					channelProperties in ps2000aSetTriggerChannelProperties.
*
* nProperties - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of propertiesArray 
*				elements / 6)
* autoTrig - see autoTriggerMilliseconds in ps2000aSetTriggerChannelProperties.
*
*
* Returns:
*
* See ps2000aSetTriggerChannelProperties return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerProperties(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties, 
	int32_t autoTrig)
{
	PS2000A_TRIGGER_CHANNEL_PROPERTIES *channelProperties = (PS2000A_TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(PS2000A_TRIGGER_CHANNEL_PROPERTIES));
	
	int16_t i;
	int16_t j = 0;
	int16_t auxEnable = 0;
	PICO_STATUS status;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper				= propertiesArray[j];
		channelProperties[i].thresholdUpperHysteresis	= propertiesArray[j + 1];
		channelProperties[i].thresholdLower				= propertiesArray[j + 2];
		channelProperties[i].thresholdLowerHysteresis	= propertiesArray[j + 3];
		channelProperties[i].channel					= (PS2000A_CHANNEL) propertiesArray[j + 4];
		channelProperties[i].thresholdMode				= (PS2000A_THRESHOLD_MODE) propertiesArray[j + 5];

		j= j + 6;
	}
	status = ps2000aSetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	return status;
}

/****************************************************************************
* SetPulseWidthQualifier
*
* This function is used to pulse-width triggering and set 
* its parameters by means of assigning the values from the pwqConditionsArray 
* to an array of PS2000A_PWQ_CONDITIONS structures which are then passed to 
* the ps2000aSetPulseWidthQualifier function with the other parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqConditionsArray - an array of sets of integers corresponding to 
*					PS2000A_PWQ_CONDITIONS structures describing the 
*					required conditions to be set. See also conditions
*					in ps2000aSetPulseWidthQualifier.
*
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of conditionsArray 
*				elements / 7)
*
* direction - the direction of the signal required for the pulse width
*				trigger to fire (See PS2000A_THRESHOLD_DIRECTION constants)
* lower - the lower limit of the pulse-width counter with relation to
			number of samples captured on the device.
* upper - the upper limit of the pulse-width counter with relation to
*			number of samples captured on the device.
* type - the pulse-width type (see ps2000aSetPulseWidthQualifier).
*
* Returns:
*
* See ps2000aSetPulseWidthQualifier return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifier(
	int16_t handle,
	uint32_t *pwqConditionsArray,
	int16_t nConditions,
	uint32_t direction,
	uint32_t lower,
	uint32_t upper,
	uint32_t type)
{
	// Allocate memory
	PS2000A_PWQ_CONDITIONS *pwqConditions = (PS2000A_PWQ_CONDITIONS *) calloc(nConditions, sizeof(PS2000A_PWQ_CONDITIONS));

	int16_t i;
	int16_t j = 0;

	PICO_STATUS status;

	for(i = 0; i < nConditions; i++)
	{
		pwqConditions[i].channelA = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j];
		pwqConditions[i].channelB = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j + 1];
		pwqConditions[i].channelC = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j + 2];
		pwqConditions[i].channelD = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j + 3];
		pwqConditions[i].external = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j + 4];
		pwqConditions[i].aux      = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j + 5];
		pwqConditions[i].digital  = (PS2000A_TRIGGER_STATE) pwqConditionsArray[j + 6];

		j = j + 7;
	}

	status = ps2000aSetPulseWidthQualifier(handle, pwqConditions, nConditions, (PS2000A_THRESHOLD_DIRECTION) direction, 
		lower, upper, (PS2000A_PULSE_WIDTH_TYPE) type);
	
	free(pwqConditions);
	return status;
}

/****************************************************************************
* setChannelCount
*
* Set the number of analogue channels and digital ports on the device. 
* This is used to assist with copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
*
* Returns:
*
* See ps2000aGetUnitInfo return values

****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setChannelCount(int16_t handle)
{
	int8_t variant[15];
	int16_t requiredSize = 0;
	PICO_STATUS status = PICO_OK;

	// Obtain the model number
	status = ps2000aGetUnitInfo(handle, variant, sizeof (variant), &requiredSize, PICO_VARIANT_INFO);

	if(status == PICO_OK)
	{
		// Set the number of analogue channels
		g_channelCount = (int16_t) variant[1];
		g_channelCount = g_channelCount - 48; // Subtract ASCII 0 (48)

		// Determine if the device is an MSO
		if (strstr(variant, "MSO") != NULL)
		{
			g_digitalPortCount = 2;
		}
		else
		{
			g_digitalPortCount = 0;
		}

	}
	
	return status;

}

/****************************************************************************
* setEnabledChannels
*
* Set the number of enabled analogue channels on the device. This is used to 
* assist with copying data in the streaming callback.
*
* setChannelCount should be called prior to calling this function.
*
* Input Arguments:
*
* handle - the device handle.
* enabledChannels - an array representing the channel states. This must be 
*					4 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if g_channelCount is invalid
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{
	if(handle > 0)
	{
		if(g_channelCount > 0 && g_channelCount <= PS2000A_MAX_CHANNELS)
		{
			memcpy_s((int16_t *) g_enabledChannels, PS2000A_MAX_CHANNELS * sizeof(int16_t), 
				(int16_t *) enabledChannels, PS2000A_MAX_CHANNELS * sizeof(int16_t));

			return PICO_OK;
		}
		else
		{
			return PICO_INVALID_PARAMETER;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}

/****************************************************************************
* setDigitalPortCount
*
* Set the number of digital ports on the device. This is used to assist 
* with copying data in the streaming callback.
*
* NOTE: This function has been deprecated, please use setChannelCount instead.
*
* Input Arguments:
*
* handle - the device handle.
* digitalPortCount - the number of digital ports on the device. This should
*					be set to 2 for the PicoScope 2205 MSO and 0 for other 
*					models.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if digitalPortCount is not 0 or 2
****************************************************************************/
//extern PICO_STATUS PREF0 PREF1 setDigitalPortCount(int16_t handle, int16_t digitalPortCount)
//{
//	if(handle > 0)
//	{
//		if(digitalPortCount == 0 || digitalPortCount == MAX_DIGITAL_PORTS)
//		{
//			g_digitalPortCount = digitalPortCount;
//
//			return PICO_OK;
//		}
//		else
//		{
//			return PICO_INVALID_PARAMETER;
//		}
//	}
//	else
//	{
//		return PICO_INVALID_HANDLE;
//	}
//}

/****************************************************************************
* setEnabledDigitalPorts
*
* Set the number of enabled digital ports on the device. This is used to 
* assist with copying data in the streaming callback.
*
* This function does not need to be called for non-MSO models.
*
* Input Arguments:
*
* handle - the device handle.
* enabledDigitalPorts - an array representing the digital port states. This 
*						must be 2 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if _digitalPortCount is not 0 or 2
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledDigitalPorts(int16_t handle, int16_t * enabledDigitalPorts)
{
	if(handle > 0)
	{
		if(g_digitalPortCount == 0 || g_digitalPortCount == MAX_DIGITAL_PORTS)
		{
			memcpy_s((int16_t *) g_enabledDigitalPorts, MAX_DIGITAL_PORTS * sizeof(int16_t), 
				(int16_t *) enabledDigitalPorts, MAX_DIGITAL_PORTS * sizeof(int16_t));

			return PICO_OK;
		}
		else
		{
			return PICO_INVALID_PARAMETER;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
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
* handle - the device handle.
* channel - the channel number (should be a numerical value corresponding to
			a PS2000A_CHANNEL enumeration value).
* appBuffer - the application buffer.
* driverBuffer - the buffer set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*				 equal).
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is invalid
* PICO_INVALID_CHANNEL, if channel is not in range
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setAppAndDriverBuffers(int16_t handle, int16_t channel, int16_t * appBuffer, int16_t * driverBuffer, int32_t bufferLength)
{
	if(handle > 0)
	{
		if(channel < PS2000A_CHANNEL_A || channel >= g_channelCount)
		{
			return PICO_INVALID_CHANNEL;
		}
		else
		{
			g_wrapBufferInfo.appBuffers[channel * 2] = appBuffer;
			g_wrapBufferInfo.driverBuffers[channel * 2] = driverBuffer;
				
			g_wrapBufferInfo.bufferLengths[channel] = bufferLength;

			return PICO_OK;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
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
* handle - the device handle.
* channel - the channel number (should be a numerical value corresponding to
			a PS2000A_CHANNEL enumeration value).
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
* PICO_INVALID_HANDLE, if handle is invalid
* PICO_INVALID_CHANNEL, if channel is not in range
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers(int16_t handle, int16_t channel, int16_t * appMaxBuffer, int16_t * appMinBuffer, int16_t * driverMaxBuffer, int16_t * driverMinBuffer, int32_t bufferLength)
{
	if(handle > 0)
	{
		if(channel < PS2000A_CHANNEL_A || channel >= g_channelCount)
		{
			return PICO_INVALID_CHANNEL;
		}
		else
		{
			g_wrapBufferInfo.appBuffers[channel * 2] = appMaxBuffer;
			g_wrapBufferInfo.driverBuffers[channel * 2] = driverMaxBuffer;

			g_wrapBufferInfo.appBuffers[channel * 2 + 1] = appMinBuffer;
			g_wrapBufferInfo.driverBuffers[channel * 2 + 1] = driverMinBuffer;

			g_wrapBufferInfo.bufferLengths[channel] = bufferLength;

			return PICO_OK;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}

/****************************************************************************
* setAppAndDriverDigiBuffers
*
* Set the application and corresponding driver buffer in order for the 
* streaming callback to copy the data for the digital port from the 
* driver buffer to the application buffer.
*
* This function applies to the PicoScope 2205 MSO only.
*
* Input Arguments:
*
* handle - the device handle.
* digiPort - the digital port number (0 or 1).
* appDigiBuffer - the application buffer for the digital port.
* driverDigitalBuffer - the buffer for the digital port set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*				 equal).
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is invalid
* PICO_INVALID_DIGITAL_PORT, if digiPort is not 0 (Port 0) or 1 (Port 1)
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setAppAndDriverDigiBuffers(int16_t handle, int16_t digiPort, int16_t * appDigiBuffer, int16_t * driverDigiBuffer, int32_t bufferLength)
{
	if(handle > 0)
	{
		if(digiPort < PS2000A_WRAP_DIGITAL_PORT0 || digiPort > PS2000A_WRAP_DIGITAL_PORT1)
		{
			return PICO_INVALID_DIGITAL_PORT;
		}
		else
		{
			g_wrapBufferInfo.appDigiBuffers[digiPort * 2] = appDigiBuffer;
			g_wrapBufferInfo.driverDigiBuffers[digiPort * 2] = driverDigiBuffer;
				
			g_wrapBufferInfo.digiBufferLengths[digiPort] = bufferLength;

			return PICO_OK;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}

/****************************************************************************
* setMaxMinAppAndDriverDigiBuffers
*
* Set the application and corresponding driver buffers in order for the 
* streaming callback to copy the data for the digital port from the 
* driver max and min buffers to the respective application buffers for 
* aggregated data collection.
*
* This function applies to the PicoScope 2205 MSO only.
*
* Input Arguments:
*
* handle - the device handle.
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
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is invalid
* PICO_INVALID_DIGITAL_PORT, if digiPort is not 0 (Port 0) or 1 (Port 1)
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverDigiBuffers(int16_t handle, int16_t digiPort, int16_t * appMaxDigiBuffer, int16_t * appMinDigiBuffer, int16_t * driverMaxDigiBuffer, 
	int16_t * driverMinDigiBuffer, int32_t bufferLength)
{
	if(handle > 0)
	{
		if(digiPort < PS2000A_WRAP_DIGITAL_PORT0 || digiPort > PS2000A_WRAP_DIGITAL_PORT1)
		{
			return PICO_INVALID_DIGITAL_PORT;
		}
		else
		{
			g_wrapBufferInfo.appDigiBuffers[digiPort * 2] = appMaxDigiBuffer;
			g_wrapBufferInfo.driverDigiBuffers[digiPort * 2] = driverMaxDigiBuffer;

			g_wrapBufferInfo.appDigiBuffers[digiPort * 2 + 1] = appMinDigiBuffer;
			g_wrapBufferInfo.driverDigiBuffers[digiPort * 2 + 1] = driverMinDigiBuffer;

			g_wrapBufferInfo.digiBufferLengths[digiPort] = bufferLength;

			return PICO_OK;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}
