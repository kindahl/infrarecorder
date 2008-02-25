/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
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

#include "stdafx.h"
#include "MultiSessionImport.h"
#include "Settings.h"
#include "LogDlg.h"
#include "TreeManager.h"

CMultiSessionImport g_MultiSessionImport;

CMultiSessionImport::CMultiSessionImport()
{
	m_iMode = -1;

	m_pSpaceMeter = NULL;
}

CMultiSessionImport::~CMultiSessionImport()
{
}

void CMultiSessionImport::AssignControls(CSpaceMeter *pSpaceMeter)
{
	m_pSpaceMeter = pSpaceMeter;
}

int CMultiSessionImport::GetMonth(const char *szDateTime)
{
	if (!strncmp(szDateTime,"Jan",3))
		return 1;
	else if (!strncmp(szDateTime,"Feb",3))
		return 2;
	else if (!strncmp(szDateTime,"Mar",3))
		return 3;
	else if (!strncmp(szDateTime,"Apr",3))
		return 4;
	else if (!strncmp(szDateTime,"May",3))
		return 5;
	else if (!strncmp(szDateTime,"Jun",3))
		return 6;
	else if (!strncmp(szDateTime,"Jul",3))
		return 7;
	else if (!strncmp(szDateTime,"Aug",3))
		return 8;
	else if (!strncmp(szDateTime,"Sep",3))
		return 9;
	else if (!strncmp(szDateTime,"Oct",3))
		return 10;
	else if (!strncmp(szDateTime,"Nov",3))
		return 11;
	else
		return 12;

	return 1;
}

void CMultiSessionImport::InfoOutput(const char *szBuffer)
{
	if (sscanf(szBuffer,"%I64d,%I64d",&m_uiLastSession,&m_uiNextSession) != 2)
	{
		m_uiLastSession = 0;
		m_uiNextSession = 0;
	}
}

