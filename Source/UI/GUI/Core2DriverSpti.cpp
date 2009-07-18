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

#include "stdafx.h"
#include <ntddscsi.h>
#include "LogDlg.h"
#include "Scsi.h"
#include "Core2DriverSpti.h"

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER
{
	SCSI_PASS_THROUGH_DIRECT spt;
	ULONG Filler;							// Realign buffer to double word boundary.
	UCHAR ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

CCore2DriverSpti::CCore2DriverSpti()
{
	m_hDevice = INVALID_HANDLE_VALUE;
	m_iNextTimeOut = -1;	// Use default.
	m_bLog = true;
}

CCore2DriverSpti::~CCore2DriverSpti()
{
	Close();
}

bool CCore2DriverSpti::GetDriveLetter(int iBus,int iTarget,int iLun,
									  TCHAR &cDriveLetter)
{
	unsigned long ulDummy;
	SCSI_ADDRESS DeviceAddress;

	CCore2DriverSpti Device;

	// For some reason it's not possible to log when this function is called. The
	// AppendText function in CEdit never returns. Is the behaviour related to
	// thread-safety?
	Device.m_bLog = false;

	for (TCHAR cCurDriveLetter = 'C'; cCurDriveLetter <= 'Z'; cCurDriveLetter++)
	{
		if (!Device.Open(cCurDriveLetter))
			continue;

		memset(&DeviceAddress,0,sizeof(SCSI_ADDRESS));
		if (!DeviceIoControl(Device.m_hDevice,IOCTL_SCSI_GET_ADDRESS,NULL,0,&DeviceAddress,sizeof(SCSI_ADDRESS),&ulDummy,FALSE))
		{
			Device.Close();
			continue;
		}

		Device.Close();

		if (DeviceAddress.PortNumber == iBus &&
			DeviceAddress.TargetId == iTarget &&
			DeviceAddress.Lun == iLun)
		{
			cDriveLetter = cCurDriveLetter;
			return true;
		}
	}

	return false;
}

bool CCore2DriverSpti::GetDriveLetter(TCHAR *szVendor,TCHAR *szIdentification,
									  TCHAR *szRevision,TCHAR &cDriveLetter)
{
#ifdef UNICODE
	char szMultiVendor[9];
	char szMultiIdentification[17];
	char szMultiRevision[5];

	WideCharToMultiByte(::AreFileApisANSI() ? CP_ACP : CP_OEMCP,0,
		szVendor,(int)lstrlen(szVendor) + 1,szMultiVendor,(int)lstrlen(szVendor) + 1,NULL,NULL);
	WideCharToMultiByte(::AreFileApisANSI() ? CP_ACP : CP_OEMCP,0,
		szIdentification,(int)lstrlen(szIdentification) + 1,szMultiIdentification,
		(int)lstrlen(szIdentification) + 1,NULL,NULL);
	WideCharToMultiByte(::AreFileApisANSI() ? CP_ACP : CP_OEMCP,0,
		szRevision,(int)lstrlen(szRevision) + 1,szMultiRevision,(int)lstrlen(szRevision) + 1,NULL,NULL);
#else
	char *szMultiVendor = szVendor;
	char *szMultiIdentification = szIdentification;
	char *szMultiRevision = szRevision;
#endif

	int iVendorLen = lstrlen(szVendor);
	int iIdentficationLen = lstrlen(szIdentification);
	int iRevisionLen = lstrlen(szRevision);

	CCore2DriverSpti Device;

	// For some reason it's not possible to log when this function is called. The
	// AppendText function in CEdit never returns. Is the behaviour related to
	// thread-safety?
	Device.m_bLog = false;

	// Initialize buffers.
	unsigned char ucBuffer[192];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_INQUIRY;
	ucCdb[4] = 192;

	for (TCHAR cCurDriveLetter = 'C'; cCurDriveLetter <= 'Z'; cCurDriveLetter++)
	{
		if (!Device.Open(cCurDriveLetter))
			continue;

		// Get the device name.
		Device.SetNextTimeOut(2);
		if (!Device.Transport(ucCdb,5,ucBuffer,192))
		{
			Device.Close();
			continue;
		}

		Device.Close();

		// Check if the vendor, identification and revision matches.
		if (!memcmp(szMultiVendor,ucBuffer + 8,iVendorLen) &&
			!memcmp(szMultiIdentification,ucBuffer + 16,iIdentficationLen) &&
			!memcmp(szMultiRevision,ucBuffer + 32,iRevisionLen))
		{
			cDriveLetter = cCurDriveLetter;
			return true;
		}
	}

	return false;
}

bool CCore2DriverSpti::GetDriveAddress(TCHAR cDriveLetter,
									   int &iBus,int &iTarget,int &iLun)
{
	unsigned long ulDummy;
	SCSI_ADDRESS DeviceAddress;

	CCore2DriverSpti Device;

	// For some reason it's not possible to log when this function is called. The
	// AppendText function in CEdit never returns. Is the behaviour related to
	// thread-safety?
	Device.m_bLog = false;

	if (!Device.Open(cDriveLetter))
		return false;

	memset(&DeviceAddress,0,sizeof(SCSI_ADDRESS));
	if (!DeviceIoControl(Device.m_hDevice,IOCTL_SCSI_GET_ADDRESS,NULL,0,&DeviceAddress,sizeof(SCSI_ADDRESS),&ulDummy,FALSE))
	{
		Device.Close();
		return false;
	}

	Device.Close();

	iBus = DeviceAddress.PortNumber;
	iTarget = DeviceAddress.TargetId;
	iLun = DeviceAddress.Lun;

	return true;
}

bool CCore2DriverSpti::Open(TCHAR cDriveLetter)
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		if (!Close())
			return false;
	}

	TCHAR szDriveString[7];
	lstrcpy(szDriveString,_T("\\\\.\\X:"));
	szDriveString[4] = cDriveLetter;

	// Create the device connection.
	m_hDevice = CreateFile(szDriveString,GENERIC_READ | GENERIC_WRITE,
						   FILE_SHARE_READ | FILE_SHARE_WRITE,
						   NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	return m_hDevice != INVALID_HANDLE_VALUE;
}

