/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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
#include "Resource.h"
#include "settings.hh"

class CAddBootImageDlg : public CDialogImpl<CAddBootImageDlg>
{
private:
	bool m_bEdit;
	CProjectBootImage *m_pBootImage;
	CComboBox m_EmuCombo;

	enum
	{
		MAX_BOOTLOAD_SIZE = 32
	};

	bool Translate();

public:
	enum { IDD = IDD_ADDBOOTIMAGEDLG };

	CAddBootImageDlg(CProjectBootImage *pBootImage,bool bEdit);
	~CAddBootImageDlg();

	BEGIN_MSG_MAP(CAddBootImageDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_ID_HANDLER(IDC_HELPBUTTON,OnHelp)
		COMMAND_HANDLER(IDC_EMULATIONCOMBO,CBN_SELCHANGE,OnEmuChange)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnEmuChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
