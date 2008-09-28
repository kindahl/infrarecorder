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

#include "stdafx.h"
#include "resource.h"
#include "../../Common/StringUtil.h"
#include "../../Common/CodecManager.h"
#include "../../Common/FileManager.h"
#include "MainDlg.h"

CCodecManager g_CodecManager;

BOOL CMainDlg::PreTranslateMessage(MSG *pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

void CALLBACK CodecMessage(int iType,const TCHAR *szMessage)
{
	switch (iType)
	{
		case IRC_MESSAGE_WARNING:
			MessageBox(HWND_DESKTOP,szMessage,_T("Warning"),MB_OK | MB_ICONWARNING);
			break;

		case IRC_MESSAGE_ERROR:
			MessageBox(HWND_DESKTOP,szMessage,_T("Error"),MB_OK | MB_ICONERROR);
			break;

		default:
			MessageBox(HWND_DESKTOP,szMessage,_T("Information"),MB_OK | MB_ICONINFORMATION);
			break;
	}
}

void CMainDlg::InitializeListView()
{
	// Create the image list.
	m_hListImageList = NULL;
	HINSTANCE hInstance = LoadLibrary(_T("shell32.dll"));
		HICON hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(278),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	FreeLibrary(hInstance);

	m_hListImageList = ImageList_Create(16,16,ILC_COLOR32,0,1);
	ImageList_AddIcon(m_hListImageList,hIcon);

	DestroyIcon(hIcon);

	// Setup the list view.
	m_ListView.SetImageList(m_hListImageList,LVSIL_NORMAL);
	m_ListView.SetImageList(m_hListImageList,LVSIL_SMALL);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Add the columns.
	m_ListView.AddColumn(_T("Name"),0);
	m_ListView.SetColumnWidth(0,150);
	m_ListView.AddColumn(_T("Version"),1,-1,LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,LVCFMT_RIGHT);
	m_ListView.SetColumnWidth(1,55);
}

void CMainDlg::FillListView()
{
	TCHAR szFileName[MAX_PATH];
	int iItemCount = 0;

	for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
	{
		if (g_CodecManager.m_Codecs[i]->GetFileName(szFileName,MAX_PATH - 1))
		{
			ExtractFileName(szFileName);
			m_ListView.AddItem(iItemCount,0,szFileName,0);
			m_ListView.AddItem(iItemCount,1,g_CodecManager.m_Codecs[i]->irc_string(IRC_STR_VERSION),0);

			iItemCount++;
		}
	}
}

void CMainDlg::FillComboBox()
{
	m_ComboBox.ResetContent();

	for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
	{
		if (g_CodecManager.m_Codecs[i]->irc_capabilities() & IRC_HAS_ENCODER)
		{
			m_ComboBox.AddString(g_CodecManager.m_Codecs[i]->irc_string(IRC_STR_ENCODER));
			m_ComboBox.SetItemData(m_ComboBox.GetCount() - 1,(DWORD_PTR)g_CodecManager.m_Codecs[i]);
		}
	}

	if (m_ComboBox.GetCount() == 0)
	{
		DisableApp();
		return;
	}

	m_ComboBox.SetCurSel(0);

	CCodec *pEncoder = (CCodec *)m_ComboBox.GetItemData(0);
	::EnableWindow(GetDlgItem(IDC_ENCODERCONFIGBUTTON),pEncoder->irc_capabilities() & IRC_HAS_CONFIG);
}

void CMainDlg::DisableApp()
{
	m_ComboBox.AddString(_T("None"));
	m_ComboBox.EnableWindow(FALSE);
	::EnableWindow(GetDlgItem(IDOK),FALSE);

	m_ComboBox.SetCurSel(0);
}

LRESULT CMainDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Center the dialog on the screen.
	CenterWindow();

	m_ListView = GetDlgItem(IDC_LIST);
	m_ComboBox = GetDlgItem(IDC_FORMATCOMBO);

	// Set icons.
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON,::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
	SetIcon(hIconSmall,FALSE);

	// Register object for message filtering and idle updates.
	CMessageLoop *pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	// Initialize the list view.
	InitializeListView();

	// Setup controls.
	::SendMessage(GetDlgItem(IDC_INPUTEDIT),EM_SETLIMITTEXT,MAX_PATH - 1,0);
	::SendMessage(GetDlgItem(IDC_OUTPUTEDIT),EM_SETLIMITTEXT,MAX_PATH - 1,0);

	// Temp...
	//SetDlgItemText(IDC_INPUTEDIT,_T("C:\\Temp\\AudioFiles\\Test2.wav"));
	//SetDlgItemText(IDC_OUTPUTEDIT,_T("C:\\Temp\\AudioOut\\"));
	// Temp...

	// Load the codecs.
	TCHAR szCodecPath[MAX_PATH];
    GetModuleFileName(NULL,szCodecPath,MAX_PATH - 1);

	ExtractFilePath(szCodecPath);
	lstrcat(szCodecPath,_T("Codecs\\"));

	if (g_CodecManager.LoadCodecs(szCodecPath))
	{
		// Fill the codec list view.
		FillListView();

		// Fill the target format combo box.
		FillComboBox();
	}
	else
	{
		MessageBox(_T("Unable to load codecs."),_T("Error"),MB_OK | MB_ICONERROR);

		DisableApp();
	}

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	MessageBox(_T("Copyright © 2006-2008 Christian Kindahl"),_T("About"),MB_OK | MB_ICONINFORMATION);
	return 0;
}

