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
#include <ckcore/process.hh>
#include <ckmmc/device.hh>
#include "Resource.h"
#include "AdvancedProgress.h"
#include "Effects.h"
#include "WinVer.h"

class CSimpleProgressDlg : public CDialogImpl<CSimpleProgressDlg>,
						   public CDialogResize<CSimpleProgressDlg>,
						   public CAdvancedProgress,public CMessageFilter,
						   public CIdleHandler
{
private:
	HIMAGELIST m_hListImageList;
	CListViewCtrl m_ListView;

	ckcore::Process *m_pProcess;
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

	void AttachProcess(ckcore::Process *pProcess);
	void AttachHost(HWND hWndHost);

	void SetAppMode(bool bAppMode);
	void SetRealMode(bool bRealMode);

	// ckcore::Progress.
	void set_status(const TCHAR *szStatus,...);
	void notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...);
	bool cancelled();

	void SetDevice(ckmmc::Device &Device);

	void NotifyCompleted();

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

		NOTIFY_HANDLER(IDC_MESSAGELIST,NM_DBLCLK,OnListViewDblClick)

		SMOKE_EVENTS

		CHAIN_MSG_MAP(CDialogResize<CSimpleProgressDlg>)
	END_MSG_MAP()

	// Resize maps.
	BEGIN_DLGRESIZE_MAP(CSimpleProgressDlg)
		DLGRESIZE_CONTROL(IDC_STATUSSTATIC,DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_MESSAGELIST,DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_BEVELSTATIC,DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_DEVICESTATIC,DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK,DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL,DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RELOADBUTTON,DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnReload(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	LRESULT OnListViewDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	SMOKE_IMPL
};

extern CSimpleProgressDlg * g_pSimpleProgressDlg;