/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#define LABELCONTAINER_COLOR_BACKGROUND			RGB(255,255,255)
#define LABELCONTAINER_COLOR_BORDER				RGB(64,154,222)
#define LABELCONTAINER_COLOR_BACKGROUNDVISTA	RGB(252,252,252)

#define LABELCONTAINER_BOTTOMBORDER_HEIGHT		4
#define LABELCONTAINER_BORDER_HEIGHT			1
#define LABELCONTAINER_MAXTEXT					32

class CLabelContainer : public CWindowImpl<CLabelContainer,CWindow,CControlWinTraits>
{
private:
	CWindow m_ClientWindow;
	int m_iHeaderHeight;

	HBRUSH m_hBorderBrush;
	TCHAR m_szLabelText[LABELCONTAINER_MAXTEXT];

	// Handle to the control that should receivce custom draw messages.
	HWND m_hWndCustomDraw;
	int m_iControlID;

	void DrawText(CDCHandle dc,RECT *pHeaderRect);
	void DrawBackground(CDCHandle dc,RECT *pHeaderRect);

public:
	static CWndClassInfo &GetWndClassInfo()
	{
		static CWndClassInfo wc =
		{
			{
				sizeof(WNDCLASSEX),CS_DBLCLKS,
				StartWindowProc,0,0,NULL,NULL,NULL,
				GetSysColorBrush(COLOR_BTNFACE),NULL,
				_T("ckLabelContainer"),NULL
			},
			NULL,NULL,IDC_ARROW,TRUE,0,_T("")
		};

		return wc;
	}

	CLabelContainer();
	~CLabelContainer();

	BEGIN_MSG_MAP(CLabelContainer)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)

		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDraw)

		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnCustomDraw(int idCtrl,LPNMHDR pnmh,BOOL &bHandled);

	void SetCustomDrawHandler(HWND hWndCustomDraw,int iID);
	void SetClient(HWND hWndClient);

	void UpdateLayout();
	void UpdateLayout(int iWidth,int iHeight);

	void SetHeaderHeight(int iHeight);
	void SetLabelText(const TCHAR *szText);
};
