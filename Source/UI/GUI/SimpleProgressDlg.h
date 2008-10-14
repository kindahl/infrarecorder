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
#include "AdvancedProgress.h"
#include "ConsolePipe.h"
#include "Effects.h"
#include "WinVer.h"

class CSimpleProgressDlg : public CDialogImpl<CSimpleProgressDlg>,public CAdvancedProgress,
	public CMessageFilter, public CIdleHandler
{
private:
	HIMAGELIST m_hListImageList;
	CListViewCtrl m_ListView;

	CConsolePipe *m_pConsolePipe;
	bool m_bAppMode;
	bool m_bRealMode;
	bool m_bCancelled;

	HWND m_hWndHost;

	bool Translate();

public:
	enum { IDD = IDD_SIMPLEPROGRESSDLG };

	CSimpleProgressDlg();
	~CSimpleProgressDlg();

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL OnIdle();

	void AttachConsolePipe(CConsolePipe *pConsolePipe);
	void AttachHost(HWND hWndHost);

	void SetAppMode(bool bAppMode);
	void SetRealMode(bool bRealMode);

	// ckcore::Progress.
	void SetStatus(const TCHAR *szStatus,...);
	void Notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...);
	bool Cancelled();

	void SetDevice(const TCHAR *szDevice);

	void NotifyComplteted();

	void AllowReload();
	void AllowCancel(bool bAllow);

	void Reset();

	bool RequestNextDisc();

	void StartSmoke();

	BEGIN_MSG_MAP(CSimpleProgressDlg)
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

extern CSimpleProgressDlg g_SimpleProgressDlg;
