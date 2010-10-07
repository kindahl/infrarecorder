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
#include "DoubleBufferedStatic.h"
#include "Effects.h"
#include "WinVer.h"

#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE      (WM_USER + 10)
#endif

#ifndef PBS_MARQUEE
#define PBS_MARQUEE         0x08
#endif

class CProgressDlg : public CDialogImpl<CProgressDlg>,
					 public CDialogResize<CProgressDlg>,
					 public CAdvancedProgress
{
private:
	#if(WINVER < 0x0500)

		typedef struct {
			UINT  cbSize;
			HWND  hwnd;
			DWORD dwFlags;
			UINT  uCount;
			DWORD dwTimeout;
		} FLASHWINFO, *PFLASHWINFO;

	#endif  // #if(WINVER < 0x0500)

	typedef BOOL (WINAPI *PointerToFlashWindowEx)(PFLASHWINFO);
	PointerToFlashWindowEx m_pFlashWindowEx;
	bool m_bNeedToFocusOkButton;

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

private:
	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_ACTIVATE,OnActivate)

		COMMAND_ID_HANDLER(IDC_RELOADBUTTON,OnReload)
		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)

		NOTIFY_HANDLER(IDC_MESSAGELIST,NM_DBLCLK,OnListViewDblClick)

		SMOKE_EVENTS

		CHAIN_MSG_MAP(CDialogResize<CProgressDlg>)
	END_MSG_MAP()

	// Resize maps.
	BEGIN_DLGRESIZE_MAP(CProgressDlg)
		DLGRESIZE_CONTROL(IDC_TOTALSTATIC,DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TOTALPROGRESS,DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STATUSSTATIC,DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_MESSAGELIST,DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_BEVELSTATIC,DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_DEVICESTATIC,DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BUFFERSTATIC,DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BUFFERPROGRESS,DLSZ_SIZE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK,DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL,DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_RELOADBUTTON,DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnActivate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnReload(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	LRESULT OnListViewDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	SMOKE_IMPL
};

extern CProgressDlg * g_pProgressDlg;