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
#include "Core2DriverASPI.h"
#include "../../Common/StringUtil.h"
#include "LogDlg.h"
#include "SCSI.h"

CCore2DriverASPI g_Core2DriverASPI;

CCore2DriverASPI::CCore2DriverASPI()
{
	m_hDllInstance = NULL;
	m_bDriverLoaded = false;

	m_iBus = -1;
	m_iTarget = -1;
	m_iLun = -1;
}

CCore2DriverASPI::~CCore2DriverASPI()
{
	UnloadDriver();
}

bool CCore2DriverASPI::LoadDriver()
{
	m_hDllInstance = LoadLibrary(_T("wnaspi32.dll"));
	if (m_hDllInstance == NULL)
	{
		g_LogDlg.print_line(_T("  Error: Unable to load ASPI driver, wnaspi32.dll could not be loaded."));
		return false;
	}

	GetASPI32SupportInfo = (tGetASPI32SupportInfo)GetProcAddress(m_hDllInstance,"GetASPI32SupportInfo");
	if (!GetASPI32SupportInfo)
		return false;

	SendASPI32Command = (tSendASPI32Command)GetProcAddress(m_hDllInstance,"SendASPI32Command");
	if (!SendASPI32Command)
		return false;

	unsigned long ulStatusCode = (GetASPI32SupportInfo() & 0xFF00) >> 8;
	if (ulStatusCode != SS_COMP && ulStatusCode != SS_NO_ADAPTERS)
	{
		g_LogDlg.print_line(_T("  Error: Unable to load ASPI driver, status code 0x%.2X."),ulStatusCode);
		return false;
	}

	m_bDriverLoaded = true;
	return true;
}

bool CCore2DriverASPI::UnloadDriver()
{
	if (m_hDllInstance == NULL)
		return false;

	FreeLibrary(m_hDllInstance);
	m_hDllInstance = NULL;

	GetASPI32SupportInfo = NULL;
	SendASPI32Command = NULL;

	m_bDriverLoaded = false;
	return true;
}

bool CCore2DriverASPI::Open(int iBus,int iTarget,int iLun)
{
	if (!m_bDriverLoaded)
	{
		// Try to load the driver.
		LoadDriver();
		if (!m_bDriverLoaded)
			return false;
	}

	m_iBus = iBus;
	m_iTarget = iTarget;
	m_iLun = iLun;

	/*unsigned long ulResult = GetASPI32SupportInfo();
	if ((ulResult & 0xFF00) >> 8 != SS_COMP)
	{
		g_LogDlg.print_line(_T("  Error: Unable to open ASPI device, GetASPI32SupportInfo() returned 0x%X."),ulResult);
		return false;
	}

	SRB_HaInquiry srbInquiry;

	unsigned char ucNumHa = (unsigned char)(ulResult & 0xFF);
	for (unsigned char ucHa = 0; ucHa < ucNumHa; ucHa++)
	{
		memset(&srbInquiry,0,sizeof(SRB_HaInquiry));
		srbInquiry.SRB_Cmd = SC_HA_INQUIRY;
		srbInquiry.SRB_HaId = ucHa;

		SendASPI32Command((LPSRB)&srbInquiry);
		if (srbInquiry.SRB_Status != SS_COMP)
		{
			g_LogDlg.print_line(_T("  Warning: Unable query host adapter %d."),ucHa);
			continue;
		}

		g_LogDlg.print_line(_T("  Found host adapter: %d."),ucHa);
	}*/

	return true;
}

bool CCore2DriverASPI::Close()
{
	if (m_iBus == -1 || m_iTarget == -1 || m_iLun == -1)
		return false;

	m_iBus = -1;
	m_iTarget = -1;
	m_iLun = -1;

	return true;
}

