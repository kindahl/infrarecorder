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
#include "toolbar_manager.hh"
#include "settings.hh"
#include "string_table.hh"
#include "lang_util.hh"
#include "trans_util.hh"
#include "main_frm.hh"

CToolBarManager g_ToolBarManager;

CToolBarManager::CCustomizeDlg::CCustomizeDlg(int iText,int iIcon)
{
	m_iText = iText;
	m_iIcon = iIcon;
}

CToolBarManager::CCustomizeDlg::~CCustomizeDlg()
{
}

LRESULT CToolBarManager::CCustomizeDlg::OnInitDialog(UINT uMsg,WPARAM wParam,
													 LPARAM lParam,BOOL &bHandled)
{
	// Increase the window height.
	RECT rcWindow,rcClient;
	GetWindowRect(&rcWindow);
	GetClientRect(&rcClient);

	rcWindow.bottom += 58;
	SetWindowPos(0,0,0,rcWindow.right - rcWindow.left,rcWindow.bottom - rcWindow.top,SWP_NOZORDER | SWP_NOMOVE);

	// Create the text options label.
	RECT rcTextStatic = { 6,rcClient.bottom + 2,80,rcClient.bottom + 19 };
	HWND hWndStatic = m_TextStatic.Create(m_hWnd,rcTextStatic,lngGetString(TBCUSTOMIZE_TEXTOPTIONS),
		WS_CHILD | WS_VISIBLE | WS_GROUP);
	m_TextStatic.SetFont(AtlGetDefaultGuiFont());
	int iTextRight = UpdateStaticWidth(m_hWnd,hWndStatic,lngGetString(TBCUSTOMIZE_TEXTOPTIONS));

	// Create the text options combo box.
	RECT rcTextCombo = { 80,rcClient.bottom,263,rcClient.bottom + 200 };
	m_TextComboBox.Create(m_hWnd,rcTextCombo,NULL,WS_CHILD | WS_VISIBLE |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL,
		0,IDC_TBCUSTOMIZE_TEXTCOMBO);
	m_TextComboBox.SetFont(AtlGetDefaultGuiFont());
	m_TextComboBox.AddString(lngGetString(TBCUSTOMIZE_SHOWTEXT));
	m_TextComboBox.AddString(lngGetString(TBCUSTOMIZE_SHOWTEXTRIGHT));
	m_TextComboBox.AddString(lngGetString(TBCUSTOMIZE_NOTEXT));
	m_TextComboBox.SetCurSel(m_iText);

	// Create the icon options label.
	RECT rcIconStatic = { 6,rcClient.bottom + 31,80,rcClient.bottom + 48 };
	hWndStatic = m_IconStatic.Create(m_hWnd,rcIconStatic,lngGetString(TBCUSTOMIZE_ICONOPTIONS),
		WS_CHILD | WS_VISIBLE | WS_GROUP);
	m_IconStatic.SetFont(AtlGetDefaultGuiFont());
	int iIconRight = UpdateStaticWidth(m_hWnd,hWndStatic,lngGetString(TBCUSTOMIZE_ICONOPTIONS));

	// Create the icon options combo box.
	RECT rcIconCombo = { 80,rcClient.bottom + 29,263,rcClient.bottom + 200 };
	m_IconComboBox.Create(m_hWnd,rcIconCombo,NULL,WS_CHILD | WS_VISIBLE |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL,
		0,IDC_TBCUSTOMIZE_ICONCOMBO);
	m_IconComboBox.SetFont(AtlGetDefaultGuiFont());
	m_IconComboBox.AddString(lngGetString(TBCUSTOMIZE_ICONSMALL));
	m_IconComboBox.AddString(lngGetString(TBCUSTOMIZE_ICONLARGE));
	m_IconComboBox.SetCurSel(m_iIcon);

	// Update the static width if necessary.
	int iMaxRight = iTextRight > iIconRight ? iTextRight : iIconRight;
	if (iMaxRight > 80)
	{
		UpdateEditPos(m_hWnd,IDC_TBCUSTOMIZE_TEXTCOMBO,iMaxRight,true);
		UpdateEditPos(m_hWnd,IDC_TBCUSTOMIZE_ICONCOMBO,iMaxRight,true);
	}

	bHandled = false;
	return 0;
}

LRESULT CToolBarManager::CCustomizeDlg::OnTCSelChange(WORD wNotifyCode,WORD wID,
													  HWND hWndCtl,BOOL &bHandled)
{
	m_iText = m_TextComboBox.GetCurSel();

	bHandled = false;
	return 0;
}

