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

#pragma once
#include "resource.h"
#include "InfraRecorder.h"

class CSaveTracksDlg : public CDialogImpl<CSaveTracksDlg>
{
private:
	CComboBox m_AudioFormatCombo;
	CCodec *m_pEncoder;

	bool Translate();

public:
	enum { IDD = IDD_SAVETRACKSDLG };

	CSaveTracksDlg();
	~CSaveTracksDlg();

	BEGIN_MSG_MAP(CSaveTracksDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_HANDLER(IDC_AUDIOFORMATCOMBO,CBN_SELCHANGE,OnAudioFormatChange)
		COMMAND_HANDLER(IDC_AUDIOFORMATBUTTON,BN_CLICKED,OnClickedAudioFormatButton)
		COMMAND_HANDLER(IDC_BROWSEBUTTON,BN_CLICKED,OnClickedBrowseButton)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnAudioFormatChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnClickedAudioFormatButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnClickedBrowseButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	CCodec *GetEncoder();
};
