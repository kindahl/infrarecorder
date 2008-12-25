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
#include "../../Common/StringUtil.h"
#include "cdrtoolsParseStrings.h"
#include "LogDlg.h"
#include "Settings.h"
#include "Core.h"
#include "Diagnostics.h"

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

	g_LogDlg.print(_T("   > "));

#ifdef UNICODE
		TCHAR szWideBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
		AnsiToUnicode(szWideBuffer,szBuffer,sizeof(szWideBuffer) / sizeof(wchar_t));
		g_LogDlg.print_line(szWideBuffer);
#else
		g_LogDlg.print_line(szBuffer);
#endif
}

void CDiagnostics::ProcessEnded()
{
	g_LogDlg.print_line(_T("CDiagnostics::ProcessEnded"));
	g_LogDlg.print_line(_T(""));
}

bool CDiagnostics::DeviceScan()
{
	g_LogDlg.print_line(_T("CDiagnostics::DeviceScan"));

	TCHAR szCommandLine[MAX_PATH];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T(CORE_WRITEAPP));
#ifdef CDRKIT
	lstrcat(szCommandLine,_T("\" -devices"));
#else
	lstrcat(szCommandLine,_T("\" -scanbus"));
#endif

	if (!Launch(szCommandLine,false))
		return false;

	return true;
}
