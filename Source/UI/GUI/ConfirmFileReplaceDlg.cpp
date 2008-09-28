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
#include <ckcore/convert.hh>
#include "ConfirmFileReplaceDlg.h"
#include "Settings.h"
#include "LangUtil.h"
#include "StringTable.h"
#include "MainFrm.h"

CConfirmFileReplaceDlg::CConfirmFileReplaceDlg()
{
	m_Mode = MODE_ASK;
	m_szNewFullPath = NULL;
	m_pNewItemData = NULL;
	m_pOldItemData = NULL;
}

CConfirmFileReplaceDlg::~CConfirmFileReplaceDlg()
{
}

bool CConfirmFileReplaceDlg::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is an erase translation section.
	if (!pLNG->EnterSection(_T("confirmfilereplace")))
		return false;

	// Translate.
	TCHAR *szStrValue;

	if (pLNG->GetValuePtr(IDD_CONFIRMFILEREPLACEDLG,szStrValue))		// Title.
		SetWindowText(szStrValue);
	if (pLNG->GetValuePtr(IDC_YESBUTTON,szStrValue))
		SetDlgItemText(IDC_YESBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_YESALLBUTTON,szStrValue))
		SetDlgItemText(IDC_YESALLBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_NOBUTTON,szStrValue))
		SetDlgItemText(IDC_NOBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_NOALLBUTTON,szStrValue))
		SetDlgItemText(IDC_NOALLBUTTON,szStrValue);
	if (pLNG->GetValuePtr(IDC_INFOSTATIC,szStrValue))
		SetDlgItemText(IDC_INFOSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_REPLACEINFOSTATIC,szStrValue))
		SetDlgItemText(IDC_REPLACEINFOSTATIC,szStrValue);
	if (pLNG->GetValuePtr(IDC_REPLACEINFO2STATIC,szStrValue))
		SetDlgItemText(IDC_REPLACEINFO2STATIC,szStrValue);

	return true;
}

/*
	CConfirmFileReplaceDlg::Reset
	-----------------------------
	Resets the dialog box causing the next call to Execute to display the dialog.
*/
void CConfirmFileReplaceDlg::Reset()
{
	m_Mode = MODE_ASK;
}

bool CConfirmFileReplaceDlg::Execute()
{
	bool bResult = false;

	switch (DoModal(g_MainFrame))	// It's okay if g_MainFrame.m_hWnd is NULL.
	{
		case IDC_YESBUTTON:
			bResult = true;
			break;
		case IDC_YESALLBUTTON:
			m_Mode = MODE_YESALL;
			bResult = true;
			break;
		case IDC_NOBUTTON:
			break;
		case IDC_NOALLBUTTON:
			m_Mode = MODE_NOALL;
			break;
	};

	return bResult;
}

/*
	CConfirmFileReplaceDlg::Execute
	-------------------------------
	Returns true if the file is selected to be replaced and false if it isn't.
	This function will determine if it's necessary to display the dialog at
	all. The user might have pressed the "No to All" or "Yes to All" buttons.
*/
bool CConfirmFileReplaceDlg::Execute(const TCHAR *szNewFullPath,CItemData *pOldItemData)
{
	// Should we ask the user?
	if (m_Mode != MODE_ASK)
		return m_Mode == MODE_YESALL ? true : false;

	m_szNewFullPath = szNewFullPath;
	m_pNewItemData = NULL;
	m_pOldItemData = pOldItemData;

	return Execute();
}

bool CConfirmFileReplaceDlg::Execute(CItemData *pNewItemData,CItemData *pOldItemData)
{
	// Should we ask the user?
	if (m_Mode != MODE_ASK)
		return m_Mode == MODE_YESALL ? true : false;

	m_szNewFullPath = NULL;
	m_pNewItemData = pNewItemData;
	m_pOldItemData = pOldItemData;

	return Execute();
}

