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
#include "CustomContainer.h"
#include "Settings.h"
#include "CtrlMessages.h"
#include "MainFrm.h"

CCustomContainer::CCustomContainer()
{
	m_iHeaderHeight = 0;

	m_hWndCustomDraw = NULL;
	m_iControlID = -1;
}

CCustomContainer::~CCustomContainer()
{
}

void CCustomContainer::SetCustomDrawHandler(HWND hWndCustomDraw,int iID)
{
	m_hWndCustomDraw = hWndCustomDraw;
	m_iControlID = iID;
}

void CCustomContainer::SetClient(HWND hWndClient)
{
	m_ClientWindow = hWndClient;

	UpdateLayout();
}

void CCustomContainer::SetImageList(HIMAGELIST hImageList)
{
	m_ToolBar.SetImageList(hImageList);
	//m_ToolBar.SetButtonStructSize();
}

void CCustomContainer::UpdateLayout()
{
	RECT rcClient;
	GetClientRect(&rcClient);

	UpdateLayout(rcClient.right,rcClient.bottom);
}

void CCustomContainer::UpdateLayout(int iWidth,int iHeight)
{
	RECT rcHeader = { 0,0,iWidth,m_iHeaderHeight };

	if (m_ClientWindow.m_hWnd != NULL)
		m_ClientWindow.SetWindowPos(NULL,0,m_iHeaderHeight,iWidth,iHeight - m_iHeaderHeight,SWP_NOZORDER);
	else
		rcHeader.bottom = iHeight;

	InvalidateRect(&rcHeader);
}

void CCustomContainer::AddToolBarSeparator()
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

void CCustomContainer::AddToolBarButton(int iCommand,int iBitmap)
{
	TBBUTTON tbButton;
	tbButton.fsState = TBSTATE_ENABLED;
	tbButton.fsStyle = TBSTYLE_BUTTON;
	tbButton.iBitmap = iBitmap;
	tbButton.idCommand = iCommand;
	tbButton.iString = NULL;
	tbButton.dwData = 0;
	m_ToolBar.InsertButton(m_ToolBar.GetButtonCount(),&tbButton);
}

void CCustomContainer::UpdateToolBar()
{
	// Update the toolbar position.
	int iToolBarWidth = 0;
	RECT rcButton;

	for (int i = 0; i < m_ToolBar.GetButtonCount(); i++)
	{
		m_ToolBar.GetItemRect(i,&rcButton);
		iToolBarWidth += rcButton.right - rcButton.left;
	}

	m_iHeaderHeight = HIWORD(m_ToolBar.GetButtonSize());
	m_ToolBar.SetWindowPos(NULL,0,0,iToolBarWidth,m_iHeaderHeight,0);
}

void CCustomContainer::EnableToolbarButton(int iID,bool bEnable)
{
	m_ToolBar.EnableButton(iID,bEnable);
}

LRESULT CCustomContainer::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	RECT rcToolBar = { 0,0,100,100 };
	m_ToolBar.Create(m_hWnd,rcToolBar,NULL,ATL_SIMPLE_TOOLBAR_PANE_STYLE,NULL);
	m_ToolBar.SetButtonStructSize();

	return 0;
}

LRESULT CCustomContainer::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	UpdateLayout(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	return 0;
}

LRESULT CCustomContainer::OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_ClientWindow.m_hWnd != NULL)
		m_ClientWindow.SetFocus();

	return 0;
}

LRESULT CCustomContainer::OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Redirect messages to the parent.
	if (m_ToolBar.m_hWnd != NULL && (HWND)lParam == m_ToolBar.m_hWnd)
		return ::SendMessage(GetParent(),WM_COMMAND,wParam,(LPARAM)m_hWnd);

	bHandled = false;
	return TRUE;
}

LRESULT CCustomContainer::OnGetIShellBrowser(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// This is very important, we need to redirect this message to the main frame that can return
	// the correct IShellBrowser object. If we do not answer to this message the CreateViewObject
	// function call will fail on Windows 98 systems for all other directories than the desktop.
	bHandled = TRUE;
	return ::SendMessage(*g_pMainFrame,WM_GETISHELLBROWSER,wParam,lParam);
}

LRESULT CCustomContainer::OnCustomDraw(int idCtrl,LPNMHDR pnmh,BOOL &bHandled)
{
	if (m_hWndCustomDraw != NULL && idCtrl == m_iControlID)
		return ::SendMessage(m_hWndCustomDraw,WM_CONTROLCUSTOMDRAW,0,(LPARAM)pnmh);
		
	bHandled = false;
	return CDRF_DODEFAULT;
}

LRESULT CCustomContainer::OnToolBarGetInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled)
{
	bHandled = true;

	// The string ID is the same as the button ID.
	LPTOOLTIPTEXT pTipText = (LPTOOLTIPTEXT)pNMH;
	//pTipText->lpszText = MAKEINTRESOURCE(pTipText->hdr.idFrom);

	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a hint translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("hint")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr((unsigned long)pTipText->hdr.idFrom,szStrValue))
			{
				pTipText->lpszText = szStrValue;
				return 0;
			}
		}
	}

	// I am not sure if I want the tool tips to be displayed on the toolbar.
	// This method is also to slow.
	/*TCHAR szBuffer[256];
	LoadString(_Module.GetResourceInstance(),pTipText->hdr.idFrom,szBuffer,sizeof(szBuffer) / sizeof(TCHAR));
	m_StatusBar.SetPaneText(ID_DEFAULT_PANE,szBuffer);*/

	pTipText->lpszText = MAKEINTRESOURCE(pTipText->hdr.idFrom);
	return 0;
}

int CCustomContainer::GetHeaderHeight()
{
	return m_iHeaderHeight;
}