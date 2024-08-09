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

void IBCI2CServer::OnRequestStart()
{
}

void IBCI2CServer::OnRequestRead()
{
	//Send the reply
	switch(m_regid)
	{
		//Read input voltage
		case IBC_REG_VIN:
			m_i2c.BlockingDeviceWrite16(GetInputVoltage());
			break;

		//Read output voltage
		case IBC_REG_VOUT:
			m_i2c.BlockingDeviceWrite16(GetOutputVoltage());
			break;

		//Read sense voltage
		case IBC_REG_VSENSE:
			m_i2c.BlockingDeviceWrite16(GetSenseVoltage());
			break;

		/*
		//Read input current
		case IBC_REG_IIN:
			m_i2c.BlockingDeviceWrite16(GetInputCurrent());
			break;

		//Read output current
		case IBC_REG_IOUT:
			m_i2c.BlockingDeviceWrite16(GetOutputCurrent());
			break;
		*/

		//Read version string
		case IBC_REG_VERSION:
			for(size_t i=0; i<sizeof(g_version); i++)
				m_i2c.BlockingDeviceWrite8(g_version[i]);
			break;

		default:
			break;
	}
}
