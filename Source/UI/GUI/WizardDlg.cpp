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
#include "CustomButton.h"
#include "ProjectManager.h"
#include "Settings.h"
#include "ActionManager.h"
#include "MainFrm.h"
#include "WizardDlg.h"

CWizardDlg::CWizardDlg() : m_DataButton(IDC_DATABUTTON,IDC_DATACDBUTTON,IDC_DATADVDBUTTON,
										IDB_NORMALDATADISCBITMAP,
										IDB_HOVERDATADISCBITMAP,
										IDB_HOVERDATADISCCDBITMAP,
										IDB_HOVERDATADISCDVDBITMAP,
										IDB_FOCUSDATADISCBITMAP),
						   m_AudioButton(IDB_NORMALAUDIODISCBITMAP,
										 IDB_HOVERAUDIODISCBITMAP,
										 IDB_FOCUSAUDIODISCBITMAP),
						   m_VideoButton(IDB_NORMALVIDEODISCBITMAP,
										 IDB_HOVERVIDEODISCBITMAP,
										 IDB_FOCUSVIDEODISCBITMAP),
						   m_ImageButton(IDB_NORMALWRITEIMAGEBITMAP,
										 IDB_HOVERWRITEIMAGEBITMAP,
										 IDB_FOCUSWRITEIMAGEBITMAP),
						   m_CopyButton(IDB_NORMALCOPYDISCBITMAP,
										IDB_HOVERCOPYDISCBITMAP,
										IDB_FOCUSCOPYDISCBITMAP),
						   m_ReadButton(IDB_NORMALREADDISCBITMAP,
										IDB_HOVERREADDISCBITMAP,
										IDB_FOCUSREADDISCBITMAP),
						   m_GradientStatic(RGB(255,255,255))
{
}

CWizardDlg::~CWizardDlg()
{
}

bool CWizardDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a config translation section.
	if (!pLNG->EnterSection(_T("wizard")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDD_WIZARDDLG,szStrValue))		// Title.
		SetWindowText(szStrValue);
	if (pLNG->GetValuePtr(IDCANCEL,szStrValue))
		SetDlgItemText(IDCANCEL,szStrValue);
	if (pLNG->GetValuePtr(IDC_STARTUPCHECK,szStrValue))
		SetDlgItemText(IDC_STARTUPCHECK,szStrValue);

	return true;
}

LRESULT CWizardDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	m_DataButton.SubclassWindow(GetDlgItem(IDC_DATABUTTON));
	m_AudioButton.SubclassWindow(GetDlgItem(IDC_AUDIOBUTTON));
	m_VideoButton.SubclassWindow(GetDlgItem(IDC_VIDEOBUTTON));
	m_ImageButton.SubclassWindow(GetDlgItem(IDC_IMAGEBUTTON));
	m_CopyButton.SubclassWindow(GetDlgItem(IDC_COPYBUTTON));
	m_ReadButton.SubclassWindow(GetDlgItem(IDC_READBUTTON));

	m_GradientStatic.SubclassWindow(GetDlgItem(IDC_GRADIENTSTATIC));

	CheckDlgButton(IDC_STARTUPCHECK,g_GlobalSettings.m_bShowWizard);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CWizardDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

LRESULT CWizardDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

LRESULT CWizardDlg::OnAction(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);

	switch (wID)
	{
		case IDC_DATABUTTON:
			g_ProjectManager.NewDataProject(false);
			break;

		case IDC_DATACDBUTTON:
			g_ProjectManager.NewDataProject(false);
			break;

		case IDC_DATADVDBUTTON:
			g_ProjectManager.NewDataProject(true);
			break;

		case IDC_AUDIOBUTTON:
			g_ProjectManager.NewAudioProject();
			break;

		case IDC_VIDEOBUTTON:
			g_ProjectManager.NewDataProject(true);
			g_ProjectSettings.m_iFileSystem = FILESYSTEM_DVDVIDEO;
			break;

		case IDC_IMAGEBUTTON:
			g_ActionManager.BurnImage(g_MainFrame,false);
			break;

		case IDC_COPYBUTTON:
			g_ActionManager.CopyDisc(g_MainFrame,false);
			break;

		case IDC_READBUTTON:
			g_ActionManager.CopyImage(g_MainFrame,false);
			break;
	}
	
	return FALSE;
}

LRESULT CWizardDlg::OnStartupCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	g_GlobalSettings.m_bShowWizard = IsDlgButtonChecked(IDC_STARTUPCHECK) == TRUE;
	return 0;
}
