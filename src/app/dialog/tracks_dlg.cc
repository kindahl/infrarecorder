/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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

#include "stdafx.hh"
#include <ckmmc/devicemanager.hh>
#include <base/string_util.hh>
#include "string_table.hh"
#include "scsi.hh"
#include "wait_dlg.hh"
#include "version.hh"
#include "progress_dlg.hh"
#include "infrarecorder.hh"
#include "device_util.hh"
#include "core.hh"
#include "core2.hh"
#include "lang_util.hh"
#include "settings.hh"
#include "save_tracks_dlg.hh"
#include "tracks_dlg.hh"

CTracksDlg::CTracksDlg(bool bAppMode)
{
	m_bAppMode = bAppMode;

	m_hListImageList = NULL;
	m_hToolBarImageList = NULL;

	m_pEncoder = NULL;
}

CTracksDlg::~CTracksDlg()
{
	if (m_hListImageList)
		ImageList_Destroy(m_hListImageList);

	if (m_hToolBarImageList)
		ImageList_Destroy(m_hToolBarImageList);
}

bool CTracksDlg::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a tracks translation section.
	if (!pLng->EnterSection(_T("tracks")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLng->GetValuePtr(IDD_TRACKSDLG,szStrValue))			// Title.
		SetWindowText(szStrValue);
	if (pLng->GetValuePtr(IDOK,szStrValue))
		SetDlgItemText(IDOK,szStrValue);
	if (pLng->GetValuePtr(IDC_HELPBUTTON,szStrValue))
		SetDlgItemText(IDC_HELPBUTTON,szStrValue);
	if (pLng->GetValuePtr(IDC_DRIVESTATIC,szStrValue))
		SetDlgItemText(IDC_DRIVESTATIC,szStrValue);

	return true;
}

unsigned long CTracksDlg::MSFToLBA(unsigned long ulMin,unsigned long ulSec,
								   unsigned long ulFrame)
{
	return ((ulMin * 60 * 75) + (ulSec * 75) + ulFrame - 150);
}

void CTracksDlg::InitListImageList()
{
	m_hListImageList = ImageList_Create(16,16,ILC_COLOR32,4,5);

	HICON hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_DATAICON),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	ImageList_AddIcon(m_hListImageList,hIcon);
	DestroyIcon(hIcon);

	hIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_AUDIOICON),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	ImageList_AddIcon(m_hListImageList,hIcon);
	DestroyIcon(hIcon);
}

void CTracksDlg::InitToolBarImageList()
{
	// Create the image list.
	HBITMAP hBitmap;

	// Get color depth (minimum requirement is 32-bits for alpha blended images).
	int iBitsPixel = GetDeviceCaps(::GetDC(HWND_DESKTOP),BITSPIXEL);
	
	if (g_WinVer.m_ulMajorCCVersion >= 6 && iBitsPixel >= 32)
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_TRACKTOOLBARBITMAP));

		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32,0,3);
		ImageList_Add(m_hToolBarImageList,hBitmap,NULL);
	}
	else
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_TRACKTOOLBARBITMAP_));

		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,3);
		ImageList_AddMasked(m_hToolBarImageList,hBitmap,RGB(255,0,255));
	}

	DeleteObject(hBitmap);
}

void CTracksDlg::AddToolBarSeparator()
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_SEP;
	tbButton.iBitmap = 0;
	tbButton.idCommand = 0;
	tbButton.iString = 0;
	tbButton.dwData = 0;
	m_ToolBar.InsertButton(m_ToolBar.GetButtonCount(),&tbButton);
}

void CTracksDlg::AddToolBarButton(int iCommand,int iBitmap)
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_BUTTON;
	tbButton.iBitmap = iBitmap;
	tbButton.idCommand = iCommand;
	tbButton.iString = 0;
	tbButton.dwData = 0;
	m_ToolBar.InsertButton(m_ToolBar.GetButtonCount(),&tbButton);
}

