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
#include <vector>
#include <map>
#include <base/XmlProcessor.h>
#include "Resource.h"

class CToolBarManager
{
private:
	class CCustomizeDlg : public CWindowImpl<CCustomizeDlg,CWindow>
	{
	private:
		CStatic m_TextStatic;
		CStatic m_IconStatic;
		CComboBox m_TextComboBox;
		CComboBox m_IconComboBox;
		int m_iText;
		int m_iIcon;

	public:
		CCustomizeDlg(int iText,int iIcon);
		~CCustomizeDlg();

		int GetTextOption();
		int GetIconOption();

		BEGIN_MSG_MAP(CCustomizeDlg)
			COMMAND_HANDLER(IDC_TBCUSTOMIZE_TEXTCOMBO,CBN_SELCHANGE,OnTCSelChange)
			COMMAND_HANDLER(IDC_TBCUSTOMIZE_ICONCOMBO,CBN_SELCHANGE,OnICSelChange)
			MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
		LRESULT OnTCSelChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
		LRESULT OnICSelChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	};

	enum
	{
		ID_TOOLBAR_SEPARATOR = 0
	};

	CToolBarCtrl *m_pToolBar;
	std::map<int,TBBUTTON> m_Buttons;
	std::vector<int> m_SelButtons;

	void AddButton(int iBitmap,int iCommand,int iString);
	void AddDropDownButton(int iBitmap,int iCommand,int iString);
	void AddSeparator();
	void Reset();

	// Customize dialog hook.
	static CCustomizeDlg *m_pCustomizeDlg;
	static HHOOK m_hCBTHook;
	static LRESULT CALLBACK CBTProc(int nCode,WPARAM wParam,LPARAM lParam);

public:
	CToolBarManager();
	~CToolBarManager();

	bool FillToolBarCtrl(CToolBarCtrl *pToolBar);

	bool Save(CXmlProcessor *pXml);
	bool Load(CXmlProcessor *pXml);

	bool Customize();

	// Events.
	LRESULT OnToolBarBeginAdjust(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnToolBarInitCustomize(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnToolBarQueryInsert(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnToolBarQueryDelete(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnToolBarGetButtonInfo(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
	LRESULT OnToolBarReset(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
};

extern CToolBarManager g_ToolBarManager;
