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

#ifndef IBCI2CServer_h
#define IBCI2CServer_h

#include <helpers/I2CServer.h>
#include <math.h>

#define ADC_VAVG 8
#define ADC_IAVG 128

class IBCI2CServer
	: public I2CServer
	, public Task
{
public:
	IBCI2CServer(I2C& i2c)
		: I2CServer(i2c)
	{}

	virtual void Iteration()
	{ Poll(); }

	uint16_t GetInputVoltage()
	{
		//48V rail is ADC1_IN7, 30.323x division
		return round(g_adc->ReadChannelScaledAveraged(7, ADC_VAVG, 3300) * 30.323);
	}

	uint16_t GetOutputVoltage()
	{
		//12V rail output is ADC_IN9, 5.094x division
		return round(g_adc->ReadChannelScaledAveraged(9, ADC_VAVG, 3300) * 5.094);
	}

	uint16_t GetSenseVoltage()
	{
		//12V remote sense (including cable loss) is ADC_IN5, 5.094x division
		return round(g_adc->ReadChannelScaledAveraged(5, ADC_VAVG, 3300) * 5.094);
	}

	uint16_t GetInputCurrent()
	{
		float vshunt = g_adc->ReadChannelScaledMedian(ADC_CHANNEL_INPUT_CURRENT, ADC_IAVG, 3300);
		return round( (vshunt * SHUNT_SCALE_INPUT_CURRENT) - g_inputCurrentShuntOffset );
	}

	uint16_t GetOutputCurrent()
	{
		float vshunt = g_adc->ReadChannelScaledMedian(ADC_CHANNEL_OUTPUT_CURRENT, ADC_IAVG, 3300);
		return round( (vshunt * SHUNT_SCALE_OUTPUT_CURRENT) - g_outputCurrentShuntOffset );
	}

protected:
	virtual void OnRequestStart() override;
	virtual void OnRequestRead() override;
};

#endif
