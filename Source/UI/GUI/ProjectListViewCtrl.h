/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "CtrlMessages.h"
#include "TreeManager.h"
#include "ProjectDropTargetBase.h"

// Color of items flagged as boot images.
#define PROJECTLISTVIEW_COLOR_BOOTIMAGE				RGB(0,102,204)

class CProjectListViewCtrl;

class CProjectListViewDropTarget : public CProjectDropTargetBase
{
private:
	CProjectListViewCtrl *m_pHost;

	bool HandleDropData(IDataObject *pDataObject,CProjectNode *pTargetNode,POINTL ptCursor);

public:
	CProjectListViewDropTarget(CProjectListViewCtrl *pHost);

	bool OnDragOver(POINTL ptCursor);
	bool OnDrop(POINTL ptCursor,IDataObject *pDataObject);
	void OnDragLeave();
};

class CProjectListViewCtrl : public CWindowImpl<CProjectListViewCtrl,CListViewCtrl>
{
private:
	HIMAGELIST m_hDragImageList;

public:
	CProjectListViewDropTarget *m_pDropTarget;

	DECLARE_WND_CLASS(_T("ckProjectListViewCtrl"));

	CProjectListViewCtrl();
	~CProjectListViewCtrl();

	void SetViewStyle(int iViewStyle);
	void BeginDrag(LPNMLISTVIEW pNMListView);
	
	BEGIN_MSG_MAP(CProjectListViewCtrl)
		COMMAND_ID_HANDLER(ID_VIEW_LARGEICONS,OnViewLargeIcons)
		COMMAND_ID_HANDLER(ID_VIEW_SMALLICONS,OnViewSmallIcons)
		COMMAND_ID_HANDLER(ID_VIEW_LIST,OnViewList)
		COMMAND_ID_HANDLER(ID_VIEW_DETAILS,OnViewDetails)
		COMMAND_ID_HANDLER(ID_EDIT_NEWFOLDER,OnNewFolder)

		MESSAGE_HANDLER(WM_KEYDOWN,OnKeyDown)
		MESSAGE_HANDLER(WM_DROPFILES,OnDropFiles)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_HANDLER(WM_CONTROLCUSTOMDRAW,OnCustomDraw)
	END_MSG_MAP()

	LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDropFiles(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnCustomDraw(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnViewLargeIcons(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnViewSmallIcons(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnViewList(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnViewDetails(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnNewFolder(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	void ForceRedraw();
	void SelectDropTarget(int iDropItemIndex);
};