void CTracksDlg::CreateToolBarCtrl()
{
	RECT rcListView;
	::GetWindowRect(GetDlgItem(IDC_TRACKLIST),&rcListView);
	ScreenToClient(&rcListView);

	RECT rcToolBar = { 0,0,100,100 };
	m_ToolBar.Create(m_hWnd,rcToolBar,NULL,ATL_SIMPLE_TOOLBAR_PANE_STYLE,NULL);
	m_ToolBar.SetImageList(m_hToolBarImageList);
	m_ToolBar.SetButtonStructSize();

	// Create the buttons.
	AddToolBarButton(ID_TRACK_READ,0);
	AddToolBarButton(ID_TRACK_VERIFY,1);
	AddToolBarSeparator();
	AddToolBarButton(ID_TRACK_ERASE,2);

	// Update the toolbar position.
	m_ToolBar.SetWindowPos(NULL,
		rcListView.left,
		rcListView.bottom + 4,
		rcListView.right - rcListView.left,
		HIWORD(m_ToolBar.GetButtonSize()),0);
}

LRESULT CTracksDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// Add the window to the task bar and add a minimize button to the dialog if
	// the windows is in application mode.
	/*if (m_bAppMode)
	{
		ModifyStyle(WS_POPUPWINDOW | WS_DLGFRAME | DS_MODALFRAME,(WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_OVERLAPPED,0);
		ModifyStyleEx(WS_EX_DLGMODALFRAME,WS_EX_APPWINDOW,0);

		// Set icons.
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
		SetIcon(hIcon,TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON,::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
		SetIcon(hIconSmall,FALSE);
	}*/
	if (m_bAppMode)
	{
		ModifyStyle(0,WS_MINIMIZEBOX | WS_SYSMENU);
		ModifyStyleEx(0,WS_EX_APPWINDOW);

		HMENU hSysMenu = GetSystemMenu(FALSE);
		::InsertMenu(hSysMenu,0,MF_BYPOSITION,SC_RESTORE,_T("&Restore"));
		::InsertMenu(hSysMenu,2,MF_BYPOSITION,SC_MINIMIZE,_T("Mi&nimize"));
		::InsertMenu(hSysMenu,3,MF_BYPOSITION | MF_SEPARATOR,0,_T(""));
	}

	// Image lists.
	InitListImageList();
	InitToolBarImageList();

	// Create tool bar.
	CreateToolBarCtrl();

	// Recorder combo box.
	m_DeviceCombo = GetDlgItem(IDC_DEVICECOMBO);

	std::vector<ckmmc::Device *>::const_iterator it;
	for (it = g_DeviceManager.devices().begin(); it !=
		g_DeviceManager.devices().end(); it++)
	{
		const ckmmc::Device *pDevice = *it;

		m_DeviceCombo.AddString(NDeviceUtil::GetDeviceName(*pDevice).c_str());
		m_DeviceCombo.SetItemData(m_DeviceCombo.GetCount() - 1,
								  reinterpret_cast<DWORD_PTR>(pDevice));
	}

	if (m_DeviceCombo.GetCount() == 0)
	{
		m_DeviceCombo.AddString(lngGetString(FAILURE_NODEVICES));
		m_DeviceCombo.EnableWindow(false);

		// Disable the OK button.
		::EnableWindow(GetDlgItem(IDOK),false);
	}

	m_DeviceCombo.SetCurSel(0);

	// List view.
	m_ListView = GetDlgItem(IDC_TRACKLIST);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	m_ListView.SetImageList(m_hListImageList,LVSIL_SMALL);

	m_ListView.InsertColumn(0,lngGetString(COLUMN_TRACK),LVCFMT_LEFT,45,0);
	m_ListView.InsertColumn(1,lngGetString(COLUMN_ADDRESS),LVCFMT_RIGHT,70,1);
	m_ListView.InsertColumn(2,lngGetString(COLUMN_LENGTH),LVCFMT_LEFT,118,2);

	// Update the list view.
	BOOL bDymmy;
	OnDeviceChange(NULL,NULL,NULL,bDymmy);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CTracksDlg::OnDeviceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// MCI_STATUS_MEDIA_PRESENT?
	bHandled = false;

	ckmmc::Device *pDevice =
		reinterpret_cast<ckmmc::Device *>(m_DeviceCombo.GetItemData(
										  m_DeviceCombo.GetCurSel()));

	// Empty the list view and disable the toolbar buttons.
	m_ListView.DeleteAllItems();
	m_ToolBar.EnableButton(ID_TRACK_READ,false);
	m_ToolBar.EnableButton(ID_TRACK_VERIFY,false);
	m_ToolBar.EnableButton(ID_TRACK_ERASE,false);

	// Rescan the bus.
	CWaitDlg WaitDlg;
	WaitDlg.Create(m_hWnd);
	WaitDlg.ShowWindow(SW_SHOW);
		// Initialize device (detect drive letter, open handle, count tracks).
		WaitDlg.SetMessage(lngGetString(INIT_DEVICECD));

		TCHAR szDriveLetter[3];
		szDriveLetter[0] = pDevice->address().device_[0];
		szDriveLetter[1] = ':';
		szDriveLetter[2] = '\0';
	
		// Open the device by specifying the device name.
		unsigned long ulResult = 0;
		unsigned long ulDeviceID = 0;
		unsigned long ulNumTracks = 0;
		MCI_OPEN_PARMS mciOpenParms;
		MCI_SET_PARMS mciSetParms;
		MCI_STATUS_PARMS mciStatusParms;

		mciOpenParms.lpstrDeviceType = (TCHAR *)MCI_DEVTYPE_CD_AUDIO;
		mciOpenParms.lpstrElementName = szDriveLetter;
		ulResult = mciSendCommand(NULL,MCI_OPEN,MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT,(DWORD_PTR)&mciOpenParms);
		if (ulResult != 0)
		{
			WaitDlg.DestroyWindow();
			return 0;
		}

		ulDeviceID = mciOpenParms.wDeviceID;

		// Set the time format to minute/second/frame (MSF) format. 
		mciSetParms.dwTimeFormat = MCI_FORMAT_MSF;
		ulResult = mciSendCommand(ulDeviceID,MCI_SET,MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSetParms);
		if (ulResult != 0) 
		{
			mciSendCommand(ulDeviceID,MCI_CLOSE,0,NULL);

			WaitDlg.DestroyWindow();
			return 0;
		}

		// Get the number of tracks.
		mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
		ulResult = mciSendCommand(ulDeviceID,MCI_STATUS,MCI_STATUS_ITEM,(DWORD_PTR)&mciStatusParms);
		if (ulResult != 0) 
		{
			mciSendCommand(ulDeviceID,MCI_CLOSE,0,NULL);

			WaitDlg.DestroyWindow();
			return 0;
		}

		ulNumTracks = (unsigned long)mciStatusParms.dwReturn;

		// Track information.
		TCHAR szBuffer[128];
		unsigned int uiListItemCount = 0;

		for (unsigned long i = 1; i <= ulNumTracks; i++)
		{
			mciStatusParms.dwTrack = i;
			mciStatusParms.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
			ulResult = mciSendCommand(ulDeviceID,MCI_STATUS,MCI_STATUS_ITEM | MCI_TRACK,(DWORD_PTR)&mciStatusParms);
			if (ulResult != 0) 
			{
				mciSendCommand(ulDeviceID,MCI_CLOSE,0,NULL);

				WaitDlg.DestroyWindow();
				return 0;
			}

			// Add the new list item.
			lsprintf(szBuffer,_T("%d"),i);
			m_ListView.AddItem(uiListItemCount,0,szBuffer,mciStatusParms.dwReturn == MCI_CDA_TRACK_AUDIO);

			// Wait dialog message.
			lsnprintf_s(szBuffer,128,lngGetString(INIT_TRACK),i);
			WaitDlg.SetMessage(szBuffer);

			// Track position.
			mciStatusParms.dwItem = MCI_STATUS_POSITION;
			ulResult = mciSendCommand(ulDeviceID,MCI_STATUS,MCI_STATUS_ITEM | MCI_TRACK,(DWORD_PTR)&mciStatusParms);
			if (ulResult != 0) 
			{
				mciSendCommand(ulDeviceID,MCI_CLOSE,0,NULL);

				WaitDlg.DestroyWindow();
				return 0;
			}

			lsprintf(szBuffer,_T("%d"),MSFToLBA(
				MCI_MSF_MINUTE(mciStatusParms.dwReturn),
				MCI_MSF_SECOND(mciStatusParms.dwReturn),
				MCI_MSF_FRAME(mciStatusParms.dwReturn)));
			m_ListView.AddItem(uiListItemCount,1,szBuffer);

			// Track length.
			mciStatusParms.dwItem = MCI_STATUS_LENGTH;
			ulResult = mciSendCommand(ulDeviceID,MCI_STATUS,MCI_STATUS_ITEM | MCI_TRACK,(DWORD_PTR)&mciStatusParms);
			if (ulResult != 0) 
			{
				mciSendCommand(ulDeviceID,MCI_CLOSE,0,NULL);

				WaitDlg.DestroyWindow();
				return 0;
			}

			unsigned long ulMin = MCI_MSF_MINUTE(mciStatusParms.dwReturn);
			unsigned long ulSec = MCI_MSF_SECOND(mciStatusParms.dwReturn);
			unsigned long ulFrame = MCI_MSF_FRAME(mciStatusParms.dwReturn);
			unsigned long ulSecLen = MSFToLBA(ulMin,ulSec,ulFrame);

			lsprintf(szBuffer,_T("%02d:%02d:%02d (%d)"),
				ulMin,ulSec,ulFrame,ulSecLen);
			m_ListView.AddItem(uiListItemCount,2,szBuffer);

			// Set the item data to the length of the track (in sectors).
			m_ListView.SetItemData(uiListItemCount,ulSecLen);

			uiListItemCount++;
		}

		mciSendCommand(ulDeviceID,MCI_CLOSE,0,NULL);
	WaitDlg.DestroyWindow();

	return 0;
}

