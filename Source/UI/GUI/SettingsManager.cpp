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
#include <ckcore/directory.hh>
#include "SettingsManager.h"
#include "../../Common/StringUtil.h"
#include "../../Common/XmlProcessor.h"
#include "StringTable.h"
#include "LangUtil.h"

CSettingsManager g_SettingsManager;

CSettingsManager::CSettingsManager()
{
	RegisterObject(&g_LanguageSettings);
	RegisterObject(&g_GlobalSettings);
	RegisterObject(&g_DynamicSettings);
	RegisterObject(&g_EraseSettings);
	RegisterObject(&g_FixateSettings);
	RegisterObject(&g_BurnImageSettings);
	RegisterObject(&g_CopyDiscSettings);
	RegisterObject(&g_BurnAdvancedSettings);
	RegisterObject(&g_SaveTracksSettings);
	RegisterObject(&g_ReadSettings);
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
	ckcore::Directory::create(szConfigPath);
#endif

	lstrcat(szConfigPath,_T("Settings.xml"));
	return true;
}

bool CSettingsManager::Save()
{
	CXmlProcessor Xml;
	bool bResult = true;

	Xml.AddElement(_T("InfraRecorder"),_T(""),true);
		Xml.AddElement(_T("Settings"),_T(""),true);
			for (unsigned int i = 0; i < m_Settings.size(); i++)
			{
				if (!m_Settings[i]->Save(&Xml))
					bResult = false;
			}
		Xml.LeaveElement();
	Xml.LeaveElement();

	// Get the correct file-path.
	TCHAR szConfigPath[MAX_PATH];
	if (!GetConfigPath(szConfigPath))
		return false;

	return bResult && Xml.Save(szConfigPath);
}

bool CSettingsManager::Load()
{
	CXmlProcessor Xml;
	bool bResult = true;

	// Get the correct file-path.
	TCHAR szConfigPath[MAX_PATH];
	if (!GetConfigPath(szConfigPath))
		return false;

	// Load the file.
	int iResult = Xml.Load(szConfigPath);
	if (iResult != XMLRES_OK && iResult != XMLRES_FILEERROR)
	{
		TCHAR szMessage[128];
		lsnprintf_s(szMessage,128,lngGetString(ERROR_LOADSETTINGS),iResult);

		MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_WARNING),MB_OK | MB_ICONWARNING);
		return false;
	}

	if (!Xml.EnterElement(_T("InfraRecorder")))
		return false;

	if (!Xml.EnterElement(_T("Settings")))
		return false;

	for (unsigned int i = 0; i < m_Settings.size(); i++)
	{
		if (!m_Settings[i]->Load(&Xml))
			bResult = false;
	}

	Xml.LeaveElement();
	Xml.LeaveElement();

	return bResult;
}
