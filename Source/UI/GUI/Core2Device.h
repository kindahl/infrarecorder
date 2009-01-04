/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

#pragma once
#include "Core2DriverSPTI.h"
#include "Core2DriverASPI.h"

class CCore2DeviceAddress
{
public:
	TCHAR m_cDriveLetter;
	int m_iBus,m_iTarget,m_iLun;

	CCore2DeviceAddress()
	{
		m_cDriveLetter = NULL;
		m_iBus = -1;
		m_iTarget = -1;
		m_iLun = -1;
	}
};

class CCore2Device
{
private:
	CCore2DriverSPTI m_DriverSPTI;
	CCore2DriverASPI m_DriverASPI;
	int m_iDeviceStatus;

	enum
	{
		DS_OPEN_SPTI,
		DS_OPEN_ASPI,
		DS_CLOSED
	};

public:
	CCore2Device();
	~CCore2Device();

	enum
	{
		DATAMODE_READ,
		DATAMODE_WRITE,
		DATAMODE_UNSPECIFIED
	};

	bool Open(CCore2DeviceAddress *pAddress);
	bool Close();
	bool IsOpen();

	bool SetNextTimeOut(unsigned int uiTimeOut);
	bool Transport(unsigned char *pCdb,unsigned char ucCdbLength,
		unsigned char *pData,unsigned long ulDataLength,int iDataMode = DATAMODE_READ);
	bool TransportWithSense(unsigned char *pCdb,unsigned char ucCdbLength,
		unsigned char *pData,unsigned long ulDataLength,
		unsigned char *pSense,unsigned char &ucResult,int iDataMode = DATAMODE_READ);
};
