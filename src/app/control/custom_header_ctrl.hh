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
#include "resource.h"
#include "ctrl_messages.hh"

#if (_WIN32_WINNT < 0x501)
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif

class CCustomHeaderCtrl : public CWindowImpl<CCustomHeaderCtrl,CHeaderCtrl>
{
private:
    bool m_bSortUp;
    unsigned int m_iSortCol;

    void SetSortColumn(unsigned int uiColIndex,bool bSortUp);

public:
    CCustomHeaderCtrl();

    BEGIN_MSG_MAP(CCustomHeaderCtrl)
        MESSAGE_HANDLER(WM_CHC_SETSORTCOLUMN,OnSetSortColumn)
    END_MSG_MAP()

    LRESULT OnSetSortColumn(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

    void ColumnClick(unsigned int uiColIndex);
};
