/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once
#include <atlscrl.h>
#include <vector>

#define MINIHTML_HORIZONTAL_INDENT				3
#define MINIHTML_COLOR_BACKGROUND				RGB(245,245,245)
#define MINIHTML_FONTHEIGHT_NORMAL				12
#define MINIHTML_FONTHEIGHT_HEADER				18

#define BOM_UTF8								0xEFBBBF
#define BOM_UTF32BE								0x0000FEFF
#define BOM_UTF32LE								0xFFFE0000
#define BOM_SCSU								0x0EFEFF

class CMiniHtmlCtrl : public CScrollWindowImpl<CMiniHtmlCtrl>
{
private:
	enum eAtomType
	{
		AT_TEXT,
		AT_IMAGE,
		AT_BREAK
	};

	enum eAtomAttr
	{
		AA_NORMAL,
		AA_BOLD,
		AA_ITALIC,
		AA_HEADER
	};

	enum eTagType
	{
		TT_UNKNOWN,
		TT_NORMAL,
		TT_BOLD,
		TT_ITALIC,
		TT_HEADER,
		TT_BREAK
	};

	class CAtom
	{
	public:
		TCHAR *m_szText;
		unsigned int m_uiTextLen;
		unsigned int m_uiWidth;
		unsigned int m_uiLine;
		CMiniHtmlCtrl::eAtomType m_Type;
		CMiniHtmlCtrl::eAtomAttr m_Attr;

		CAtom(TCHAR *szText,unsigned int uiTextLen,CMiniHtmlCtrl::eAtomType Type,
			CMiniHtmlCtrl::eAtomAttr Attr)
		{
			m_szText = szText;
			m_uiTextLen = uiTextLen;
			m_uiWidth = 0;
			m_uiLine = 0;
			m_Type = Type;
			m_Attr = Attr;
		}
	};

	TCHAR *m_pDocBuffer;
	unsigned long m_ulDocLength;
	std::vector<CAtom> m_Atoms;
	std::vector<unsigned int> m_LineWordCount;
	HFONT m_hNormalFont;
	HFONT m_hBoldFont;
	HFONT m_hItalicFont;
	HFONT m_hHeaderFont;
	unsigned int m_uiNormalSpaceWidth;
	unsigned int m_uiBoldSpaceWidth;
	unsigned int m_uiItalicSpaceWidth;
	unsigned int m_uiHeaderSpaceWidth;

	static HBRUSH m_hBackgroundBrush;

	HFONT GetAtomFont(const CAtom *pAtom);
	unsigned int GetAtomWidth(const CAtom *pAtom);
	unsigned int GetAtomSpaceWidth(const CAtom *pAtom);
	unsigned int GetAtomsTotalWidth(unsigned int uiFirstAtom,unsigned int uiLastAtom);

	int RenderText(CDCHandle dc,const RECT *pRect,const CAtom *pAtom);
	int RenderImage(CDCHandle dc,const RECT *pRect,const CAtom *pAtom);
	void Render(CDCHandle dc);
	void Layout(unsigned int uiWidth,unsigned int uiHeight);

	eTagType ParseTag(const TCHAR *szTag);
	eAtomAttr TagToAttr(eTagType TagType);
	bool ParseBuffer();

public:
	static CWndClassInfo &GetWndClassInfo()
	{
		static CWndClassInfo wc =
		{
			{
				sizeof(WNDCLASSEX),CS_DBLCLKS,
				StartWindowProc,0,0,NULL,NULL,NULL,
				m_hBackgroundBrush,NULL,
				_T("ckMiniHTML"),NULL
			},
			NULL,NULL,IDC_ARROW,TRUE,0,_T("")
		};

		return wc;
	}

	CMiniHtmlCtrl();
	~CMiniHtmlCtrl();

	bool Load(const TCHAR *szFileName);
	void Close();

	BEGIN_MSG_MAP(CMiniHtmlCtrl)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_SIZE,OnSize)

        DEFAULT_REFLECTION_HANDLER()

		CHAIN_MSG_MAP(CScrollWindowImpl<CMiniHtmlCtrl>)
    END_MSG_MAP()

	LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
};
