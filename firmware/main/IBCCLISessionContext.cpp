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
#include "IBCCLISessionContext.h"
#include <math.h>

const char* g_iincalObjectName = "cal.iin";
const char* g_ioutcalObjectName = "cal.iout";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command IDs

enum cmdid_t
{
	CMD_CALIBRATE,
	CMD_CAT,
	CMD_COMMIT,
	CMD_IIN,
	CMD_IOUT
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// "calibrate"

static const clikeyword_t g_iincalCommands[] =
{
	{"<mA>",		FREEFORM_TOKEN,		nullptr,			"Externally measured 48V input current, in integer mA"},
	{nullptr,		INVALID_COMMAND,	nullptr,			nullptr}
};

static const clikeyword_t g_ioutcalCommands[] =
{
	{"<mA>",		FREEFORM_TOKEN,		nullptr,			"Externally measured 12V output current, in integer mA"},
	{nullptr,		INVALID_COMMAND,	nullptr,			nullptr}
};

static const clikeyword_t g_calibrateCommands[] =
{
	{"iin",			CMD_IIN,			g_iincalCommands,	"Input current" },
	{"iout",		CMD_IOUT,			g_ioutcalCommands,	"Output current" },
	{nullptr,		INVALID_COMMAND,	nullptr,			nullptr }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Top level commands

static const clikeyword_t g_rootCommands[] =
{
	{"calibrate",	CMD_CALIBRATE,		g_calibrateCommands,	"Calibrate ADCs" },
	{"cat",			CMD_CAT,			nullptr,				"meow" },
	{"commit",		CMD_COMMIT,			nullptr,				"Commit pending calibrations to flash" },
	{nullptr,		INVALID_COMMAND,	nullptr,				nullptr }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction / destruction

IBCCLISessionContext::IBCCLISessionContext()
	: CLISessionContext(g_rootCommands)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Top level command dispatch

void IBCCLISessionContext::PrintPrompt()
{
	m_stream->Printf("%s@ibc# ", m_username);
	m_stream->Flush();
}

void IBCCLISessionContext::OnExecute()
{
	switch(m_command[0].m_commandID)
	{
		case CMD_CALIBRATE:
			OnCalibrate();
			break;

		case CMD_CAT:
			m_stream->Printf("nyaa~\n");
			break;

		case CMD_COMMIT:
			OnCommit();
			break;

		default:
			m_stream->Printf("Unrecognized command\n");
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// "calibrate"

void IBCCLISessionContext::OnCalibrate()
{
	switch(m_command[1].m_commandID)
	{
		case CMD_IIN:
			OnCalibrateInputCurrent();
			break;

		case CMD_IOUT:
			OnCalibrateOutputCurrent();
			break;

		default:
			break;
	}
}

void IBCCLISessionContext::OnCalibrateInputCurrent()
{
	m_stream->Printf("Calibrating input current\n");

	//Baseline
	int16_t expected = atoi(m_command[2].m_text);
	m_stream->Printf("    Expected:     %d mA\n", expected);

	//Measured value with no calibration
	int16_t measured = round(
		g_adc->ReadChannelScaledAveraged(ADC_CHANNEL_INPUT_CURRENT, 128) *
		SHUNT_SCALE_INPUT_CURRENT);
	m_stream->Printf("    Uncalibrated: %d mA\n", measured);

	//Calculate the cal coefficient
	g_inputCurrentShuntOffset = measured - expected;
	m_stream->Printf("    Offset:       %d mA\n", g_inputCurrentShuntOffset);

	//Re-measure to confirm it's good
	int16_t remeasured = GetInputCurrent();
	m_stream->Printf("    Calibrated:   %d mA\n", remeasured);
}

void IBCCLISessionContext::OnCalibrateOutputCurrent()
{
	m_stream->Printf("Calibrating output current\n");

	//Baseline
	int16_t expected = atoi(m_command[2].m_text);
	m_stream->Printf("    Expected:     %d mA\n", expected);

	//Measured value with no calibration
	int16_t measured = round(
		g_adc->ReadChannelScaledAveraged(ADC_CHANNEL_OUTPUT_CURRENT, 128) *
		SHUNT_SCALE_OUTPUT_CURRENT);
	m_stream->Printf("    Uncalibrated: %d mA\n", measured);

	//Calculate the cal coefficient
	g_outputCurrentShuntOffset = measured - expected;
	m_stream->Printf("    Offset:       %d mA\n", g_outputCurrentShuntOffset);

	//Re-measure to confirm it's good
	int16_t remeasured = GetOutputCurrent();
	m_stream->Printf("    Calibrated:   %d mA\n", remeasured);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// "commit"

void IBCCLISessionContext::OnCommit()
{
	g_kvs->StoreObjectIfNecessary(g_inputCurrentShuntOffset, (uint16_t)0, g_iincalObjectName);
	g_kvs->StoreObjectIfNecessary(g_outputCurrentShuntOffset, (uint16_t)0, g_ioutcalObjectName);
}
