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
#include "resource.h"

class CProjectPropBootPage : public CPropertyPageImpl<CProjectPropBootPage>
{
private:
	HIMAGELIST m_hToolBarImageList;
	CListViewCtrl m_ListView;
	CToolBarCtrl m_ToolBar;

	bool Translate();

	void InitToolBarImageList();
	void AddToolBarButton(int iCommand,int iBitmap);
	void CreateToolBarCtrl();

	void FillListView();

public:
	enum { IDD = IDD_PROPPAGE_PROJECTPROPBOOT };

	CProjectPropBootPage();
	~CProjectPropBootPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CProjectPropBootPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(ID_BOOT_ADD,OnListAdd)
		COMMAND_ID_HANDLER(ID_BOOT_REMOVE,OnListRemove)
		COMMAND_ID_HANDLER(ID_BOOT_EDIT,OnListEdit)

		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnToolBarGetInfo)
		NOTIFY_HANDLER(IDC_LIST,LVN_ITEMCHANGED,OnListItemChanged)
		NOTIFY_HANDLER(IDC_LIST,LVN_GETDISPINFO,OnListGetDispInfo)

		CHAIN_MSG_MAP(CPropertyPageImpl<CProjectPropBootPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnListAdd(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnListRemove(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnListEdit(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	LRESULT OnToolBarGetInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnListItemChanged(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnListGetDispInfo(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled);
};
