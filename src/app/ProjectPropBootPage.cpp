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

#include "stdafx.h"
#include "ProjectPropBootPage.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "WinVer.h"
#include "AddBootImageDlg.h"
#include "ProjectManager.h"
#include "TransUtil.h"

CProjectPropBootPage::CProjectPropBootPage()
{
	m_hToolBarImageList = NULL;

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TITLE_BOOT,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropBootPage::~CProjectPropBootPage()
{
	if (m_hToolBarImageList)
		ImageList_Destroy(m_hToolBarImageList);
}

bool CProjectPropBootPage::Translate()
{
	if (g_LanguageSettings.m_pLngProcessor == NULL)
		return false;

	CLngProcessor *pLng = g_LanguageSettings.m_pLngProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLng->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	if (pLng->GetValuePtr(IDC_BOOTSTATIC,szStrValue))
		SetDlgItemText(IDC_BOOTSTATIC,szStrValue);

	return true;
}

void CProjectPropBootPage::InitToolBarImageList()
{
	// Create the image list.
	HBITMAP hBitmap;
	
	if (g_WinVer.m_ulMajorCCVersion >= 6)
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MINITOOLBARBITMAP));

		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32,0,6);
		ImageList_Add(m_hToolBarImageList,hBitmap,NULL);
	}
	else
	{
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_MINITOOLBARBITMAP_));

		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
		ImageList_AddMasked(m_hToolBarImageList,hBitmap,RGB(255,0,255));
	}

	DeleteObject(hBitmap);
}

void CProjectPropBootPage::AddToolBarButton(int iCommand,int iBitmap)
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

void CProjectPropBootPage::CreateToolBarCtrl()
{
	RECT rcListView;
	::GetWindowRect(GetDlgItem(IDC_LIST),&rcListView);
	ScreenToClient(&rcListView);

	RECT rcToolBar = { 0,0,100,100 };
	m_ToolBar.Create(m_hWnd,rcToolBar,NULL,ATL_SIMPLE_TOOLBAR_PANE_STYLE,NULL);
	m_ToolBar.SetImageList(m_hToolBarImageList);
	m_ToolBar.SetButtonStructSize();

	// Create the buttons.
	AddToolBarButton(ID_BOOT_ADD,1);
	AddToolBarButton(ID_BOOT_REMOVE,3);
	AddToolBarButton(ID_BOOT_EDIT,6);
	m_ToolBar.EnableButton(ID_BOOT_REMOVE,false);
	m_ToolBar.EnableButton(ID_BOOT_EDIT,false);

	// Update the toolbar position.
	int iToolBarWidth = 0;
	RECT rcButton;

	for (int i = 0; i < m_ToolBar.GetButtonCount(); i++)
	{
		m_ToolBar.GetItemRect(i,&rcButton);
		iToolBarWidth += rcButton.right - rcButton.left;
	}

	m_ToolBar.SetWindowPos(NULL,
		rcListView.right - iToolBarWidth,
		rcListView.top - HIWORD(m_ToolBar.GetButtonSize()),
		iToolBarWidth,
		HIWORD(m_ToolBar.GetButtonSize()),0);
}

void CProjectPropBootPage::FillListView()
{
	int iItemCount = 0;

	std::list <CProjectBootImage *>::iterator itImageObject;
	for (itImageObject = g_ProjectSettings.m_BootImages.begin(); itImageObject != g_ProjectSettings.m_BootImages.end(); itImageObject++)
	{
		m_ListView.AddItem(iItemCount,0,LPSTR_TEXTCALLBACK);
		m_ListView.AddItem(iItemCount,1,LPSTR_TEXTCALLBACK);
		m_ListView.SetItemData(iItemCount++,(DWORD_PTR)*itImageObject);
	}
}

bool CProjectPropBootPage::OnApply()
{
	return true;
}

void CProjectPropBootPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CProjectPropBootPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Create the toolbar.
	InitToolBarImageList();
	CreateToolBarCtrl();

	// Initialize the list view.
	m_ListView = GetDlgItem(IDC_LIST);
	m_ListView.AddColumn(lngGetString(COLUMN_NAME),0);
	m_ListView.AddColumn(lngGetString(COLUMN_EMULATION),1);
	m_ListView.SetColumnWidth(0,253);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	// Translate the window.
	Translate();

	// Fill the list view.
	FillListView();

	return TRUE;
}

