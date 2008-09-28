/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include "Core2Device.h"
#include "LogDlg.h"

CCore2Device::CCore2Device()
{
	m_iDeviceStatus = DS_CLOSED;
}

CCore2Device::~CCore2Device()
{
	Close();
}

bool CCore2Device::Open(CCore2DeviceAddress *pAddress)
{
	// Close any opened device.
	if (m_iDeviceStatus != DS_CLOSED)
	{
		if (!Close())
			return false;
	}

	// Open the new device.
	if (m_DriverSPTI.Open(pAddress->m_cDriveLetter))
	{
		m_iDeviceStatus = DS_OPEN_SPTI;
		g_LogDlg.PrintLine(_T("  Opened device '%C' using SPTI."),pAddress->m_cDriveLetter);
	}
	else if (m_DriverASPI.Open(pAddress->m_iBus,pAddress->m_iTarget,pAddress->m_iLun))
	{
		m_iDeviceStatus = DS_OPEN_ASPI;
		g_LogDlg.PrintLine(_T("  Opened device [%d,%d,%d] using ASPI."),pAddress->m_iBus,pAddress->m_iTarget,pAddress->m_iLun);
	}
	else
		m_iDeviceStatus = DS_CLOSED;

	return m_iDeviceStatus != DS_CLOSED;
}

bool CCore2Device::Close()
{
	bool bResult = false;

	if (m_iDeviceStatus == DS_OPEN_SPTI)
		bResult =  m_DriverSPTI.Close();
	else if (m_iDeviceStatus = DS_OPEN_ASPI)
		bResult = m_DriverASPI.Close();

	m_iDeviceStatus = DS_CLOSED;
	return bResult;
}

bool CCore2Device::IsOpen()
{
	return m_iDeviceStatus != DS_CLOSED;
}

bool CCore2Device::SetNextTimeOut(unsigned int uiTimeOut)
{
	if (m_iDeviceStatus == DS_OPEN_SPTI)
		return m_DriverSPTI.SetNextTimeOut(uiTimeOut);

	return false;
}

bool CCore2Device::Transport(unsigned char *pCdb,unsigned char ucCdbLength,
							 unsigned char *pData,unsigned long ulDataLength,int iDataMode)
{
	if (m_iDeviceStatus == DS_OPEN_SPTI)
		return m_DriverSPTI.Transport(pCdb,ucCdbLength,pData,ulDataLength,iDataMode);
	else if (m_iDeviceStatus = DS_OPEN_ASPI)
		return m_DriverASPI.Transport(pCdb,ucCdbLength,pData,ulDataLength,iDataMode);

	return false;
}

bool CCore2Device::TransportWithSense(unsigned char *pCdb,unsigned char ucCdbLength,
									  unsigned char *pData,unsigned long ulDataLength,
									  unsigned char *pSense,unsigned char &ucResult,int iDataMode)
{
	if (m_iDeviceStatus == DS_OPEN_SPTI)
		return m_DriverSPTI.TransportWithSense(pCdb,ucCdbLength,pData,ulDataLength,pSense,ucResult,iDataMode);
	else if (m_iDeviceStatus = DS_OPEN_ASPI)
		return m_DriverASPI.TransportWithSense(pCdb,ucCdbLength,pData,ulDataLength,pSense,ucResult,iDataMode);

	return true;
}
