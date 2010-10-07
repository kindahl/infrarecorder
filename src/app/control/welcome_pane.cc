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
#include "Resource.h"
#include "main_frm.hh"
#include "project_manager.hh"
#include "settings.hh"
#include "string_table.hh"
#include "lang_util.hh"
#include "action_manager.hh"
#include "welcome_pane.hh"

CWelcomePane::CStandardButton::CStandardButton(CWelcomePane *pParent,
											   unsigned short usImageId,
											   int iX,int iY,long lCtrlId) :
	CButton(pParent,iX,iY),m_State(STATE_NORMAL),m_bFocus(false),m_lCtrlId(lCtrlId)
{
	m_Image.Open(usImageId);
}

void CWelcomePane::CStandardButton::Draw(HDC hDC)
{
	switch (m_State)
	{
		case STATE_NORMAL:
			if (m_bFocus)
			{
				m_pParent->m_StandardFocusImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			else
			{
				m_pParent->m_StandardNormalImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			break;

		case STATE_HOT:
			if (m_bFocus)
			{
				m_pParent->m_StandardHoverFocusImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			else
			{
				m_pParent->m_StandardHoverImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			break;
	}

	m_Image.Draw(hDC,GetX() + 18,GetY() + 12,GetX() + 18 + BUTTON_WIDTH,GetY() + 12 + BUTTON_HEIGHT);
}

bool CWelcomePane::CStandardButton::HoverTest(int iX,int iY,bool &bChanged)
{
	bool bHitX = iX > GetX() && iX < GetX() + BUTTON_WIDTH;
	bool bHitY = iY > GetY() && iY < GetY() + BUTTON_HEIGHT;

	if (bHitX && bHitY)
	{
		// Update status bar.
		if (m_State != STATE_HOT)
			m_pParent->SetStatusText(m_lCtrlId);

		bChanged = m_State != STATE_HOT;
		m_State = STATE_HOT;
		return true;
	}

	bChanged = m_State != STATE_NORMAL;
	m_State = STATE_NORMAL;
	return false;
}

long CWelcomePane::CStandardButton::ClickTest(int iX,int iY)
{
	bool bHitX = iX > GetX() && iX < GetX() + BUTTON_WIDTH;
	bool bHitY = iY > GetY() && iY < GetY() + BUTTON_HEIGHT;

	if (bHitX && bHitY)
		return m_lCtrlId;
	return -1;
}

CWelcomePane::CMultiButton::CMultiButton(CWelcomePane *pParent,
										 unsigned short usImageId,
										 int iX,int iY,
										 long lCtrlMainId,
										 long lCtrlSub1Id,
										 long lCtrlSub2Id) :
	CButton(pParent,iX,iY),m_State(STATE_NORMAL),m_bFocus(false),
	m_lCtrlMainId(lCtrlMainId),m_lCtrlSub1Id(lCtrlSub1Id),m_lCtrlSub2Id(lCtrlSub2Id)
{
	m_Image.Open(usImageId);
}

void CWelcomePane::CMultiButton::Draw(HDC hDC)
{
	switch (m_State)
	{
		case STATE_NORMAL:
			if (m_bFocus)
			{
				m_pParent->m_MultiFocusImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			else
			{
				m_pParent->m_MultiNormalImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			break;

		case STATE_HOTMAIN:
			if (m_bFocus)
			{
				m_pParent->m_MultiHoverFocusImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			else
			{
				m_pParent->m_MultiHoverImage.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			break;

		case STATE_HOTSUB1:
			if (m_bFocus)
			{
				m_pParent->m_MultiHoverFocusSub1Image.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			else
			{
				m_pParent->m_MultiHoverSub1Image.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			break;

		case STATE_HOTSUB2:
			if (m_bFocus)
			{
				m_pParent->m_MultiHoverFocusSub2Image.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			else
			{
				m_pParent->m_MultiHoverSub2Image.Draw(hDC,GetX(),GetY(),
					GetX() + BUTTON_WIDTH,GetY() + BUTTON_HEIGHT);
			}
			break;
	}

	m_Image.Draw(hDC,GetX() + 15,GetY() + 12,GetX() + 18 + BUTTON_WIDTH,GetY() + 12 + BUTTON_HEIGHT);
}

bool CWelcomePane::CMultiButton::HoverTest(int iX,int iY,bool &bChanged)
{
	bool bHitX = iX > GetX() && iX < GetX() + BUTTON_WIDTH;
	bool bHitY = iY > GetY() && iY < GetY() + BUTTON_HEIGHT;

	if (bHitX && bHitY)
	{
		int iSplitterX = GetX() + SPLITTER_X;
		int iSplitterY = GetY() + SPLITTER_Y;

		long lCtrlId;
		eState NewState;
		if (iX < iSplitterX)
		{
			lCtrlId = m_lCtrlMainId;
			NewState = STATE_HOTMAIN;
		}
		else if (iY < iSplitterY)
		{
			lCtrlId = m_lCtrlSub1Id;
			NewState = STATE_HOTSUB1;
		}
		else
		{
			lCtrlId = m_lCtrlSub2Id;
			NewState = STATE_HOTSUB2;
		}

		// Update status bar.
		if (NewState != m_State)
			m_pParent->SetStatusText(lCtrlId);

		bChanged = NewState != m_State;
		m_State = NewState;
		return true;
	}

	bChanged = m_State != STATE_NORMAL;
	m_State = STATE_NORMAL;
	return true;
}

long CWelcomePane::CMultiButton::ClickTest(int iX,int iY)
{
	bool bHitX = iX > GetX() && iX < GetX() + BUTTON_WIDTH;
	bool bHitY = iY > GetY() && iY < GetY() + BUTTON_HEIGHT;

	if (bHitX && bHitY)
	{
		int iSplitterX = GetX() + SPLITTER_X;
		int iSplitterY = GetY() + SPLITTER_Y;

		if (iX < iSplitterX)
			return m_lCtrlMainId;
		else if (iY < iSplitterY)
			return m_lCtrlSub1Id;
		else
			return m_lCtrlSub2Id;
	}

	return -1;
}

CWelcomePane::CWelcomePane()
{
	//m_LogoImage.Open(_T("C:\\test.png"));
	m_LogoImage.Open(IDR_WELCOMELOGOPNG);

	// Load button images.
	m_StandardNormalImage.Open(IDR_BUTTONNPNG);
	m_StandardFocusImage.Open(IDR_BUTTONFPNG);
	m_StandardHoverImage.Open(IDR_BUTTONHPNG);
	m_StandardHoverFocusImage.Open(IDR_BUTTONHFPNG);

	// Load multi button images.
	m_MultiNormalImage.Open(IDR_MBUTTONNPNG);
	m_MultiFocusImage.Open(IDR_MBUTTONFPNG);
	m_MultiHoverImage.Open(IDR_MBUTTONHPNG);
	m_MultiHoverSub1Image.Open(IDR_MBUTTONHS1PNG);
	m_MultiHoverSub2Image.Open(IDR_MBUTTONHS2PNG);
	m_MultiHoverFocusImage.Open(IDR_MBUTTONFPNG);
	m_MultiHoverFocusSub1Image.Open(IDR_MBUTTONHFS1PNG);
	m_MultiHoverFocusSub2Image.Open(IDR_MBUTTONHFS2PNG);

	// Add buttons.
	m_Buttons.push_back(new CMultiButton(this,IDR_WIZARDDATAPNG,
		CButton::BUTTON_WIDTH * 0 + 10,10,
		ID_NEWPROJECT_DATA,ID_NEWPROJECT_DATACD,ID_NEWPROJECT_DATADVD));
	m_Buttons.push_back(new CStandardButton(this,IDR_WIZARDAUDIOPNG,
		CButton::BUTTON_WIDTH * 1 + 10,10,
		ID_NEWPROJECT_AUDIO));
	m_Buttons.push_back(new CStandardButton(this,IDR_WIZARDVIDEOPNG,
		CButton::BUTTON_WIDTH * 2 + 10,10,
		ID_NEWPROJECT_DVDVIDEO));
	m_Buttons.push_back(new CStandardButton(this,IDR_WIZARDWRITEPNG,
		CButton::BUTTON_WIDTH * 0 + 10,CButton::BUTTON_HEIGHT + 10,
		ID_ACTIONS_BURNIMAGE));
	m_Buttons.push_back(new CStandardButton(this,IDR_WIZARDCOPYPNG,
		CButton::BUTTON_WIDTH * 1 + 10,CButton::BUTTON_HEIGHT + 10,
		ID_COPYDISC_COMPACTDISC));
	m_Buttons.push_back(new CStandardButton(this,IDR_WIZARDREADPNG,
		CButton::BUTTON_WIDTH * 2 + 10,CButton::BUTTON_HEIGHT + 10,
		ID_COPYDISC_DISCIMAGE));
}

CWelcomePane::~CWelcomePane()
{
	// Delete all buttons.
	std::vector<CButton *>::iterator itButton;
	for (itButton = m_Buttons.begin(); itButton != m_Buttons.end(); itButton++)
		delete *itButton;

	m_Buttons.clear();
}

void CWelcomePane::SetStatusText(long lCtrlId)
{
	bool bSuccess = false;
	if (g_LanguageSettings.m_pLngProcessor != NULL &&
		g_LanguageSettings.m_pLngProcessor->EnterSection(_T("hint")))
	{
		TCHAR *szStrValue;
		if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(lCtrlId,szStrValue))
		{
			::SendMessage(g_pMainFrame->m_hWndStatusBar,SB_SETTEXT,0,(LPARAM)szStrValue);
			bSuccess = true;
		}
	}

	if (!bSuccess)
	{
		TCHAR szBuffer[256];
		LoadString(_Module.GetResourceInstance(),lCtrlId,szBuffer,sizeof(szBuffer) / sizeof(TCHAR));
		::SendMessage(g_pMainFrame->m_hWndStatusBar,SB_SETTEXT,0,(LPARAM)szBuffer);
	}
}

LRESULT CWelcomePane::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(&ps);

	// Calculate client rectangle.
	RECT rcClient;
	GetClientRect(&rcClient);

	// Center the buttons.
	int iDrawWidth = CButton::BUTTON_WIDTH * 3 + 20;
	int iDrawHeight = CButton::BUTTON_HEIGHT * 2 + 10;

	int iOffsetX = (rcClient.right >> 1) - (iDrawWidth >> 1);
	int iOffsetY = (rcClient.bottom >> 1) - (iDrawHeight >> 1);

	std::vector<CButton *>::iterator itButton;
	for (itButton = m_Buttons.begin(); itButton != m_Buttons.end(); itButton++)
		(*itButton)->Offset(iOffsetX,iOffsetY);

	// Use double-buffering.
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC,rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,hMemBitmap);

	// Draw into the memory buffer.
	FillRect(hMemDC,&rcClient,GetSysColorBrush(COLOR_WINDOW));

	m_LogoImage.Draw(hMemDC,LOGO_INDENT_LEFT,LOGO_INDENT_TOP,rcClient.right,rcClient.bottom);

	// Draw buttons.
	//std::vector<CButton *>::iterator itButton;
	for (itButton = m_Buttons.begin(); itButton != m_Buttons.end(); itButton++)
		(*itButton)->Draw(hMemDC);

	// Copy the memory buffer into the frame buffer.
	BitBlt(hDC,0,0,rcClient.right,rcClient.bottom,hMemDC,0,0,SRCCOPY);

	// Free memory buffer handles.
	SelectObject(hMemDC,hOldBitmap);
	DeleteObject(hMemBitmap);
	DeleteDC(hMemDC);

	EndPaint(&ps);
	return 0;
}

LRESULT CWelcomePane::OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	return 0;
}

LRESULT CWelcomePane::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	int iPosX = GET_X_LPARAM(lParam); 
	int iPosY = GET_Y_LPARAM(lParam);

	bool bRedraw = false,bHasHit = false;

	std::vector<CButton *>::iterator itButton;
	for (itButton = m_Buttons.begin(); itButton != m_Buttons.end(); itButton++)
	{
		CWelcomePane::CButton *pButton = *itButton;
		
		bool bChanged = false;
		if (pButton->HoverTest(iPosX,iPosY,bChanged))
			bHasHit = true;
		if (bChanged)
			bRedraw = true;
	}

	if (bRedraw)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		InvalidateRect(&rcClient);
	}

	if (!bHasHit)
		::SetWindowText(g_pMainFrame->m_hWndStatusBar,_T(""));

	return 0;
}

LRESULT CWelcomePane::OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	int iPosX = GET_X_LPARAM(lParam); 
	int iPosY = GET_Y_LPARAM(lParam);

	std::vector<CButton *>::iterator itButton;
	for (itButton = m_Buttons.begin(); itButton != m_Buttons.end(); itButton++)
	{
		CWelcomePane::CButton *pButton = *itButton;

		long lAction = pButton->ClickTest(iPosX,iPosY);
		switch (lAction)
		{
			case ID_NEWPROJECT_DATA:
				g_pMainFrame->ShowWelcomePane(false);
				break;

			case ID_NEWPROJECT_DATACD:
				g_pMainFrame->ShowWelcomePane(false);
				g_ProjectManager.NewDataProject(SPACEMETER_SIZE_703MB);
				break;

			case ID_NEWPROJECT_DATADVD:
				g_pMainFrame->ShowWelcomePane(false);
				g_ProjectManager.NewDataProject(SPACEMETER_SIZE_DVD);
				break;

			case ID_NEWPROJECT_AUDIO:
				g_pMainFrame->ShowWelcomePane(false);
				g_ProjectManager.NewAudioProject(SPACEMETER_SIZE_80MIN);
				break;

			case ID_NEWPROJECT_DVDVIDEO:
				g_pMainFrame->ShowWelcomePane(false);
				g_ProjectManager.NewDataProject(SPACEMETER_SIZE_DVD);
				g_ProjectSettings.m_iFileSystem = FILESYSTEM_DVDVIDEO;
				break;

			case ID_ACTIONS_BURNIMAGE:
				g_ActionManager.BurnImage(*g_pMainFrame,false);
				break;

			case ID_COPYDISC_COMPACTDISC:
				g_ActionManager.CopyDisc(*g_pMainFrame,false);
				break;

			case ID_COPYDISC_DISCIMAGE:
				g_ActionManager.CopyImage(*g_pMainFrame,false);
				break;
		}
	}

	return 0;
}