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

class CTitleTipListViewCtrl : public CWindowImpl<CTitleTipListViewCtrl,CListViewCtrl>
{
private:
    CToolTipCtrl m_ToolTip;
    TOOLINFO m_ti;
    bool m_bTrackingMouseLeave;

    // The current item that's beeing hovered.
    int m_iCurrentItem;

public:
    DECLARE_WND_CLASS(_T("ckToolTipListViewCtrl"));

    CTitleTipListViewCtrl();
    ~CTitleTipListViewCtrl();

    bool Initialize();

    BEGIN_MSG_MAP(CTitleTipListViewCtrl)
        MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSELEAVE,OnMouseLeave)
        NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnGetDispInfo)
    END_MSG_MAP()

    LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnGetDispInfo(int idCtrl,LPNMHDR pnmh,BOOL &bHandled);
};