void CMainDlg::BeginProcess()
{
	::ShowWindow(GetDlgItem(IDC_INPUTSTATIC),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_INPUTEDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_INPUTBUTTON),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_OUTPUTSTATIC),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_OUTPUTEDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_OUTPUTBUTTON),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_FORMATSTATIC),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_FORMATCOMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_OPTIONSSTATIC),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_PROGRESS),SW_SHOW);
	SendDlgItemMessage(IDC_PROGRESS,PBM_SETRANGE32,0,100);
}

void CMainDlg::EndProcess()
{
	::ShowWindow(GetDlgItem(IDC_INPUTSTATIC),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_INPUTEDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_INPUTBUTTON),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_OUTPUTSTATIC),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_OUTPUTEDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_OUTPUTBUTTON),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_FORMATSTATIC),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_FORMATCOMBO),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_OPTIONSSTATIC),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_PROGRESS),SW_HIDE);
}

LRESULT CMainDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	TCHAR szSourceFile[MAX_PATH];
	GetDlgItemText(IDC_INPUTEDIT,szSourceFile,MAX_PATH - 1);

	// Display the progress bar.
	BeginProcess();

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

		if (g_CodecManager.m_Codecs[i]->irc_decode_init(szSourceFile,iNumChannels,
			iSampleRate,iBitRate,uiDuration))
		{
			pDecoder = g_CodecManager.m_Codecs[i];
			break;
		}
	}

	if (pDecoder == NULL)
	{
		MessageBox(_T("There is no installed codec that support the specified source file."),_T("Error"),MB_OK | MB_ICONERROR);

		EndProcess();
		return 0;
	}

	// Encoder.
	CCodec *pEncoder = (CCodec *)m_ComboBox.GetItemData(m_ComboBox.GetCurSel());

	// Setup target path.
	TCHAR szTargetFile[MAX_PATH],szTargetFileName[MAX_PATH];
	lstrcpy(szTargetFileName,szSourceFile);
	ExtractFileName(szTargetFileName);
	ChangeFileExt(szTargetFileName,pEncoder->irc_string(IRC_STR_FILEEXT));

	GetDlgItemText(IDC_OUTPUTEDIT,szTargetFile,MAX_PATH - 1);
	IncludeTrailingBackslash(szTargetFile);
	lstrcat(szTargetFile,szTargetFileName);

	// Initialize the encoder.
	if (!pEncoder->irc_encode_init(szTargetFile,iNumChannels,iSampleRate,iBitRate))
	{
		MessageBox(_T("Failed to initialize the encoder."),_T("Error"),MB_OK | MB_ICONERROR);

		EndProcess();
		return 0;
	}

	// Setup callbacks.
	pDecoder->irc_set_callback(CodecMessage);
	pEncoder->irc_set_callback(CodecMessage);

	// Encode/decode-process.
	__int64 iBytesRead = 0;
	unsigned __int64 uiCurrentTime = 0;

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
			MessageBox(_T("Failed to encode data."),_T("Error"),MB_OK | MB_ICONERROR);
			break;
		}

		// Update the progres bar.
		int iPercent = (int)(((double)uiCurrentTime/uiDuration) * 100);
		SendDlgItemMessage(IDC_PROGRESS,PBM_SETPOS,(WPARAM)iPercent,0);

		TCHAR szTemp[25];
		swprintf(szTemp,_T("%d"),iPercent);
		SetWindowText(szTemp);
	}

	// Free buffer memory.
	delete [] pBuffer;

	// Flush.
	pEncoder->irc_encode_flush();

	// Destroy the codecs.
	pEncoder->irc_encode_exit();
	pDecoder->irc_decode_exit();

	// Hide the progress bar.
	EndProcess();

	return 0;
}

LRESULT CMainDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int iVal)
{
	DestroyWindow();
	::PostQuitMessage(iVal);

	// Destroy the image list.
	if (m_hListImageList)
		ImageList_Destroy(m_hListImageList);
}

LRESULT CMainDlg::OnBnClickedInputbutton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("All Files (*.*)\0*.*\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
		SetDlgItemText(IDC_INPUTEDIT,FileDialog.m_szFileName);

	return 0;
}

LRESULT CMainDlg::OnBnClickedOutputbutton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CFolderDialog FolderDialog(m_hWnd,_T("Please select a target folder:"),BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS);
	
	if (FolderDialog.DoModal() == IDOK)
		SetDlgItemText(IDC_OUTPUTEDIT,FolderDialog.GetFolderPath());

	return 0;
}

LRESULT CMainDlg::OnBnClickedHelpbutton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	MessageBox(_T("A decoder will automatically be choosen to match the selected source file. If no such decoder can be found the process will be aborted."),_T("Help"),MB_OK | MB_ICONINFORMATION);
	return 0;
}

LRESULT CMainDlg::OnBnClickedEncoderConfigButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CCodec *pEncoder = (CCodec *)m_ComboBox.GetItemData(m_ComboBox.GetCurSel());
	pEncoder->irc_encode_config();

	return 0;
}

LRESULT CMainDlg::OnEncoderChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CCodec *pEncoder = (CCodec *)m_ComboBox.GetItemData(m_ComboBox.GetCurSel());
	::EnableWindow(GetDlgItem(IDC_ENCODERCONFIGBUTTON),pEncoder->irc_capabilities() & IRC_HAS_CONFIG);

	return 0;
}

LRESULT CMainDlg::OnListDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (m_ListView.GetSelectedCount() > 0)
	{
		unsigned int uiCodec = m_ListView.GetSelectedIndex();

		MessageBox(g_CodecManager.m_Codecs[uiCodec]->irc_string(IRC_STR_ABOUT),_T("Information"),MB_OK | MB_ICONINFORMATION);
	}

	bHandled = false;
	return 0;
}
