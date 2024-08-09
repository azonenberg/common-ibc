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

	//1 Hz timer event
	//static uint32_t nextHealthPrint = 0;
	if(g_logTimer.GetCount() >= next1HzTick)
	{
		next1HzTick = g_logTimer.GetCount() + 10000;

		//DEBUG: log sensor values
		/*
		if(nextHealthPrint == 0)
		{
			g_log("Health sensors\n");
			LogIndenter li(g_log);
			PrintSensorValues();
			nextHealthPrint = 60;
		}
		nextHealthPrint --;
		*/
	}

	//Check for I2C activity
	static IBCI2CServer server(g_i2c);
	server.Poll();
}

uint16_t GetInputVoltage()
{
	//48V rail is ADC1_IN7, 30.323x division
	return round(g_adc->ReadChannelScaled(7) * 30.323);
}

uint16_t GetOutputVoltage()
{
	//read and throw out a value to wake up the ADC
	g_adc->ReadChannel(9);

	//12V rail output is ADC_IN9, 5.094x division
	return round(g_adc->ReadChannelScaled(9) * 5.094);
}

uint16_t GetSenseVoltage()
{
	//read and throw out a value to wake up the ADC
	g_adc->ReadChannel(5);

	//12V remote sense (including cable loss) is ADC_IN5, 5.094x division
	return round(g_adc->ReadChannelScaled(5) * 5.094);
}

/*

uint16_t GetInputCurrent()
{
	//Input shunt is 1A / 500 mV on ADC_IN1
	//(but amplifier has ~80 mV offset)
	//2 mA / mV, so one LSB = 1.612 mA??
	//Integrate a lot of samples to account for noise in output at switching frequency
	//(next board rev should have LPF or something?)
	const int navg = 32;

	//sum first to avoid delays during acquisition so we sample somewhat evenly
	//TODO: use hardware averaging mode to avoid the need for this
	int64_t iin = 0;
	g_adc->SetSampleTime(3);
	for(int i=0; i<navg; i++)
		iin += g_adc->ReadChannel(1);

	//Convert raw adc counts to uV
	iin = (iin * 806) / navg;

	//Subtract zero offset
	iin -= 80000;

	//Now we have shunt voltage in uV
	//1 amp / 500000 uV
	//so 1000 mA / 500000 uV
	//or 1 mA / 500 uV
	//(Not sure where the 10 mA offset is creeping in, but seems to match R&S PSU better that way)
	return (iin / 500) + 10;
}

uint16_t GetOutputCurrent()
{
	//Output shunt is 1A / 100 mV on ADC_IN7
	//i.e. 10 mA/mV, or 8.058 mA/code
	const int navg = 32;

	//sum first to avoid delays during acquisition so we sample somewhat evenly
	//TODO: use hardware averaging mode to avoid the need for this
	int64_t iout = 0;
	g_adc->SetSampleTime(3);
	for(int i=0; i<navg; i++)
		iout += g_adc->ReadChannel(7);

	//Convert raw adc counts to uV
	iout = (iout * 806) / navg;

	//Subtract zero offset
	iout -= 80000;

	//Now we have shunt voltage in uV
	//1 amp / 100000 uV
	//so 1000 mA / 100000 uV
	//or 1 mA / 100 uV
	return iout / 100;
}
*/

/**
	@brief Read a temperature sensor at the given I2C address and return the temperature (in 8.8 fixed point format)
 */
uint16_t ReadThermalSensor(uint8_t addr)
{
	if(!g_i2c.BlockingWrite8(addr, 0x00))
		return 0xff;
	uint16_t reply;
	if(!g_i2c.BlockingRead16(addr, reply))
		return 0xff;

	g_log("temp = %x\n", reply);

	return reply;
}

