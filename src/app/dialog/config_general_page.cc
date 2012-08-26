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
#include "string_table.hh"
#include "settings.hh"
#include "lang_util.hh"
#include "registry.hh"
#include "config_general_page.hh"

CConfigGeneralPage::CConfigGeneralPage()
{
    // Try to load translated string.
    if (g_LanguageSettings.m_pLngProcessor != NULL)
    {	
        // Make sure that there is a strings translation section.
        if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
        {
            TCHAR *szStrValue;
            if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_GENERAL,szStrValue))
                SetTitle(szStrValue);
        }
    }

    m_psp.dwFlags |= PSP_HASHELP;
}

CConfigGeneralPage::~CConfigGeneralPage()
{
}

/*
    CConfigGeneralPage::IsFileExtRegistered
    ---------------------------------------
    Returns true if InfraRecorder is associated with the specified file
    extension.
*/
bool CConfigGeneralPage::IsFileExtRegistered(const TCHAR *szFileExt)
{
    CRegistry Reg;
    Reg.SetRoot(HKEY_CLASSES_ROOT);

    if (!Reg.OpenKey(szFileExt,false))
        return false;

    TCHAR szKeyName[64];
    if (Reg.ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)))
    {
        Reg.CloseKey();

        if (lstrcmp(szKeyName,_T("")))
        {
            TCHAR szFullName[256];
            lstrcpy(szFullName,szKeyName),
            lstrcat(szFullName,_T("\\shell\\open\\command"));

            if (!Reg.OpenKey(szFullName,false))
                return false;

            // Compare the file path of this application instance with the file
            // path in the standard registry key.
            TCHAR szFilePath[MAX_PATH];
            lstrcpy(szFilePath,_T("\""));
            GetModuleFileName(NULL,szFilePath + 1,MAX_PATH - 1);
            lstrcat(szFilePath,_T("\""));

            TCHAR szKeyFilePath[MAX_PATH];
            if (!Reg.ReadString(_T(""),szKeyFilePath,MAX_PATH * sizeof(TCHAR)) ||
                lstrncmp(szFilePath,szKeyFilePath,lstrlen(szFilePath)))
            {
                Reg.CloseKey();
                return false;
            }

            Reg.CloseKey();
            return true;
        }
    }

    return false;
}

/*
    CConfigGeneralPage::RegisterFileExt
    -----------------------------------
    Associates InfraRecorder with the specified file extension. Returns true if
    successfull, otherwise false. szTypeKeyName may not contain more than 64
    characters.
*/
bool CConfigGeneralPage::RegisterFileExt(const TCHAR *szFileExt,const TCHAR *szTypeKeyName,
                                         const TCHAR *szTypeDesc)
{
    CRegistry Reg;
    Reg.SetRoot(HKEY_CLASSES_ROOT);

    if (Reg.OpenKey(szFileExt))
    {
        TCHAR szKeyName[64];

        bool bCreate = false;
        if (Reg.ReadString(_T(""),szKeyName,sizeof(szKeyName)) || !lstrcmp(szKeyName,_T("")))
        {
            if (lstrcmp(szKeyName,szTypeKeyName))
                bCreate = true;
        }
        else
        {
            bCreate = true;
        }

        // If no key is specified we create our own.
        //if (!Reg.ReadString(_T(""),szKeyName,sizeof(szKeyName)) || !lstrcmp(szKeyName,_T("")))
        if (bCreate)
        {
            // Extension key name.
            if (!Reg.WriteString(_T(""),(TCHAR *)szTypeKeyName,lstrlen(szTypeKeyName) * sizeof(TCHAR)))
            {
                Reg.CloseKey();
                return false;
            }

            Reg.CloseKey();

            // Key name description.
            if (!Reg.OpenKey(szTypeKeyName,true))
                return false;

            if (!Reg.WriteString(_T(""),(TCHAR *)szTypeDesc,lstrlen(szTypeDesc) * sizeof(TCHAR)))
            {
                Reg.CloseKey();
                return false;
            }
        }

        // Open key name and install shell extension.
        Reg.CloseKey();

        TCHAR szFullName[256];
        lstrcpy(szFullName,/*szKeyName*/szTypeKeyName),
        lstrcat(szFullName,_T("\\shell\\open\\command"));

        if (!Reg.OpenKey(szFullName))
            return false;

        TCHAR szCommand[MAX_PATH];
        lstrcpy(szCommand,_T("\""));
        GetModuleFileName(NULL,szCommand + 1,MAX_PATH - 1);
        lstrcat(szCommand,_T("\" %1"));

        if (!Reg.WriteString(_T(""),szCommand,MAX_PATH * sizeof(TCHAR)))
        {
            Reg.CloseKey();
            return false;
        }

        Reg.CloseKey();

        // Set default icon.
        lstrcpy(szFullName,szKeyName),
        lstrcat(szFullName,_T("\\DefaultIcon"));

        if (!Reg.OpenKey(szFullName))
            return false;

        TCHAR szIconPath[MAX_PATH + 2];
        GetModuleFileName(NULL,szIconPath,MAX_PATH - 1);
        lstrcat(szIconPath,_T(",5"));

        if (!Reg.WriteString(_T(""),szIconPath,MAX_PATH + 2 * sizeof(TCHAR)))
        {
            Reg.CloseKey();
            return false;
        }

        Reg.CloseKey();
        return true;
    }

    return false;
}

