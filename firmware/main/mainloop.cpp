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
	/*
	const int logTimerMax = 60000;
	const uint32_t displayRefreshInterval = 3600;						//1 hour
	const uint32_t fullRefreshInterval = displayRefreshInterval * 24;	//1 day

	static uint32_t nextDisplayRefresh = 30;
	static uint32_t nextFullRefresh = 5;		//Wait 5 sec after power up to do the first full refresh,
										//so main FPGA and MCU have a chance to come up

	static uint32_t next1HzTick = 0;
	static uint8_t nbyte = 0;
	static uint8_t cmd = 0;
	static uint32_t secSinceLastMcuUpdate = 0;

	//Check for overflows on our log message timer
	if(g_log.UpdateOffset(logTimerMax) && (next1HzTick >= logTimerMax) )
		next1HzTick -= logTimerMax;

	//Run the display state machine
	g_display->OnTick();

	//1 Hz timer event for display refreshes
	if(g_logTimer.GetCount() >= next1HzTick)
	{
		//g_log("1 Hz tick nextFull = %d nextDisplay = %d refreshing=%d\n",
		//	nextFullRefresh, nextDisplayRefresh, g_display->IsRefreshInProgress());

		next1HzTick = g_logTimer.GetCount() + 10000;

		//Watchdog timer to detect main MCU acting up
		secSinceLastMcuUpdate ++;
		if(secSinceLastMcuUpdate > 5)
		{
			if(!g_mainMCUDown)
			{
				g_mainMCUDown = true;
				nextDisplayRefresh = 1;
			}
		}

		//Update display if needed
		if(!g_display->IsRefreshInProgress())
		{
			//Full refresh once a day
			if(nextFullRefresh == 0)
			{
				nextFullRefresh = fullRefreshInterval;
				nextDisplayRefresh = displayRefreshInterval;
				RefreshDisplay(true);
			}

			//Default to refreshing the display once an hour
			else if(nextDisplayRefresh == 0)
			{
				nextDisplayRefresh = displayRefreshInterval;
				RefreshDisplay(false);
			}

			//Bump timer counts
			nextDisplayRefresh --;
			nextFullRefresh --;
		}
	}

	//Read and process SPI events
	if(g_fpgaSPI.HasEvents())
	{
		auto event = g_fpgaSPI.GetEvent();

		//Reset byte count on CS# rising or falling edge
		if(event.type == SPIEvent::TYPE_CS)
			nbyte = 0;

		//Process data byte
		else
		{
			auto data = event.data;

			//If main MCU was down, it's now up again
			secSinceLastMcuUpdate = 0;
			if(g_mainMCUDown)
			{
				g_mainMCUDown = false;
				nextDisplayRefresh = 1;
			}

			//First byte is command
			if(nbyte == 0)
			{
				cmd = data;

				switch(cmd)
				{
					//Reboot in bootloader mode
					case FRONT_ENTER_DFU:
						g_bbram->m_state = STATE_DFU;
						Reset();
						break;

					//Commands that produce SPI output
					case FRONT_GET_STATUS:
						{
							if(g_misoIsJtag)
								SetMisoToSPIMode();

							const uint8_t tmp = FRONT_NORMAL;
							g_fpgaSPI.NonblockingWriteFifo(&tmp, sizeof(tmp));
						}
						break;

					//sending any other SPI command returns us to JTAG mode
					default:
						if(!g_misoIsJtag)
							SetMisoToJTAGMode();
						break;
				}
			}

			//Then comes data bytes
			else
			{
				switch(cmd)
				{
					//Readback commands do nothing here
					case FRONT_GET_STATUS:
						break;

					//Schedule a display refresh
					case FRONT_REFRESH_FAST:
						nextDisplayRefresh = 0;
						break;

					case FRONT_REFRESH_FULL:
						nextFullRefresh = 0;
						break;

					//Link speed
					case FRONT_ETH_LINK:

						//If speed changed, trigger a display refresh
						if(g_linkSpeed != data)
							nextDisplayRefresh = 0;

						g_linkSpeed = data;
						break;

					//IPv4 address
					case FRONT_IP4_ADDR:
						if(nbyte <= 4)
						{
							//If IP changed, trigger a display refresh
							if(g_ipv4Addr[nbyte-1] != data)
								nextDisplayRefresh = 0;

							g_ipv4Addr[nbyte-1] = data;
						}
						break;

					//DHCP enable flag
					case FRONT_IPV4_DHCP:
						if(nbyte == 0)
							g_staticIP = nbyte ? true : false;
						break;

					//IPv6 address
					case FRONT_IP6_ADDR:
						if(nbyte <= 16)
						{
							int nword = (nbyte-1)/2;
							int half = (nbyte-1) % 2;

							if(half)
								g_ipv6Addr[nword] |= data;
							else
								g_ipv6Addr[nword] = data << 8;
						}
						break;

					//Serial number
					case FRONT_SERIAL:
						if(nbyte <= 8)
							g_serial[nbyte-1] = data;
						break;

					//Main MCU firmware
					case FRONT_MCU_FW:
						if(RxSPIString(nbyte, g_mcuFirmware, sizeof(g_mcuFirmware), data))
							nextDisplayRefresh = 0;
						break;

					//IBC MCU firmware
					case FRONT_IBC_FW:
						if(RxSPIString(nbyte, g_ibcFirmware, sizeof(g_ibcFirmware), data))
							nextDisplayRefresh = 0;
						break;

					//Supervisor MCU firmware
					case FRONT_SUPER_FW:
						if(RxSPIString(nbyte, g_superFirmware, sizeof(g_superFirmware), data))
							nextDisplayRefresh = 0;
						break;

					//FPGA firmware
					case FRONT_FPGA_FW:
						if(RxSPIString(nbyte, g_fpgaFirmware, sizeof(g_fpgaFirmware), data))
							nextDisplayRefresh = 0;
						break;

					//FPGA die temperature
					case FRONT_FPGA_TEMP:
						if(nbyte == 1)
							g_fpgaTemp = data;
						else if(nbyte == 2)
							g_fpgaTemp |= data << 8;
						break;

					//MCU die temperature
					case FRONT_MCU_TEMP:
						if(nbyte == 1)
							g_mcuTemp = data;
						else if(nbyte == 2)
							g_mcuTemp |= data << 8;
						break;

					//IBC board temperature
					case FRONT_IBC_TEMP:
						if(nbyte == 1)
							g_ibcTemp = data;
						else if(nbyte == 2)
							g_ibcTemp |= data << 8;
						break;

					//IBC input
					case FRONT_IBC_VIN:
						if(nbyte == 1)
							g_vin = data;
						else if(nbyte == 2)
							g_vin |= data << 8;
						break;
					case FRONT_IBC_IIN:
						if(nbyte == 1)
							g_iin = data;
						else if(nbyte == 2)
							g_iin |= data << 8;
						break;

					//IBC output
					case FRONT_IBC_VOUT:
						if(nbyte == 1)
							g_vout = data;
						else if(nbyte == 2)
							g_vout |= data << 8;
						break;
					case FRONT_IBC_IOUT:
						if(nbyte == 1)
							g_iout = data;
						else if(nbyte == 2)
							g_iout |= data << 8;
						break;

					//Fan RPM indicator
					case FRONT_FAN_RPM:
						if(nbyte == 1)
							g_fanspeed = data;
						else if(nbyte == 2)
							g_fanspeed |= data << 8;
						break;

					//Subnet mask
					case FRONT_IP4_SUBNET:
						if(nbyte == 1)
						{
							//If prefix changed, trigger a display refresh
							if(g_ipv4SubnetSize != data)
								nextDisplayRefresh = 0;

							g_ipv4SubnetSize = data;
						}
						break;
					case FRONT_IP6_SUBNET:
						if(nbyte == 1)
							g_ipv6SubnetSize = data;
						else if(nbyte == 2)
							g_ipv6SubnetSize |= data << 8;
						break;

					//Timestamp of sensor values
					case FRONT_TIMESTAMP:
						RxSPIString(nbyte, g_dataTimestamp, sizeof(g_dataTimestamp), data);
						break;

					//Port direction indicator LEDs
					case FRONT_DIR_LEDS:
						*g_outmodeLED[0] = (data & 1) == 1;
						*g_outmodeLED[1] = (data & 2) == 2;
						*g_outmodeLED[2] = (data & 4) == 4;
						*g_outmodeLED[3] = (data & 8) == 8;

						*g_inmodeLED[0] = (data & 0x10) == 0x10;
						*g_inmodeLED[1] = (data & 0x20) == 0x20;
						*g_inmodeLED[2] = (data & 0x40) == 0x40;
						*g_inmodeLED[3] = (data & 0x80) == 0x80;
						break;

					//Port status LEDs
					case FRONT_PORT_LEDS:
						if(nbyte <= 3)
							g_expander->BatchUpdateValue(nbyte-1, data);
						if(nbyte == 3)
							g_expander->BatchCommitValue();
						break;

					default:
						g_log("Unrecognized command %02x\n", cmd);
						break;

				}
			}

			nbyte ++;
		}
	}
	*/
}
