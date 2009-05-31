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

#pragma once
#include <ckcore/types.hh>

#define MAINVIEW_NORMALBORDER_SIZE				2
#define MAINVIEW_THEMEDBORDER_SIZE				4
#define MAINVIEW_HINTBAR_SIZE					28

class CMainView : public CSplitterWindowImpl<CMainView,false>
{
public:
	enum eHintType
	{
		HT_INFORMATION = 0,
		HT_WARNING = 1,
		HT_ERROR = 2,
		HT_EXTERNAL = 3
	};

private:
	enum eButtonState
	{
		BS_NORMAL = 0,
		BS_HOVER = 1,
		BS_DOWN = 2,
		BS_DISABLED = 3
	};

	unsigned int m_uiBorderSize;

	bool m_bHintBar;
	eHintType m_HintType;
	ckcore::tstring m_HintMsg;

	eButtonState m_ButtonState;

	HIMAGELIST m_hIconImageList;
	HIMAGELIST m_hCloseImageList;

	void DrawHintBar(HDC hDC,RECT &rcHintBar);

public:
	CMainView();
	~CMainView();

	typedef CSplitterWindowImpl<CMainView,false> _baseClass;

	BEGIN_MSG_MAP(CMainView)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_NCCALCSIZE,OnNCCalcSize)
		MESSAGE_HANDLER(WM_NCPAINT,OnNCPaint)
		MESSAGE_HANDLER(WM_NCMOUSEMOVE,OnNCMouseMove)
		MESSAGE_HANDLER(WM_NCLBUTTONDOWN,OnNCMouseDown)
		MESSAGE_HANDLER(WM_NCLBUTTONUP,OnNCMouseUp)
		MESSAGE_HANDLER(WM_NCHITTEST,OnNCHitTest)

		CHAIN_MSG_MAP(_baseClass);
    END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCCalcSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCMouseDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCMouseUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnNCHitTest(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	void ShowHintMsg(eHintType HintType,const TCHAR *szHintMsg);
	void HideHintMsg();
};