LRESULT CConfirmFileReplaceDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	CenterWindow(GetParent());

	// Make sure that all necessary data has been supplied.
	if ((m_szNewFullPath == NULL && m_pNewItemData == NULL) || m_pOldItemData == NULL)
		return TRUE;

	// Set the copy icon.
	::SendMessage(GetDlgItem(IDC_ICONSTATIC),STM_SETICON,
		(WPARAM)LoadIcon(LoadLibrary(_T("shell32.dll")),MAKEINTRESOURCE(146)),0L);

	// Translate the window.
	Translate();

	// File name.
	TCHAR szNewFileText[128];
	::GetWindowText(GetDlgItem(IDC_INFOSTATIC),szNewFileText,
		sizeof(szNewFileText) - 2 - lstrlen(m_pOldItemData->GetFileName()));
	lstrcat(szNewFileText,_T("\n"));
	lstrcat(szNewFileText,m_pOldItemData->GetFileName());
	::SetWindowText(GetDlgItem(IDC_INFOSTATIC),szNewFileText);

	// Set old icon.
	SHFILEINFO shInfo;
	if (SHGetFileInfo(m_pOldItemData->szFullPath,0,&shInfo,sizeof(shInfo),
		SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
	{
		::SendMessage(GetDlgItem(IDC_OLDICONSTATIC),STM_SETICON,(WPARAM)shInfo.hIcon,0L);
	}

	// Set old size.
	TCHAR szTempBuffer[128];
	FormatBytes(szTempBuffer,m_pOldItemData->uiSize);
	::SetWindowText(GetDlgItem(IDC_OLDSIZESTATIC),szTempBuffer);

	// Set old date.
	FILETIME ft;
	SYSTEMTIME st;

	TCHAR *szDatePattern = new TCHAR[lstrlen(lngGetString(MISC_MODIFIED)) + 24];
	lstrcpy(szDatePattern,_T("'"));
	lstrcat(szDatePattern,lngGetString(MISC_MODIFIED));
	lstrcat(szDatePattern,_T(" 'dddd',' dd MMMM yyyy"));

	::DosDateTimeToFileTime(m_pOldItemData->usFileDate,m_pOldItemData->usFileTime,&ft);
	::FileTimeToSystemTime(&ft,&st);
	::GetDateFormat(LOCALE_USER_DEFAULT,0,&st,szDatePattern,szTempBuffer,sizeof(szTempBuffer));
	::SetWindowText(GetDlgItem(IDC_OLDDATESTATIC),szTempBuffer);

	if (m_szNewFullPath != NULL)
	{
		// Set new icon.
		if (SHGetFileInfo(m_szNewFullPath,0,&shInfo,sizeof(shInfo),
			SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
		{
			::SendMessage(GetDlgItem(IDC_NEWICONSTATIC),STM_SETICON,(WPARAM)shInfo.hIcon,0L);
		}

		// Set new size.
		FormatBytes(szTempBuffer,ckcore::File::Size(m_szNewFullPath));
		::SetWindowText(GetDlgItem(IDC_NEWSIZESTATIC),szTempBuffer);

		// Set new date.
		unsigned short usFileDate = 0,usFileTime = 0;

		struct tm AccessTime,ModifyTime,CreateTime;
		ckcore::File::Time(m_szNewFullPath,AccessTime,ModifyTime,CreateTime);
		ckcore::convert::tm_to_dostime(ModifyTime,usFileDate,usFileTime);

		::DosDateTimeToFileTime(usFileDate,usFileTime,&ft);
		::FileTimeToSystemTime(&ft,&st);
		::GetDateFormat(LOCALE_USER_DEFAULT,0,&st,szDatePattern,szTempBuffer,sizeof(szTempBuffer));
		::SetWindowText(GetDlgItem(IDC_NEWDATESTATIC),szTempBuffer);
	}
	else
	{
		// Set new icon.
		if (SHGetFileInfo(m_pNewItemData->szFullPath,0,&shInfo,sizeof(shInfo),
			SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
		{
			::SendMessage(GetDlgItem(IDC_NEWICONSTATIC),STM_SETICON,(WPARAM)shInfo.hIcon,0L);
		}

		// Set new size.
		FormatBytes(szTempBuffer,m_pNewItemData->uiSize);
		::SetWindowText(GetDlgItem(IDC_NEWSIZESTATIC),szTempBuffer);

		// Set new date.
		::DosDateTimeToFileTime(m_pNewItemData->usFileDate,m_pNewItemData->usFileTime,&ft);
		::FileTimeToSystemTime(&ft,&st);
		::GetDateFormat(LOCALE_USER_DEFAULT,0,&st,szDatePattern,szTempBuffer,sizeof(szTempBuffer));
		::SetWindowText(GetDlgItem(IDC_NEWDATESTATIC),szTempBuffer);	
	}

	delete [] szDatePattern;

	return TRUE;
}

LRESULT CConfirmFileReplaceDlg::OnButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	EndDialog(wID);
	return FALSE;
}
