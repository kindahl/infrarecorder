/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include "add_boot_image_dlg.hh"
#include "string_table.hh"
#include "lang_util.hh"

/*
	CAddBootImageDlg::CAddBootImageDlg
	----------------------------------
	bEdit detemines if the title should state than a boot image is beeing
	edited or if we're adding a new boot image to the project. It also determines
	if the path edit should be enabled.
*/
CAddBootImageDlg::CAddBootImageDlg(CProjectBootImage *pBootImage,bool bEdit)
{
	m_bEdit = bEdit;
	m_pBootImage = pBootImage;
}

CAddBootImageDlg::~CAddBootImageDlg()
{
}

bool CAddBootImageDlg::Translate()
{
	// Set the title.
	if (m_bEdit)
		SetWindowText(lngGetString(TITLE_EDITBOOTIMAGE));

	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a addbootimage translation section.
	if (!pLng->EnterSection(_T("addbootimage")))
		return false;

	TCHAR *szStrValue;

	// Set the title.
	if (!m_bEdit)
	{
		if (pLng->GetValuePtr(IDD_ADDBOOTIMAGEDLG,szStrValue))
			SetWindowText(szStrValue);
	}

	// Translate the rest.
	if (pLng->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLng->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLng->GetValuePtr(IDC_HELPBUTTON,szStrValue))
		SetDlgItemText(IDC_HELPBUTTON,szStrValue);
	if (pLng->GetValuePtr(IDC_EMULATIONSTATIC,szStrValue))
		SetDlgItemText(IDC_EMULATIONSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_OPTIONSSTATIC,szStrValue))
		SetDlgItemText(IDC_OPTIONSSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_NOBOOTCHECK,szStrValue))
		SetDlgItemText(IDC_NOBOOTCHECK,szStrValue);
	if (pLng->GetValuePtr(IDC_BOOTSEGMENTSTATIC,szStrValue))
		SetDlgItemText(IDC_BOOTSEGMENTSTATIC,szStrValue);
	if (pLng->GetValuePtr(IDC_BOOTSIZESTATIC,szStrValue))
		SetDlgItemText(IDC_BOOTSIZESTATIC,szStrValue);

	return true;
}

LRESULT CAddBootImageDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// Set edit field length limit.
	::SendMessage(GetDlgItem(IDC_BOOTSEGMENTEDIT),EM_SETLIMITTEXT,MAX_BOOTLOAD_SIZE - 1,0);
	::SendMessage(GetDlgItem(IDC_BOOTSIZEEDIT),EM_SETLIMITTEXT,MAX_BOOTLOAD_SIZE - 1,0);

	// Initialize the emulation combo box.
	m_EmuCombo = GetDlgItem(IDC_EMULATIONCOMBO);
	m_EmuCombo.AddString(lngGetString(BOOTEMU_NONE));
	m_EmuCombo.AddString(lngGetString(BOOTEMU_FLOPPY));
	m_EmuCombo.AddString(lngGetString(BOOTEMU_HARDDISK));
	m_EmuCombo.SetCurSel(m_pBootImage->m_iEmulation);

	BOOL bDummy;
	OnEmuChange(NULL,NULL,NULL,bDummy);

	// Setup the default settings.
	CheckDlgButton(IDC_NOBOOTCHECK,m_pBootImage->m_bNoBoot);

	TCHAR szBuffer[32];
	lsnprintf_s(szBuffer,32,_T("0x%x"),m_pBootImage->m_uiLoadSegment);
	SetDlgItemText(IDC_BOOTSEGMENTEDIT,szBuffer);

	lsnprintf_s(szBuffer,32,_T("0x%x"),m_pBootImage->m_uiLoadSize);
	SetDlgItemText(IDC_BOOTSIZEEDIT,szBuffer);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CAddBootImageDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Copy all information over to the CProjectBootImage object.
	m_pBootImage->m_iEmulation = m_EmuCombo.GetCurSel();
	m_pBootImage->m_bNoBoot = IsDlgButtonChecked(IDC_NOBOOTCHECK) == TRUE;

	TCHAR szBuffer[MAX_BOOTLOAD_SIZE];
	GetDlgItemText(IDC_BOOTSEGMENTEDIT,szBuffer,MAX_BOOTLOAD_SIZE - 1);
	lsscanf(szBuffer,_T("0x%x"),&m_pBootImage->m_uiLoadSegment);

	GetDlgItemText(IDC_BOOTSIZEEDIT,szBuffer,MAX_BOOTLOAD_SIZE - 1);
	lsscanf(szBuffer,_T("0x%x"),&m_pBootImage->m_uiLoadSize);

	TCHAR szTemp[32];
	lsprintf(szTemp,_T("%d %d"),m_pBootImage->m_uiLoadSegment,m_pBootImage->m_uiLoadSize);

	EndDialog(wID);
	return FALSE;
}

LRESULT CAddBootImageDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

LRESULT CAddBootImageDlg::OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/add_boot_image.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
	return 0;
}

LRESULT CAddBootImageDlg::OnEmuChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (m_EmuCombo.GetCurSel() == PROJECTBI_BOOTEMU_NONE)
	{
		::EnableWindow(GetDlgItem(IDC_NOBOOTCHECK),FALSE);

		::EnableWindow(GetDlgItem(IDC_BOOTSEGMENTSTATIC),TRUE);
		::EnableWindow(GetDlgItem(IDC_BOOTSEGMENTEDIT),TRUE);
		::EnableWindow(GetDlgItem(IDC_BOOTSIZESTATIC),TRUE);
		::EnableWindow(GetDlgItem(IDC_BOOTSIZEEDIT),TRUE);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_NOBOOTCHECK),TRUE);

		::EnableWindow(GetDlgItem(IDC_BOOTSEGMENTSTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BOOTSEGMENTEDIT),FALSE);
		::EnableWindow(GetDlgItem(IDC_BOOTSIZESTATIC),FALSE);
		::EnableWindow(GetDlgItem(IDC_BOOTSIZEEDIT),FALSE);
	}

	return 0;
}
