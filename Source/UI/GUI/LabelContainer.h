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

#define LABELCONTAINER_COLOR_BACKGROUND				RGB(255,255,255)
#define LABELCONTAINER_COLOR_BORDER					RGB(64,154,222)
#define LABELCONTAINER_COLOR_BACKGROUNDVISTA		RGB(252,252,252)
#define LABELCONTAINER_COLOR_BACKGROUNDALT			::GetSysColor(COLOR_BTNSHADOW)

#define LABELCONTAINER_BOTTOMBORDER_HEIGHT			4
#define LABELCONTAINER_BORDER_HEIGHT				1
#define LABELCONTAINER_MAXTEXT						32

#define LABELCONTAINER_BUTTON_TOPSPACING			1
#define LABELCONTAINER_BUTTON_RIGHTSPACING			0

#define PANE_BUTTON_NORMAL							0
#define PANE_BUTTON_HOVER							1
#define PANE_BUTTON_DOWN							2
#define PANE_BUTTON_DISABLED						3

class CLabelContainer : public CWindowImpl<CLabelContainer,CWindow,CControlWinTraits>
{
private:
	CWindow m_ClientWindow;
	int m_iHeaderHeight;

	HBRUSH m_hBorderBrush;
	TCHAR m_szLabelText[LABELCONTAINER_MAXTEXT];

	// Button related.
	HIMAGELIST m_hCloseImageList;
	int m_iButtonState;
	bool m_bButtonDown;

	// Handle to the control that should receivce custom draw messages.
	HWND m_hWndCustomDraw;
	int m_iControlID;

	// The host window that should receive the close message.
	HWND m_hWndCloseHost;

	void InitializeImageList();

	void DrawText(CDCHandle dc,RECT *pHeaderRect);
	void DrawBackground(CDCHandle dc,RECT *pHeaderRect);
	void DrawButton(CDCHandle dc,RECT *pHeaderRect);

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

	CLabelContainer(bool bClosable = false);
	~CLabelContainer();

	BEGIN_MSG_MAP(CLabelContainer)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN,OnMouseDown)
		MESSAGE_HANDLER(WM_LBUTTONUP,OnMouseUp)

		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDraw)

		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnMouseUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnCustomDraw(int idCtrl,LPNMHDR pnmh,BOOL &bHandled);

	void SetCustomDrawHandler(HWND hWndCustomDraw,int iID);
	void SetClient(HWND hWndClient);
	void SetCloseHost(HWND hWndCloseHost);

	void UpdateLayout();
	void UpdateLayout(int iWidth,int iHeight);

	void SetHeaderHeight(int iHeight);
	void SetLabelText(const TCHAR *szText);
};