bool CTracksDlg::IsDataTrack(int iTrackIndex)
{
	LVITEM lvItem;
	memset(&lvItem,0,sizeof(LVITEM));

	lvItem.iItem = iTrackIndex;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_IMAGE;
		m_ListView.GetItem(&lvItem);

	return lvItem.iImage == 0;
}

unsigned long CTracksDlg::GetTrackAddress(int iTrackIndex)
{
	TCHAR szTextBuffer[32];

	LVITEM lvItem;
	memset(&lvItem,0,sizeof(LVITEM));

	lvItem.iItem = iTrackIndex;
	lvItem.iSubItem = 1;
	lvItem.pszText = szTextBuffer;
	lvItem.cchTextMax = 32;
	lvItem.mask = LVIF_TEXT;
		m_ListView.GetItem(&lvItem);

#ifdef UNICODE
	return _wtoi(szTextBuffer);
#else
	return atoi(szTextBuffer);
#endif
}

bool CTracksDlg::EncodeTrack(const TCHAR *szFileName,CCodec *pEncoder)
{
	// Find which codec that can be uses for decoding the source file.
	CCodec *pDecoder = NULL;

	// Source file information.
	int iNumChannels = -1;
	int iSampleRate = -1;
	int iBitRate = -1;
	unsigned __int64 uiDuration = 0;

	for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
	{
		// We're only interested in decoders.
		if ((g_CodecManager.m_Codecs[i]->irc_capabilities() & IRC_HAS_DECODER) == 0)
			continue;

		if (g_CodecManager.m_Codecs[i]->irc_decode_init(szFileName,iNumChannels,
			iSampleRate,iBitRate,uiDuration))
		{
			pDecoder = g_CodecManager.m_Codecs[i];
			break;
		}
	}

	if (pDecoder == NULL)
	{
		TCHAR szNameBuffer[MAX_PATH];
		lstrcpy(szNameBuffer,szFileName);
		ExtractFileName(szNameBuffer);

		g_pProgressDlg->notify(ckcore::Progress::ckERROR,
			lngGetString(ERROR_NODECODER),szNameBuffer);
		return false;
	}

	// Setup the encoder.
	TCHAR szTargetFile[MAX_PATH];
	lstrcpy(szTargetFile,szFileName);
	ChangeFileExt(szTargetFile,pEncoder->irc_string(IRC_STR_FILEEXT));

	// Initialize the encoder.
	if (!pEncoder->irc_encode_init(szTargetFile,iNumChannels,iSampleRate,iBitRate))
	{
		g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_CODECINIT),
			pEncoder->irc_string(IRC_STR_ENCODER),
			iNumChannels,iSampleRate,iBitRate,uiDuration);

		pDecoder->irc_decode_exit();
		return false;
	}

	// Encode/decode-process.
	__int64 iBytesRead = 0;
	unsigned __int64 uiCurrentTime = 0;