LRESULT CToolBarManager::CCustomizeDlg::OnICSelChange(WORD wNotifyCode,WORD wID,
													  HWND hWndCtl,BOOL &bHandled)
{
	m_iIcon = m_IconComboBox.GetCurSel();

	bHandled = false;
	return 0;
}

int CToolBarManager::CCustomizeDlg::GetTextOption()
{
	return m_iText;
}

int CToolBarManager::CCustomizeDlg::GetIconOption()
{
	return m_iIcon;
}

CToolBarManager::CToolBarManager()
{
	m_pToolBar = NULL;
	Reset();
}

CToolBarManager::~CToolBarManager()
{
}

void CToolBarManager::AddSeparator()
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_SEP;
	tbButton.iBitmap = NULL;
	tbButton.idCommand = NULL;
	tbButton.iString = NULL;
	tbButton.dwData = NULL;

	m_Buttons[ID_TOOLBAR_SEPARATOR] = tbButton;
}

void CToolBarManager::AddButton(int iCommand,int iBitmap,int iString)
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE;
	tbButton.iBitmap = iBitmap;
	tbButton.idCommand = iCommand;
	tbButton.iString = NULL;
	tbButton.dwData = iString;

	m_Buttons[iCommand] = tbButton;
}

void CToolBarManager::AddDropDownButton(int iBitmap,int iCommand,int iString)
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_BUTTON | BTNS_DROPDOWN | TBSTYLE_AUTOSIZE;
	tbButton.iBitmap = iBitmap;
	tbButton.idCommand = iCommand;
	tbButton.iString = NULL;
	tbButton.dwData = iString;

	m_Buttons[iCommand] = tbButton;
}

void CToolBarManager::Reset()
{
	m_Buttons.clear();
	m_SelButtons.clear();

	// Add all different buttons to the internal button structure.
	AddButton(ID_FILE_OPEN,0,TOOLBAR_OPEN);
	AddButton(ID_FILE_SAVE,1,TOOLBAR_SAVE);
	AddButton(ID_FILE_PROJECTPROPERTIES,3,TOOLBAR_PROJECTPROPERTIES);
	AddButton(ID_APP_EXIT,4,TOOLBAR_EXIT);
	AddButton(ID_BURNCOMPILATION_COMPACTDISC,8,TOOLBAR_BURNCOMPILATION);
	AddButton(ID_ACTIONS_BURNIMAGE,9,TOOLBAR_BURNIMAGE);
	AddButton(ID_COPYDISC_COMPACTDISC,10,TOOLBAR_COPY);
	AddButton(ID_ACTIONS_MANAGETRACKS,11,TOOLBAR_TRACKS);
	AddButton(ID_ACTIONS_ERASERE,12,TOOLBAR_ERASE);
	AddButton(ID_ACTIONS_FIXATEDISC,13,TOOLBAR_FIXATE);
	AddButton(ID_VIEW_PROGRAMLOG,14,TOOLBAR_LOG);
	AddButton(ID_OPTIONS_CONFIGURATION,15,TOOLBAR_CONFIGURATION);
	AddButton(ID_OPTIONS_DEVICES,16,TOOLBAR_DEVICES);
	AddButton(ID_HELP_HELPTOPICS,17,TOOLBAR_HELP);
	AddButton(ID_APP_ABOUT,18,TOOLBAR_ABOUT);
	AddSeparator();

	// Setup the default toolbar button configuration.
	m_SelButtons.push_back(ID_FILE_OPEN);
	m_SelButtons.push_back(ID_FILE_SAVE);
	m_SelButtons.push_back(ID_TOOLBAR_SEPARATOR);
	m_SelButtons.push_back(ID_BURNCOMPILATION_COMPACTDISC);
	m_SelButtons.push_back(ID_ACTIONS_BURNIMAGE);
	m_SelButtons.push_back(ID_COPYDISC_COMPACTDISC);
	m_SelButtons.push_back(ID_ACTIONS_ERASERE);
	m_SelButtons.push_back(ID_TOOLBAR_SEPARATOR);
	m_SelButtons.push_back(ID_OPTIONS_CONFIGURATION);
	m_SelButtons.push_back(ID_OPTIONS_DEVICES);
	m_SelButtons.push_back(ID_TOOLBAR_SEPARATOR);
	m_SelButtons.push_back(ID_APP_EXIT);
}

