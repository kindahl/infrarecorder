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
#include "../../Common/StringUtil.h"
#include "Diagnostics.h"
#include "LogDlg.h"
#include "Settings.h"
#include "cdrtoolsParseStrings.h"

CDiagnostics g_Diagnostics;

CDiagnostics::CDiagnostics()
{
}

CDiagnostics::~CDiagnostics()
{
}

void CDiagnostics::FlushOutput(const char *szBuffer)
{
	// Always skip the copyright line.
	if (!strncmp(szBuffer,CDRTOOLS_COPYRIGHT,CDRTOOLS_COPYRIGHT_LENGTH))
		return;

	g_LogDlg.AddString(_T("   > "));

#ifdef UNICODE
		TCHAR szWideBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
		AnsiToUnicode(szWideBuffer,szBuffer,sizeof(szWideBuffer) / sizeof(wchar_t));
		g_LogDlg.AddLine(szWideBuffer);
#else
		g_LogDlg.AddLine(szBuffer);
#endif
}

void CDiagnostics::ProcessEnded()
{
	g_LogDlg.AddLine(_T("CDiagnostics::ProcessEnded"));
	g_LogDlg.AddLine(_T(""));
}

bool CDiagnostics::DeviceScan()
{
	g_LogDlg.AddLine(_T("CDiagnostics::DeviceScan"));

	TCHAR szCommandLine[MAX_PATH];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T("cdrecord.exe\" -scanbus"));

	if (!Launch(szCommandLine,false))
		return false;

	return true;
}