#define ENCODE_BUFFER_FACTOR		1024

	// Allocate buffer memory.
	unsigned int uiBufferSize = iNumChannels * ((iBitRate / iSampleRate) >> 3) * ENCODE_BUFFER_FACTOR;
	unsigned char *pBuffer = new unsigned char[uiBufferSize];

	while (true)
	{
		iBytesRead = pDecoder->irc_decode_process(pBuffer,uiBufferSize,uiCurrentTime);
		if (iBytesRead <= 0)
			break;

		if (pEncoder->irc_encode_process(pBuffer,iBytesRead) < 0)
		{
			g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_ENCODEDATA));
			break;
		}

		// Update the progres bar.
		unsigned char ucPercent = (unsigned char)(((double)uiCurrentTime/uiDuration) * 100);
		g_pProgressDlg->set_progress(ucPercent);
	}

	// Free buffer memory.
	delete [] pBuffer;

	// Flush.
	pEncoder->irc_encode_flush();
	g_pProgressDlg->set_progress(100);

	// Destroy the codecs.
	pEncoder->irc_encode_exit();
	pDecoder->irc_decode_exit();

	ExtractFileName(szTargetFile);

	g_pProgressDlg->notify(ckcore::Progress::ckINFORMATION,
		lngGetString(SUCCESS_ENCODETRACK),szTargetFile);

	return true;
}

