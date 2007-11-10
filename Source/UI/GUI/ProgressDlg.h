/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "AdvancedProgress.h"
#include "ConsolePipe.h"
#include "DoubleBufferedStatic.h"
#include "Effects.h"
#include "WinVer.h"

class CProgressDlg : public CDialogImpl<CProgressDlg>,public CAdvancedProgress
{
private:
	HIMAGELIST m_hListImageList;
	CListViewCtrl m_ListView;
	CDoubleBufferedStatic m_TotalStatic;
	CDoubleBufferedStatic m_StatusStatic;

	CConsolePipe *m_pConsolePipe;
	bool m_bAppMode;
	bool m_bRealMode;
	bool m_bCanceled;

	HWND m_hWndHost;

	int m_iPercent;

	bool Translate();

public:
	enum { IDD = IDD_PROGRESSDLG };

	CProgressDlg();
	~CProgressDlg();

	void AttachConsolePipe(CConsolePipe *pConsolePipe);
	void AttachHost(HWND hWndHost);

	void AddLogEntry(int iType,const TCHAR *szMessage,...);
	void SetStatus(const TCHAR *szStatus,...);
	void SetDevice(const TCHAR *szDevice);

	void NotifyComplteted();

	void SetAppMode(bool bAppMode);
	void SetRealMode(bool bRealMode);

	void SetProgress(int iPercent);
	void SetBuffer(int iPercent);
	void AllowReload();
	void AllowCancel(bool bAllow);
	bool IsCanceled();

	void Reset();

	void StartSmoke();

	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(IDC_RELOADBUTTON,OnReload)
		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)

		SMOKE_EVENTS
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnReload(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	SMOKE_IMPL
};

extern CProgressDlg g_ProgressDlg;
