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
#include "TwoHzTimerTask.h"

void TwoHzTimerTask::OnTimer()
{
#ifdef _DEBUG
	static ITMStream sensorStream(0);

	sensorStream.Printf(
		"CSV-NAME,"
		"VIN,IIN,MCUTEMP,CPTEMP,VOUT,IOUT,VSENSE,3V3_SB"
		"\n"
		);

	sensorStream.Printf(
		"CSV-UNIT,"
		"V,A,°C,°C,V,A,V,V"
		"\n"
		);

	uint16_t vin = g_i2cServer->GetInputVoltage();
	uint16_t iin = g_i2cServer->GetInputCurrent();
	uint16_t mcutemp = g_adc->GetTemperature();
	uint16_t cptemp = 0;
	uint16_t vsense = g_i2cServer->GetSenseVoltage();
	uint16_t vout = g_i2cServer->GetOutputVoltage();
	uint16_t iout = g_i2cServer->GetOutputCurrent();
	uint16_t v3v3 = g_adc->GetSupplyVoltage();

	if(g_freestandingMode)
	{
		uint8_t regid = 0;
		g_i2c.BlockingWrite(g_tempI2cAddress, &regid, 1);
		g_i2c.BlockingRead(g_tempI2cAddress, (uint8_t*)&cptemp, sizeof(cptemp));
		cptemp = __builtin_bswap16(cptemp);
	}

	sensorStream.Printf(
		"CSV-DATA,"
		"%d.%03d,"
		"%d.%03d,"
		"%uhk,"
		"%uhk,"
		"%d.%03d,"
		"%d.%03d,"
		"%d.%03d,"
		"%d.%03d"
		"\n",
		vin / 1000, vin % 1000,
		iin / 1000, iin % 1000,
		mcutemp,
		cptemp,
		vout / 1000, vout % 1000,
		iout / 1000, iout % 1000,
		vsense / 1000, vsense % 1000,
		v3v3 / 1000, v3v3 % 1000
		);
#endif
}

