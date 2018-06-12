/**************************************************************************
 *
 * Filename: ps5000aWrap.c
 *
 * Description:
 *  The source code in this release is for use with Pico products when 
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
 *  Please refer to the PicoScope 5000 Series (A API) Programmer's Guide
 *  for descriptions of the underlying functions where stated.
 *
 * Copyright (C) 2013-2018 Pico Technology Ltd. See LICENSE file for terms.
 *
 **************************************************************************/

#include "ps5000aWrap.h"

/////////////////////////////////
//
//	Function declarations
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See ps5000aStreamingReady (callback)
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
	WRAP_BUFFER_INFO * _wrapBufferInfo = NULL;
	
	if (pParameter != NULL)
	{
		_wrapBufferInfo = (WRAP_BUFFER_INFO *) pParameter;
	}

	_numSamples = noOfSamples;
	_autoStop = autoStop;
	_startIndex = startIndex;

	_triggered = triggered;
	_triggeredAt = triggerAt;

	_overflow = overflow;

	if (_wrapBufferInfo != NULL && noOfSamples)
	{
		// Analogue channels
		for (channel = (int16_t) PS5000A_CHANNEL_A; channel < _channelCount; channel++)
		{
			if (_enabledChannels[channel])
			{
				if (_wrapBufferInfo->appBuffers && _wrapBufferInfo->driverBuffers)
				{
					// Max buffers
					if (_wrapBufferInfo->appBuffers[channel * 2]  && _wrapBufferInfo->driverBuffers[channel * 2])
					{
						memcpy_s (&_wrapBufferInfo->appBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t),
							&_wrapBufferInfo->driverBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t));
					}

					// Min buffers
					if (_wrapBufferInfo->appBuffers[channel * 2 + 1] && _wrapBufferInfo->driverBuffers[channel * 2 + 1])
					{
						memcpy_s (&_wrapBufferInfo->appBuffers[channel * 2 + 1][startIndex], noOfSamples * sizeof(int16_t),
							&_wrapBufferInfo->driverBuffers[channel * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
					}
				}
			}
		}

		// Digital channels
		if (_digitalPortCount > 0)
		{
			// Use index 0 to indicate Digital Port 0
			for (digitalPort = (int16_t)PS5000A_WRAP_DIGITAL_PORT0; digitalPort < _digitalPortCount; digitalPort++)
			{
				if (_enabledDigitalPorts[digitalPort])
				{
					// Copy data...
					if (_wrapBufferInfo->appDigiBuffers && _wrapBufferInfo->driverDigiBuffers)
					{
						// Max digital buffers
						if (_wrapBufferInfo->appDigiBuffers[digitalPort * 2] && _wrapBufferInfo->driverDigiBuffers[digitalPort * 2])
						{
								memcpy_s(&_wrapBufferInfo->appDigiBuffers[digitalPort * 2][startIndex], noOfSamples * sizeof(int16_t),
										&_wrapBufferInfo->driverDigiBuffers[digitalPort * 2][startIndex], noOfSamples * sizeof(int16_t));
						}

						// Min digital buffers
						if (_wrapBufferInfo->appDigiBuffers[digitalPort * 2 + 1] && _wrapBufferInfo->driverDigiBuffers[digitalPort * 2 + 1])
						{
								memcpy_s(&_wrapBufferInfo->appDigiBuffers[digitalPort * 2 + 1][startIndex], noOfSamples * sizeof(int16_t),
										&_wrapBufferInfo->driverDigiBuffers[digitalPort * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
						}
					}
				}
			}
		}
	}
  
  _ready = 1;
}

/****************************************************************************
* BlockCallback
*
* See ps5000aBlockReady (callback)
*
****************************************************************************/
void PREF1 BlockCallback(int16_t handle, PICO_STATUS status, void * pParameter)
{
  _ready = 1;
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
* preTriggerSamples - see noOfPreTriggerSamples in ps5000aRunBlock.
* postTriggerSamples - see noOfPreTriggerSamples in ps5000aRunBlock.
* timebase - see ps5000aRunBlock.
* segmentIndex - see ps5000aRunBlock.
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 RunBlock(int16_t handle, int32_t preTriggerSamples, int32_t postTriggerSamples,
            uint32_t timebase, uint32_t segmentIndex)
{
	_ready = 0;
	_numSamples = preTriggerSamples + postTriggerSamples;

	return ps5000aRunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, 
		NULL, segmentIndex, BlockCallback, NULL);
}

/****************************************************************************
* GetStreamingLatestValues
*
* facilitates communication with the driver to return the next block of 
* values to your application when capturing data in streaming mode. Use with 
* programming languages that do not support callback functions.
*
* Input Arguments:
*
* handle - the handle of the required device.
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues(int16_t handle)
{
	_ready = 0;
	_numSamples = 0;
	_autoStop = 0;

	return ps5000aGetStreamingLatestValues(handle, StreamingCallback, &_wrapBufferInfo);
}

/****************************************************************************
* AvailableData
*
* Returns the number of samples returned from the driver and shows 
* the start index of the data in the buffer when collecting data in 
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
extern uint32_t PREF0 PREF1 AvailableData(int16_t handle, uint32_t *startIndex)
{
	if ( _ready ) 
	{
		*startIndex = _startIndex;
		return _numSamples;
	}

	return 0;
}

/****************************************************************************
* AutoStopped
*
* Indicates if the device has stopped on collection of the number of samples 
* specified in the call to the ps5000aRunStreaming function (if the 
* ps5000aRunStreaming function's autostop flag is set).
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
	if ( _ready) 
	{
		return _autoStop;
	}
	else
	{
		return 0;
	}
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
	return _ready;
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
	if (_triggered)
	{
		*triggeredAt = _triggeredAt;
	}

	return _triggered;
}

/****************************************************************************
* ClearTriggerReady
*
* Clears the triggered and triggeredAt flags in relation to streaming mode 
* capture.
*
* Input Arguments:
*
* handle - the handle of the required device.
*
* Returns:
*
* PICO_OK
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 ClearTriggerReady(int16_t handle)
{
	_triggeredAt = 0;
	_triggered = FALSE;
	return PICO_OK;
}

/****************************************************************************
* SetTriggerConditions
*
* NOTE: This function is deprecated - use SetTriggerConditionsV2 instead.
*
* This function sets up trigger conditions on the scope's inputs. The trigger 
* is defined by one or more sets of integers corresponding to 
* PS5000A_TRIGGER_CONDITIONS structures which are then converted and passed 
* to the ps5000aSetTriggerChannelConditions function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* conditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
* created its structures. (i.e. the number of conditionsArray elements / 7)
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditions(int16_t handle, int32_t *conditionsArray, int16_t nConditions)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS5000A_TRIGGER_CONDITIONS *conditions = (PS5000A_TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(PS5000A_TRIGGER_CONDITIONS));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA				= (PS5000A_TRIGGER_STATE) conditionsArray[j];
		conditions[i].channelB				= (PS5000A_TRIGGER_STATE) conditionsArray[j + 1];
		conditions[i].channelC				= (PS5000A_TRIGGER_STATE) conditionsArray[j + 2];
		conditions[i].channelD				= (PS5000A_TRIGGER_STATE) conditionsArray[j + 3];
		conditions[i].external				= (PS5000A_TRIGGER_STATE) conditionsArray[j + 4];
		conditions[i].aux					= (PS5000A_TRIGGER_STATE) conditionsArray[j + 5];
		conditions[i].pulseWidthQualifier	= (PS5000A_TRIGGER_STATE) conditionsArray[j + 6];

		j = j + 7;
	}
	status = ps5000aSetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return status;
}

/****************************************************************************
* SetTriggerProperties
*
* NOTE: This function is deprecated - use SetTriggerPropertiesV2 instead.
* 
* This function is used to enable or disable triggering and set its 
* parameters by means of assigning the values from the propertiesArray to an 
* array of TRIGGER_CHANNEL_PROPERTIES structures which are then passed to 
* the ps5000aSetTriggerChannelProperties function with the other parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to 
*					TRIGGER_CHANNEL_PROPERTIES structures describing the 
*					required properties to be set. See also channelProperties
*					in ps5000aSetTriggerChannelProperties.
*
* nProperties - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of propertiesArray 
*				elements / 6)
* autoTrig - see autoTriggerMilliseconds in ps5000aSetTriggerChannelProperties.
*
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerProperties(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties, 
	int32_t autoTrig)
{
	PS5000A_TRIGGER_CHANNEL_PROPERTIES *channelProperties = (PS5000A_TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(PS5000A_TRIGGER_CHANNEL_PROPERTIES));
	int16_t i;
	int16_t j=0;
	int16_t auxEnable = 0;
	PICO_STATUS status;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper				= propertiesArray[j];
		channelProperties[i].thresholdUpperHysteresis	= propertiesArray[j + 1];
		channelProperties[i].thresholdLower				= propertiesArray[j + 2];
		channelProperties[i].thresholdLowerHysteresis	= propertiesArray[j + 3];
		channelProperties[i].channel					= (PS5000A_CHANNEL) propertiesArray[j + 4];
		channelProperties[i].thresholdMode				= (PS5000A_THRESHOLD_MODE) propertiesArray[j + 5];

		j = j + 6;
	}
	status = ps5000aSetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	
	return status;
}

/****************************************************************************
* SetPulseWidthQualifier
*
* This function is used to enable pulse-width triggering and set 
* its parameters by means of assigning the values from the pwqConditionsArray 
* to an array of PS5000A_PWQ_CONDITIONS structures which are then passed to 
* the ps5000aSetPulseWidthQualifier function with the other parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqConditionsArray - an array of sets of integers corresponding to 
*					PS5000A_PWQ_CONDITIONS structures describing the 
*					required properties to be set. See also conditions
*					in ps5000aSetPulseWidthQualifier.
*
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of conditionsArray 
*				elements / 6)
*
* direction - the direction of the signal required for the pulse width
*				trigger to fire (See PS5000A_THRESHOLD_DIRECTION constants)
* lower - the lower limit of the pulse-width counter with relation to
			number of samples captured on the device.
* upper - the upper limit of the pulse-width counter with relation to
*			number of samples captured on the device.
* type - the pulse-width type (see ps5000aSetPulseWidthQualifier).
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifier(
	int16_t handle,
	int32_t *pwqConditionsArray,
	int16_t nConditions,
	int32_t direction,
	uint32_t lower,
	uint32_t upper,
	int32_t type)
{
	// Allocate memory
	PS5000A_PWQ_CONDITIONS *pwqConditions = (PS5000A_PWQ_CONDITIONS *) calloc(nConditions, sizeof(PS5000A_PWQ_CONDITIONS));

	int16_t i;
	int16_t j = 0;

	PICO_STATUS status;

	for (i = 0; i < nConditions; i++)
	{
		pwqConditions[i].channelA = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j];
		pwqConditions[i].channelB = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j + 1];
		pwqConditions[i].channelC = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j + 2];
		pwqConditions[i].channelD = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j + 3];
		pwqConditions[i].external = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j + 4];
		pwqConditions[i].aux      = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j + 5];

		j = j + 6;
	}

	status = ps5000aSetPulseWidthQualifier(handle, pwqConditions, nConditions, (PS5000A_THRESHOLD_DIRECTION) direction, lower, upper, (PS5000A_PULSE_WIDTH_TYPE) type);
	free(pwqConditions);
	return status;
}

/****************************************************************************
* setChannelCount
*
* Sets the number of analogue channels and digital ports on the device.  
* copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* channelCount - not used.
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setChannelCount(int16_t handle, int16_t channelCount)
{
  int8_t variant[15];
	int16_t requiredSize = 0;
	PICO_STATUS status = PICO_OK;
	
	// Obtain the model number
	status = ps5000aGetUnitInfo(handle, variant, sizeof(variant), &requiredSize, PICO_VARIANT_INFO);
	
	if (status == PICO_OK)
	{
			// Set the number of analogue channels
			_channelCount = (int16_t) variant[1];
			_channelCount = _channelCount - 48; // Subtract ASCII 0 (48)

			// Determine if the device is an MSO
			if (strstr(variant, "MSO") != NULL)
			{
				 _digitalPortCount = 2;
			}
			else
			{
			  _digitalPortCount = 0;
			}
	}

	return status;
}

/****************************************************************************
* setEnabledChannels
*
* Sets the number of enabled analogue channels on the device. This is used to 
* assist with copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* enabledChannels - an array representing the channel states. This should be 
*					4 elements in size.
*
* Returns:
*
* PICO_OK if successful,
* PICO_INVALID_HANDLE if handle <= 0, or 
* PICO_INVALID_PARAMETER if _channelCount is out of range
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{
	if (handle > 0)
	{
		if (_channelCount > 0 && _channelCount <= PS5000A_MAX_CHANNELS)
		{
			memcpy_s((int16_t *)_enabledChannels, PS5000A_MAX_CHANNELS * sizeof(int16_t), 
				(int16_t *)enabledChannels, PS5000A_MAX_CHANNELS * sizeof(int16_t));
			
			return PICO_OK;
		}
		else
		{
		  return PICO_INVALID_PARAMETER;
		}
	}

	return PICO_INVALID_HANDLE;
}

/****************************************************************************
* setAppAndDriverBuffers
*
* Set the application and corresponding driver buffer in order for the 
* streaming callback to copy the data for the channel/digital port from the 
* driver buffer to the application buffer.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel/ digital port number (should be a PS5000A_CHANNEL 
*						enumeration value).
* appBuffer - the application buffer.
* driverBuffer - the buffer set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*								equal).
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if an invalid handle is used, or 
* PICO_INVALID_CHANNEL if an invalid channel/digital port is used, or
* PICO_INVALID_PARAMETER if the bufferLength is less than or equal to 0.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setAppAndDriverBuffers(int16_t handle, PS5000A_CHANNEL channel, int16_t * appBuffer, int16_t * driverBuffer, uint32_t bufferLength)
{
  // Map port number to internal enumeration for MSO devices.
	PS5000A_WRAP_DIGITAL_PORT_INDEX portIndex = PS5000A_WRAP_DIGITAL_PORT0;

	if (handle > 0)
	{
		if (bufferLength <= 0)
		{
		  return PICO_INVALID_PARAMETER;
		}

		if (channel == PS5000A_DIGITAL_PORT0 || channel == PS5000A_DIGITAL_PORT1)
		{
				if (channel == PS5000A_DIGITAL_PORT0)
				{
					portIndex = PS5000A_WRAP_DIGITAL_PORT0;
				}
				else
				{
					portIndex = PS5000A_WRAP_DIGITAL_PORT1;
				}

				_wrapBufferInfo.appDigiBuffers[portIndex * 2] = appBuffer;
				_wrapBufferInfo.driverDigiBuffers[portIndex * 2] = driverBuffer;

				_wrapBufferInfo.digiBufferLengths[portIndex] = bufferLength;

				return PICO_OK;
		}
		else if (channel < PS5000A_CHANNEL_A || channel >= PS5000A_MAX_CHANNELS)
		{
			return PICO_INVALID_CHANNEL;
		}
		else
		{
			_wrapBufferInfo.appBuffers[channel * 2] = appBuffer;
			_wrapBufferInfo.driverBuffers[channel * 2] = driverBuffer;
				
			_wrapBufferInfo.bufferLengths[channel] = bufferLength;

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
* streaming callback to copy the data for the channel/digital port from the 
* driver max and min buffers to the respective application buffers for 
* aggregated data collection.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel/digital port number (should be a PS5000A_CHANNEL 
*						enumeration value).
* appMaxBuffer - the application max buffer.
* appMinBuffer - the application min buffer.
* driverMaxBuffer - the max buffer set by the driver.
* driverMinBuffer - the min buffer set by the driver.
* bufferLength - the length of the buffers (the length of the buffers must be
*								equal).
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if an invalid handle is used, or
* PICO_INVALID_CHANNEL if an invalid channel/digital port is used, or
* PICO_INVALID_PARAMETER if the bufferLength is less than or equal to 0.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers(int16_t handle, PS5000A_CHANNEL channel, int16_t * appMaxBuffer, int16_t * appMinBuffer, int16_t * driverMaxBuffer, int16_t * driverMinBuffer, uint32_t bufferLength)
{
	// Map port number to internal enumeration for MSO devices.
	PS5000A_WRAP_DIGITAL_PORT_INDEX portIndex = PS5000A_WRAP_DIGITAL_PORT0;

	if (handle > 0)
	{
		if (bufferLength <= 0)
		{
				return PICO_INVALID_PARAMETER;
		}

		if (channel == PS5000A_DIGITAL_PORT0 || channel == PS5000A_DIGITAL_PORT1)
		{
				if (channel == PS5000A_DIGITAL_PORT0)
				{
						portIndex = PS5000A_WRAP_DIGITAL_PORT0;
				}
				else
				{
						portIndex = PS5000A_WRAP_DIGITAL_PORT1;
				}

				_wrapBufferInfo.appDigiBuffers[portIndex * 2] = appMaxBuffer;
				_wrapBufferInfo.driverDigiBuffers[portIndex * 2] = driverMaxBuffer;

				_wrapBufferInfo.appDigiBuffers[portIndex * 2 + 1] = appMinBuffer;
				_wrapBufferInfo.driverDigiBuffers[portIndex * 2 + 1] = driverMinBuffer;

				_wrapBufferInfo.digiBufferLengths[portIndex] = bufferLength;

				return PICO_OK;
		}
		else if (channel < PS5000A_CHANNEL_A || channel >= PS5000A_MAX_CHANNELS)
		{
			return PICO_INVALID_CHANNEL;
		}
		else
		{
			_wrapBufferInfo.appBuffers[channel * 2] = appMaxBuffer;
			_wrapBufferInfo.driverBuffers[channel * 2] = driverMaxBuffer;

			_wrapBufferInfo.appBuffers[channel * 2 + 1] = appMinBuffer;
			_wrapBufferInfo.driverBuffers[channel * 2 + 1] = driverMinBuffer;

			_wrapBufferInfo.bufferLengths[channel] = bufferLength;

			return PICO_OK;
		}
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}

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
*												must be 2 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if _digitalPortCount is not 0 or 2
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledDigitalPorts(int16_t handle, int16_t * enabledDigitalPorts)
{
		if (handle > 0)
		{
				if (_digitalPortCount == 0 || _digitalPortCount == PS5000A_WRAP_MAX_DIGITAL_PORTS)
				{
						memcpy_s((int16_t *) _enabledDigitalPorts, PS5000A_WRAP_MAX_DIGITAL_PORTS * sizeof(int16_t),
								(int16_t *) enabledDigitalPorts, PS5000A_WRAP_MAX_DIGITAL_PORTS * sizeof(int16_t));

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
* getOverflow
*
* Returns indication if there has been an overvoltage on one or more 
* analogue inputs when collecting data in streaming mode.
*
* Input Arguments:
*
* handle - the handle of the required device.
* overflow - on exit, a set of flags that indicate whether an overvoltage 
*						has occurred on any of the channels. It is a bit field with 
*						bit 0 denoting channel A.
*
* Returns:
*
* PICO_OK, if successful, or
* PICO_INVALID_HANDLE if handle is less than or equal to 0
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 getOverflow(int16_t handle, int16_t * overflow)
{
  if (handle > 0)
	{
		*overflow = _overflow;
		return PICO_OK;
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}

/****************************************************************************
* SetTriggerConditionsV2
*
* This function sets up trigger conditions on the scope's inputs. The trigger
* is defined by one or more sets of integers corresponding to
* PS5000A_CONDITION structures which are then converted and passed
* to the ps5000aSetTriggerChannelConditionsV2 function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* conditionsArray - an array of integer values specifying the conditions
*										for each channel.
* nConditions - the number that will be passed after the wrapper code has
*								created its structures (i.e. the number of conditionsArray 
*								elements / 2). Set to 0 to switch off triggering.
* info - see ps5000aSetTriggerChannelConditionsV2
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditionsV2(int16_t handle, int32_t *conditionsArray, int16_t nConditions, PS5000A_CONDITIONS_INFO info)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS5000A_CONDITION *conditions = (PS5000A_CONDITION *)calloc(nConditions, sizeof(PS5000A_CONDITION));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].source = (PS5000A_CHANNEL)conditionsArray[j];
		conditions[i].condition = (PS5000A_TRIGGER_STATE)conditionsArray[j + 1];

		j = j + 2;
	}
	status = ps5000aSetTriggerChannelConditionsV2(handle, conditions, nConditions, info);
	free(conditions);

	return status;
}


/****************************************************************************
* SetTriggerDirectionsV2
*
* This function sets the direction of the trigger for each channel. The 
* directions are defined by one or more sets of integers corresponding to
* PS5000A_DIRECTION structures which are then converted and passed to the 
* ps5000aSetTriggerChannelDirectionsV2 function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* directionsArray - an array of integer values specifying the directions
*										for each channel.
* nConditions - the number that will be passed after the wrapper code has
*								created its structures (i.e. the number of directionsArray
*								elements / 3). 
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerDirectionsV2(int16_t handle, int32_t * directionsArray, int16_t nDirections)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS5000A_DIRECTION *directions = (PS5000A_DIRECTION *)calloc(nDirections, sizeof(PS5000A_DIRECTION));

	for (i = 0; i < nDirections; i++)
	{
		directions[i].source = (PS5000A_CHANNEL) directionsArray[j];
		directions[i].direction = (PS5000A_THRESHOLD_DIRECTION) directionsArray[j + 1];
		directions[i].mode = (PS5000A_THRESHOLD_MODE) directionsArray[j + 2];

		j = j + 3;
	}

	status = ps5000aSetTriggerChannelDirectionsV2(handle, directions, (uint16_t)nDirections);
	free(directions);

	return status;
}

/****************************************************************************
* SetTriggerPropertiesV2
*
* This function is used to enable or disable triggering and set its
* parameters by means of assigning the values from the propertiesArray to an
* array of PS5000A_TRIGGER_CHANNEL_PROPERTIES_V2 structures which are then 
* passed to the ps5000aSetTriggerChannelPropertiesV2 function with the other
* parameters.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to
*					PS5000A_TRIGGER_CHANNEL_PROPERTIES_V2 structures describing the
*					required properties to be set. See also channelProperties
*					in ps5000aSetTriggerChannelPropertiesV2.
*
* nProperties - the number that will be passed after the wrapper code has
*								created its structures. (i.e. the number of propertiesArray
*								elements / 5)
*
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerPropertiesV2(int16_t handle, int32_t *propertiesArray, int16_t nProperties)
{
	int16_t i;
	int16_t j = 0;
	int16_t auxEnable = 0;
	PICO_STATUS status;

	PS5000A_TRIGGER_CHANNEL_PROPERTIES_V2 *channelProperties = (PS5000A_TRIGGER_CHANNEL_PROPERTIES_V2 *)calloc(nProperties, sizeof(PS5000A_TRIGGER_CHANNEL_PROPERTIES_V2));

	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper = propertiesArray[j];
		channelProperties[i].thresholdUpperHysteresis = propertiesArray[j + 1];
		channelProperties[i].thresholdLower = propertiesArray[j + 2];
		channelProperties[i].thresholdLowerHysteresis = propertiesArray[j + 3];
		channelProperties[i].channel = (PS5000A_CHANNEL) propertiesArray[j + 4];

		j = j + 5;
	}

	status = ps5000aSetTriggerChannelPropertiesV2(handle, channelProperties, nProperties, auxEnable);
	free(channelProperties);

	return status;
}

/****************************************************************************
* SetTriggerDigitalPortProperties
*
* This function is used to set the individual digital channels' trigger 
* directions by means of assigning the values from the digitalDirections 
* array to an array of PS5000A_DIGITAL_CHANNEL_DIRECTIONS structures 
* which are then passed to the ps5000aSetTriggerDigitalPortProperties 
* function with the other parameters.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* digitalDirections - an array of sets of integers corresponding to
*											PS5000A_DIGITAL_CHANNEL_DIRECTIONS structures 
*											describing the required properties to be set. See 
*											also directions in ps5000aSetTriggerDigitalPortProperties.
*
* nDirections - the number that will be passed after the wrapper code has
*								created its structures. (i.e. the number of 
*								digitalDirections elements / 2)
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerDigitalPortProperties(int16_t handle, int32_t *digitalDirections, int16_t nDirections)
{
	int16_t i;
	int16_t j = 0;
	PICO_STATUS status;

	PS5000A_DIGITAL_CHANNEL_DIRECTIONS *directions = (PS5000A_DIGITAL_CHANNEL_DIRECTIONS *)calloc(nDirections, sizeof(PS5000A_DIGITAL_CHANNEL_DIRECTIONS));

	for (i = 0; i < nDirections; i++)
	{
		directions[i].channel = digitalDirections[j];
		directions[i].direction = digitalDirections[j + 1];

		j = j + 2;
	}

	status = ps5000aSetTriggerDigitalPortProperties(handle, directions, nDirections);
	free(directions);

	return status;
}

/****************************************************************************
* SetPulseWidthQualifierConditions
*
* This function applies a condition to the pulse-width qualifier. The condition
* is defined by one or more sets of integers corresponding to
* PS5000A_CONDITION structures which are then converted and passed
* to the ps5000aSetPulseWidthQualifierConditions function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqConditionsArray - an array of integer values specifying the conditions
*										for each channel.
* nConditions - the number that will be passed after the wrapper code has
*								created its structures (i.e. the number of pwqConditionsArray
*								elements / 2). Set to 0 to switch off the pulse-width qualifier.
* info - see ps5000aSetPulseWidthQualifierConditions
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifierConditions(int16_t handle, int32_t *pwqConditionsArray, int16_t nConditions, PS5000A_CONDITIONS_INFO info)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS5000A_CONDITION * pwqConditions = (PS5000A_CONDITION *)calloc(nConditions, sizeof(PS5000A_CONDITION));

	for (i = 0; i < nConditions; i++)
	{
		pwqConditions[i].source = (PS5000A_CHANNEL) pwqConditionsArray[j];
		pwqConditions[i].condition = (PS5000A_TRIGGER_STATE) pwqConditionsArray[j + 1];

		j = j + 2;
	}

	status = ps5000aSetPulseWidthQualifierConditions(handle, pwqConditions, nConditions, info);
	free(pwqConditions);

	return status;
}

/****************************************************************************
* SetPulseWidthQualifierDirections
*
* This function sets the direction ofor all trigger sources used with the 
* pulse width qualifier. The directions are defined by one or more sets of 
* integers corresponding to PS5000A_DIRECTION structures which are then 
* converted and passed to the ps5000aSetPulseWidthQualifierDirections function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqDirectionsArray - an array of integer values specifying the directions
*										for each channel.
* nConditions - the number that will be passed after the wrapper code has
*								created its structures (i.e. the number of pwqDirectionsArray
*								elements / 3).
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthQualifierDirections(int16_t handle, int32_t * pwqDirectionsArray, int16_t nDirections)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS5000A_DIRECTION * pwqDirections = (PS5000A_DIRECTION *)calloc(nDirections, sizeof(PS5000A_DIRECTION));

	for (i = 0; i < nDirections; i++)
	{
		pwqDirections[i].source = (PS5000A_CHANNEL) pwqDirectionsArray[j];
		pwqDirections[i].direction = (PS5000A_THRESHOLD_DIRECTION) pwqDirectionsArray[j + 1];
		pwqDirections[i].mode = (PS5000A_THRESHOLD_MODE) pwqDirectionsArray[j + 2];

		j = j + 3;
	}

	status = ps5000aSetPulseWidthQualifierDirections(handle, pwqDirections, nDirections);
	free(pwqDirections);

	return status;
}

/****************************************************************************
* SetPulseWidthDigitalPortProperties
*
* This function is used to set the individual digital channels' pulse-width 
* trigger directions by means of assigning the values from the digitalDirections
* array to an array of PS5000A_DIGITAL_CHANNEL_DIRECTIONS structures
* which are then passed to the ps5000aSetSetPulseWidthDigitalPortProperties
* function with the other parameters.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqDigitalDirections - an array of sets of integers corresponding to
*											PS5000A_DIGITAL_CHANNEL_DIRECTIONS structures
*											describing the required properties to be set. See
*											also directions in ps5000aSetPulseWidthDigitalPortProperties.
*
* nDirections - the number that will be passed after the wrapper code has
*								created its structures. (i.e. the number of
*								pwqDigitalDirections elements / 2)
*
* Returns:
*
* PICO_OK or other code from PicoStatus.h
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetPulseWidthDigitalPortProperties(int16_t handle, int32_t *pwqDigitalDirections, int16_t nDirections)
{
	int16_t i;
	int16_t j = 0;
	PICO_STATUS status;

	PS5000A_DIGITAL_CHANNEL_DIRECTIONS *pwqDirections = (PS5000A_DIGITAL_CHANNEL_DIRECTIONS *)calloc(nDirections, sizeof(PS5000A_DIGITAL_CHANNEL_DIRECTIONS));

	for (i = 0; i < nDirections; i++)
	{
		pwqDirections[i].channel = pwqDigitalDirections[j];
		pwqDirections[i].direction = pwqDigitalDirections[j + 1];

		j = j + 2;
	}

	status = ps5000aSetPulseWidthDigitalPortProperties(handle, pwqDirections, nDirections);
	free(pwqDirections);

	return status;
}