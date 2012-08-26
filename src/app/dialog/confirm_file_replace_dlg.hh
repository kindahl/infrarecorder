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
#include "tree_manager.hh"

class CConfirmFileReplaceDlg : public CDialogImpl<CConfirmFileReplaceDlg>
{
private:
    enum eMode
    {
        MODE_ASK,
        MODE_YESALL,
        MODE_NOALL
    };

    eMode m_Mode;
    const TCHAR *m_szNewFullPath;
    CItemData *m_pNewItemData;
    CItemData *m_pOldItemData;

    bool Translate();
    bool Execute();

public:
    enum { IDD = IDD_CONFIRMFILEREPLACEDLG };

    CConfirmFileReplaceDlg();
    ~CConfirmFileReplaceDlg();

    void Reset();
    bool Execute(const TCHAR *szNewFullPath,CItemData *pOldItemData);
    bool Execute(CItemData *pNewItemData,CItemData *pOldItemData);

    BEGIN_MSG_MAP(CConfirmFileReplaceDlg)
        MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

        COMMAND_ID_HANDLER(IDC_YESBUTTON,OnButton)
        COMMAND_ID_HANDLER(IDC_YESALLBUTTON,OnButton)
        COMMAND_ID_HANDLER(IDC_NOBUTTON,OnButton)
        COMMAND_ID_HANDLER(IDC_NOALLBUTTON,OnButton)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

    LRESULT OnButton(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