bool CCore2DriverASPI::Transport(unsigned char *pCdb,unsigned char ucCdbLength,
							 unsigned char *pData,unsigned long ulDataLength,int iDataMode)
{
	if (m_iBus == -1 || m_iTarget == -1 || m_iLun == -1)
		return false;

	// Verify CDB data.
	if (pCdb == NULL || ucCdbLength > 16)
		return false;

	SRB_ExecSCSICmd srbCommand;
	memset(&srbCommand,0,sizeof(SRB_ExecSCSICmd));

	srbCommand.SRB_Cmd = SC_EXEC_SCSI_CMD;
	srbCommand.SRB_HaId = m_iBus;
	srbCommand.SRB_Target = m_iTarget;
	srbCommand.SRB_Lun = m_iLun;

	srbCommand.SRB_SenseLen = 24;
	srbCommand.SRB_BufPointer = pData;
	srbCommand.SRB_BufLen = ulDataLength;
	srbCommand.SRB_CDBLen = ucCdbLength;
	memcpy(srbCommand.CDBByte,pCdb,ucCdbLength);

	switch (iDataMode)
	{
		case DATAMODE_READ:
			srbCommand.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
			break;

		case DATAMODE_WRITE:
			srbCommand.SRB_Flags = SRB_DIR_OUT | SRB_EVENT_NOTIFY;
			break;

		case DATAMODE_UNSPECIFIED:
			srbCommand.SRB_Flags = 0;
			break;

		default:
			return false;
	}

	HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	ResetEvent(hEvent);
	srbCommand.SRB_PostProc = (void (__cdecl *)(void))hEvent;

	unsigned long ulResult = SendASPI32Command((LPSRB)&srbCommand);

	// Wait for the command to finish.
	if (ulResult == SS_PENDING)
		WaitForSingleObject(hEvent,INFINITE);

	CloseHandle(hEvent);

	if (srbCommand.SRB_Status != SS_COMP)
	{
		g_LogDlg.print_line(_T("  SendASPI32Command failed, status: 0x%.2X, last error: %d."),
			srbCommand.SRB_Status,GetLastError());
		return false;
	}

	if (srbCommand.SRB_TargStat != SCSISTAT_GOOD)
	{
		g_LogDlg.print_line(_T("  Error: SCSI command failed, returned: 0x%.2X."),srbCommand.SRB_TargStat);

		// Dump CDB.
		g_LogDlg.print(_T("  CDB:\t"));
		for (unsigned int i = 0; i < ucCdbLength; i++)
		{
			if (i == 0)
				g_LogDlg.print(_T("0x%.2X"),pCdb[i]);
			else
				g_LogDlg.print(_T(",0x%.2X"),pCdb[i]);
		}

		g_LogDlg.print_line(_T(""));

		// Dump sense information.
		g_LogDlg.print_line(_T("  Sense:\t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
			srbCommand.SenseArea[0],srbCommand.SenseArea[1],srbCommand.SenseArea[2],srbCommand.SenseArea[3],
			srbCommand.SenseArea[4],srbCommand.SenseArea[5],srbCommand.SenseArea[6],srbCommand.SenseArea[7]);

		g_LogDlg.print_line(_T("  \t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
			srbCommand.SenseArea[ 8],srbCommand.SenseArea[ 9],srbCommand.SenseArea[10],srbCommand.SenseArea[11],
			srbCommand.SenseArea[12],srbCommand.SenseArea[13],srbCommand.SenseArea[14],srbCommand.SenseArea[15]);

		g_LogDlg.print_line(_T("  \t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
			srbCommand.SenseArea[16],srbCommand.SenseArea[17],srbCommand.SenseArea[18],srbCommand.SenseArea[19],
			srbCommand.SenseArea[20],srbCommand.SenseArea[21],srbCommand.SenseArea[22],srbCommand.SenseArea[23]);

		return false;
	}

/*#ifdef _DEBUG
	g_LogDlg.print_line(_T("DEBUG: Command 0x%.2X returned %d bytes of data."),pCdb[0],ulReturned);
#endif*/

	return true;
}

bool CCore2DriverASPI::TransportWithSense(unsigned char *pCdb,unsigned char ucCdbLength,
									  unsigned char *pData,unsigned long ulDataLength,
									  unsigned char *pSense,unsigned char &ucResult,int iDataMode)
{
	if (m_iBus == -1 || m_iTarget == -1 || m_iLun == -1)
		return false;

	// Verify CDB data.
	if (pCdb == NULL || ucCdbLength > 16)
		return false;

	// Verify sense buffer.
	if (pSense == NULL)
		return false;

	SRB_ExecSCSICmd srbCommand;
	memset(&srbCommand,0,sizeof(SRB_ExecSCSICmd));

	srbCommand.SRB_Cmd = SC_EXEC_SCSI_CMD;
	srbCommand.SRB_HaId = m_iBus;
	srbCommand.SRB_Target = m_iTarget;
	srbCommand.SRB_Lun = m_iLun;

	srbCommand.SRB_SenseLen = 24;
	srbCommand.SRB_BufPointer = pData;
	srbCommand.SRB_BufLen = ulDataLength;
	srbCommand.SRB_CDBLen = ucCdbLength;
	memcpy(srbCommand.CDBByte,pCdb,ucCdbLength);

	switch (iDataMode)
	{
		case DATAMODE_READ:
			srbCommand.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
			break;

		case DATAMODE_WRITE:
			srbCommand.SRB_Flags = SRB_DIR_OUT | SRB_EVENT_NOTIFY;
			break;

		case DATAMODE_UNSPECIFIED:
			srbCommand.SRB_Flags = 0;
			break;

		default:
			return false;
	}

	HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	ResetEvent(hEvent);
	srbCommand.SRB_PostProc = (void (__cdecl *)(void))hEvent;

	unsigned long ulResult = SendASPI32Command((LPSRB)&srbCommand);

	// Wait for the command to finish.
	if (ulResult == SS_PENDING)
		WaitForSingleObject(hEvent,INFINITE);

	CloseHandle(hEvent);

	/*if (srbCommand.SRB_Status != SS_COMP)
	{
		g_LogDlg.print_line(_T("  SendASPI32Command failed, status: 0x%.2X, last error: %d."),
			srbCommand.SRB_Status,GetLastError());
		return false;
	}*/

/*#ifdef _DEBUG
	g_LogDlg.print_line(_T("DEBUG: Command 0x%.2X returned %d bytes of data."),pCdb[0],ulReturned);
#endif*/

	memcpy(pSense,srbCommand.SenseArea,24);
	ucResult = srbCommand.SRB_TargStat;

	return true;
}
