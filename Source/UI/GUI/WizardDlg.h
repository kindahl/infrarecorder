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

#pragma once
#include "resource.h"
#include "CustomButton.h"
#include "CustomMultiButton.h"
#include "GradientStatic.h"

class CWizardDlg : public CDialogImpl<CWizardDlg>
{
private:
	CCustomMultiButton m_DataButton;
	CCustomButton m_AudioButton;
	CCustomButton m_VideoButton;
	CCustomButton m_ImageButton;
	CCustomButton m_CopyButton;
	CCustomButton m_ReadButton;

	CGradientStatic m_GradientStatic;

	bool Translate();

public:
	enum { IDD = IDD_WIZARDDLG };

	CWizardDlg();
	~CWizardDlg();

	BEGIN_MSG_MAP(CWizardDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_ID_HANDLER(IDC_DATABUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_DATACDBUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_DATADVDBUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_AUDIOBUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_VIDEOBUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_IMAGEBUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_COPYBUTTON,OnAction)
		COMMAND_ID_HANDLER(IDC_READBUTTON,OnAction)

		COMMAND_HANDLER(IDC_STARTUPCHECK, BN_CLICKED, OnStartupCheck)

		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnAction(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnStartupCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