void CMultiSessionImport::ImportOutput(const char *szBuffer)
{
	// Listing a directory.
	if (!strncmp(szBuffer,"Directory",9))
	{
#ifdef UNICODE
		AnsiToUnicode(m_szCurDir,szBuffer + 21,sizeof(m_szCurDir) / sizeof(wchar_t));
#else
		strcpy(m_szCurDir,szBuffer + 21);
#endif
	}
	else
	{
		// Is it a folder?
		bool bIsFolder = szBuffer[0] == 'd';

		// File size.
		unsigned __int64 uiFileSize = _atoi64(szBuffer + 30);
		
		// Time and date.
		char szDateTime[32];
		strncpy(szDateTime,szBuffer + 41 - 5,11); // RSS:080105 - added offset -5
		szDateTime[11] = '\0';

		// File name.
		TCHAR szFileName[MAX_PATH];
#ifdef UNICODE
		AnsiToUnicode(szFileName,szBuffer + 61,sizeof(szFileName) / sizeof(wchar_t));
#else
		strcpy(szFileName,szBuffer + 61);
#endif

		// If it's a file we want to remove the "version information" separated by a semicolon.
		if (bIsFolder)
		{
			unsigned int uiLastChar = lstrlen(szFileName) - 1;

			if (szFileName[uiLastChar] == ' ')
				szFileName[uiLastChar] = '\0';
		}
		else
		{
			// UPDATE: 2006-08-09
			int iLastDelim = LastDelimiter(szFileName,';');
			if (iLastDelim > 0)
				szFileName[iLastDelim] = '\0';
		}

		// Ignore the "." and ".." entries.
		if (szFileName[0] == '.')
		{
			if (szFileName[1] == '.')
			{
				if (szFileName[2] == '\0')
					return;
			}
			else if (szFileName[1] == '\0')
				return;
		}

		// Decode the file date.
		unsigned short usFileDate = (atoi(szDateTime + 7) - 1980) << 9;
		usFileDate |= GetMonth(szDateTime) << 5;
		usFileDate |= atoi(szDateTime + 4);

		if (bIsFolder)
		{
			TCHAR szFullName[MAX_PATH];
			lstrcpy(szFullName,m_szCurDir);
			lstrcat(szFullName,szFileName);
			lstrcat(szFullName,_T("/"));

			CProjectNode *pNode = g_TreeManager.AddPath(szFullName);
			pNode->pItemData->ucFlags |= PROJECTITEM_FLAG_ISLOCKED | PROJECTITEM_FLAG_ISIMPORTED;
			pNode->pItemData->usFileDate = usFileDate;
		}
		else
		{
			CItemData *pItemData = new CItemData();
			pItemData->ucFlags = PROJECTITEM_FLAG_ISLOCKED | PROJECTITEM_FLAG_ISIMPORTED;

			// Paths.
			pItemData->SetFileName(szFileName);
			pItemData->SetFilePath(m_szCurDir);

			TCHAR szFullName[MAX_PATH];
			lstrcpy(szFullName,m_szCurDir);
			lstrcat(szFullName,szFileName);

			// File type.
			SHFILEINFO shFileInfo;
			if (SHGetFileInfo(pItemData->GetFileName(),FILE_ATTRIBUTE_NORMAL,&shFileInfo,
				sizeof(shFileInfo),SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
			{
				lstrcpy(pItemData->szFileType,shFileInfo.szTypeName);
			}

			CProjectNode *pCurrentNode;

			if (pItemData->GetFilePath()[1] != '\0')	// pData->szFilePath[1] == '\0' => pData->szFilePath = "\\"
				pCurrentNode = g_TreeManager.AddPath(szFullName);
			else
				pCurrentNode = g_TreeManager.GetRootNode();

			// Modified time.
			pItemData->usFileDate = usFileDate;

			// Size.
			pItemData->uiSize = uiFileSize;

			pCurrentNode->m_Files.push_back(pItemData);
		}
	}
}

void CMultiSessionImport::FlushOutput(const char *szBuffer)
{
	switch (m_iMode)
	{
		case MODE_INFO:
			InfoOutput(szBuffer);
			break;

		case MODE_IMPORT:
			ImportOutput(szBuffer);
			break;
	}
}

void CMultiSessionImport::ProcessEnded()
{
	switch (m_iMode)
	{
		case MODE_INFO:
			break;

		case MODE_IMPORT:
			// Update the space meter. We do not update the allocated size here since
			// counting from the tree will give an incorrect size.
			if (m_pSpaceMeter != NULL)
				m_pSpaceMeter->ForceRedraw();

			// Update the list view.
			g_TreeManager.Refresh();
			break;
	}
}

bool CMultiSessionImport::GetInfo(tDeviceInfo *pDeviceInfo,unsigned __int64 &uiLastSession,
								  unsigned __int64 &uiNextSession)
{
	m_iMode = MODE_INFO;

	TCHAR szBuffer[128];

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CMultiSessImport::GetInfo"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,
			pDeviceInfo->Address.m_iTarget,pDeviceInfo->Address.m_iLun,
			pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
	}

	TCHAR szCommandLine[MAX_PATH + 21];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T("cdrecord.exe\" -msinfo dev="));

	// Device address.
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	lstrcat(szCommandLine,szBuffer);

	if (Launch(szCommandLine,true))
	{
		uiLastSession = m_uiLastSession;
		uiNextSession = m_uiNextSession;
		return true;
	}

	return false;
}

bool CMultiSessionImport::Import(tDeviceInfo *pDeviceInfo,unsigned __int64 uiSessionStart)
{
	m_iMode = MODE_IMPORT;

	TCHAR szBuffer[128];

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CMultiSessImport::Import"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,
			pDeviceInfo->Address.m_iTarget,pDeviceInfo->Address.m_iLun,
			pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Session start = %I64d."),uiSessionStart);
	}

	TCHAR szCommandLine[MAX_PATH + 32];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T("isoinfo.exe\" -l -J dev="));

	// Device address.
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	lstrcat(szCommandLine,szBuffer);

	lsprintf(szBuffer,_T(" -T %I64d"),uiSessionStart);
	lstrcat(szCommandLine,szBuffer);

	// RSS:080105 - addded codepage for isoinfo
	// isoinfo requires strict codepage for non latin symbols
	// 
	// Added automatic codepage detection and manual selection from 
	// section [isoinfo] in language file with key 0x0001=codepage
	// E.g.:
	//   [isoinfo]
	//    0x0001=cp1251
	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;	
	// Config codepage for isoinfo
	if (pLNG && pLNG->EnterSection(_T("isoinfo")))
	{
		TCHAR *szStrValue;
		if (pLNG->GetValuePtr(1,szStrValue))
		{
			lstrcat(szCommandLine,_T(" -j "));
			lstrcat(szCommandLine,szStrValue);
		}
	}
	else
	{
		int iCurCharSet = CodePageToCharacterSet(GetACP());
		if (iCurCharSet > 0)
		{
			lstrcat(szCommandLine,_T(" -j "));
			lstrcat(szCommandLine,g_szCharacterSets[iCurCharSet]);
		}
	}

	return Launch(szCommandLine);
}
