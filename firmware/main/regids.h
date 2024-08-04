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

#ifndef ibc_regids_h
#define ibc_regids_h

enum ibc_mode_t
{
	IBC_NORMAL		= 0x55,
	IBC_BOOTLOADER	= 0xaa
};

enum ibc_regid_t
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Commands in 00 - 7f available for application

	IBC_REG_VIN			= 0x00,		//Input voltage (uint16 mV)
	IBC_REG_VOUT		= 0x01,		//Output voltage (uint16 mV)
	IBC_REG_VSENSE		= 0x02,		//Sense voltage (uint16_t mV)

	IBC_REG_IIN			= 0x10,		//Input current (uint16_t mA)
	IBC_REG_IOUT		= 0x11,		//Output current (uint16_t mA)

	IBC_REG_VERSION		= 0x20,		//Version string (20 bytes ASCII)

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Commands in 80-ff reserved for the bootloader (or entry to it)

	IBC_ENTER_DFU		= 0x80,	//reboot to DFU mode
	IBC_GET_STATUS		= 0x81,	//Return 0x55 in normal mode
								//or 0xaa in bootloader mode
	IBC_BOOT_APP		= 0x82,	//reboot in application mode
	IBC_ERASE_APP		= 0x83,	//erase application partition of flash
								//returns 0s until complete, then 1
	IBC_FLASH_ADDR		= 0x84,	//32-bit flash destination address
	IBC_FLASH_WRITE		= 0x85,	//data to be written to FLASH_ADDR
	IBC_FLASH_STATUS	= 0x86,	//read status of a flash write
								//returns 0s until complete, then 1
	IBC_FLASH_FLUSH		= 0x87,	//flush pending writes
	IBC_FLASH_SYNC		= 0x88,	//return constant 0xcc
	IBC_EXPECTED_CRC	= 0x89	//write expected image CRC to KVS
};

#endif
