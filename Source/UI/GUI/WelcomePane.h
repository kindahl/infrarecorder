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

#pragma once
#include <vector>
#include "PngFile.h"
#include "CustomButton.h"
#include "CustomMultiButton.h"

class CWelcomePane : public CWindowImpl<CWelcomePane,CWindow>
{
private:
	enum
	{
		LOGO_INDENT_LEFT = 10,
		LOGO_INDENT_TOP = 10
	};

	class CButton
	{
	public:
		enum
		{
			BUTTON_WIDTH = 125,
			BUTTON_HEIGHT = 72
		};

	private:
		int m_iX;
		int m_iY;
		int m_iOffsetX;
		int m_iOffsetY;

	protected:
		CWelcomePane *m_pParent;

	public:
		CButton(CWelcomePane *pParent,int iX,int iY) : m_pParent(pParent),m_iX(iX),m_iY(iY),m_iOffsetX(0),m_iOffsetY(0) {}

		void Offset(int iOffsetX,int iOffsetY)
		{
			m_iOffsetX = iOffsetX;
			m_iOffsetY = iOffsetY;
		}

		int GetX() { return m_iX + m_iOffsetX; }
		int GetY() { return m_iY + m_iOffsetY; }

		virtual void Draw(HDC hDC) = 0;

		virtual bool HoverTest(int iX,int iY,bool &bChanged) = 0;
		virtual long ClickTest(int iX,int iY) = 0;
	};

	class CStandardButton : public CButton
	{
	private:
		enum eState
		{
			STATE_NORMAL,
			STATE_HOT
		};

		bool m_bFocus;
		long m_lCtrlId;
		eState m_State;
		CPngFile m_Image;
	
	public:
		CStandardButton(CWelcomePane *pParent,unsigned short usImageId,int iX,int iY,long lCtrlId);

		void Draw(HDC hDC);

		bool HoverTest(int iX,int iY,bool &bChanged);
		long ClickTest(int iX,int iY);
	};

	class CMultiButton : public CButton
	{
	private:
		enum
		{
			SPLITTER_X = 99,
			SPLITTER_Y = 36
		};

		enum eState
		{
			STATE_NORMAL,
			STATE_HOTMAIN,
			STATE_HOTSUB1,
			STATE_HOTSUB2
		};

		bool m_bFocus;
		long m_lCtrlMainId;
		long m_lCtrlSub1Id;
		long m_lCtrlSub2Id;
		eState m_State;
		CPngFile m_Image;
	
	public:
		CMultiButton(CWelcomePane *pParent,unsigned short usImageId,int iX,int iY,
					 long lCtrlMainId,long lCtrlSub1Id,long lCtrlSub2Id);

		void Draw(HDC hDC);

		bool HoverTest(int iX,int iY,bool &bChanged);
		long ClickTest(int iX,int iY);
	};

	std::vector<CButton *> m_Buttons;

	CPngFile m_LogoImage;

	CPngFile m_StandardNormalImage;
	CPngFile m_StandardFocusImage;
	CPngFile m_StandardHoverImage;
	CPngFile m_StandardHoverFocusImage;

	CPngFile m_MultiNormalImage;
	CPngFile m_MultiFocusImage;
	CPngFile m_MultiHoverImage;
	CPngFile m_MultiHoverSub1Image;
	CPngFile m_MultiHoverSub2Image;
	CPngFile m_MultiHoverFocusImage;
	CPngFile m_MultiHoverFocusSub1Image;
	CPngFile m_MultiHoverFocusSub2Image;

	void SetStatusText(long lCtrlId);

public:
	CWelcomePane();
	~CWelcomePane();

	void Initialize();

	BEGIN_MSG_MAP(CWelcomePane)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN,OnLButtonDown)

		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
