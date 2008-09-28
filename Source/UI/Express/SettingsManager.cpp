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
#include "SettingsManager.h"
#include "../../Common/StringUtil.h"
#include "../../Common/XMLProcessor.h"
#include "../../Common/FileManager.h"
#include "StringTable.h"
#include "LangUtil.h"

CSettingsManager g_SettingsManager;

CSettingsManager::CSettingsManager()
{
	RegisterObject(&g_LanguageSettings);
}

CSettingsManager::~CSettingsManager()
{
	m_Settings.clear();
}

void CSettingsManager::RegisterObject(ISettings *pSettings)
{
	m_Settings.push_back(pSettings);
}

bool CSettingsManager::GetConfigPath(TCHAR *szConfigPath)
{
#ifdef PORTABLE
    GetModuleFileName(NULL,szConfigPath,MAX_PATH - 1);
	ExtractFilePath(szConfigPath);
#else
#ifdef UNICODE
	if (!SUCCEEDED(SHGetFolderPath(HWND_DESKTOP,CSIDL_APPDATA | CSIDL_FLAG_CREATE,NULL,
		SHGFP_TYPE_CURRENT,szConfigPath)))
		return false;
#else	// Win 9x.
	if (!SUCCEEDED(SHGetSpecialFolderPath(HWND_DESKTOP,szConfigPath,CSIDL_APPDATA,true)))
		return false;
#endif
	IncludeTrailingBackslash(szConfigPath);
	lstrcat(szConfigPath,_T("InfraRecorder\\"));

	// Create the file path if it doesn't exist.
	fs_createpath(szConfigPath);
#endif

	lstrcat(szConfigPath,_T("Settings.xml"));
	return true;
}

bool CSettingsManager::Load()
{
	CXMLProcessor XML;
	bool bResult = true;

	// Get the correct file-path.
	TCHAR szConfigPath[MAX_PATH];
    if (!GetConfigPath(szConfigPath))
		return false;

	// Load the file.
	int iResult = XML.Load(szConfigPath);
	if (iResult != XMLRES_OK && iResult != XMLRES_FILEERROR)
	{
		TCHAR szMessage[128];
		lsprintf(szMessage,lngGetString(ERROR_LOADSETTINGS),iResult);

		MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_WARNING),MB_OK | MB_ICONWARNING);
		return false;
	}

	if (!XML.EnterElement(_T("InfraRecorder")))
		return false;

	if (!XML.EnterElement(_T("Settings")))
		return false;

	for (unsigned int i = 0; i < m_Settings.size(); i++)
	{
		if (!m_Settings[i]->Load(&XML))
			bResult = false;
	}

	XML.LeaveElement();
	XML.LeaveElement();

	return bResult;
}
