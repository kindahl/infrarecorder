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
#include "resource.h"
#include "CtrlMessages.h"
#include "TreeManager.h"
#include "ProjectDropTargetBase.h"

// Color of items flagged as imported.
#define PROJECTLISTVIEW_COLOR_IMPORTED				RGB(92,53,102)

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
