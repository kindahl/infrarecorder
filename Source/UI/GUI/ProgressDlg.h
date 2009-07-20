/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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
#include "resource.h"
#include "AdvancedProgress.h"
#include "DoubleBufferedStatic.h"
#include "Effects.h"
#include "WinVer.h"

#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE      (WM_USER + 10)
#endif

#ifndef PBS_MARQUEE
#define PBS_MARQUEE         0x08
#endif

class CProgressDlg : public CDialogImpl<CProgressDlg>,public CAdvancedProgress
{
private:
	HIMAGELIST m_hListImageList;
	CListViewCtrl m_ListView;
	CDoubleBufferedStatic m_TotalStatic;
	CDoubleBufferedStatic m_StatusStatic;

	ckcore::Process *m_pProcess;
	bool m_bAppMode;
	bool m_bRealMode;
	bool m_bCancelled;

	HWND m_hWndHost;

	unsigned char m_ucPercent;

	TCHAR *m_szHostTitle;

	bool Translate();

public:
	enum { IDD = IDD_PROGRESSDLG };

	CProgressDlg();
	~CProgressDlg();

	void AttachProcess(ckcore::Process *pProcess);
	void AttachHost(HWND hWndHost);

	void SetAppMode(bool bAppMode);
	void SetRealMode(bool bRealMode);

	// ckcore::Progress.
	void set_progress(unsigned char ucPercent);
	void set_marquee(bool bMarquee);
	void set_status(const TCHAR *szStatus,...);
	void notify(ckcore::Progress::MessageType Type,const TCHAR *szMessage,...);
	bool cancelled();

	void SetDevice(const TCHAR *szDevice);
	void SetDevice(ckmmc::Device &Device);

	void NotifyCompleted();

	void SetBuffer(int iPercent);
	void AllowReload();
	void AllowCancel(bool bAllow);

	void Reset();

	bool RequestNextDisc();

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

extern CProgressDlg * g_pProgressDlg;