bool CCore2DriverSpti::Close()
{
	if (m_hDevice == INVALID_HANDLE_VALUE)
		return false;

	if (CloseHandle(m_hDevice) != FALSE)
	{
		m_hDevice = INVALID_HANDLE_VALUE;
		return true;
	}

	return false;
}

bool CCore2DriverSpti::SetNextTimeOut(unsigned int uiTimeOut)
{
	m_iNextTimeOut = uiTimeOut;
	return true;
}

bool CCore2DriverSpti::Transport(unsigned char *pCdb,unsigned char ucCdbLength,
								 unsigned char *pData,unsigned long ulDataLength,int iDataMode)
{
	if (m_hDevice == INVALID_HANDLE_VALUE)
		return false;

	// Verify CDB data.
	if (pCdb == NULL || ucCdbLength > 16)
		return false;

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptwb;
	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	sptwb.spt.DataTransferLength = ulDataLength;
	sptwb.spt.DataBuffer = pData;
	sptwb.spt.CdbLength = ucCdbLength;
	//sptwb.spt.TargetId = 1;
	memcpy(sptwb.spt.Cdb,pCdb,ucCdbLength);

	if (m_iNextTimeOut < 0)
	{
		sptwb.spt.TimeOutValue = DEFAULT_TIMEOUT;
	}
	else
	{
		sptwb.spt.TimeOutValue = m_iNextTimeOut;
		m_iNextTimeOut = -1;
	}

	switch (iDataMode)
	{
		case DATAMODE_READ:
			sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
			break;

		case DATAMODE_WRITE:
			sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;
			break;

		case DATAMODE_UNSPECIFIED:
			sptwb.spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
			break;

		default:
			return false;
	}

	unsigned long ulReturned = 0;
	if (!DeviceIoControl(m_hDevice,IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
		&sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),&ulReturned,FALSE))
	{
		if (m_bLog)
			g_pLogDlg->print_line(_T("  DeviceIoControl failed, last error: %d."),GetLastError());

		return false;
	}

	if (sptwb.spt.ScsiStatus != SCSISTAT_GOOD)
	{
		if (m_bLog)
		{
			g_pLogDlg->print_line(_T("  Error: SCSI command failed, returned: 0x%.2X."),sptwb.spt.ScsiStatus);

			// Dump CDB.
			g_pLogDlg->print(_T("  CDB:\t"));
			for (unsigned int i = 0; i < ucCdbLength; i++)
			{
				if (i == 0)
					g_pLogDlg->print(_T("0x%.2X"),pCdb[i]);
				else
					g_pLogDlg->print(_T(",0x%.2X"),pCdb[i]);
			}

			g_pLogDlg->print_line(_T(""));

			// Dump sense information.
			g_pLogDlg->print_line(_T("  Sense:\t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
				sptwb.ucSenseBuf[0],sptwb.ucSenseBuf[1],sptwb.ucSenseBuf[2],sptwb.ucSenseBuf[3],
				sptwb.ucSenseBuf[4],sptwb.ucSenseBuf[5],sptwb.ucSenseBuf[6],sptwb.ucSenseBuf[7]);

			g_pLogDlg->print_line(_T("  \t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
				sptwb.ucSenseBuf[ 8],sptwb.ucSenseBuf[ 9],sptwb.ucSenseBuf[10],sptwb.ucSenseBuf[11],
				sptwb.ucSenseBuf[12],sptwb.ucSenseBuf[13],sptwb.ucSenseBuf[14],sptwb.ucSenseBuf[15]);

			g_pLogDlg->print_line(_T("  \t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
				sptwb.ucSenseBuf[16],sptwb.ucSenseBuf[17],sptwb.ucSenseBuf[18],sptwb.ucSenseBuf[19],
				sptwb.ucSenseBuf[20],sptwb.ucSenseBuf[21],sptwb.ucSenseBuf[22],sptwb.ucSenseBuf[23]);
		}

		return false;
	}

	return true;
}

bool CCore2DriverSpti::TransportWithSense(unsigned char *pCdb,unsigned char ucCdbLength,
										  unsigned char *pData,unsigned long ulDataLength,
										  unsigned char *pSense,unsigned char &ucResult,int iDataMode)
{
	if (m_hDevice == INVALID_HANDLE_VALUE)
		return false;

	// Verify CDB data.
	if (pCdb == NULL || ucCdbLength > 16)
		return false;

	// Verify sense buffer.
	if (pSense == NULL)
		return false;

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptwb;
	memset(&sptwb,0,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	sptwb.spt.DataTransferLength = ulDataLength;
	sptwb.spt.DataBuffer = pData;
	sptwb.spt.CdbLength = ucCdbLength;
	memcpy(sptwb.spt.Cdb,pCdb,ucCdbLength);

	if (m_iNextTimeOut < 0)
	{
		sptwb.spt.TimeOutValue = DEFAULT_TIMEOUT;
	}
	else
	{
		sptwb.spt.TimeOutValue = m_iNextTimeOut;
		m_iNextTimeOut = -1;
	}

	switch (iDataMode)
	{
		case DATAMODE_READ:
			sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
			break;

		case DATAMODE_WRITE:
			sptwb.spt.DataIn = SCSI_IOCTL_DATA_OUT;
			break;

		case DATAMODE_UNSPECIFIED:
			sptwb.spt.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
			break;

		default:
			return false;
	}

	unsigned long ulReturned = 0;
	if (!DeviceIoControl(m_hDevice,IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
		&sptwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),&ulReturned,FALSE))
	{
		if (m_bLog)
			g_pLogDlg->print_line(_T("  DeviceIoControl failed, last error: %d."),GetLastError());

		return false;
	}

	memcpy(pSense,sptwb.ucSenseBuf,24);
	ucResult = sptwb.spt.ScsiStatus;

	return true;
}