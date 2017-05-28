/**************************************************************************
 *
 * Filename: ps5000Wrap.c
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
 *  Please refer to the PicoScope 5000 Series Programmer's Guide
 *  for descriptions of the underlying functions where stated.
 *
 * Copyright (C) 2006 - 2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 **************************************************************************/

#include "ps5000Wrap.h"


/////////////////////////////////
//
//	Function declarations
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See ps5000StreamingReady (callback)
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
		for (channel = (int16_t) PS5000_CHANNEL_A; channel < _channelCount; channel++)
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
	}
  
	_ready = 1;
}

/****************************************************************************
* BlockCallback
*
* See ps5000BlockReady (callback)
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
* preTriggerSamples - see noOfPreTriggerSamples in ps5000RunBlock.
* postTriggerSamples - see noOfPreTriggerSamples in ps5000RunBlock.
* timebase - see ps5000RunBlock.
* oversample - see ps5000RunBlock.
* segmentIndex - see ps5000RunBlock.
*
*
* Returns:
*
* See ps5000RunBlock return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 RunBlock(int16_t handle, int32_t preTriggerSamples, int32_t postTriggerSamples,
            uint32_t timebase, int16_t oversample, uint16_t segmentIndex)
{
	_ready = 0;
	_numSamples = preTriggerSamples + postTriggerSamples;

	return ps5000RunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, oversample, 
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
* See ps5000GetStreamingLatestValues return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues(int16_t handle)
{
  _ready = 0;
  _numSamples = 0;
  _autoStop = 0;

  return ps5000GetStreamingLatestValues(handle, StreamingCallback, &_wrapBufferInfo);
}

/****************************************************************************
* Available Data
*
* Returns the number of samples returned from the driver and shows 
* the start index of the data in the buffer when collecting data in 
* streaming mode.
*
* Input Arguments:
*
* handle - the handle of the required device.
* startIndex - on exit, a zero-based index to the first valid sample in the  
*			   buffer (when data is available).
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
	return 0l;
}

/****************************************************************************
* AutoStopped
*
* Indicates if the device has stopped on collection of the number of samples 
* specified in the call to the ps5000RunStreaming function (if the 
* ps5000RunStreaming function's autostop flag is set).
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
* None
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
****************************************************************************/
extern int16_t PREF0 PREF1 ClearTriggerReady(int16_t handle)
{
	if (handle > 0)
	{
		_triggeredAt = 0;
		_triggered = FALSE;

		return PICO_OK;
	}
	else
	{
		return PICO_INVALID_HANDLE;
	}
}

