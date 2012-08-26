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

#define CUSTOMCOMBO_ICONSPACING			1

class CCustomComboBox : public CWindowImpl<CCustomComboBox,CComboBox>,
    public COwnerDraw<CCustomComboBox>
{
private:
    HIMAGELIST m_hImageList;
    int m_iImageList;

public:
    DECLARE_WND_CLASS(_T("ckComboBox"));

    CCustomComboBox();
    ~CCustomComboBox();

    BEGIN_MSG_MAP(CCustomComboBox)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CCustomComboBox>,1)
    END_MSG_MAP()

    LRESULT OnChar(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

    void SetImageList(HIMAGELIST hImageList,int iImageList);

    // For ownerdraw.
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};
