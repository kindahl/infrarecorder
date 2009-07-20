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
#include "Resource.h"
#include "InfraRecorder.h"

class CTracksDlg : public CDialogImpl<CTracksDlg>
{
private:
	bool m_bAppMode;
	HIMAGELIST m_hListImageList;
	HIMAGELIST m_hToolBarImageList;

	CComboBox m_DeviceCombo;
	CListViewCtrl m_ListView;
	CToolBarCtrl m_ToolBar;

	// Needed to make the thread access what target folder that where selected.
	TCHAR m_szFolderPath[MAX_PATH];
	CCodec *m_pEncoder;

	static bool EncodeTrack(const TCHAR *szFileName,CCodec *pEncoder);
	static unsigned long WINAPI ReadTrackThread(LPVOID lpThreadParameter);
	static unsigned long WINAPI ScanTrackThread(LPVOID lpThreadParameter);

	unsigned long MSFToLBA(unsigned long ulMin,unsigned long ulSec,unsigned long ulFrame);

	bool Translate();

	void InitListImageList();
	void InitToolBarImageList();

	void AddToolBarSeparator();
	void AddToolBarButton(int iCommand,int iBitmap);
	void CreateToolBarCtrl();

	bool IsDataTrack(int iTrackIndex);
	unsigned long GetTrackAddress(int iTrackIndex);

public:
	enum { IDD = IDD_TRACKSDLG };

	CTracksDlg(bool bAppMode);
	~CTracksDlg();

private:
	BEGIN_MSG_MAP(CTracksDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		COMMAND_HANDLER(IDC_DEVICECOMBO,CBN_SELCHANGE,OnDeviceChange)
		COMMAND_ID_HANDLER(ID_TRACK_READ,OnReadTrack)
		COMMAND_ID_HANDLER(ID_TRACK_VERIFY,OnVerifyTrack)
		COMMAND_ID_HANDLER(ID_TRACK_ERASE,OnEraseTrack)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnToolBarGetInfo)
		NOTIFY_HANDLER(IDC_TRACKLIST,LVN_ITEMCHANGED,OnListItemChanged)
		NOTIFY_HANDLER(IDC_TRACKLIST,LVN_KEYDOWN,OnListKeyDown)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnOK)
		COMMAND_HANDLER(IDC_HELPBUTTON,BN_CLICKED,OnHelp)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDeviceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnReadTrack(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnVerifyTrack(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnEraseTrack(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnToolBarGetInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnListItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnListKeyDown(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};