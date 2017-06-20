/**************************************************************************
 *
 * Filename: ps2000Wrap.c
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
 *	The functions provided in this source file and it's corresponding 
 *	library are to be used for Fast Streaming Data capture for the 
 *	following devices:
 *
 *		PicoScope 2203, 2204, 2204A, 2205 and 2205A
 *
 *   Please refer to the PicoScope 2000 Series Programmer's Guide for 
 *   descriptions of the underlying functions where stated.
 *
 * Copyright (C) 2009-2017 Pico Technology Ltd. See LICENSE file for terms.
 *
 **************************************************************************/

#include "ps2000Wrap.h"

// Function implementation

/****************************************************************************
* Streaming Callback
*
* See my_get_overview_buffers (callback)
*
****************************************************************************/

void PREF1 GetValuesCallback(int16_t **overviewBuffers, int16_t overflow, uint32_t triggeredAt, int16_t triggered,
  int16_t auto_stop,uint32_t nValues)
{    
	if (_ready == 0)
	{
		uint16_t channel = 0;
		uint32_t bufferSize = 0;

		_overflow = overflow;
		_triggeredAt = triggeredAt;
		_triggered = triggered;
		_auto_stop = auto_stop;

		g_totalValues = g_totalValues + nValues;

		if (nValues > 0 && g_appBufferFull == 0)
		{
			for (channel = 0; channel < DUAL_SCOPE; channel++)
			{
				if (_enabledChannels[channel] == 1)
				{
					if (g_totalValues <= g_wrapBufferInfo.bufferSizes[channel * 2] && !g_appBufferFull)
					{
						_nValues = nValues;
					}
					else if (g_startIndex < g_wrapBufferInfo.bufferSizes[channel * 2])
					{
						_nValues = g_wrapBufferInfo.bufferSizes[channel * 2] - (g_startIndex + 1); // Only copy data into application buffer up to end
						g_totalValues = g_wrapBufferInfo.bufferSizes[channel * 2];	// Total samples limited to application buffer
						g_appBufferFull = 1;
					}
					else
					{
						// g_startIndex might be >= buffer length
						_nValues = 0;
						g_totalValues = g_wrapBufferInfo.bufferSizes[channel * 2];
						g_appBufferFull = 1;
					}

					// Copy data...

					// Max buffers
					if (overviewBuffers[channel * 2] && g_wrapBufferInfo.appBuffers[channel * 2])
					{
						memcpy_s((void *) (g_wrapBufferInfo.appBuffers[channel * 2] + g_startIndex), _nValues * sizeof(int16_t), 
										(void *) (overviewBuffers[channel * 2]), _nValues * sizeof(int16_t));

					}

					// Min buffers
					if (overviewBuffers[channel * 2 + 1] && g_wrapBufferInfo.appBuffers[channel * 2 + 1])
					{
						memcpy_s((void *) (g_wrapBufferInfo.appBuffers[channel * 2 + 1] + g_startIndex), _nValues * sizeof(int16_t), 
										(void *) (overviewBuffers[channel * 2 + 1]), _nValues * sizeof(int16_t));
					}
				}

			}

			g_prevStartIndex = g_startIndex;
			g_startIndex = g_totalValues;
	
		}

		if (g_overview_buffer_size > 0)
		{
			// Callback could be called again if there is more data i.e. overview buffer has wrapped
			_ready = (g_totalValues % g_overview_buffer_size) != 0 || g_collection_size == g_totalValues;
		}
		else if (nValues == 0)
		{
			// Could be ready even with 0 values e.g. auto stop
			_ready = 1;
		}
		else
		{
			_ready = 0;
		}
	}
}

/****************************************************************************
* PollFastStreaming
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
* See ps2000_get_streaming_last_values return values.
*
****************************************************************************/
extern int16_t PREF0 PREF1 PollFastStreaming(int16_t handle)
{
	_ready = 0;
	return ps2000_get_streaming_last_values(handle, GetValuesCallback);
}

/****************************************************************************
* SetBuffer
*
* Set the application buffer in order for the streaming callback to copy the 
* data for the channel from the driver buffer to the application buffer.
*
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a PS2000_CHANNEL enumeration value).
* buffer - pointer to the application buffer.
* bufferSize - the length of the buffer defined as the TOTAL number of 
*				samples to be collected.
*
* Returns:
*
* None.
*
****************************************************************************/
extern void PREF0 PREF1 SetBuffer(int16_t handle, int16_t channel, int16_t * buffer, uint32_t bufferSize)
{
	int16_t index = channel * 2;

	if (index >= DUAL_SCOPE * 2) 
	{
		return;
	}

	g_wrapBufferInfo.appBuffers[index] = NULL;
	g_wrapBufferInfo.appBuffers[index + 1] = NULL;

	g_wrapBufferInfo.bufferSizes[index] = 0;
	g_wrapBufferInfo.bufferSizes[index + 1] = 0;

	if (buffer != NULL)
	{
		g_wrapBufferInfo.appBuffers[index] = buffer;
		g_wrapBufferInfo.bufferSizes[index] = bufferSize;
	}

}

