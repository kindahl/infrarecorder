/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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

#include "stdafx.hh"
#include <ckcore/file.hh>
#include "settings.hh"

CLanguageSettings g_LanguageSettings;
CGlobalSettings g_GlobalSettings;

bool CLanguageSettings::Load(CXmlProcessor *pXml)
{
    if (pXml == NULL)
        return false;

    if (!pXml->EnterElement(_T("Language")))
        return false;

    pXml->GetSafeElementData(_T("LanguageFile"),m_szLanguageFile,MAX_PATH - 1);

    // Calculate full path.
    TCHAR szFullPath[MAX_PATH];
    ::GetModuleFileName(NULL,szFullPath,MAX_PATH - 1);
    ExtractFilePath(szFullPath);
    lstrcat(szFullPath,_T("Languages\\"));
    lstrcat(szFullPath,m_szLanguageFile);

    if (ckcore::File::exist(szFullPath))
    {
        m_pLngProcessor = new CLngProcessor(szFullPath);
        m_pLngProcessor->Load();
    }

    pXml->LeaveElement();
    return true;
}

bool CGlobalSettings::Load(CXmlProcessor *pXml)
{
    if (pXml == NULL)
        return false;

    if (!pXml->EnterElement(_T("Global")))
        return false;

    // Shell extension.
    if (pXml->EnterElement(_T("ShellExtension")))
    {
        pXml->GetSafeElementAttrValue(_T("submenu"),&m_bShellExtSubMenu);
        pXml->GetSafeElementAttrValue(_T("icons"),&m_bShellExtIcon);

        pXml->LeaveElement();
    }

    pXml->LeaveElement();
    return true;
}