unsigned long WINAPI CTracksDlg::ReadTrackThread(LPVOID lpThreadParameter)
{
	CTracksDlg *pTracksDlg = (CTracksDlg *)lpThreadParameter;

	ckmmc::Device *pDevice =
		reinterpret_cast<ckmmc::Device *>(pTracksDlg->m_DeviceCombo.GetItemData(
										  pTracksDlg->m_DeviceCombo.GetCurSel()));

	// Get the selected tracks.
	TCHAR szTextBuffer[32];
	bool bData;
	unsigned long ulAddress;
	unsigned long ulLength;

	unsigned int uiSelCount = pTracksDlg->m_ListView.GetSelectedCount();
	unsigned int uiCurTrack = 1;

	int iItemIndex = -1;
	iItemIndex = pTracksDlg->m_ListView.GetNextItem(iItemIndex,LVNI_SELECTED);

	// Holds the full track name.
	TCHAR szFilePath[MAX_PATH];

	while (iItemIndex != -1)
	{
		// Get the track type.
		bData = pTracksDlg->IsDataTrack(iItemIndex);

		// File path.
		if (bData)
			lsprintf(szTextBuffer,_T("Track %d.iso"),iItemIndex + 1);
		else
			lsprintf(szTextBuffer,_T("Track %d.wav"),iItemIndex + 1);

		lstrcpy(szFilePath,pTracksDlg->m_szFolderPath);
		lstrcat(szFilePath,_T("\\"));
		lstrcat(szFilePath,szTextBuffer);

		// Get the track address.
		ulAddress = pTracksDlg->GetTrackAddress(iItemIndex);

		// Get the track length.
		ulLength = (unsigned long)pTracksDlg->m_ListView.GetItemData(iItemIndex);

		// Check if we're on the last track. We need to know when to finish the progress window.
		if (uiCurTrack == uiSelCount)
		{
			if (bData)
			{
				bool bResult = g_Core2.ReadDataTrack(*pDevice,g_pProgressDlg,
													 static_cast<unsigned char>(iItemIndex + 1),
													 true,szFilePath);

				g_pProgressDlg->set_progress(100);
				g_pProgressDlg->NotifyCompleted();

				if (bResult)
				{
					g_pProgressDlg->set_status(lngGetString(PROGRESS_DONE));
				}
				else
				{
					g_pProgressDlg->set_status(lngGetString(PROGRESS_FAILED));
					ckcore::File::remove(szFilePath);
					return 0;
				}
			}
			else
			{
				if (pTracksDlg->m_pEncoder != NULL)
				{
					if (g_Core.ReadAudioTrackEx(*pDevice,g_pProgressDlg,szFilePath,iItemIndex + 1) != RESULT_OK)
					{
						ckcore::File::remove(szFilePath);
						return 0;
					}

					// Encode.
					TCHAR szStatus[256];
					lsnprintf_s(szStatus,256,lngGetString(PROGRESS_ENCODETRACK),
						pTracksDlg->m_pEncoder->irc_string(IRC_STR_ENCODER));

					g_pProgressDlg->set_status(szStatus);

					if (EncodeTrack(szFilePath,pTracksDlg->m_pEncoder))
						ckcore::File::remove(szFilePath);

					g_pProgressDlg->set_status(lngGetString(PROGRESS_DONE));
					g_pProgressDlg->NotifyCompleted();
				}
				else
				{
					if (!g_Core.ReadAudioTrack(*pDevice,g_pProgressDlg,szFilePath,iItemIndex + 1))
					{
						ckcore::File::remove(szFilePath);
						return 0;
					}
				}
			}
		}
		else
		{
			if (bData)
			{
				if (g_Core2.ReadDataTrack(*pDevice,g_pProgressDlg,static_cast<unsigned char>(iItemIndex + 1),
										  true,szFilePath))
				{
					g_pProgressDlg->set_progress(0);
				}
				else
				{
					ckcore::File::remove(szFilePath);

					g_pProgressDlg->set_progress(100);
					g_pProgressDlg->set_status(lngGetString(PROGRESS_FAILED));
					g_pProgressDlg->NotifyCompleted();
					return 0;
				}
			}
			else
			{
				if (g_Core.ReadAudioTrackEx(*pDevice,g_pProgressDlg,szFilePath,iItemIndex + 1) != RESULT_OK)
				{
					ckcore::File::remove(szFilePath);
					return 0;
				}

				// Encode audio.
				if (pTracksDlg->m_pEncoder != NULL)
				{
					TCHAR szStatus[256];
					lsnprintf_s(szStatus,256,lngGetString(PROGRESS_ENCODETRACK),
						pTracksDlg->m_pEncoder->irc_string(IRC_STR_ENCODER));

					g_pProgressDlg->set_status(szStatus);

					if (EncodeTrack(szFilePath,pTracksDlg->m_pEncoder))
						ckcore::File::remove(szFilePath);
				}

				// Check if the encoding has been canceled.
				if (g_pProgressDlg->cancelled())
				{
					g_pProgressDlg->NotifyCompleted();
					return 0;
				}
			}
		}

		uiCurTrack++;

		iItemIndex = pTracksDlg->m_ListView.GetNextItem(iItemIndex,LVNI_SELECTED);
	}

	return 0;
}