/****************************************************************************
* SetBuffers
*
* Set the application buffers in order for the streaming callback to copy the 
* data for the channel's max and min from the driver buffers to the 
* application buffers when collecting data with aggregation.
*
* Input Arguments:
*
* handle - the device handle.
* channel - the channel number (should be a PS2000_CHANNEL enumeration value).
* bufferMax - a pointer to the application buffer for the maximum aggregated 
*				sample values.
* bufferMin - a pointer to the application buffer for the minimum aggregated 
*				sample values.
* bufferSize - the length of the buffer defined as the TOTAL number of 
*				samples to be collected.
*
* Returns:
*
* None.
*
****************************************************************************/
extern void PREF0 PREF1 SetAggregateBuffer(int16_t handle, int16_t channel, int16_t * bufferMax, int16_t * bufferMin, uint32_t bufferSize)
{
    int16_t index = channel * 2;
    
	if (index >= DUAL_SCOPE * 2) 
	{
		return;
	}

	g_wrapBufferInfo.appBuffers[index] = NULL;
	g_wrapBufferInfo.appBuffers[index + 1] = NULL;
	g_wrapBufferInfo.bufferSizes[index] = 0;
	g_wrapBufferInfo.bufferSizes[index + 1] = 0;

	if (bufferMax != NULL)
	{
		g_wrapBufferInfo.appBuffers[index] = bufferMax;
		g_wrapBufferInfo.bufferSizes[index] = bufferSize;
	}

	if (bufferMin != NULL)
	{
		g_wrapBufferInfo.appBuffers[index + 1] = bufferMin;
		g_wrapBufferInfo.bufferSizes[index + 1] = bufferSize;
	}
}

/****************************************************************************
* FastStreamingReady
*
* This function is used to poll the driver to verify that data is ready to be 
* received. PollFastStreaming function must have been called prior to 
* calling this function.
*
* Input Arguments:
*
* handle - the handle of the required device.
*
* Returns:
*
* 0 - Data is not yet available.
* 1 - Data is ready to be collected.
*
****************************************************************************/
extern int16_t PREF0 PREF1 FastStreamingReady(int16_t handle)
{
	return _ready;
}

/****************************************************************************
* GetFastStreamingDetails
*
* Returns the number of samples returned from the driver and provides 
* information when collecting data in streaming mode.
*
* Input Arguments:
*
* handle - the handle of the required device.
* overflow - a bit field indicating whether the voltage on each of the 
*				input channels has overflowed.
* triggeredAt - an index into the buffers indicating the number of the the 
*				sample at the trigger reference point. It is valid only 
*				when trigger is TRUE.
* triggered - the function writes a Boolean here indicating that a trigger has 
*				occurred and triggerAt is valid.
* auto_stop - indicates if the device has stopped after max_samples in the 
*				call to ps2000_run_streaming_ns has been reached.
* appBufferFull - indicates if any of the application buffera has become 
*					full.
* startIndex - the start index of the next set of data received
*
* Returns:
*
* 0 - Data is not yet available.
* Non-zero - the total number of samples collected from the driver.
*
****************************************************************************/
extern uint32_t PREF0 PREF1 GetFastStreamingDetails(int16_t handle, int16_t * overflow, uint32_t * triggeredAt,
		int16_t * triggered, int16_t * auto_stop, int16_t * appBufferFull, uint32_t * startIndex)
{
	if (_ready)
	{
		*overflow		= _overflow;
		*triggeredAt	= _triggeredAt;
		*triggered		= _triggered;
		*auto_stop		= _auto_stop;	
		*appBufferFull	= g_appBufferFull;
		*startIndex		= g_prevStartIndex;
		
	}
	else
	{
		*overflow		= (int16_t) 0;
		*triggeredAt	= (uint32_t) 0;
		*triggered		= (int16_t) 0;
	}

	return g_totalValues;
}

/****************************************************************************
* setEnabledChannels
*
* Sets the states of the channels on the device to indicate if they are 
* enabled. This is used to assist with copying data in the streaming 
* callback.
*
* Input Arguments:
*
* handle - the device handle.
* enabledChannels - an array representing the channel states. This should be 
*					2 elements in size.
*
* Returns:
*
* None.
*	
****************************************************************************/
extern void PREF0 PREF1 setEnabledChannels(int16_t handle, int16_t * enabledChannels)
{
	if (handle > 0)
	{
		memcpy_s((int16_t *)_enabledChannels, DUAL_SCOPE * sizeof(int16_t), 
			(int16_t *)enabledChannels, DUAL_SCOPE * sizeof(int16_t));
		
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
* None.
*	
****************************************************************************/
extern void PREF0 PREF1 clearFastStreamingParameters(int16_t handle)
{
	_overflow	 = 0;
	_triggeredAt = 0;
	_triggered	 = 0;
	_auto_stop	 = 0;
	_nValues	 = 0;

	g_totalValues	 = 0;
	g_startIndex	 = 0;
	g_prevStartIndex = 0;		
	g_appBufferFull  = 0;
}

/****************************************************************************
* setCollectionInfo
*
* Informs the wrapper of the number of samples to collect and the overview
* buffer size.
*
* Input Arguments:
*
* handle - the device handle.
* collectionSize - the number of samples to collect.
* overviewBufferSize - the size of the overview buffer set with the driver.
*
* Returns:
*
* 1, if collectionSize and overviewBufferSize are greater than zero, 
*		0 otherwise.
*
****************************************************************************/
extern int16_t PREF0 PREF1 setCollectionInfo(int16_t handle, uint32_t collectionSize, uint32_t overviewBufferSize)
{
	if (handle > 0)
	{
		if (collectionSize > 0 && overviewBufferSize > 0)
		{
			g_collection_size = collectionSize;
			g_overview_buffer_size = overviewBufferSize;

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
