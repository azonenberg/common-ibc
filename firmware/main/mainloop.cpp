/***********************************************************************************************************************
*                                                                                                                      *
* common-ibc                                                                                                           *
*                                                                                                                      *
* Copyright (c) 2024 Andrew D. Zonenberg and contributors                                                              *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

#include "ibc.h"
#include "regids.h"
#include <math.h>

///@brief Output stream for local serial console
UARTOutputStream g_localConsoleOutputStream;

///@brief Session context for local serial console
IBCCLISessionContext g_localConsoleSessionContext;

void BSP_MainLoopIteration()
{
	const int logTimerMax = 60000;
	static uint32_t next1HzTick = 0;

	//Check for output enable toggles
	static bool loadRequestOn = false;
	bool enableRequest = g_outEnableFromLoad;
	if(enableRequest != loadRequestOn)
	{
		if(enableRequest)
			g_log("Output enabled by host board\n");
		else
			g_log("Output disabled by host board\n");

		loadRequestOn = enableRequest;

		//Make output-enable LED track the actual load power enable state
		g_onLED = enableRequest;
	}

	//Check for overflows on our log message timer
	if(g_log.UpdateOffset(logTimerMax) && (next1HzTick >= logTimerMax) )
		next1HzTick -= logTimerMax;

	//Poll for UART input
	if(g_uart.HasInput())
		g_localConsoleSessionContext.OnKeystroke(g_uart.BlockingRead());

	//1 Hz timer event
	if(g_logTimer.GetCount() >= next1HzTick)
	{
		next1HzTick = g_logTimer.GetCount() + 10000;
		//nothing to do in here for now
	}

	//Check for I2C activity
	static IBCI2CServer server(g_i2c);
	server.Poll();
}

uint16_t GetInputVoltage()
{
	//48V rail is ADC1_IN7, 30.323x division
	return round(g_adc->ReadChannelScaledAveraged(7, 32) * 30.323);
}

uint16_t GetOutputVoltage()
{
	//12V rail output is ADC_IN9, 5.094x division
	return round(g_adc->ReadChannelScaledAveraged(9, 32) * 5.094);
}

uint16_t GetSenseVoltage()
{
	//12V remote sense (including cable loss) is ADC_IN5, 5.094x division
	return round(g_adc->ReadChannelScaledAveraged(5, 32) * 5.094);
}

uint16_t GetInputCurrent()
{
	float vshunt = g_adc->ReadChannelScaledAveraged(ADC_CHANNEL_INPUT_CURRENT, 32);
	return round( (vshunt * SHUNT_SCALE_INPUT_CURRENT) - g_inputCurrentShuntOffset );
}

uint16_t GetOutputCurrent()
{
	float vshunt = g_adc->ReadChannelScaledAveraged(ADC_CHANNEL_OUTPUT_CURRENT, 32);
	return round( (vshunt * SHUNT_SCALE_OUTPUT_CURRENT) - g_outputCurrentShuntOffset );
}
