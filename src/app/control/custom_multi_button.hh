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
#include "png_file.hh"

class CCustomMultiButton : public CWindowImpl<CCustomMultiButton,CButton>,
    public COwnerDraw<CCustomMultiButton>
{
private:
    enum eState
    {
        STATE_NORMAL,
        STATE_HOTMAIN,
        STATE_HOTSUB1,
        STATE_HOTSUB2,
        STATE_DOWN
    };

    enum
    {
        SPLITTER_X = 99,
        SPLITTER_Y = 36
    };

    eState m_State;
    long m_lCtrlMainId;
    long m_lCtrlSub1Id;
    long m_lCtrlSub2Id;
    int m_iCoverLeft;
    int m_iCoverTop;

    CPngFile m_CoverImage;
    CPngFile m_NormalImage;
    CPngFile m_FocusImage;
    CPngFile m_HoverImage;
    CPngFile m_HoverSub1Image;
    CPngFile m_HoverSub2Image;
    CPngFile m_HoverFocusImage;
    CPngFile m_HoverFocusSub1Image;
    CPngFile m_HoverFocusSub2Image;

public:
    DECLARE_WND_CLASS(_T("ckMultiButton"));

    CCustomMultiButton(long lCtrlMainId,long lCtrlSub1Id,long lCtrlSub2Id,
        unsigned short usCoverPng,int iCoverLeft,int iCoverRight);
    ~CCustomMultiButton();

    BEGIN_MSG_MAP(CCustomMultiButton)
        MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSELEAVE,OnMouseLeave)
        MESSAGE_HANDLER(WM_LBUTTONDOWN,OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP,OnLButtonUp)
        MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)

        CHAIN_MSG_MAP_ALT(COwnerDraw<CCustomMultiButton>,1)
    END_MSG_MAP()

    LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

    // For ownerdraw.
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};
