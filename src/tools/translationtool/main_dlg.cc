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

#include "stdafx.hh"
#include "resource.h"
#include "lng_analyzer.hh"
#include "main_dlg.hh"

BOOL CMainDlg::PreTranslateMessage(MSG *pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

void CMainDlg::CloseDialog(int iVal)
{
	DestroyWindow();
	::PostQuitMessage(iVal);
}

LRESULT CMainDlg::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Center the dialog on the screen.
	CenterWindow();

	DlgResize_Init(true,true,WS_CLIPCHILDREN);

	// Set icons.
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON),LR_DEFAULTCOLOR);
	SetIcon(hIcon,TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON,::GetSystemMetrics(SM_CXSMICON),::GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
	SetIcon(hIconSmall,FALSE);

	// Register object for message filtering and idle updates.
	CMessageLoop *pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	// Setup controls.
	::SendMessage(GetDlgItem(IDC_REFEDIT),EM_SETLIMITTEXT,MAX_PATH - 1,0);
	::EnableWindow(GetDlgItem(IDC_EXPORTBUTTON),FALSE);
	m_TransList = GetDlgItem(IDC_TRANSLIST);

	return TRUE;
}

LRESULT CMainDlg::OnDropFiles(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	HDROP hDrop = (HDROP)wParam;

	POINT ptDrop;
	TCHAR szFullName[MAX_PATH];

	if (DragQueryPoint(hDrop,&ptDrop) > 0)
	{
		unsigned int uiNumFiles = DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);

		for (unsigned int i = 0; i < uiNumFiles; i++)
		{
			if (DragQueryFile(hDrop,i,szFullName,MAX_PATH - 1))
				m_TransList.AddString(szFullName);
		}
	}

	DragFinish(hDrop);

	bHandled = false;
	return 0;
}

bool CMainDlg::AnalyzeTranslation(const TCHAR *szFileName,CXmlProcessor *pXml)
{
	::EnableWindow(GetDlgItem(IDC_EXPORTBUTTON),FALSE);

	TCHAR szRefFile[MAX_PATH];
	GetDlgItemText(IDC_REFEDIT,szRefFile,MAX_PATH - 1);

	CLngAnalyzer RefFile(szRefFile);
	CLngAnalyzer LangFile(szFileName);

	if (RefFile.Load() != LNGRES_OK)
		return false;

	if (LangFile.Load() != LNGRES_OK)
		return false;

	// Add some translation information.
	pXml->AddElement(_T("Header"),_T(""),true);
		if (LangFile.EnterSection(_T("translation")))
		{
			TCHAR *szStrValue;
			// Author.
			if (LangFile.GetValuePtr(1,szStrValue))
				pXml->AddElement(_T("Author"),szStrValue);
			else
				pXml->AddElement(_T("Author"),_T("Unknown"));

			// Date.
			if (LangFile.GetValuePtr(2,szStrValue))
				pXml->AddElement(_T("Date"),szStrValue);
			else
				pXml->AddElement(_T("Date"),_T("Unknown"));

			// Version.
			if (LangFile.GetValuePtr(3,szStrValue))
				pXml->AddElement(_T("Version"),szStrValue);
			else
				pXml->AddElement(_T("Version"),_T("Unknown"));

			// Help.
			if (LangFile.GetValuePtr(4,szStrValue))
				pXml->AddElement(_T("Help"),_T("Yes"));
			else
				pXml->AddElement(_T("Help"),_T("No"));
		}
		else
		{
			pXml->AddElement(_T("Author"),_T("Unknown"));
			pXml->AddElement(_T("Version"),_T("Unknown"));
			pXml->AddElement(_T("Date"),_T("Unknown"));
			pXml->AddElement(_T("Help"),_T("Unknown"));
		}

		// Ignore the values in the translation section.
		unsigned int uiRefTransCount = 0;
		CLngSection *pTempSection = RefFile.GetSection(_T("translation"));
		if (pTempSection != NULL)
			uiRefTransCount = (unsigned int)pTempSection->m_Values.size();

		unsigned int uiLangTransCount = 0;
		pTempSection = LangFile.GetSection(_T("translation"));
		if (pTempSection != NULL)
			uiLangTransCount = (unsigned int)pTempSection->m_Values.size();

		pXml->AddElement(_T("SectionRatio"),(double)LangFile.GetNumSections()/RefFile.GetNumSections());
		pXml->AddElement(_T("ValueRatio"),(double)(LangFile.GetNumValues() - uiLangTransCount)/(RefFile.GetNumValues() - uiRefTransCount));
	pXml->LeaveElement();

	pXml->AddElement(_T("Missing"),_T(""),true);
		// Perform the comparission.
		TCHAR szBuffer[32];
		unsigned int uiSectionCount = 0;

		for (unsigned int i = 0; i < RefFile.GetNumSections(); i++)
		{
			CLngSection *pRefSection = RefFile.GetSection(i);
			if (pRefSection == NULL)
				return false;

			// Skip the [translation] section.
			if (!lstrcmp(pRefSection->m_szName,_T("translation")))
				continue;

			CLngSection *pLangSection = LangFile.GetSection(pRefSection->m_szName);
			if (pLangSection == NULL)
			{
				lsprintf(szBuffer,_T("Item%d"),uiSectionCount++);

				pXml->AddElement(szBuffer,_T(""),true);
				pXml->AddElementAttr(_T("name"),pRefSection->m_szName);
					for (unsigned int j = 0; j < pRefSection->m_Values.size(); j++)
					{
						lsprintf(szBuffer,_T("Item%d"),j);

						pXml->AddElement(szBuffer,pRefSection->m_Values[j]->m_szValue,true);
						pXml->AddElementAttr(_T("name"),(long)pRefSection->m_Values[j]->ulName);
						pXml->LeaveElement();
					}
				pXml->LeaveElement();
			}
			else
			{
				bool bAddSection = false;
				unsigned int uiValueCount = 0;

				// Compare the section values.
				for (unsigned int j = 0; j < pRefSection->m_Values.size(); j++)
				{
					if (!LangFile.SectionHasValue(pLangSection,pRefSection->m_Values[j]->ulName))
					{
						// Add a new section if it hasn't already been added.
						if (!bAddSection)
						{
							lsprintf(szBuffer,_T("Item%d"),uiSectionCount++);

							pXml->AddElement(szBuffer,_T(""),true);
							pXml->AddElementAttr(_T("name"),pRefSection->m_szName);

							bAddSection = true;
						}

						lsprintf(szBuffer,_T("Item%d"),uiValueCount++);

						pXml->AddElement(szBuffer,pRefSection->m_Values[j]->m_szValue,true);
						pXml->AddElementAttr(_T("name"),(long)pRefSection->m_Values[j]->ulName);
						pXml->LeaveElement();
					}
				}

				// Don't forget to leave any entered sections.
				if (bAddSection)
					pXml->LeaveElement();
			}
		}
	pXml->LeaveElement();

	return true;
}

