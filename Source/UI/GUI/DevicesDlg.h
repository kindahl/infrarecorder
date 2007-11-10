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
#include "resource.h"

class CDevicesDlg : public CDialogImpl<CDevicesDlg>
{
private:
	class CInternalListViewCtrl : public CWindowImpl<CInternalListViewCtrl,CListViewCtrl>
	{
	public:
		BEGIN_MSG_MAP(CInternalListViewCtrl)
			MESSAGE_HANDLER(WM_KEYDOWN,OnKeyDown)
		END_MSG_MAP()

		CInternalListViewCtrl()
		{
		}

		LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
		{
			LRESULT lResult = DefWindowProc(uMsg,wParam,lParam);

			if (wParam == VK_RETURN)
			{
				if (GetSelectedCount() > 0)
				{
					::MessageBox(NULL,_T(""),_T(""),MB_OK);
				}
			}

			return lResult;
		}
	};

	HIMAGELIST m_hListImageList;
	CInternalListViewCtrl m_ListView;

	bool Translate();

	void InitializeListView();
	void FillListView();

public:
	enum { IDD = IDD_DEVICESDLG };

	CDevicesDlg();
	~CDevicesDlg();

	BEGIN_MSG_MAP(CDevicesDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY,OnDestroy)

		NOTIFY_HANDLER(IDC_DEVICELIST,NM_DBLCLK,OnListDblClick)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnOK)
		COMMAND_ID_HANDLER(IDC_RESCANBUTTON,OnRescan)
		COMMAND_HANDLER(IDC_HELPBUTTON,BN_CLICKED,OnHelp)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnListDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnRescan(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnHelp(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