unsigned long WINAPI CTracksDlg::ScanTrackThread(LPVOID lpThreadParameter)
{
	CTracksDlg *pTracksDlg = (CTracksDlg *)lpThreadParameter;

	ckmmc::Device *pDevice =
		reinterpret_cast<ckmmc::Device *>(pTracksDlg->m_DeviceCombo.GetItemData(
										  pTracksDlg->m_DeviceCombo.GetCurSel()));

	// Get the selected tracks.
	unsigned long ulAddress;
	unsigned long ulLength;

	unsigned int uiSelCount = pTracksDlg->m_ListView.GetSelectedCount();
	unsigned int uiCurTrack = 1;

	int iItemIndex = -1;
	iItemIndex = pTracksDlg->m_ListView.GetNextItem(iItemIndex,LVNI_SELECTED);

	while (iItemIndex != -1)
	{
		// Get the track address.
		ulAddress = pTracksDlg->GetTrackAddress(iItemIndex);

		// Get the track length.
		ulLength = (unsigned long)pTracksDlg->m_ListView.GetItemData(iItemIndex);

		// Check if we're on the last track. We need to know when to finish the progress window.
		if (uiCurTrack == uiSelCount)
		{
			if (!g_Core.ScanTrack(*pDevice,g_pProgressDlg,iItemIndex + 1,
								  ulAddress,ulAddress + ulLength))
			{
				return 0;
			}
		}
		else
		{
			if (g_Core.ScanTrackEx(*pDevice,g_pProgressDlg,iItemIndex + 1,
								   ulAddress,ulAddress + ulLength) != RESULT_OK)
			{
				return 0;
			}
		}

		uiCurTrack++;

		iItemIndex = pTracksDlg->m_ListView.GetNextItem(iItemIndex,LVNI_SELECTED);
	}

	return 0;
}

