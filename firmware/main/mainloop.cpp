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

//I2C2 runs off our APB1 clock (40 MHz)
//Prescale by 4 to get 10 MHz
//Divide by 100 after that to get 100 kHz
//I2C g_i2c(&I2C2, 4, 100);

void BSP_MainLoopIteration()
{
	const int logTimerMax = 60000;
	static uint32_t next1HzTick = 0;

	//Make output-enable LED track the actual load power enable state
	g_onLED = g_loadEnableSense;

	/*
	static uint8_t nbyte = 0;
	static uint8_t cmd = 0;
	static uint32_t secSinceLastMcuUpdate = 0;
	*/
	//Check for overflows on our log message timer
	if(g_log.UpdateOffset(logTimerMax) && (next1HzTick >= logTimerMax) )
		next1HzTick -= logTimerMax;

	//1 Hz timer event for display refreshes
	if(g_logTimer.GetCount() >= next1HzTick)
	{
		next1HzTick = g_logTimer.GetCount() + 10000;
	}

	/*

	*/
}