LRESULT CMainDlg::OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	for (int i = 0; i < m_TransList.GetCount(); i++)
	{
		TCHAR szTransFileName[MAX_PATH];
		m_TransList.GetText(i,szTransFileName);

		// Analyze the current translation.
		CXmlProcessor Xml(CXmlProcessor::MODE_HTML);
		Xml.AddElement(_T("InfraRecorder"),_T(""),true);
			Xml.AddElement(_T("Translation"),_T(""),true);

				AnalyzeTranslation(szTransFileName,&Xml);

			Xml.LeaveElement();
		Xml.LeaveElement();

		// Calculate new file name.
		ExtractFileName(szTransFileName);
		ChangeFileExt(szTransFileName,_T(".xml"));

#ifdef UNICODE
		szTransFileName[0] = towlower(szTransFileName[0]);
#else
		szTransFileName[0] = tolower(szTransFileName[0]);
#endif

		TCHAR szFileName[MAX_PATH];
		::GetModuleFileName(NULL,szFileName,MAX_PATH - 1);
		ExtractFilePath(szFileName);
		lstrcat(szFileName,szTransFileName);

		// Save the Xml document.
		Xml.Save(szFileName);
	}

	return 0;
}

LRESULT CMainDlg::OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnClickedBrowseButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Infra Recorder Languages (*.irl)\0*.irl\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
		SetDlgItemText(IDC_REFEDIT,FileDialog.m_szFileName);

	return 0;
}

LRESULT CMainDlg::OnClickedTransAddButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Infra Recorder Languages (*.irl)\0*.irl\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
		m_TransList.AddString(FileDialog.m_szFileName);

	return 0;
}

LRESULT CMainDlg::OnClickedTransRemoveButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	if (m_TransList.GetCount() > 0 && m_TransList.GetCurSel() >= 0)
		m_TransList.DeleteString(m_TransList.GetCurSel());

	return 0;
}

LRESULT CMainDlg::OnClickedTransClearButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	m_TransList.ResetContent();

	return 0;
}
