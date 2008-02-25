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
#include "DeviceManager.h"
#include "Core2.h"

class CEraseDlg : public CDialogImpl<CEraseDlg>
{
private:
	bool m_bAppMode;
	HICON m_hRefreshIcon;
	HIMAGELIST m_hRefreshImageList;
	unsigned int m_uiRecorderTextLen;

	CComboBox m_RecorderCombo;
	CComboBox m_MethodCombo;
	CComboBox m_SpeedCombo;
	CCore2Device m_CurDevice;

	enum
	{
		TIMER_ID = 42,
		TIMER_INTERVAL = 1000
	};

	bool Translate();
	bool InitRecorderMedia();
	void InitRefreshButton();
	void CheckRecorderMedia();

public:
	enum { IDD = IDD_ERASEDLG };

	CEraseDlg(bool bAppMode);
	~CEraseDlg();

	BEGIN_MSG_MAP(CEraseDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY,OnDestroy)
		//MESSAGE_HANDLER(WM_DEVICECHANGE,OnDeviceChange)
		MESSAGE_HANDLER(WM_TIMER,OnTimer)
		COMMAND_HANDLER(IDC_RECORDERCOMBO,CBN_SELCHANGE,OnRecorderChange)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_HANDLER(IDC_HELPBUTTON,BN_CLICKED,OnHelp)
		COMMAND_HANDLER(IDC_REFRESHBUTTON,BN_CLICKED,OnRefresh)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	//LRESULT OnDeviceChange(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnRecorderChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
