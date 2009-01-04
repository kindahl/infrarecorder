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

#include "stdafx.h"
#include "resource.h"
#include "AboutDlg.h"
#include "../../Common/StringUtil.h"
#include "InfraRecorder.h"
#include "Settings.h"
#include "LangUtil.h"

CAboutDlg::CAboutDlg()
{
}

CAboutDlg::~CAboutDlg()
{
	if (m_hCodecImageList)
		ImageList_Destroy(m_hCodecImageList);
}

void CAboutDlg::UpdateVersionInfo()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	unsigned long ulDummy;
	unsigned long ulDataSize = GetFileVersionInfoSize(szFileName,&ulDummy);

	if (ulDataSize != 0)
	{
		unsigned char *pBlock = new unsigned char[ulDataSize];

		struct LANGANDCODEPAGE
		{
			unsigned short usLanguage;
			unsigned short usCodePage;
		} *pTranslate;

		if (GetFileVersionInfo(szFileName,NULL,ulDataSize,pBlock) > 0)
		{
			// Get language information (only one language should be present).
			VerQueryValue(pBlock,_T("\\VarFileInfo\\Translation"),
				(LPVOID *)&pTranslate,(unsigned int *)&ulDummy);

			// Calculate the FileVersion sub block path.
			TCHAR szStrBuffer[128];
			lsprintf(szStrBuffer,_T("\\StringFileInfo\\%04x%04x\\FileVersion"),
				pTranslate[0].usLanguage,pTranslate[0].usCodePage);

			unsigned char *pBuffer;
			VerQueryValue(pBlock,szStrBuffer,(LPVOID *)&pBuffer,(unsigned int *)&ulDataSize);

			// Windows version.
#if (_WIN32_WINNT >= 0x500)
			TCHAR *szWindowsStr = _T("2000/XP/Vista");
#else
			TCHAR *szWindowsStr = _T("9x/ME/2000/XP/Vista");
#endif

			// Architecture.
#ifdef _M_IA64
			TCHAR *szArcStr = _T("IA64");
#elif defined _M_X64
			TCHAR *szArcStr = _T("x64");
#else
			TCHAR *szArcStr = _T("x86");
#endif

			// Character coding.
#ifdef PORTABLE
			lsprintf(szStrBuffer,_T("Version %s portable (unicode, %s)\n for Windows %s"),
				(TCHAR *)pBuffer,szArcStr,szWindowsStr);
#elif UNICODE
			lsprintf(szStrBuffer,_T("Version %s (unicode, %s)\n for Windows %s"),
				(TCHAR *)pBuffer,szArcStr,szWindowsStr);
#else
			lsprintf(szStrBuffer,_T("Version %s\n for Windows %s"),(TCHAR *)pBuffer,szWindowsStr);
#endif

			// Append translator information if possible.
			if (g_LanguageSettings.m_pLNGProcessor != NULL)
			{	
				// Make sure that there is a strings translation section.
				if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("translation")))
				{
					TCHAR *szStrValue;
					if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TRANSLATION_ID_AUTHOR,szStrValue))
					{
						lstrcat(szStrBuffer,_T("\n\nTranslated by "));
						lstrcat(szStrBuffer,szStrValue);
					}
				}
			}

			SetDlgItemText(IDC_VERSIONSTATIC,szStrBuffer);
		}

		delete [] pBlock;
	}
}

void CAboutDlg::InitCodecListView()
{
	// Create the image list.
	m_hCodecImageList = NULL;
	HINSTANCE hInstance = LoadLibrary(_T("shell32.dll"));
		HICON hIcon = (HICON)LoadImage(hInstance,MAKEINTRESOURCE(278),IMAGE_ICON,16,16,LR_LOADTRANSPARENT);
	FreeLibrary(hInstance);

	m_hCodecImageList = ImageList_Create(16,16,ILC_COLOR32,0,1);
	ImageList_AddIcon(m_hCodecImageList,hIcon);

	DestroyIcon(hIcon);

	// Setup the list view.
	m_CodecList.SetImageList(m_hCodecImageList,LVSIL_NORMAL);
	m_CodecList.SetImageList(m_hCodecImageList,LVSIL_SMALL);
	m_CodecList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Add the columns.
	m_CodecList.AddColumn(_T("Name"),0);
	m_CodecList.SetColumnWidth(0,150);
	m_CodecList.AddColumn(_T("Version"),1,-1,LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,LVCFMT_RIGHT);
	m_CodecList.SetColumnWidth(1,70);
}

void CAboutDlg::FillCodecListView()
{
	TCHAR szFileName[MAX_PATH];
	int iItemCount = 0;

	for (unsigned int i = 0; i < g_CodecManager.m_Codecs.size(); i++)
	{
		if (g_CodecManager.m_Codecs[i]->GetFileName(szFileName,MAX_PATH - 1))
		{
			ExtractFileName(szFileName);
			m_CodecList.AddItem(iItemCount,0,szFileName,0);
			m_CodecList.AddItem(iItemCount,1,g_CodecManager.m_Codecs[i]->irc_string(IRC_STR_VERSION),0);

			iItemCount++;
		}
	}
}

LRESULT CAboutDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// Create a smaller font of the system font.
	HFONT hSmallFont = NULL;
	LOGFONT lf = { 0 };

	if (::GetObject(AtlGetDefaultGuiFont(),sizeof(LOGFONT),&lf) == sizeof(LOGFONT))
	{
		lf.lfHeight = -8;
		hSmallFont = ::CreateFontIndirect(&lf);

		::SendMessage(GetDlgItem(IDC_VERSIONSTATIC),WM_SETFONT,(WPARAM)hSmallFont,TRUE);
	}

	// Update the version information.
	UpdateVersionInfo();

	// Initialize the codec list view.
	m_CodecList = GetDlgItem(IDC_CODECLIST);
	InitCodecListView();
	FillCodecListView();

	return TRUE;
}

LRESULT CAboutDlg::OnCtlColorStatic(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	HDC hDC = (HDC)wParam;

	// Begin: Fix Windows XP bug.
	RECT rcRect;
	::GetClientRect((HWND)lParam,&rcRect);

	HDC hSrcDC = ::GetDC((HWND)lParam);
	BitBlt(hDC,0,0,rcRect.right - rcRect.left,rcRect.bottom - rcRect.top,
		hSrcDC,rcRect.left,rcRect.top,SRCCOPY);
	ReleaseDC(hSrcDC);
	// End.

	::SetBkMode(hDC,TRANSPARENT);

	bHandled = true;
	//return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
	return (LRESULT)GetStockObject(HOLLOW_BRUSH);
}

LRESULT CAboutDlg::OnClose(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CAboutDlg::OnWebsite(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	::ShellExecute(m_hWnd,_T("open"),_T("http://infrarecorder.org"),NULL,NULL,SW_SHOW);
	return 0;
}

LRESULT CAboutDlg::OnCodecListDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (m_CodecList.GetSelectedCount() > 0)
	{
		unsigned int uiCodec = m_CodecList.GetSelectedIndex();

		MessageBox(g_CodecManager.m_Codecs[uiCodec]->irc_string(IRC_STR_ABOUT),_T("Information"),MB_OK | MB_ICONINFORMATION);
	}

	bHandled = false;
	return 0;
}