/*
    CConfigGeneralPage::UnregisterFileExt
    -------------------------------------
    Deassociate InfraRecorder with the specified file extension. Returns true
    if the process was successfull, false otherwise.
*/
bool CConfigGeneralPage::UnregisterFileExt(const TCHAR *szFileExt)
{
    CRegistry Reg;
    Reg.SetRoot(HKEY_CLASSES_ROOT);

    if (!Reg.OpenKey(szFileExt,false))
        return true;

    TCHAR szKeyName[64];
    if (Reg.ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)))
    {
        Reg.CloseKey();

        if (lstrcmp(szKeyName,_T("")))
        {
            TCHAR szFullName[256];
            lstrcpy(szFullName,szKeyName),
            lstrcat(szFullName,_T("\\shell\\open"));

            if (!Reg.OpenKey(szFullName,false))
                return false;

            // Delete the command key.
            if (!Reg.DeleteKey(_T("command")))
            {
                Reg.CloseKey();
                return false;
            }

            Reg.CloseKey();

            // Delete the open key.
            szFullName[LastDelimiter(szFullName,'\\')] = '\0';

            if (!Reg.OpenKey(szFullName,false))
                return false;

            if (!Reg.DeleteKey(_T("open")))
            {
                Reg.CloseKey();
                return false;
            }

            // Remove the icon.
            if (!Reg.OpenKey(szKeyName,false))
                return false;

            bool bResult = Reg.DeleteKey(_T("DefaultIcon"));
                Reg.CloseKey();
            return bResult;
        }
    }

    return true;
}

bool CConfigGeneralPage::Translate()
{
    if (g_LanguageSettings.m_pLngProcessor == NULL)
        return false;

    CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
    
    // Make sure that there is a config translation section.
    if (!pLng->EnterSection(_T("config")))
        return false;

    // Translate.
    TCHAR *szStrValue;

    if (pLng->GetValuePtr(IDC_AUTORUNCHECK,szStrValue))
        SetDlgItemText(IDC_AUTORUNCHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_WIZARDCHECK,szStrValue))
        SetDlgItemText(IDC_WIZARDCHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_ASSOCIATECHECK,szStrValue))
        SetDlgItemText(IDC_ASSOCIATECHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_ASSOCIATEDISCIMAGECHECK,szStrValue))
        SetDlgItemText(IDC_ASSOCIATEDISCIMAGECHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_SHELLFOLDERGROUPSTATIC,szStrValue))
        SetDlgItemText(IDC_SHELLFOLDERGROUPSTATIC,szStrValue);
    if (pLng->GetValuePtr(IDC_SHELLFOLDERINFOSTATIC,szStrValue))
        SetDlgItemText(IDC_SHELLFOLDERINFOSTATIC,szStrValue);
    if (pLng->GetValuePtr(IDC_REMEMBERSHELLCHECK,szStrValue))
        SetDlgItemText(IDC_REMEMBERSHELLCHECK,szStrValue);
    if (pLng->GetValuePtr(IDC_TEMPFOLDERGROUPSTATIC,szStrValue))
        SetDlgItemText(IDC_TEMPFOLDERGROUPSTATIC,szStrValue);
    if (pLng->GetValuePtr(IDC_TEMPFOLDERINFOSTATIC,szStrValue))
        SetDlgItemText(IDC_TEMPFOLDERINFOSTATIC,szStrValue);

    return true;
}

