/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

class CCore2DriverSPTI
{
private:
	HANDLE m_hDevice;
	int m_iNextTimeOut;
	bool m_bLog;

public:
	CCore2DriverSPTI();
	~CCore2DriverSPTI();

	enum
	{
		DATAMODE_READ,
		DATAMODE_WRITE,
		DATAMODE_UNSPECIFIED
	};

	enum
	{
		DEFAULT_TIMEOUT = 60
	};

	static bool GetDriveLetter(int iBus,int iTarget,int iLun,TCHAR &cDriveLetter);
	static bool GetDriveLetter(TCHAR *szVendor,TCHAR *szIdentification,
		TCHAR *szRevision,TCHAR &cDriveLetter);

	bool Open(TCHAR cDriveLetter);
	bool Close();

	bool SetNextTimeOut(unsigned int uiTimeOut);
	bool Transport(unsigned char *pCdb,unsigned char ucCdbLength,
		unsigned char *pData,unsigned long ulDataLength,int iDataMode = DATAMODE_READ);
	bool TransportWithSense(unsigned char *pCdb,unsigned char ucCdbLength,
		unsigned char *pData,unsigned long ulDataLength,
		unsigned char *pSense,unsigned char &ucResult,int iDataMode = DATAMODE_READ);
};
