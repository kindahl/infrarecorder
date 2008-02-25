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
#include "CtrlMessages.h"
#include "TreeManager.h"
#include "ProjectDropTargetBase.h"

class CProjectTreeViewDropTarget : public CProjectDropTargetBase
{
private:
	CTreeViewCtrlEx *m_pHost;

	bool HandleDropData(IDataObject *pDataObject,CProjectNode *pTargetNode);

public:
	CProjectTreeViewDropTarget(CTreeViewCtrlEx *pHost);

	bool OnDragOver(POINTL ptCursor);	
	bool OnDrop(POINTL ptCursor,IDataObject *pDataObject);
	void OnDragLeave();
};

class CProjectTreeViewCtrl : public CWindowImpl<CProjectTreeViewCtrl,CTreeViewCtrlEx>
{
public:
	CProjectTreeViewDropTarget *m_pDropTarget;

	DECLARE_WND_CLASS(_T("ckProjectTreeViewCtrl"));

	CProjectTreeViewCtrl();
	~CProjectTreeViewCtrl();

	BEGIN_MSG_MAP(CProjectTreeViewCtrl)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		MESSAGE_HANDLER(WM_CONTROLCUSTOMDRAW,OnCustomDraw)
	END_MSG_MAP()

	LRESULT OnSetFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnCustomDraw(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	void BeginDrag(LPNMTREEVIEW pNMTreeView);
	void ForceRedraw();
};
