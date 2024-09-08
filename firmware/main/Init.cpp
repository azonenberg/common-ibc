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

/**
	@file
	@author	Andrew D. Zonenberg
	@brief	Boot-time hardware intialization
 */
#include "ibc.h"
#include "regids.h"
#include <math.h>

//Indicator LEDs
GPIOPin g_standbyLED(&GPIOB, 12, GPIOPin::MODE_OUTPUT, GPIOPin::SLEW_SLOW);
GPIOPin g_onLED(&GPIOB, 15, GPIOPin::MODE_OUTPUT, GPIOPin::SLEW_SLOW);
GPIOPin g_faultLED(&GPIOA, 8, GPIOPin::MODE_OUTPUT, GPIOPin::SLEW_SLOW);

//Output enables
GPIOPin g_outEnableFromLoad(&GPIOH, 0, GPIOPin::MODE_INPUT, GPIOPin::SLEW_SLOW);
GPIOPin g_outEnableFromProtection(&GPIOB, 10, GPIOPin::MODE_OUTPUT, GPIOPin::SLEW_SLOW);
GPIOPin g_loadEnableSense(&GPIOB, 7, GPIOPin::MODE_INPUT, GPIOPin::SLEW_SLOW);

//Hardware version straps
GPIOPin g_hwVersionStrap(&GPIOB, 0, GPIOPin::MODE_INPUT, GPIOPin::SLEW_SLOW);

//I2C2 defaults to running of APB clock (40 MHz)
//Prescale by 4 to get 10 MHz
//Divide by 100 after that to get 100 kHz
I2C g_i2c(&I2C2, 4, 100);

//The ADC (can't be initialized before InitClocks() so can't be a global object)
ADC* g_adc = nullptr;

///@brief Firmware version string
char g_version[20] = {0};

///@brief Software version string
char g_hwversion[20] = {0};

void ZeroizeOffsets();

//Offset voltage of the output current shunt
uint16_t g_outputCurrentShuntOffset = 0;
uint16_t g_inputCurrentShuntOffset = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Peripheral initialization

void App_Init()
{
	//Turn off the output
	g_outEnableFromProtection = 0;

	RCCHelper::Enable(&_RTC);
	InitGPIOs();
	InitI2C();
	InitADC();

	//Format version string
	StringBuffer buf(g_version, sizeof(g_version));
	static const char* buildtime = __TIME__;
	buf.Printf("%s %c%c%c%c%c%c",
		__DATE__, buildtime[0], buildtime[1], buildtime[3], buildtime[4], buildtime[6], buildtime[7]);
	g_log("Firmware version %s\n", g_version);

	//Format hardware version string
	StringBuffer hbuf(g_hwversion, sizeof(g_hwversion));
	if(g_hwVersionStrap)
		hbuf.Printf("0.5");
	else
		hbuf.Printf("0.4");
	g_log("Hardware version %s\n", g_hwversion);

	//Turn on our internal output enable (TODO: check protections)
	g_outEnableFromProtection = 1;

	g_log("Init complete, output turned off until start requested by host board\n");

	//Initialize the local console
	g_localConsoleOutputStream.Initialize(&g_uart);
	g_localConsoleSessionContext.Initialize(&g_localConsoleOutputStream, "localadmin");

	//Show the initial prompt
	g_localConsoleSessionContext.PrintPrompt();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other hardware init

void InitGPIOs()
{
	g_log("Initializing GPIOs\n");

	//Version strap pins default low unless explicitly set high
	g_hwVersionStrap.SetPullMode(GPIOPin::PULL_DOWN);

	g_standbyLED = 1;
	g_faultLED = 0;
	g_onLED = 0;

	//DEBUG: backfeed output enable from load to enable running without a smart load
	//g_outEnableFromLoad = 1;
}

void InitI2C()
{
	g_log("Initializing I2C interface\n");

	static GPIOPin i2c_scl(&GPIOB, 13, GPIOPin::MODE_PERIPHERAL, GPIOPin::SLEW_SLOW, 4, true);
	static GPIOPin i2c_sda(&GPIOB, 14, GPIOPin::MODE_PERIPHERAL, GPIOPin::SLEW_SLOW, 4, true);

	//Set our device address, somewhat arbitrarily, to 0x42
	g_i2c.SetThisNodeAddress(0x42);
}

void InitADC()
{
	g_log("Initializing ADC\n");
	LogIndenter li(g_log);

	//Run ADC at sysclk/10 (10 MHz)
	static ADC adc(&_ADC, &_ADC.chans[0], 10);
	g_adc = &adc;
	g_logTimer.Sleep(20);

	//Set up sampling time. Need minimum 5us to accurately read temperature
	//With ADC clock of 8 MHz = 125 ns per cycle this is 40 cycles
	//Max 8 us / 64 clocks for input channels
	//47.5 clocks fits both requirements, use it for everything
	int tsample = 95;
	for(int i=0; i <= 18; i++)
		adc.SetSampleTime(tsample, i);

	//Load ADC cal coefficient
	g_inputCurrentShuntOffset = g_kvs->ReadObject<uint16_t>(0, g_iincalObjectName);
	g_outputCurrentShuntOffset = g_kvs->ReadObject<uint16_t>(0, g_ioutcalObjectName);

	//First boot, no cal coefficients? Warn
	//(amplifiers typically have ~80 mV offset, so cal coefficient should never be zero if valid)
	if(g_inputCurrentShuntOffset == 0)
	{
		g_log(
			Logger::WARNING,
			"No calibration data found for input current shunt amplifier, readings will be inaccurate\n");
	}
	else
		g_log("Input current shunt offset:  %d mA\n", g_inputCurrentShuntOffset);

	if(g_outputCurrentShuntOffset == 0)
	{
		g_log(
			Logger::WARNING,
			"No calibration data found for output current shunt amplifier, readings will be inaccurate\n");
	}
	else
		g_log("Output current shunt offset: %d mA\n", g_outputCurrentShuntOffset);

	PrintSensorValues();
}

void PrintSensorValues()
{
	auto temp = g_adc->GetTemperature();
	g_log("MCU temperature: %uhk C\n", temp);
	auto vdd = g_adc->GetSupplyVoltage();
	g_log("Supply voltage:  %2d.%03d V\n", vdd/1000, vdd % 1000);

	auto vin = GetInputVoltage();
	g_log("Input voltage:   %2d.%03d V\n", vin/1000, vin % 1000);

	auto vout = GetOutputVoltage();
	g_log("Output voltage:  %2d.%03d V\n", vout/1000, vout % 1000);

	auto vsense = GetSenseVoltage();
	g_log("Output sense:    %2d.%03d V\n", vsense/1000, vsense % 1000);

	auto iin = GetInputCurrent();
	g_log("Input current:   %2d.%03d A\n", iin/1000, iin % 1000);

	auto iout = GetOutputCurrent();
	g_log("Output current:  %2d.%03d A\n", iout/1000, iout % 1000);
}
