/*******************************************************************************
 * Filename: ps4000aWrap.c
 *
 * Description:
 *  The source code in this release is for use with Pico products when 
 *	interfaced with Microsoft Excel VBA, National Instruments LabVIEW and 
 *	MathWorks MATLAB or any third-party programming language or application 
 *	that is unable to support C-style callback functions or structures.
 *
 *  Please refer to the PicoScope 4000 Series (A API) Programmer's Guide
 *  for descriptions of the underlying functions where stated.
 *
 *  Copyright (C) 2014 - 2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 ******************************************************************************/

#include "ps4000aWrap.h"

/////////////////////////////////
//
//	Function definitions
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See ps4000aStreamingReady (callback)
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
		for (channel = (int16_t) PS4000A_CHANNEL_A; channel < _channelCount; channel++)
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
* See ps4000aBlockReady (callback)
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
* preTriggerSamples - see noOfPreTriggerSamples in ps4000aRunBlock.
* postTriggerSamples - see noOfPreTriggerSamples in ps4000aRunBlock.
* timebase - see ps4000aRunBlock.
* segmentIndex - see ps4000aRunBlock.
*
*
* Returns:
*
* See ps4000aRunBlock return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 RunBlock(int16_t handle, int32_t preTriggerSamples, int32_t postTriggerSamples,
            uint32_t timebase, uint32_t segmentIndex)
{
	_ready = 0;
	_numSamples = preTriggerSamples + postTriggerSamples;

	return ps4000aRunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, 
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
* See ps4000aGetStreamingLatestValues return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues(int16_t handle)
{
	_ready = 0;
	_numSamples = 0;
	_autoStop = 0;

	return ps4000aGetStreamingLatestValues(handle, StreamingCallback, &_wrapBufferInfo);
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
	if( _ready ) 
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
* specified in the call to the ps4000aRunStreaming function (if the 
* ps4000aRunStreaming function's autostop flag is set).
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
	if( _ready) 
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
* triggeredAt � on exit, the index of the sample in the buffer where the 
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
* 1
*
****************************************************************************/
extern int16_t PREF0 PREF1 ClearTriggerReady(int16_t handle)
{
	_triggeredAt = 0;
	_triggered = FALSE;
	return 1;
}

/****************************************************************************
* setChannelCount
*
* Sets the number of channels on the device. This is used to assist with 
* copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* channelCount - the number of channels on the device.
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if channelCount is less than 0 or greater than
*	the number of analogue channels
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setChannelCount(int16_t handle, int16_t channelCount)
{
	if(handle > 0)
	{
		if(channelCount > 0 && channelCount <= PS4000A_MAX_CHANNELS)
		{
			_channelCount = channelCount;

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
* setEnabledChannels
*
* Sets the number of enabled channels on the device. This is used to assist with 
* copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* enabledChannels - an array representing the channel states. This should be 
*					8 elements in size.
*
* Returns:
*
* PICO_OK, if successful
* PICO_INVALID_HANDLE, if handle is less than or equal to 0
* PICO_INVALID_PARAMETER, if _channelCount is invalid
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{

	if(handle > 0)
	{
		if(_channelCount > 0 && _channelCount <= PS4000A_MAX_CHANNELS)
		{
			memcpy_s((int16_t *)_enabledChannels, PS4000A_MAX_CHANNELS * sizeof(int16_t), 
				(int16_t *)enabledChannels, PS4000A_MAX_CHANNELS * sizeof(int16_t));
			
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
* streaming callback to copy the data for the channel from the driver buffer 
* to the application buffer.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a PS4000A_CHANNEL enumeration value).
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
		if(channel < PS4000A_CHANNEL_A || channel >= _channelCount)
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
* streaming callback to copy the data for the channel from the driver max and 
* min buffers to the respective application buffers for aggregated data 
* collection.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a PS4000A_CHANNEL enumeration value).
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
		if(channel < PS4000A_CHANNEL_A || channel >= _channelCount)
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
* setTriggerConditions
*
* This function sets up trigger conditions on the scope's inputs. The trigger 
* is defined by one or more sets of integers corresponding to 
* PS4000A_CONDITION structures which are then converted and passed 
* to the ps4000aSetTriggerChannelConditions function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the device handle.
* conditionsArray - an array of integer values specifying the conditions 
*					for each channel.
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures 
				(i.e. the number of conditionsArray elements / 2).
* info - see info in ps4000SetTriggerChannelConditions 
*
* Returns:
*
* See ps4000aSetTriggerChannelConditions return values.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setTriggerConditions(int16_t handle, int32_t *conditionsArray, int16_t nConditions, int32_t info)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS4000A_CONDITION *conditions = (PS4000A_CONDITION *) calloc(nConditions, sizeof(PS4000A_CONDITION));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].source	= (PS4000A_CHANNEL) conditionsArray[j];
		conditions[i].condition = (PS4000A_TRIGGER_STATE) conditionsArray[j + 1];

		j = j + 2;
	}

	status = ps4000aSetTriggerChannelConditions(handle, conditions, nConditions, (PS4000A_CONDITIONS_INFO) info);
	free(conditions);

	return status;
}

/****************************************************************************
* setTriggerDirections
*
* This function sets the direction of the trigger for the specified channels.
* The trigger direction(s) is(are) defined by one or more sets of integers 
* corresponding to PS4000A_DIRECTION structures which are then converted and 
* passed to the ps4000aSetTriggerChannelDirections function.
*
* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the device handle.
* directionsArray - an array of integer values specifying the directions 
*					for each channel.
* nDirections - the number that will be passed after the wrapper code has 
*				created its structures 
*				(i.e. the number of directionsArray elements / 2).
*
* Returns:
*
* See ps4000aSetTriggerChannelDirections return values.
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setTriggerDirections(int16_t handle, int32_t *directionsArray, int16_t nDirections)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS4000A_DIRECTION *directions = (PS4000A_DIRECTION *) calloc(nDirections, sizeof(PS4000A_DIRECTION));

	for (i = 0; i < nDirections; i++)
	{
		directions[i].channel	= (PS4000A_CHANNEL) directionsArray[j];
		directions[i].direction = (PS4000A_THRESHOLD_DIRECTION) directionsArray[j + 1];

		j = j + 2;
	}

	status = ps4000aSetTriggerChannelDirections(handle, directions, nDirections);
	free(directions);

	return status;
}

/****************************************************************************
* setTriggerProperties
*
* This function is used to enable or disable triggering and set its
* parameters by means of assigning the values from the propertiesArray to an
* array of PS4000A_TRIGGER_CHANNEL_PROPERTIES structures which are then 
* passed to the ps4000aSetTriggerChannelProperties function with the other 
* parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to
*					PS4000A_TRIGGER_CHANNEL_PROPERTIES structures describing the
*					required properties to be set. See also channelProperties
*					in ps4000aSetTriggerChannelProperties.
* nProperties - the number that will be passed after the wrapper code has
*				created its structures. (i.e. the number of propertiesArray
*				elements / 6)
* autoTrig - see autoTriggerMilliseconds in ps4000aSetTriggerChannelProperties.
*
*
* Returns:
*
* See ps4000aSetChannelTriggerProperties return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 setTriggerProperties(int16_t handle, int32_t *propertiesArray, int16_t nProperties, int32_t autoTrig)
{
	PS4000A_TRIGGER_CHANNEL_PROPERTIES *channelProperties = (PS4000A_TRIGGER_CHANNEL_PROPERTIES *) 
																calloc(nProperties, sizeof(PS4000A_TRIGGER_CHANNEL_PROPERTIES));
	
	int16_t i;
	int16_t j = 0;
	int16_t auxEnable = 0;
	PICO_STATUS status;

	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper				= (int16_t) propertiesArray[j];
		channelProperties[i].thresholdUpperHysteresis	= (uint16_t) propertiesArray[j + 1];
		channelProperties[i].thresholdLower				= (int16_t) propertiesArray[j + 2];
		channelProperties[i].thresholdLowerHysteresis	= (uint16_t) propertiesArray[j + 3];
		channelProperties[i].channel					= (PS4000A_CHANNEL) propertiesArray[j + 4];
		channelProperties[i].thresholdMode				= (PS4000A_THRESHOLD_MODE) propertiesArray[j + 5];

		j = j + 6;
	}
	status = ps4000aSetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);

	return status;
}