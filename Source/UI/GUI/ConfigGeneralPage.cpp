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
#include "ConfigGeneralPage.h"
#include "StringTable.h"
#include "Settings.h"
#include "LangUtil.h"
#include "Registry.h"

CConfigGeneralPage::CConfigGeneralPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_GENERAL,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CConfigGeneralPage::~CConfigGeneralPage()
{
}

/*
	CConfigGeneralPage::IsProjectExtRegistered
	------------------------------------------
	Returns true if InfraRecorder is associated with .irp files.
*/
bool CConfigGeneralPage::IsProjectExtRegistered()
{
	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	if (!Reg.OpenKey(_T(".irp"),false))
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
	CConfigGeneralPage::RegisterProjectExt
	--------------------------------------
	Associate InfraRecorder with .irp files. Returns true if successfull,
	otherwise false.
*/
bool CConfigGeneralPage::RegisterProjectExt()
{
	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	if (Reg.OpenKey(_T(".irp")))
	{
		TCHAR szKeyName[64];

		// If no key is specified we create our own.
		if (!Reg.ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)) || !lstrcmp(szKeyName,_T("")))
		{
			// Extension key name.
			lstrcpy(szKeyName,_T("irproject"));

			if (!Reg.WriteString(_T(""),szKeyName,64 * sizeof(TCHAR)))
			{
				Reg.CloseKey();
				return false;
			}

			Reg.CloseKey();

			// Key name description.
			if (!Reg.OpenKey(_T("irproject"),true))
				return false;

			if (!Reg.WriteString(_T(""),_T("InfraRecorder project"),25 * sizeof(TCHAR)))
			{
				Reg.CloseKey();
				return false;
			}
		}

		// Open key name and install shell extension.
		Reg.CloseKey();

		TCHAR szFullName[256];
		lstrcpy(szFullName,szKeyName),
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
		lstrcat(szIconPath,_T(",4"));

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
	CConfigGeneralPage::UnregisterProjectExt
	----------------------------------------
	Deassociate InfraRecorder with .irp files. Returns true if the process
	was successfull, false otherwise.
*/
bool CConfigGeneralPage::UnregisterProjectExt()
{
	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	if (!Reg.OpenKey(_T(".irp"),false))
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
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a config translation section.
	if (!pLNG->EnterSection(_T("config")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDC_AUTORUNCHECK,szStrValue))
		SetDlgItemText(IDC_AUTORUNCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_ASSOCIATECHECK,szStrValue))
		SetDlgItemText(IDC_ASSOCIATECHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_SHELLFOLDERGROUPSTATIC,szStrValue))
		SetDlgItemText(IDC_SHELLFOLDERGROUPSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_SHELLFOLDERINFOSTATIC,szStrValue))
		SetDlgItemText(IDC_SHELLFOLDERINFOSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_REMEMBERSHELLCHECK,szStrValue))
		SetDlgItemText(IDC_REMEMBERSHELLCHECK,szStrValue);
	if (pLNG->GetValuePtr(IDC_TEMPFOLDERGROUPSTATIC,szStrValue))
		SetDlgItemText(IDC_TEMPFOLDERGROUPSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_TEMPFOLDERINFOSTATIC,szStrValue))
		SetDlgItemText(IDC_TEMPFOLDERINFOSTATIC,szStrValue);

	return true;
}

bool CConfigGeneralPage::OnApply()
{
	// Remember the configuration.
	g_GlobalSettings.m_bAutoRunCheck = IsDlgButtonChecked(IDC_AUTORUNCHECK) == TRUE;
	g_GlobalSettings.m_bRememberShell = IsDlgButtonChecked(IDC_REMEMBERSHELLCHECK) == TRUE;

	GetDlgItemText(IDC_SHELLFOLDEREDIT,g_DynamicSettings.m_szShellDir,MAX_PATH - 1);
	GetDlgItemText(IDC_TEMPFOLDEREDIT,g_GlobalSettings.m_szTempPath,MAX_PATH - 1);
	IncludeTrailingBackslash(g_GlobalSettings.m_szTempPath);

	// Register the project file extension.
	if (IsDlgButtonChecked(IDC_ASSOCIATECHECK) == TRUE)
		RegisterProjectExt();
	else
	{
		if (IsProjectExtRegistered())
			UnregisterProjectExt();
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
	CheckDlgButton(IDC_REMEMBERSHELLCHECK,g_GlobalSettings.m_bRememberShell);

	::SendMessage(GetDlgItem(IDC_SHELLFOLDEREDIT),EM_SETLIMITTEXT,MAX_PATH - 1,0);
	SetDlgItemText(IDC_SHELLFOLDEREDIT,g_DynamicSettings.m_szShellDir);

	CheckDlgButton(IDC_ASSOCIATECHECK,IsProjectExtRegistered());

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
	}

	if (FolderDialog.DoModal() == IDOK)
		SetDlgItemText(iControlID,FolderDialog.GetFolderPath());

	bHandled = false;
	return 0;
}
