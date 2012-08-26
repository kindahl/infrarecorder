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
#include <vector>
#include "resource.h"

typedef struct
{
    TCHAR szFileName[MAX_PATH];
} tLangFileData;

class CConfigLanguagePage : public CPropertyPageImpl<CConfigLanguagePage>
{
private:
    CComboBox m_LangCombo;

    std::vector<tLangFileData> m_LangFileList;

    bool Translate();
    ckcore::tstring GetTranslationDisplayName(TCHAR *szFileName);
    void FillLangCombo();

public:
    enum { IDD = IDD_PROPPAGE_CONFIGLANGUAGE };

    CConfigLanguagePage();
    ~CConfigLanguagePage();

    bool OnApply();
    void OnHelp();

    BEGIN_MSG_MAP(CConfigAdvancedPage)
        MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

        CHAIN_MSG_MAP(CPropertyPageImpl<CConfigLanguagePage>)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
