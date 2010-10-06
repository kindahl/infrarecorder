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
#include "WinVer.h"

#ifdef VERSION_COMPATIBILITY_CHECK
CWinVer g_WinVer;

#pragma comment(lib,"Version.lib")

CWinVer::CWinVer()
{
	m_ulMajorVersion = 0;
	m_ulMinorVersion = 0;

	m_ulMajorIEVersion = 0;
	m_ulMinorIEVersion = 0;

	// Get Windows version information.
	OSVERSIONINFO ovInfo;
	ovInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&ovInfo))
	{
		m_ulMajorVersion = ovInfo.dwMajorVersion;
		m_ulMinorVersion = ovInfo.dwMinorVersion;
	}

	// Get Internet Explorer version information.
	VS_FIXEDFILEINFO *wxFileInfo;

	unsigned long ulDummy,ulVersionSize = GetFileVersionInfoSize(_T("shlwapi.dll"),NULL);

	if (ulVersionSize > 0)
	{
		unsigned char *pBuffer = new unsigned char[ulVersionSize];

		if (GetFileVersionInfo(_T("shlwapi.dll"),NULL,ulVersionSize,pBuffer))
		{
			if (VerQueryValue(pBuffer,_T("\\"),(void **)&wxFileInfo,(unsigned int *)&ulDummy))
			{
				m_ulMajorIEVersion = wxFileInfo->dwFileVersionMS/65536;
				m_ulMinorIEVersion = wxFileInfo->dwFileVersionMS%65536;
			}
		}

		delete [] pBuffer;
	}

	// Get Common Controls version information;
	ulVersionSize = GetFileVersionInfoSize(_T("comctl32.dll"),NULL);

	if (ulVersionSize > 0)
	{
		unsigned char *pBuffer = new unsigned char[ulVersionSize];

		if (GetFileVersionInfo(_T("comctl32.dll"),NULL,ulVersionSize,pBuffer))
		{
			if (VerQueryValue(pBuffer,_T("\\"),(void **)&wxFileInfo,(unsigned int *)&ulDummy))
			{
				m_ulMajorCCVersion = wxFileInfo->dwFileVersionMS/65536;
				m_ulMinorCCVersion = wxFileInfo->dwFileVersionMS%65536;
			}
		}

		delete [] pBuffer;
	}
}

CWinVer::~CWinVer()
{
}

#endif