bool CConfigGeneralPage::OnApply()
{
    // Remember the configuration.
    g_GlobalSettings.m_bAutoRunCheck = IsDlgButtonChecked(IDC_AUTORUNCHECK) == TRUE;
    g_GlobalSettings.m_bShowWizard = IsDlgButtonChecked(IDC_WIZARDCHECK) == TRUE;
    g_GlobalSettings.m_bRememberShell = IsDlgButtonChecked(IDC_REMEMBERSHELLCHECK) == TRUE;

    GetDlgItemText(IDC_SHELLFOLDEREDIT,g_DynamicSettings.m_szShellDir,MAX_PATH - 1);
    GetDlgItemText(IDC_TEMPFOLDEREDIT,g_GlobalSettings.m_szTempPath,MAX_PATH - 1);
    IncludeTrailingBackslash(g_GlobalSettings.m_szTempPath);

    // Register the project file extension.
    if (IsDlgButtonChecked(IDC_ASSOCIATECHECK) == TRUE)
    {
        RegisterFileExt(_T(".irp"),_T("irproject"),_T("InfraRecorder project"));
    }
    else
    {
        if (IsFileExtRegistered(_T(".irp")))
            UnregisterFileExt(_T(".irp"));
    }

    // Register the disc image extensions.
    if (IsDlgButtonChecked(IDC_ASSOCIATEDISCIMAGECHECK) == TRUE)
    {
        RegisterFileExt(_T(".iso"),_T("irdiscimage"),_T("InfraRecorder disc image"));
        RegisterFileExt(_T(".img"),_T("irdiscimage"),_T("InfraRecorder disc image"));
        RegisterFileExt(_T(".cue"),_T("irdiscimage"),_T("InfraRecorder disc image"));
    }
    else
    {
        if (IsFileExtRegistered(_T(".iso")))
            UnregisterFileExt(_T(".iso"));
        if (IsFileExtRegistered(_T(".img")))
            UnregisterFileExt(_T(".img"));
        if (IsFileExtRegistered(_T(".cue")))
            UnregisterFileExt(_T(".cue"));
    }

    return true;
}

void CConfigGeneralPage::OnHelp()
{
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

    ExtractFilePath(szFileName);
    lstrcat(szFileName,lngGetManual());
    lstrcat(szFileName,_T("::/how_to_use/configuration.html"));

    HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CConfigGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    // Load configuration.
    CheckDlgButton(IDC_AUTORUNCHECK,g_GlobalSettings.m_bAutoRunCheck);
    CheckDlgButton(IDC_WIZARDCHECK,g_GlobalSettings.m_bShowWizard);
    CheckDlgButton(IDC_REMEMBERSHELLCHECK,g_GlobalSettings.m_bRememberShell);

    ::SendMessage(GetDlgItem(IDC_SHELLFOLDEREDIT),EM_SETLIMITTEXT,MAX_PATH - 1,0);
    SetDlgItemText(IDC_SHELLFOLDEREDIT,g_DynamicSettings.m_szShellDir);

    CheckDlgButton(IDC_ASSOCIATECHECK,IsFileExtRegistered(_T(".irp")));
    CheckDlgButton(IDC_ASSOCIATEDISCIMAGECHECK,IsFileExtRegistered(_T(".iso")) ||
        IsFileExtRegistered(_T(".img")) || IsFileExtRegistered(_T(".cue")));

    if (g_GlobalSettings.m_bRememberShell)
    {
        ::EnableWindow(GetDlgItem(IDC_SHELLFOLDEREDIT),false);
        ::EnableWindow(GetDlgItem(IDC_SHELLFOLDERBROWSEBUTTON),false);
    }

    ::SendMessage(GetDlgItem(IDC_TEMPFOLDEREDIT),EM_SETLIMITTEXT,MAX_PATH - 1,0);
    SetDlgItemText(IDC_TEMPFOLDEREDIT,g_GlobalSettings.m_szTempPath);

    // Translate the window.
    Translate();

    return TRUE;
}

LRESULT CConfigGeneralPage::OnRememberShellCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
    bool bEnable = !IsDlgButtonChecked(IDC_REMEMBERSHELLCHECK) == TRUE;

    ::EnableWindow(GetDlgItem(IDC_SHELLFOLDEREDIT),bEnable);
    ::EnableWindow(GetDlgItem(IDC_SHELLFOLDERBROWSEBUTTON),bEnable);

    bHandled = false;
    return 0;
}

LRESULT CConfigGeneralPage::OnFolderBrowse(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
    CFolderDialog FolderDialog(m_hWnd,lngGetString(MISC_SPECIFYFOLDER),BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS);
    int iControlID;

    switch (wID)
    {
        case IDC_SHELLFOLDERBROWSEBUTTON:
            iControlID = IDC_SHELLFOLDEREDIT;
            break;

        case IDC_TEMPFOLDERBROWSEBUTTON:
            iControlID = IDC_TEMPFOLDEREDIT;
            break;

        default:
        {
            bHandled = false;
            return 0;
        }
    }

    if (FolderDialog.DoModal() == IDOK)
        SetDlgItemText(iControlID,FolderDialog.GetFolderPath());

    bHandled = false;
    return 0;
}
