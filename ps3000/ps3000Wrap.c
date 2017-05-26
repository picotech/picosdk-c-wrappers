/**************************************************************************
 *
 * Filename: ps3000Wrap.c
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
 *  Please refer to the PicoScope 3000 Series Programmer's Guide
 *  for descriptions of the underlying functions where stated.
 *
 *	This wrapper will only support streaming from a single device.
 *
 * Copyright (C) 2010 - 2017 Pico Technology Ltd. See LICENSE file for terms. 
 *
 **************************************************************************/

#include "ps3000Wrap.h"

/////////////////////////////////
//
//	Function definitions
//
/////////////////////////////////

/****************************************************************************
* Streaming Callback
*
* See my_get_overview_buffers (callback)
*
****************************************************************************/
void PREF1 my_get_overview_buffers(
	int16_t **overviewBuffers,
	int16_t overflow,
	uint32_t triggeredAt,
	int16_t triggered,
	int16_t auto_stop,
	uint32_t nValues)
{
	int16_t channel;	
	
	g_overflow		= overflow;
	g_triggeredAt	= triggeredAt;
	g_triggered		= triggered;
	g_auto_stop		= auto_stop;
	g_nValues		= nValues;

	for(channel = (int16_t) PS3000_CHANNEL_A; channel < g_channelCount; channel++)
	{
		if(nValues > 0)
		{
			if(g_enabledChannels[channel])
			{
				// Ensure number of values to copy is correct as buffer length could be less than nValues
				if(g_bufferLengthsSet[channel] == TRUE && g_bufferLengths[channel] < nValues)
				{
					g_nValues = g_bufferLengths[channel];
				}

				// Copy data...

				// Max buffers
				if(g_overviewBuffers[channel * 2] && overviewBuffers[channel * 2])
				{
					memcpy_s(g_overviewBuffers[channel * 2], g_nValues * sizeof(int16_t),
						overviewBuffers[channel * 2], g_nValues * sizeof(int16_t));
				}

				// Min buffers
				if(g_overviewBuffers[channel * 2 + 1] && overviewBuffers[channel * 2 + 1])
				{
					memcpy_s(g_overviewBuffers[channel * 2 + 1], g_nValues * sizeof(int16_t),
						overviewBuffers[channel * 2 + 1], g_nValues * sizeof(int16_t));
				}
			}
		}
		
	}

	g_ready = 1;
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
* See ps3000_get_streaming_last_values return values.
*
****************************************************************************/
extern int16_t PREF0 PREF1 GetStreamingLastValues(int16_t handle)
{
	int16_t result;
	g_ready = 0;

	result = ps3000_get_streaming_last_values(handle, &my_get_overview_buffers);

	return result;
}

/****************************************************************************
* IsReady
*
* This function is used to poll the driver to verify that data is ready to be 
* received. The GetStreamingLastValues function must have been 
* called prior to calling this function.
*
* Input Arguments:
*
* handle - the handle of the required device.
*
* Returns:
*
* 0 � Data is not yet available.
* Non-zero � Data is ready to be collected.
*
****************************************************************************/
extern int16_t PREF0 PREF1 IsReady(int16_t handle)
{
	return g_ready;
}

/****************************************************************************
* Available Data
*
* Provides information relating to data capture when collecting data in 
* streaming mode.
*
* Input Arguments:
*
* handle - the handle of the required device.
* overflow - an index into the overview buffers, indicating the sample at the 
*				trigger event. Valid only when triggered is TRUE.
* triggeredAt - an index into the overview buffers, indicating the sample at 
*				the trigger event. Valid only when triggered is TRUE.
* triggered - a Boolean indicating whether a trigger event has
*				occurred and triggeredAt is valid. Any non-zero value 
*				signifies TRUE.
* auto_stop - a Boolean indicating whether streaming data capture has 
*				automatically stopped. Any non-zero value signifies
*				TRUE.
* nValues - the number of data values copied to the application buffer(s).
*
* Returns:
*
* 0 - Data is not yet available.
* 1 - Streaming mode parameters have been obtained.
*
* Please also refer to the Argument descriptions for the 
* my_get_overview_buffers callback function.
****************************************************************************/
extern int16_t PREF0 PREF1 AvailableData(int16_t handle, int16_t *overflow, uint32_t *triggeredAt, int16_t *triggered, int16_t *auto_stop, uint32_t *nValues)
{
	if(!g_ready)
	{
		*overflow = 0;
		*triggeredAt = 0;
		*triggered = 0;
		*auto_stop = 0;
		*nValues = 0;
		return 0;
	}
	else
	{
		*overflow		= g_overflow;
		*triggeredAt	= g_triggeredAt;
		*triggered		= g_triggered;
		*auto_stop		= g_auto_stop;
		*nValues		= g_nValues;
	}
	
	return 1;
}

/****************************************************************************
* ClearTriggerInfo
*
* Clears the g_triggered and g_triggeredAt flags in relation to streaming 
* mode capture.
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
extern int16_t PREF0 PREF1 ClearTriggerInfo(int16_t handle)
{
	g_triggeredAt = 0;
	g_triggered = FALSE;
	return 1;
}

/****************************************************************************
* setDataBuffer
*
* Set the application buffer in order for the streaming callback to copy 
* non-aggregated data for the specified channel from the overview buffer.
*
* For aggregated data collection, please use the setDataBuffers2 function.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a numerical value corresponding to
*			a PS3000_CHANNEL enumeration value).
* buffer - the application buffer.
* bufferLength - the length of the application buffer.
*
* Returns:
*
* 0, if unsuccessful (handle or channel is invalid).
* 1, if successful
****************************************************************************/
extern int16_t PREF0 PREF1 SetDataBuffer(int16_t handle, int16_t channel, int16_t * buffer, uint32_t bufferLength)
{
	if(handle > 0)
	{
		if(channel >= PS3000_CHANNEL_A && channel < g_channelCount)
		{
			g_overviewBuffers[channel * 2] = buffer;
			g_bufferLengths[channel] = bufferLength;
			g_bufferLengthsSet[channel] = TRUE;
		
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

/****************************************************************************
* setDataBuffers
*
* Sets the application maximum and minimum buffers in order for the streaming 
* callback to copy the data for the specified channel from the overview 
* buffers.
*
* NOTE: This is a legacy function. Users creating new applications should 
*		use the setDataBuffers2 function instead.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a numerical value corresponding to
*			a PS3000_CHANNEL enumeration value).
* buffer - the application buffer.
* bufferLength - the length of the application buffer.
*
* Returns:
*
* None
****************************************************************************/
extern void PREF0 PREF1 SetDataBuffers(int16_t handle, int16_t channel, int16_t *minBuffer, int16_t *maxBuffer)
{
	g_overviewBuffers[channel * 2]			= maxBuffer;
	g_overviewBuffers[(channel * 2) + 1]	= minBuffer;
	g_bufferLengthsSet[channel] = FALSE;
}

/****************************************************************************
* setDataBuffersV2
*
* Sets the application maximum and minimum buffers in order for the streaming 
* callback to copy the data for the specified channel from the overview 
* buffers.
*
* For non-aggregated data collection, please use the setDataBuffer function.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a numerical value corresponding to
*			a PS3000_CHANNEL enumeration value).
* minBuffer - the application buffer for the minimum sample values.
* maxBuffer - the application buffer for the maximum sample values.
* bufferLength - the length of the application buffers (these MUST be equal)
*
* Returns:
*
* 0, if unsuccessful (handle or channel is invalid).
* 1, if successful
****************************************************************************/

extern int16_t PREF0 PREF1 SetDataBuffersV2(int16_t handle, int16_t channel, int16_t * minBuffer, int16_t * maxBuffer, uint32_t bufferLength)
{
	if(handle > 0)
	{
		if(channel >= PS3000_CHANNEL_A && channel < g_channelCount)
		{
			g_overviewBuffers[channel * 2]			= maxBuffer;
			g_overviewBuffers[(channel * 2) + 1]	= minBuffer;
			g_bufferLengths[channel] = bufferLength;
			g_bufferLengthsSet[channel] = TRUE;

			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

/****************************************************************************
* setChannelCount
*
* Set the number of analogue channels on the device. This is used to assist 
* with copying data in the streaming callback.
*
* Input Arguments:
*
* handle - the device handle.
* channelCount - the number of channels on the device.
*
* Returns:
*
* 0, if handle or channel count is invalid.
* 1, if successful
****************************************************************************/
extern int16_t PREF0 PREF1 setChannelCount(int16_t handle, int16_t channelCount)
{
	if(handle > 0)
	{
		if(channelCount == DUAL_SCOPE || channelCount == QUAD_SCOPE)
		{
			g_channelCount = channelCount;

			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
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
* 0, if unsuccessful
* 1, if successful
****************************************************************************/
extern int16_t PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{
	if(handle > 0)
	{
		if(g_channelCount == DUAL_SCOPE || g_channelCount == QUAD_SCOPE)
		{
			memcpy_s((int16_t *)g_enabledChannels, MAX_CHANNELS * sizeof(int16_t), 
				(int16_t *)enabledChannels, MAX_CHANNELS * sizeof(int16_t));

			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

/****************************************************************************
* clearFastStreamingParameters
*
* Resets parameters used for fast streaming to 0.
*
* Input Arguments:
*
* handle - the device handle.
*
* Returns:
*
* 0, if handle is invalid
* 1, if sucessful
****************************************************************************/
extern int16_t PREF0 PREF1 clearFastStreamingParameters(int16_t handle)
{
	if(handle > 0)
	{
		g_overflow	 = 0;
		g_triggeredAt = 0;
		g_triggered	 = 0;
		g_auto_stop	 = 0;
		g_nValues	 = 0;

		return 1;
	}
	else
	{
		return 0;
	}

}
