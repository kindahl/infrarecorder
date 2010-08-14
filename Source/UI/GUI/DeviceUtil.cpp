/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include <ckcore/convert.hh>
#include "DeviceUtil.h"

namespace NDeviceUtil
{
	/**
	 * Convert the address of the specified device into string form making
	 * it compatible with command line usage of the back-end applications.
	 * @param [in] device The device to obtain the address from.
	 * @return The address in command line string form.
	 */
	ckcore::tstring GetDeviceAddr(const ckmmc::Device &device)
	{
		const ckmmc::Device::Address &addr = device.address();
	#ifdef USE_CDRKIT
		return addr.device_;
	#else
		ckcore::tchar buffer[64];
		ckcore::convert::sprintf(buffer,sizeof(buffer),ckT("%d,%d,%d"),
								 addr.bus_,addr.target_,addr.lun_);

		return buffer;
	#endif
	}

	/**
	 * Convert the address of the specified device into string form making
	 * it compatible with command line usage of the back-end applications.
	 * @param [in] device The device to obtain the address from.
	 * @return The address in command line string form.
	 */
	std::string GetDeviceAddrA(const ckmmc::Device &device)
	{
		const ckmmc::Device::Address &addr = device.address();
	#ifdef USE_CDRKIT
		return addr.device_;
	#else
		char buffer[64];
		sprintf(buffer,"%d,%d,%d",addr.bus_,addr.target_,addr.lun_);

		return buffer;
	#endif
	}

	ckcore::tstring GetDeviceName(const ckmmc::Device &device)
	{
		const ckmmc::Device::Address &addr = device.address();

		return addr.device_ + ckT(": ") + device.name();
	}
};
