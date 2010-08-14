/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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
#include "Resource.h"

class CConfigGeneralPage : public CPropertyPageImpl<CConfigGeneralPage>
{
private:
	bool IsFileExtRegistered(const TCHAR *szFileExt);
	bool RegisterFileExt(const TCHAR *szFileExt,const TCHAR *szTypeKeyName,
		const TCHAR *szTypeDesc);
	bool UnregisterFileExt(const TCHAR *szFileExt);

	bool Translate();

public:
	enum { IDD = IDD_PROPPAGE_CONFIGGENERAL };

	CConfigGeneralPage();
	~CConfigGeneralPage();

	bool OnApply();
	void OnHelp();

	BEGIN_MSG_MAP(CBurnImageGeneralPage)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

		COMMAND_ID_HANDLER(IDC_REMEMBERSHELLCHECK,OnRememberShellCheck)
		COMMAND_ID_HANDLER(IDC_SHELLFOLDERBROWSEBUTTON,OnFolderBrowse)
		COMMAND_ID_HANDLER(IDC_TEMPFOLDERBROWSEBUTTON,OnFolderBrowse)

		CHAIN_MSG_MAP(CPropertyPageImpl<CConfigGeneralPage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

	LRESULT OnRememberShellCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnFolderBrowse(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
