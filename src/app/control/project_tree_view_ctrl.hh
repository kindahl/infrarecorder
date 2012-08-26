/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "ctrl_messages.hh"
#include "tree_manager.hh"
#include "project_drop_target_base.hh"

#define PROJECTTREEVIEW_COLOR_IMPORTED				RGB(92,53,102)

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
