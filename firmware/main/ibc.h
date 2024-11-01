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

#ifndef ibc_h
#define ibc_h

#include <core/platform.h>

#include <peripheral/ADC.h>
#include <peripheral/I2C.h>
#include <peripheral/ITM.h>
#include <peripheral/ITMStream.h>

#include <embedded-utils/FIFO.h>
#include <embedded-utils/StringBuffer.h>

#include <bootloader/BootloaderAPI.h>
#include "../bsp/hwinit.h"

#include <cli/UARTOutputStream.h>

void InitGPIOs();
void InitI2C();
void InitADC();

extern UART<16, 256> g_uart;
extern I2C g_i2c;
extern ADC* g_adc;
extern char g_version[20];
extern char g_hwversion[20];

extern GPIOPin g_standbyLED;
extern GPIOPin g_onLED;
extern GPIOPin g_faultLED;

extern GPIOPin g_loadEnableSense;
extern GPIOPin g_outEnableFromLoad;
extern GPIOPin g_outEnableFromProtection;

void PrintSensorValues();

extern uint16_t g_inputCurrentShuntOffset;
extern uint16_t g_outputCurrentShuntOffset;

//ADC channels
enum adc_channel_index
{
	ADC_CHANNEL_INPUT_CURRENT	= 6,
	ADC_CHANNEL_OUTPUT_CURRENT	= 12
};

//ADC scale factors
#define SHUNT_SCALE_INPUT_CURRENT 2
#define SHUNT_SCALE_OUTPUT_CURRENT 10

extern const char* g_iincalObjectName;
extern const char* g_ioutcalObjectName;

extern bool g_freestandingMode;
extern const uint8_t g_tempI2cAddress;

#include "IBCI2CServer.h"
#include "IBCCLISessionContext.h"

extern IBCI2CServer* g_i2cServer;

#endif
