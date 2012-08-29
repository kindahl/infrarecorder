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
#include "config_advanced_page.hh"
#include "string_table.hh"
#include "settings.hh"
#include "lang_util.hh"

CConfigAdvancedPage::CConfigAdvancedPage()
{
    // Try to load translated string.
    if (g_LanguageSettings.m_pLngProcessor != NULL)
    {	
        // Make sure that there is a strings translation section.
        if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
        {
            TCHAR *szStrValue;
            if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_ADVANCED,szStrValue))
                SetTitle(szStrValue);
        }
    }

    m_psp.dwFlags |= PSP_HASHELP;
}

CConfigAdvancedPage::~CConfigAdvancedPage()
{
}

bool CConfigAdvancedPage::Translate()
{
    if (g_LanguageSettings.m_pLngProcessor == NULL)
        return false;

    CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
    
    // Make sure that there is a config translation section.
    if (!pLng->EnterSection(_T("config")))
        return false;

    // Translate.
    TCHAR *szStrValue;

    if (pLng->GetValuePtr(IDC_LOGCHECK,szStrValue))
        SetDlgItemText(IDC_LOGCHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_SMOKECHECK,szStrValue))
        SetDlgItemText(IDC_SMOKECHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_FIFOGROUPSTATIC,szStrValue))
        SetDlgItemText(IDC_FIFOGROUPSTATIC,szStrValue);
    if (pLng->GetValuePtr(IDC_FIFOINFOSTATIC,szStrValue))
        SetDlgItemText(IDC_FIFOINFOSTATIC,szStrValue);

    return true;
}

bool CConfigAdvancedPage::OnApply()
{
    TCHAR szFIFO[4];
    GetDlgItemText(IDC_FIFOEDIT,szFIFO,4);
    int iFIFOSize = _wtoi(szFIFO);

    if (iFIFOSize < FIFO_MIN || iFIFOSize > FIFO_MAX)
    {
        TCHAR szMessage[128];
        lsnprintf_s(szMessage,128,lngGetString(ERROR_FIFOSIZE),FIFO_MIN,FIFO_MAX);
        MessageBox(szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
        return false;
    }

    // Remember the configuration.
    g_GlobalSettings.m_bLog = IsDlgButtonChecked(IDC_LOGCHECK) == TRUE;
    g_GlobalSettings.m_bSmoke = IsDlgButtonChecked(IDC_SMOKECHECK) == TRUE;
    g_GlobalSettings.m_iFIFOSize = iFIFOSize;

    return true;
}

void CConfigAdvancedPage::OnHelp()
{
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

    ExtractFilePath(szFileName);
    lstrcat(szFileName,lngGetManual());
    lstrcat(szFileName,_T("::/how_to_use/configuration.html"));

    HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CConfigAdvancedPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    // Load configuration.
    CheckDlgButton(IDC_LOGCHECK,g_GlobalSettings.m_bLog);
    CheckDlgButton(IDC_SMOKECHECK,g_GlobalSettings.m_bSmoke);

    ::SendMessage(GetDlgItem(IDC_FIFOEDIT),EM_SETLIMITTEXT,3,0);
    TCHAR szFIFO[4];
    _itow(g_GlobalSettings.m_iFIFOSize,szFIFO,10);
    SetDlgItemText(IDC_FIFOEDIT,szFIFO);

    // Translate the window.
    Translate();

    return TRUE;
}