bool CToolBarManager::FillToolBarCtrl(CToolBarCtrl *pToolBar)
{
	m_pToolBar = pToolBar;

	pToolBar->SetButtonStructSize();

	std::vector<int>::const_iterator itButtons;
	int iIndex = 0;
	for (itButtons = m_SelButtons.begin(); itButtons !=
		m_SelButtons.end(); itButtons++,iIndex++)
	{
		// Should be use button text?
		if (g_DynamicSettings.m_iToolBarText == TOOLBAR_TEXT_DONTSHOW)
			m_Buttons[*itButtons].iString = (unsigned int)NULL;
		else
			m_Buttons[*itButtons].iString = (INT_PTR)lngGetString((unsigned int)m_Buttons[*itButtons].dwData);

		if (!pToolBar->InsertButton(iIndex,&m_Buttons[*itButtons]))
			return false;
	}

	return true;
}

bool CToolBarManager::Save(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	pXml->AddElement(_T("Buttons"),_T(""),true);
		TCHAR szTemp[16];

		int iIndex = 0;
		std::vector<int>::const_iterator itSelButton;
		for (itSelButton = m_SelButtons.begin(); itSelButton !=
			m_SelButtons.end(); itSelButton++,iIndex++)
		{
			lsprintf(szTemp,_T("Item%i"),iIndex);
			pXml->AddElement(szTemp,*itSelButton);
		}

	pXml->LeaveElement();
	return true;
}

bool CToolBarManager::Load(CXmlProcessor *pXml)
{
	if (pXml == NULL)
		return false;

	if (!pXml->EnterElement(_T("Buttons")))
		return false;

	m_SelButtons.clear();

	TCHAR szTemp[16];
	for (unsigned int i = 0; i < pXml->GetElementChildCount(); i++)
	{
		lsprintf(szTemp,_T("Item%i"),i);

		int iButton = -1;
		pXml->GetSafeElementData(szTemp,&iButton);
		m_SelButtons.push_back(iButton);
	}

	pXml->LeaveElement();
	return true;
}

bool CToolBarManager::Customize()
{
	if (m_pToolBar == NULL)
		return false;

	m_pCustomizeDlg = new CCustomizeDlg(g_DynamicSettings.m_iToolBarText,
		g_DynamicSettings.m_iToolBarIcon);
	m_hCBTHook = ::SetWindowsHookEx(WH_CBT,CBTProc,0,::GetCurrentThreadId());

	m_pToolBar->Customize();

	::UnhookWindowsHookEx(m_hCBTHook);
	m_hCBTHook = NULL;

	// Fix to make sure that the toolbar is appropriately updated according the its buttons.
	if (m_pCustomizeDlg->GetTextOption() == TOOLBAR_TEXT_SHOW &&
		g_DynamicSettings.m_iToolBarText != TOOLBAR_TEXT_SHOW)
	{
		SIZE sButton;
		sButton.cx = 50;
		sButton.cy = 50;
		m_pToolBar->SetButtonSize(sButton);
	}

	// Update the settings.
	g_DynamicSettings.m_iToolBarText = m_pCustomizeDlg->GetTextOption();
	g_DynamicSettings.m_iToolBarIcon = m_pCustomizeDlg->GetIconOption();

	delete m_pCustomizeDlg;
	m_pCustomizeDlg = NULL;

	// Update the toolbar style.
	unsigned long ulStyle = m_pToolBar->GetStyle() & (~TBSTYLE_LIST);
	ulStyle |= g_DynamicSettings.m_iToolBarText == TOOLBAR_TEXT_SHOWRIGHT ? TBSTYLE_LIST : 0;
	m_pToolBar->SetStyle(ulStyle);

	// Update the button selection map.
	m_SelButtons.clear();

	for (int i = 0; i < m_pToolBar->GetButtonCount(); i++)
	{
		TBBUTTON tbButton;
		m_pToolBar->GetButton(i,&tbButton);

		if (tbButton.fsStyle & TBSTYLE_SEP)
			m_SelButtons.push_back(ID_TOOLBAR_SEPARATOR);
		else
			m_SelButtons.push_back(tbButton.idCommand);
	}

	// Delete all buttons.
	for (int i = m_pToolBar->GetButtonCount() - 1; i >= 0; i--)
		m_pToolBar->DeleteButton(i);

	switch (g_DynamicSettings.m_iToolBarIcon)
	{
		case TOOLBAR_ICON_SMALL:
			m_pToolBar->SetImageList(g_pMainFrame->GetToolBarSmall());
			break;

		case TOOLBAR_ICON_LARGE:
			m_pToolBar->SetImageList(g_pMainFrame->GetToolBarLarge());
			break;
	}

	// Re-add all buttons.
	FillToolBarCtrl(m_pToolBar);

	// Update toolbar height.
	int iButtonHeight = HIWORD(m_pToolBar->GetButtonSize());

	RECT rcToolBar;
	m_pToolBar->GetWindowRect(&rcToolBar);
	GetParentWindow(m_pToolBar).ScreenToClient(&rcToolBar);
	m_pToolBar->MoveWindow(rcToolBar.left,rcToolBar.top,rcToolBar.right - rcToolBar.left,iButtonHeight);

	// Repaint.
	m_pToolBar->Invalidate();

	return true;
}

