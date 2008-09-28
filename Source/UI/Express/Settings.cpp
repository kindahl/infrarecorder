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
#include "Settings.h"
#include "../../Common/FileManager.h"

CLanguageSettings g_LanguageSettings;

bool CLanguageSettings::Save(CXMLProcessor *pXML)
{
	return true;
}

bool CLanguageSettings::Load(CXMLProcessor *pXML)
{
	if (pXML == NULL)
		return false;

	if (!pXML->EnterElement(_T("Language")))
		return false;

	pXML->GetSafeElementData(_T("LanguageFile"),m_szLanguageFile,MAX_PATH - 1);

	// Calculate full path.
	TCHAR szFullPath[MAX_PATH];
	::GetModuleFileName(NULL,szFullPath,MAX_PATH - 1);
	ExtractFilePath(szFullPath);
	lstrcat(szFullPath,_T("Languages\\"));
	lstrcat(szFullPath,m_szLanguageFile);

	if (fs_fileexists(szFullPath))
	{
		m_pLNGProcessor = new CLNGProcessor();
		m_pLNGProcessor->Load(szFullPath);
	}

	pXML->LeaveElement();
	return true;
}
