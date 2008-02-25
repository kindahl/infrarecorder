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

#define ENCODE_BUFFER_FACTOR			1024

#pragma once

class CMainDlg : public CDialogImpl<CMainDlg>,public CUpdateUI<CMainDlg>,
	public CMessageFilter,public CIdleHandler
{
private:
	HIMAGELIST m_hListImageList;
	CListViewCtrl m_ListView;
	CComboBox m_ComboBox;

	void InitializeListView();
	void FillListView();
	void FillComboBox();

	void DisableApp();

	void BeginProcess();
	void EndProcess();

public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		COMMAND_ID_HANDLER(ID_APP_ABOUT,OnAppAbout)
		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_HANDLER(IDC_INPUTBUTTON,BN_CLICKED,OnBnClickedInputbutton)
		COMMAND_HANDLER(IDC_OUTPUTBUTTON,BN_CLICKED,OnBnClickedOutputbutton)
		COMMAND_HANDLER(IDC_HELPBUTTON,BN_CLICKED,OnBnClickedHelpbutton)
		COMMAND_HANDLER(IDC_ENCODERCONFIGBUTTON,BN_CLICKED,OnBnClickedEncoderConfigButton)
		COMMAND_HANDLER(IDC_FORMATCOMBO,CBN_SELCHANGE,OnEncoderChange)
		NOTIFY_HANDLER(IDC_LIST,NM_DBLCLK,OnListDblClick)
	END_MSG_MAP()

	void CloseDialog(int iVal);

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnAppAbout(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnBnClickedInputbutton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnBnClickedOutputbutton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnBnClickedHelpbutton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnBnClickedEncoderConfigButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnEncoderChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnListDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
};
