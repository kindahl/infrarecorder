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

#pragma once

#ifndef WM_GETISHELLBROWSER
	#define WM_GETISHELLBROWSER				WM_USER + 7
#endif

class CCustomContainer : public CWindowImpl<CCustomContainer,CWindow,CControlWinTraits>
{
private:
	CToolBarCtrl m_ToolBar;
	CWindow m_ClientWindow;

	int m_iHeaderHeight;

	// Handle to the control that should receivce custom draw messages.
	HWND m_hWndCustomDraw;
	int m_iControlID;

public:
	static CWndClassInfo &GetWndClassInfo()
	{
		static CWndClassInfo wc =
		{
			{
				sizeof(WNDCLASSEX),CS_DBLCLKS,
				StartWindowProc,0,0,NULL,NULL,NULL,
				GetSysColorBrush(COLOR_BTNFACE),NULL,
				_T("ckCustomContainer"),NULL
			},
			NULL,NULL,IDC_ARROW,TRUE,0,_T("")
		};

		return wc;
	}

	CCustomContainer();
	~CCustomContainer();

	void SetCustomDrawHandler(HWND hWndCustomDraw,int iID);

	BEGIN_MSG_MAP(CCustomContainer)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_HANDLER(WM_COMMAND,OnCommand)
		MESSAGE_HANDLER(WM_GETISHELLBROWSER,OnGetIShellBrowser)

		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDraw)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnToolBarGetInfo)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnCustomDraw(int idCtrl,LPNMHDR pnmh,BOOL &bHandled);
	LRESULT OnGetIShellBrowser(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnToolBarGetInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);

	void SetClient(HWND hWndClient);
	void SetImageList(HIMAGELIST hImageList);

	void UpdateLayout();
	void UpdateLayout(int iWidth,int iHeight);

	void AddToolBarSeparator();
	void AddToolBarButton(int iCommand,int iBitmap);
	void UpdateToolBar();
	void EnableToolbarButton(int iID,bool bEnable);

	int GetHeaderHeight();
};