LRESULT CTracksDlg::OnReadTrack(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CSaveTracksDlg SaveTracksDlg;
	if (SaveTracksDlg.DoModal() == IDOK)
	{
		ckmmc::Device *pDevice =
			reinterpret_cast<ckmmc::Device *>(m_DeviceCombo.GetItemData(
											  m_DeviceCombo.GetCurSel()));

		// Disable the main frame.
		EnableWindow(false);

		// Create and display the progress dialog.
		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(m_hWnd);

		g_pProgressDlg->ShowWindow(true);
		g_pProgressDlg->SetWindowText(lngGetString(STITLE_READTRACK));
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(m_hWnd);
		ProcessMessages();

		// Set the device information.
		g_pProgressDlg->SetDevice(*pDevice);
		g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

		lstrcpy(m_szFolderPath,g_SaveTracksSettings.m_szTarget);
		m_pEncoder = SaveTracksDlg.GetEncoder();

		// Create the new thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,ReadTrackThread,this,0,&ulThreadID);
		::CloseHandle(hThread);
	}

	bHandled = false;
	return 0;
}

LRESULT CTracksDlg::OnVerifyTrack(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	ckmmc::Device *pDevice =
		reinterpret_cast<ckmmc::Device *>(m_DeviceCombo.GetItemData(
										  m_DeviceCombo.GetCurSel()));

	// Disable the main frame.
	EnableWindow(false);

	// Create and display the progress dialog.
	if (!g_pProgressDlg->IsWindow())
		g_pProgressDlg->Create(m_hWnd);

	g_pProgressDlg->ShowWindow(true);
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_SCANTRACK));
	g_pProgressDlg->Reset();
	g_pProgressDlg->AttachProcess(&g_Core);
	g_pProgressDlg->AttachHost(m_hWnd);
	ProcessMessages();

	// Set the device information.
	g_pProgressDlg->SetDevice(*pDevice);
	g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

	// Create the new thread.
	unsigned long ulThreadID = 0;
	HANDLE hThread = ::CreateThread(NULL,0,ScanTrackThread,this,0,&ulThreadID);
	::CloseHandle(hThread);

	bHandled = false;
	return 0;
}

LRESULT CTracksDlg::OnEraseTrack(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	MessageBox(_T("Not yet implemented."),_T("Information"),MB_OK | MB_ICONINFORMATION);
	bHandled = false;
	return 0;
}

LRESULT CTracksDlg::OnToolBarGetInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;

	// The string ID is the same as the button ID.
	LPTOOLTIPTEXT pTipText = (LPTOOLTIPTEXT)pNMH;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a hint translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("hint")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr((unsigned long)pTipText->hdr.idFrom,szStrValue))
			{
				pTipText->lpszText = szStrValue;
				return 0;
			}
		}
	}

	pTipText->lpszText = MAKEINTRESOURCE(pTipText->hdr.idFrom);
	return 0;
}

LRESULT CTracksDlg::OnListItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (m_ListView.GetSelectedCount() > 0)
	{
		m_ToolBar.EnableButton(ID_TRACK_READ,true);
		m_ToolBar.EnableButton(ID_TRACK_VERIFY,true);
		m_ToolBar.EnableButton(ID_TRACK_ERASE,true);
	}
	else
	{
		m_ToolBar.EnableButton(ID_TRACK_READ,false);
		m_ToolBar.EnableButton(ID_TRACK_VERIFY,false);
		m_ToolBar.EnableButton(ID_TRACK_ERASE,false);
	}

	bHandled = false;
	return 0;
}

LRESULT CTracksDlg::OnListKeyDown(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
    LPNMLVKEYDOWN KeyDown = (LPNMLVKEYDOWN)pNMH;

	// Ctrl+A = select all
	if (KeyDown->wVKey == _T('A') && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		const int iItemCount = m_ListView.GetItemCount();

		for (int i = 0; i < iItemCount; ++i )
		{
			ATLVERIFY(TRUE == m_ListView.SetItemState(i,LVIS_SELECTED,LVIS_SELECTED));
		}

		bHandled = TRUE;
	}
	else
	{
		bHandled = FALSE;
	}

	return 0;
}

LRESULT CTracksDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}

LRESULT CTracksDlg::OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/manage_tracks.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
	return 0;
}