/****************************************************************************
* SetTriggerConditions
*
* This function sets up trigger conditions on the scope's inputs. The trigger 
* is defined by one or more sets of integers corresponding to 
* PS5000_TRIGGER_CONDITIONS structures which are then converted and passed 
* to the ps5000SetTriggerChannelConditions function.
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
* See ps5000SetTriggerConditions return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditions(int16_t handle, int32_t* conditionsArray, int16_t nConditions)
{
	PICO_STATUS status;
	int16_t i = 0;
	TRIGGER_CONDITIONS *conditions = (TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(TRIGGER_CONDITIONS));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA				= (TRIGGER_STATE) conditionsArray[0 * (i+1)];
		conditions[i].channelB				= (TRIGGER_STATE) conditionsArray[1 * (i+1)];
		conditions[i].channelC				= (TRIGGER_STATE) conditionsArray[2 * (i+1)];
		conditions[i].channelD				= (TRIGGER_STATE) conditionsArray[3 * (i+1)];
		conditions[i].external				= (TRIGGER_STATE) conditionsArray[4 * (i+1)];
		conditions[i].aux					= (TRIGGER_STATE) conditionsArray[5 * (i+1)];
		conditions[i].pulseWidthQualifier	= (TRIGGER_STATE) conditionsArray[6 * (i+1)];
	}
		
	status = ps5000SetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return status;
}

/****************************************************************************
* SetTriggerProperties
*
* This function is used to enable or disable triggering and set its 
* parameters by means of assigning the values from the propertiesArray to an 
* array of TRIGGER_CHANNEL_PROPERTIES structures which are then passed to 
* the ps5000SetTriggerChannelProperties function with the other parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to 
*					TRIGGER_CHANNEL_PROPERTIES structures describing the 
*					required properties to be set. See also channelProperties
*					in ps4000SetTriggerChannelProperties.
*
* nProperties - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of propertiesArray 
*				elements / 5)
* autoTrig - see autoTriggerMilliseconds in ps5000SetTriggerChannelProperties.
*
*
* Returns:
*
* See ps5000SetTriggerProperties return values.
*
****************************************************************************/
extern int16_t PREF0 PREF1 SetTriggerProperties(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties, 
	int16_t auxEnable, 
	int32_t autoTrig)
{
	TRIGGER_CHANNEL_PROPERTIES *channelProperties = (TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(TRIGGER_CHANNEL_PROPERTIES));
	int16_t i;
	int16_t j = 0;
	PICO_STATUS status;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdMajor = propertiesArray[j];
		channelProperties[i].thresholdMinor = propertiesArray[j + 1];
		channelProperties[i].hysteresis		= propertiesArray[j + 2];
		channelProperties[i].channel		= (PS5000_CHANNEL) propertiesArray[j + 3];
		channelProperties[i].thresholdMode	= (THRESHOLD_MODE) propertiesArray[j + 4];
		j= j + 5;
	}
	
	status = ps5000SetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	return status;
}

/****************************************************************************
* hasOverflowed
*
* Indicates if an overvoltage has occurred on any of the channels during
* streaming mode data collection.
*
* Input Arguments:
*
* handle - the handle of the required device.
*
* Returns:
*
* 0 - Overflow has not occurred.
* Non-zero - a bit pattern with bit 0 denoting Channel A and bit 1 Channel B.
*
****************************************************************************/
extern int16_t PREF0 PREF1 hasOverflowed(int16_t handle)
{
	return _overflow;
}

/****************************************************************************
* setEnabledChannels
*
* Set the number of enabled channels on the device. This is used to assist with 
* copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* enabledChannels - an array representing the channel states. This must be 
*					2 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if _channelCount is invalid
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{

	if (handle > 0)
	{
		if (_channelCount > 0 && _channelCount <= DUAL_SCOPE)
		{
			memcpy_s((int16_t *) _enabledChannels, DUAL_SCOPE * sizeof(int16_t), 
				(int16_t *) enabledChannels, DUAL_SCOPE * sizeof(int16_t));
			
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
* SetAppAndDriverBuffers
*
* Set the application and corresponding driver buffer in order for the 
* streaming callback to copy the data for the channel from the driver buffer 
* to the application buffer.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a PS4000_CHANNEL enumeration value).
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
extern PICO_STATUS PREF0 PREF1 setAppAndDriverBuffers(int16_t handle, int16_t channel, int16_t * appBuffer, int16_t * driverBuffer, uint32_t bufferLength)
{
	if (handle > 0)
	{
		if (channel < 0 || channel >= _channelCount)
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
* SetMaxMinAppAndDriverBuffers
*
* Set the application and corresponding driver buffers in order for the 
* streaming callback to copy the data for the channel from the driver max and 
* min buffers to the respective application buffers for aggregated data 
* collection.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a PS4000_CHANNEL enumeration value).
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
extern PICO_STATUS PREF0 PREF1 setMaxMinAppAndDriverBuffers(int16_t handle, int16_t channel, int16_t * appMaxBuffer, int16_t * appMinBuffer, int16_t * driverMaxBuffer, int16_t * driverMinBuffer, uint32_t bufferLength)
{
	if (handle > 0)
	{
		if (channel < 0 || channel >= _channelCount)
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