LRESULT CProjectPropBootPage::OnListAdd(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	bHandled = false;

	// The maximum number of allowed boot images is 63.
	if (m_ListView.GetItemCount() == 63)
	{
		MessageBox(lngGetString(ERROR_NUMBOOTIMAGES),lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
		return 0;
	}

	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("All Files (*.*)\0*.*\0\0"),m_hWnd);

	if (FileDialog.DoModal() == IDOK)
	{
		CProjectBootImage *pBootImage = new CProjectBootImage();
		pBootImage->m_FullPath = FileDialog.m_szFileName;

		ExtractFileName(FileDialog.m_szFileName);
		pBootImage->m_LocalName = FileDialog.m_szFileName;

		CAddBootImageDlg AddBootImageDlg(pBootImage,false);

		if (AddBootImageDlg.DoModal() == IDOK)
		{
			g_ProjectSettings.m_BootImages.push_back(pBootImage);

			int iItemCount = m_ListView.GetItemCount();

			m_ListView.AddItem(iItemCount,0,LPSTR_TEXTCALLBACK);
			m_ListView.AddItem(iItemCount,1,LPSTR_TEXTCALLBACK);
			m_ListView.SetItemData(iItemCount,(DWORD_PTR)pBootImage);

			return 0;
		}

		delete pBootImage;
	}

	return 0;
}

LRESULT CProjectPropBootPage::OnListRemove(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	bHandled = false;

	CProjectBootImage *pBootImage =
		reinterpret_cast<CProjectBootImage *>(m_ListView.GetItemData(m_ListView.GetSelectedIndex()));
	g_ProjectSettings.m_BootImages.remove(pBootImage);

	// Deltete the item from the list view.
	m_ListView.DeleteItem(m_ListView.GetSelectedIndex());

	return 0;
}

LRESULT CProjectPropBootPage::OnListEdit(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	int iSelected = m_ListView.GetSelectedIndex();
	CProjectBootImage *pBootImage = (CProjectBootImage *)m_ListView.GetItemData(iSelected);

	CAddBootImageDlg AddBootImageDlg(pBootImage,true);
	if (AddBootImageDlg.DoModal() == IDOK)
	{
		m_ListView.Update(iSelected);
	}

	bHandled = false;
	return 0;
}

LRESULT CProjectPropBootPage::OnToolBarGetInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
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

LRESULT CProjectPropBootPage::OnListItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (m_ToolBar.IsWindow())
	{
		if (m_ListView.GetSelectedCount() > 0)
		{
			m_ToolBar.EnableButton(ID_BOOT_REMOVE,true);
			m_ToolBar.EnableButton(ID_BOOT_EDIT,true);
		}
		else
		{
			m_ToolBar.EnableButton(ID_BOOT_REMOVE,false);
			m_ToolBar.EnableButton(ID_BOOT_EDIT,false);
		}
	}
	bHandled = false;
	return 0;
}

LRESULT CProjectPropBootPage::OnListGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMH;
	CProjectBootImage *pBootImage = (CProjectBootImage *)pDispInfo->item.lParam;

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		switch (pDispInfo->item.iSubItem)
		{
			case 0:
				lstrcpy(pDispInfo->item.pszText,pBootImage->m_FullPath.c_str());
				break;

			case 1:
				{
					switch (pBootImage->m_iEmulation)
					{
						case PROJECTBI_BOOTEMU_FLOPPY:
							lstrcpy(pDispInfo->item.pszText,lngGetString(BOOTEMU_FLOPPY));
							break;

						case PROJECTBI_BOOTEMU_HARDDISK:
							lstrcpy(pDispInfo->item.pszText,lngGetString(BOOTEMU_HARDDISK));
							break;

						default:
							lstrcpy(pDispInfo->item.pszText,lngGetString(BOOTEMU_NONE));
							break;
					}
				}
				break;
		}
	}

	bHandled = false;
	return 0;
}
