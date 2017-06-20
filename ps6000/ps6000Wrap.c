/**************************************************************************
 *
 * Filename: ps6000Wrap.c
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
 *   Please refer to the PicoScope 6000 Series Programmer's Guide
 *   for descriptions of the underlying functions where stated.
 *
 * Copyright (C) 2009-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 **************************************************************************/

#include "ps6000Wrap.h"

/////////////////////////////////
//
//	Function definitions
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See ps6000StreamingReady (callback)
*
****************************************************************************/
void PREF1 StreamingCallback(
    int16_t handle,
	uint32_t noOfSamples,
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

	// Verify if wrapper buffer info set and data received
	if (_wrapBufferInfo != NULL && noOfSamples)
	{
		
		for (channel = (int16_t) PS6000_CHANNEL_A; channel < _channelCount; channel++)
		{
			if (_enabledChannels[channel])
			{

				// Copy data...
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
						memcpy_s (&_wrapBufferInfo->appBuffers[channel * 2 + 1][startIndex],  noOfSamples * sizeof(int16_t),
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
* See ps6000BlockReady (callback)
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
* preTriggerSamples - see noOfPreTriggerSamples in ps6000RunBlock.
* postTriggerSamples - see noOfPreTriggerSamples in ps6000RunBlock.
* timebase - see ps6000RunBlock.
* oversample - see ps6000RunBlock.
* segmentIndex - see ps6000RunBlock.
*
*
* Returns:
*
* See ps6000RunBlock return values.
*
****************************************************************************/
extern int16_t PREF0 PREF1 RunBlock(int16_t handle, uint32_t preTriggerSamples, uint32_t postTriggerSamples,
            uint32_t timebase, int16_t oversample, uint32_t segmentIndex)
{
	_ready = 0;
	_numSamples = preTriggerSamples + postTriggerSamples;

	return (int16_t) ps6000RunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, oversample, 
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
* See ps6000GetStreamingLatestValues return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 GetStreamingLatestValues(int16_t handle)
{
	_ready = 0;
	_numSamples = 0;
	_autoStop = 0;

	return ps6000GetStreamingLatestValues(handle, StreamingCallback, &_wrapBufferInfo);
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
extern uint32_t PREF0 PREF1 AvailableData(int16_t handle, uint32_t *startIndex)
{
	if( _ready ) 
	{
		*startIndex = _startIndex;
		return _numSamples;
	}
	return (uint32_t) 0;
}

/****************************************************************************
* AutoStopped
*
* Indicates if the device has stopped on collection of the number of samples 
* specified in the call to the ps6000RunStreaming function (if the 
* ps6000RunStreaming function's autostop flag is set).
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
	if (_ready)
	{
		return _autoStop;
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
* SetTriggerConditions
*
* This function sets up trigger conditions on the scope's inputs. The trigger 
* is defined by one or more sets of integers corresponding to 
* PS6000_TRIGGER_CONDITIONS structures which are then converted and passed 
* to the ps6000SetTriggerChannelConditions function.
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
*				elements / 7)
*
* Returns:
*
* See ps6000SetTriggerChannelConditions return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerConditions(int16_t handle, int32_t *conditionsArray, int16_t nConditions)
{
	PICO_STATUS status;
	int16_t i = 0;
	int16_t j = 0;

	PS6000_TRIGGER_CONDITIONS *conditions = (PS6000_TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(PS6000_TRIGGER_CONDITIONS));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA 				= (PS6000_TRIGGER_STATE) conditionsArray[j];
		conditions[i].channelB 				= (PS6000_TRIGGER_STATE) conditionsArray[j + 1];
		conditions[i].channelC 				= (PS6000_TRIGGER_STATE) conditionsArray[j + 2];
		conditions[i].channelD 				= (PS6000_TRIGGER_STATE) conditionsArray[j + 3];
		conditions[i].external 				= (PS6000_TRIGGER_STATE) conditionsArray[j + 4];
		conditions[i].aux					= (PS6000_TRIGGER_STATE) conditionsArray[j + 5];
		conditions[i].pulseWidthQualifier	= (PS6000_TRIGGER_STATE) conditionsArray[j + 6];

		j = j + 7;
	}
	status = ps6000SetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return status;
}

/****************************************************************************
* SetTriggerProperties
*
* This function is used to enable or disable triggering and set its 
* parameters by means of assigning the values from the propertiesArray to an 
* array of TRIGGER_CHANNEL_PROPERTIES structures which are then passed to 
* the ps6000SetTriggerChannelProperties function with the other parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* propertiesArray - an array of sets of integers corresponding to 
*					TRIGGER_CHANNEL_PROPERTIES structures describing the 
*					required properties to be set. See also channelProperties
*					in ps6000SetTriggerChannelProperties.
*
* nProperties - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of propertiesArray 
*				elements / 6)
* autoTrig - see autoTriggerMilliseconds in ps6000SetTriggerChannelProperties.
*
*
* Returns:
*
* See ps6000SetTriggerChannelProperties return values.
*
****************************************************************************/
extern PICO_STATUS PREF0 PREF1 SetTriggerProperties(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties,  
	int32_t autoTrig)
{
	PS6000_TRIGGER_CHANNEL_PROPERTIES *channelProperties = (PS6000_TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(PS6000_TRIGGER_CHANNEL_PROPERTIES));
	int16_t i;
	int16_t j = 0;
	int16_t auxEnable = 0;
	PICO_STATUS status;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper		= propertiesArray[j];
		channelProperties[i].hysteresisUpper	= propertiesArray[j + 1];
		channelProperties[i].thresholdLower		= propertiesArray[j + 2];
		channelProperties[i].hysteresisLower	= propertiesArray[j + 3];
		channelProperties[i].channel			= (PS6000_CHANNEL) propertiesArray[j + 4];
		channelProperties[i].thresholdMode		= (PS6000_THRESHOLD_MODE) propertiesArray[j + 5];

		j = j + 6;
	}
	
	status = ps6000SetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	return status;
}

/****************************************************************************
* SetPulseWidthQualifier
*
* This function is used to pulse-width triggering and set 
* its parameters by means of assigning the values from the pwqConditionsArray 
* to an array of PS6000_PWQ_CONDITIONS structures which are then passed to 
* the ps6000SetPulseWidthQualifier function with the other parameters.

* Use this function with programming languages that do not support structs.
*
* Input Arguments:
*
* handle - the handle of the required device.
* pwqConditionsArray - an array of sets of integers corresponding to 
*					PS6000_PWQ_CONDITIONS structures describing the 
*					required properties to be set. See also conditions
*					in ps6000SetPulseWidthQualifier.
*
* nConditions - the number that will be passed after the wrapper code has 
*				created its structures. (i.e. the number of conditionsArray 
*				elements / 6)
*
* direction - the direction of the signal required for the pulse width
*				trigger to fire (See PS6000_THRESHOLD_DIRECTION constants)
* lower - the lower limit of the pulse-width counter with relation to
			number of samples captured on the device.
* upper - the upper limit of the pulse-width counter with relation to
*			number of samples captured on the device.
* type - the pulse-width type (see ps6000SetPulseWidthQualifier).
*
* Returns:
*
* See ps6000SetPulseWidthQualifier return values.
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
	PS6000_PWQ_CONDITIONS *pwqConditions = (PS6000_PWQ_CONDITIONS *) calloc(nConditions, sizeof(PS6000_PWQ_CONDITIONS));

	int16_t i;
	int16_t j = 0;

	PICO_STATUS status;

	for(i = 0; i < nConditions; i++)
	{
		pwqConditions[i].channelA = (PS6000_TRIGGER_STATE) pwqConditionsArray[j];
		pwqConditions[i].channelB = (PS6000_TRIGGER_STATE) pwqConditionsArray[j + 1];
		pwqConditions[i].channelC = (PS6000_TRIGGER_STATE) pwqConditionsArray[j + 2];
		pwqConditions[i].channelD = (PS6000_TRIGGER_STATE) pwqConditionsArray[j + 3];
		pwqConditions[i].external = (PS6000_TRIGGER_STATE) pwqConditionsArray[j + 4];
		pwqConditions[i].aux      = (PS6000_TRIGGER_STATE) pwqConditionsArray[j + 5];

		j = j + 6;
	}

	status = ps6000SetPulseWidthQualifier(handle, pwqConditions, nConditions, (PS6000_THRESHOLD_DIRECTION) direction, lower, upper, (PS6000_PULSE_WIDTH_TYPE) type);
	free(pwqConditions);
	return status;
}

/****************************************************************************
* setChannelCount
*
* Set the number of channels on the device. This is used to assist with 
* copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* channelCount - the number of channels on the device.
*
****************************************************************************/
extern void PREF0 PREF1 setChannelCount(int16_t handle, int16_t channelCount)
{
	_channelCount = channelCount;
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
*					4 elements in size.
*
* Returns:
*
* 0 if successful,
* -1 if handle <= 0 or channelCount is out of range
****************************************************************************/
extern int16_t PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{

	if(handle > 0)
	{
		if(_channelCount > 0 && _channelCount <= PS6000_MAX_CHANNELS)
		{
			memcpy_s((int16_t *)_enabledChannels, PS6000_MAX_CHANNELS * sizeof(int16_t), 
				(int16_t *)enabledChannels, PS6000_MAX_CHANNELS * sizeof(int16_t));
			return 0;
		}
	}

	return -1;

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
* channel - the channel number (should be a numerical value corresponding to 
			an PS6000_CHANNEL enumeration value).
* appBuffer - the application buffer.
* driverBuffer - the buffer set by the driver.
* bufferLength - the length of the buffers (length must be equal).
*
* Returns:
*
* 0, if successful
* -1, otherwise
****************************************************************************/
extern int16_t PREF0 PREF1 setAppAndDriverBuffers(int16_t handle, int16_t channel, int16_t * appBuffer, int16_t * driverBuffer, uint32_t bufferLength)
{
	if(handle > 0)
	{
		if(channel < PS6000_CHANNEL_A || channel >= PS6000_MAX_CHANNELS)
		{
			return -1;
		}
		else
		{
			_wrapBufferInfo.appBuffers[channel * 2] = appBuffer;
			_wrapBufferInfo.driverBuffers[channel * 2] = driverBuffer;
				
			_wrapBufferInfo.bufferLengths[channel] = bufferLength;

			return 0;
		}
	}
	else
	{
		return -1;
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
* channel - the channel number ((should be a numerical value corresponding to 
			an PS6000_CHANNEL enumeration value).
* appMaxBuffer - the application max buffer.
* appMinBuffer - the application min buffer.
* driverMaxBuffer - the max buffer set by the driver.
* driverMinBuffer - the min buffer set by the driver.
* bufferLength - the length of the buffers (length must be equal).
*
* Returns:
*
* 0, if successful
* -1, otherwise
****************************************************************************/
extern int16_t PREF0 PREF1 setMaxMinAppAndDriverBuffers(int16_t handle, int16_t channel, int16_t * appMaxBuffer, int16_t * appMinBuffer, int16_t * driverMaxBuffer, int16_t * driverMinBuffer, uint32_t bufferLength)
{
	if(handle > 0)
	{
		if(channel < PS6000_CHANNEL_A || channel >= PS6000_MAX_CHANNELS)
		{
			return -1;
		}
		else
		{
			_wrapBufferInfo.appBuffers[channel * 2] = appMaxBuffer;
			_wrapBufferInfo.driverBuffers[channel * 2] = driverMaxBuffer;

			_wrapBufferInfo.appBuffers[channel * 2 + 1] = appMinBuffer;
			_wrapBufferInfo.driverBuffers[channel * 2 + 1] = driverMinBuffer;

			_wrapBufferInfo.bufferLengths[channel] = bufferLength;

			return 0;
		}
	}
	else
	{
		return -1;
	}
}

/****************************************************************************
* clearStreamingParameters
*
* Sets streaming parameters to 0.
*
* Input Arguments:
*
* None
*
****************************************************************************/
extern void PREF0 PREF1 clearStreamingParameters(int16_t handle)
{
	_ready = 0;
	_autoStop = 0;
	_numSamples = 0;
	_triggeredAt = 0;
	_triggered = FALSE;
	_startIndex = 0;
	_overflow = 0;
}
