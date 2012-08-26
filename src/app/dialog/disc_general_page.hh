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
#include <ckmmc/device.hh>
#include "resource.h"
#include "core2_info.hh"

class CDiscGeneralPage : public CPropertyPageImpl<CDiscGeneralPage>
{
private:
    HICON m_hIcon;

    ckmmc::Device &m_Device;
    TCHAR m_szDiscLabel[64];

    bool Translate();

    void DisplayDiscType(ckmmc::Device::Profile Profile);
    void DisplayBookType(unsigned char ucBookType,unsigned char ucBookRev);
    void DisplayStatus(CCore2DiscInfo *pDiscInfo);

public:
    enum { IDD = IDD_PROPPAGE_DISCGENERAL };

    CDiscGeneralPage(const TCHAR *szDiscLabel,ckmmc::Device &Device);
    ~CDiscGeneralPage();

    BEGIN_MSG_MAP(CDiscGeneralPage)
        MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

        CHAIN_MSG_MAP(CPropertyPageImpl<CDiscGeneralPage>)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
