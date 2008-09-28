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
#include "CustomPageCtrl.h"

class CMainDlg : public CDialogImpl<CMainDlg>,public CUpdateUI<CMainDlg>,
		public CMessageFilter,public CIdleHandler
{
private:
	CImageList m_PageImageList;
	CCustomPageCtrl m_PageCtrl;

	CCustomPage m_DataPage;
	CCustomPage m_AudioPage;
	CCustomPage m_VideoPage;
	CCustomPage m_CopyPage;
	CCustomPage m_OtherPage;

	CCustomPageItem m_DataCDItem;
	CCustomPageItem m_DataDVDItem;
	CCustomPageItem m_MixedItem;
	CCustomPageItem m_AudioItem;
	CCustomPageItem m_DVDVideoItem;
	CCustomPageItem m_BurnImageItem;
	CCustomPageItem m_CreateImageItem;
	CCustomPageItem m_EraseItem;
	CCustomPageItem m_FixateItem;
	CCustomPageItem m_TracksItem;
	CCustomPageItem m_CopyItem;

	void InitializeImageList();
	void InitializePageControl();

public:
	enum { IDD = IDD_MAINDLG };

	CMainDlg();
	~CMainDlg();

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)
		COMMAND_ID_HANDLER(ID_ACTION_DATACD,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_DATADVD,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_MIXED,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_AUDIO,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_DVDVIDEO,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_BURNIMAGE,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_CREATEIMAGE,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_ERASE,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_FIXATE,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_MANAGETRACKS,OnAction)
		COMMAND_ID_HANDLER(ID_ACTION_COPY,OnAction)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnAction(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
