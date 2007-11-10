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

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include "../../Common/StringUtil.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"

CMainDlg::CMainDlg() :
	m_DataCDItem(0,ID_ACTION_DATACD),
	m_DataDVDItem(10,ID_ACTION_DATADVD),
	m_MixedItem(1,ID_ACTION_MIXED),
	m_AudioItem(2,ID_ACTION_AUDIO),
	m_DVDVideoItem(9,ID_ACTION_DVDVIDEO),
	m_BurnImageItem(3,ID_ACTION_BURNIMAGE),
	m_CreateImageItem(4,ID_ACTION_CREATEIMAGE),
	m_EraseItem(5,ID_ACTION_ERASE),
	m_FixateItem(6,ID_ACTION_FIXATE),
	m_TracksItem(7,ID_ACTION_MANAGETRACKS),
	m_CopyItem(8,ID_ACTION_COPY)
{
	// Pages.
	m_DataPage.SetTitle(lngGetString(TAB_DATA));
	m_AudioPage.SetTitle(lngGetString(TAB_AUDIO));
	m_VideoPage.SetTitle(lngGetString(TAB_VIDEO));
	m_CopyPage.SetTitle(lngGetString(TAB_COPY));
	m_OtherPage.SetTitle(lngGetString(TAB_OTHER));

	// Item titles and descriptions.
	m_DataCDItem.SetText(lngGetString(TITLE_DATACD),lngGetString(DESC_DATACD));
	m_DataDVDItem.SetText(lngGetString(TITLE_DATADVD),lngGetString(DESC_DATADVD));
	m_MixedItem.SetText(lngGetString(TITLE_MIXED),lngGetString(DESC_MIXED));
	m_AudioItem.SetText(lngGetString(TITLE_AUDIO),lngGetString(DESC_AUDIO));
	m_DVDVideoItem.SetText(lngGetString(TITLE_DVDVIDEO),lngGetString(DESC_DVDVIDEO));
	m_BurnImageItem.SetText(lngGetString(TITLE_BURNIMAGE),lngGetString(DESC_BURNIMAGE));
	m_CreateImageItem.SetText(lngGetString(TITLE_CREATEIMAGE),lngGetString(DESC_CREATEIMAGE));
	m_EraseItem.SetText(lngGetString(TITLE_ERASE),lngGetString(DESC_ERASE));
	m_FixateItem.SetText(lngGetString(TITLE_FIXATE),lngGetString(DESC_FIXATE));
	m_TracksItem.SetText(lngGetString(TITLE_MANAGETRACKS),lngGetString(DESC_MANAGETRACKS));
	m_CopyItem.SetText(lngGetString(TITLE_COPY),lngGetString(DESC_COPY));
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::PreTranslateMessage(MSG *pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

void CMainDlg::InitializeImageList()
{
	HBITMAP hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_PAGEBUTTONSBITMAP));

	m_PageImageList.Create(32,32,ILC_COLOR32,0,11);
	m_PageImageList.Add(hBitmap);

	DeleteObject(hBitmap);
}

void CMainDlg::InitializePageControl()
{
	RECT rcClient;
	GetClientRect(&rcClient);

	m_PageCtrl.Create(m_hWnd,rcClient,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,0,(unsigned int)0,NULL);

	m_DataCDItem.SetImageList(&m_PageImageList);
	m_DataDVDItem.SetImageList(&m_PageImageList);
	m_MixedItem.SetImageList(&m_PageImageList);
	m_AudioItem.SetImageList(&m_PageImageList);
	m_DVDVideoItem.SetImageList(&m_PageImageList);
	m_BurnImageItem.SetImageList(&m_PageImageList);
	m_CreateImageItem.SetImageList(&m_PageImageList);
	m_EraseItem.SetImageList(&m_PageImageList);
	m_FixateItem.SetImageList(&m_PageImageList);
	m_TracksItem.SetImageList(&m_PageImageList);
	m_CopyItem.SetImageList(&m_PageImageList);

	m_PageCtrl.AddPage(&m_DataPage);
		m_DataPage.SetReceiver(m_hWnd);
		m_DataPage.AddItem(&m_DataCDItem);
		m_DataPage.AddItem(&m_DataDVDItem);
		m_DataPage.AddItem(&m_MixedItem);
	m_PageCtrl.AddPage(&m_AudioPage);
		m_AudioPage.SetReceiver(m_hWnd);
		m_AudioPage.AddItem(&m_AudioItem);
		m_AudioPage.AddItem(&m_MixedItem);
	m_PageCtrl.AddPage(&m_VideoPage);
		m_VideoPage.SetReceiver(m_hWnd);
		m_VideoPage.AddItem(&m_DVDVideoItem);
	m_PageCtrl.AddPage(&m_CopyPage);
		m_CopyPage.SetReceiver(m_hWnd);
		m_CopyPage.AddItem(&m_BurnImageItem);
		m_CopyPage.AddItem(&m_CreateImageItem);
		m_CopyPage.AddItem(&m_CopyItem);
	m_PageCtrl.AddPage(&m_OtherPage);
		m_OtherPage.SetReceiver(m_hWnd);
		m_OtherPage.AddItem(&m_EraseItem);
		m_OtherPage.AddItem(&m_FixateItem);
		m_OtherPage.AddItem(&m_TracksItem);
}

LRESULT CMainDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow();

	// Set icons.
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME),
		IMAGE_ICON,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	SetIcon(hIcon,TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME),
		IMAGE_ICON,::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// Register object for message filtering and idle updates.
	CMessageLoop *pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	// Create the image list.
	InitializeImageList();

	// Create the page control.
	InitializePageControl();

	return TRUE;
}

LRESULT CMainDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	DestroyWindow();
	::PostQuitMessage(wID);

	return 0;
}

LRESULT CMainDlg::OnAction(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// File name.
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL,szFileName,MAX_PATH - 1);
	ExtractFilePath(szFileName);
	lstrcat(szFileName,_T("InfraRecorder.exe"));

	// Parameter.
	TCHAR szParam[32];
	
	switch (wID)
	{
		case ID_ACTION_DATACD:
			lstrcpy(szParam,_T(""));
			break;

		case ID_ACTION_DATADVD:
			lstrcpy(szParam,_T("-datadvdproject"));
			break;

		case ID_ACTION_MIXED:
			lstrcpy(szParam,_T("-mixedproject"));
			break;

		case ID_ACTION_AUDIO:
			lstrcpy(szParam,_T("-audioproject"));
			break;

		case ID_ACTION_DVDVIDEO:
			lstrcpy(szParam,_T("-dvdvideoproject"));
			break;

		case ID_ACTION_BURNIMAGE:
			lstrcpy(szParam,_T("-burnimage"));
			break;

		case ID_ACTION_CREATEIMAGE:
			lstrcpy(szParam,_T("-copyimage"));
			break;

		case ID_ACTION_ERASE:
			lstrcpy(szParam,_T("-erase"));
			break;

		case ID_ACTION_FIXATE:
			lstrcpy(szParam,_T("-fixate"));
			break;

		case ID_ACTION_MANAGETRACKS:
			lstrcpy(szParam,_T("-tracks"));
			break;

		case ID_ACTION_COPY:
			lstrcpy(szParam,_T("-copydisc"));
			break;
	}

	ShellExecute(HWND_DESKTOP,_T("open"),szFileName,szParam,NULL,SW_SHOWDEFAULT);

	return 0;
}