LRESULT CToolBarManager::OnToolBarBeginAdjust(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = false;
	return 0;
}

CToolBarManager::CCustomizeDlg *CToolBarManager::m_pCustomizeDlg = NULL;
HHOOK CToolBarManager::m_hCBTHook = NULL;

LRESULT CALLBACK CToolBarManager::CBTProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	try
	{
		if ((nCode == HCBT_CREATEWND) && (m_pCustomizeDlg) && (m_pCustomizeDlg->m_hWnd == NULL))
		{
			// Subclass standard customize toolbar dialog.
			HWND hWnd = (HWND)wParam;
			m_pCustomizeDlg->SubclassWindow(hWnd);
		}
	}
	catch (...)	// If exception is not caught then we can get in infinite loop as assert window is opened.
	{
		// Call next hook.
		LRESULT lResult = ::CallNextHookEx(m_hCBTHook,nCode,wParam,lParam);

		::UnhookWindowsHookEx(m_hCBTHook);
		m_hCBTHook = NULL;

		if (m_pCustomizeDlg)
		{
			delete m_pCustomizeDlg;
			m_pCustomizeDlg = NULL;
		}

		return lResult;
	}

    return ::CallNextHookEx(m_hCBTHook,nCode,wParam,lParam);
}

LRESULT CToolBarManager::OnToolBarInitCustomize(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	// Remove the help button.
	bHandled = true;
	return TBNRF_HIDEHELP;
}

LRESULT CToolBarManager::OnToolBarQueryInsert(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;
	return true;
}

LRESULT CToolBarManager::OnToolBarQueryDelete(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;
	return true;
}

LRESULT CToolBarManager::OnToolBarGetButtonInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	LPTBNOTIFY lpTbNotify = (LPTBNOTIFY)pNMH;

	if (lpTbNotify->iItem < (int)m_Buttons.size())
	{
		std::map<int,TBBUTTON>::const_iterator itButton = m_Buttons.begin();
		for (int i = 0; i < lpTbNotify->iItem; i++)
			itButton++;

		lpTbNotify->tbButton = itButton->second;
		lpTbNotify->tbButton.iString = (INT_PTR)lngGetString(
			(unsigned int)lpTbNotify->tbButton.dwData);

		lstrcpy(lpTbNotify->pszText,lngGetString(
			(unsigned int)lpTbNotify->tbButton.dwData));
		lpTbNotify->cchText = lstrlen(lpTbNotify->pszText);

		// Syncronize with the menu. If the corresponding menu item is disabled
		// the toolbar button should be disabled as well.
		/*CMenuItemInfo mii;
		mii.fMask = MIIM_STATE;

		if (GetMenuItemInfo(g_pMainFrame->m_CmdBar.GetMenu(),lpTbNotify->tbButton.idCommand,FALSE,&mii))
		{
			if (mii.fState & MFS_DISABLED)
				lpTbNotify->tbButton.fsState = TBSTATE_INDETERMINATE;
			else
				lpTbNotify->tbButton.fsState = TBSTATE_ENABLED;
		}*/

		bHandled = true;
		return true;
	}

	bHandled = false;
	return 0;
}

LRESULT CToolBarManager::OnToolBarReset(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	// Reset the toolbar button configuration.
	Reset();

	// Delete all buttons.
	for (int i = m_pToolBar->GetButtonCount() - 1; i >= 0; i--)
		m_pToolBar->DeleteButton(i);

	// Re-add all buttons.
	FillToolBarCtrl(m_pToolBar);

	bHandled = true;
	return 0;
}