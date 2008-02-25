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
#include "../../Common/XMLProcessor.h"

class CMainDlg : public CDialogImpl<CMainDlg>,public CUpdateUI<CMainDlg>,
	public CMessageFilter,public CIdleHandler,public CDialogResize<CMainDlg>
{
private:
	CListBox m_TransList;

	bool AnalyzeTranslation(const TCHAR *szFileName,CXMLProcessor *pXML);

public:
	enum { IDD = IDD_MAINDLG };

	void CloseDialog(int iVal);

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_DLGRESIZE_MAP(CMainDlg)
		DLGRESIZE_CONTROL(IDOK,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_REFBUTTON,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TRANSADDBUTTON,DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_TRANSREMOVEBUTTON,DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_TRANSCLEARBUTTON,DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_REFEDIT,DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRANSLIST,DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_DROPFILES,OnDropFiles)
		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_HANDLER(IDC_REFBUTTON,BN_CLICKED,OnClickedBrowseButton)
		COMMAND_HANDLER(IDC_TRANSADDBUTTON,BN_CLICKED,OnClickedTransAddButton)
		COMMAND_HANDLER(IDC_TRANSREMOVEBUTTON,BN_CLICKED,OnClickedTransRemoveButton)
		COMMAND_HANDLER(IDC_TRANSCLEARBUTTON,BN_CLICKED,OnClickedTransClearButton)

		CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDropFiles(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	LRESULT OnClickedBrowseButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnClickedTransAddButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnClickedTransRemoveButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnClickedTransClearButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
