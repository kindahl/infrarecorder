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

#include "stdafx.h"
#include "MiniHtmlCtrl.h"
#include "../../Common/StringUtil.h"
#include "../../Common/FileManager.h"

HBRUSH CMiniHtmlCtrl::m_hBackgroundBrush = NULL;

CMiniHtmlCtrl::CMiniHtmlCtrl()
{
	m_pDocBuffer = NULL;
	m_ulDocLength = 0;

	//Load(_T("C:\\Users\\Christian Kindahl\\Desktop\\Active Projects\\InfraRecorder\\trunk\\Binary64\\Test.xml"));

	// Create fonts.
	m_hNormalFont = NULL;
	m_hBoldFont = NULL;
	m_hHeaderFont = NULL;

	LOGFONT lf = { 0 };
	if (::GetObject(AtlGetDefaultGuiFont(),sizeof(LOGFONT),&lf) == sizeof(LOGFONT))
	{
		lf.lfHeight = MINIHTML_FONTHEIGHT_NORMAL;
		lstrcpy(lf.lfFaceName,_T("Verdana"));
		m_hNormalFont = ::CreateFontIndirect(&lf);
	}

	if (m_hNormalFont == NULL)
		m_hNormalFont = AtlGetDefaultGuiFont();

	m_hBoldFont = AtlCreateBoldFont(m_hNormalFont);

	lf.lfItalic = 1;
	m_hItalicFont = ::CreateFontIndirect(&lf);
	if (m_hItalicFont == NULL)
		m_hItalicFont = m_hNormalFont;

	lf.lfItalic = 0;
	lf.lfHeight = MINIHTML_FONTHEIGHT_HEADER;
	m_hHeaderFont = AtlCreateBoldFont(::CreateFontIndirect(&lf));
	if (m_hHeaderFont == NULL)
		m_hHeaderFont = m_hBoldFont;

	// Create the background brush.
	if (m_hBackgroundBrush == NULL)
		m_hBackgroundBrush = ::CreateSolidBrush(MINIHTML_COLOR_BACKGROUND);

	// Set default font widths to zer.
	m_uiNormalSpaceWidth = 0;
	m_uiBoldSpaceWidth = 0;
	m_uiItalicSpaceWidth = 0;
	m_uiHeaderSpaceWidth = 0;
}

CMiniHtmlCtrl::~CMiniHtmlCtrl()
{
	// Close any opened document.
	Close();

	if (m_hNormalFont != AtlGetDefaultGuiFont())
		::DeleteObject(m_hNormalFont);

	if (m_hBoldFont != NULL)
		::DeleteObject(m_hBoldFont);

	if (m_hItalicFont != NULL)
		::DeleteObject(m_hItalicFont);

	if (m_hHeaderFont != NULL)
		::DeleteObject(m_hHeaderFont);

	// Delete the background brush.
	if (m_hBackgroundBrush != NULL)
	{
		::DeleteObject(m_hBackgroundBrush);
		m_hBackgroundBrush = NULL;
	}
}

bool CMiniHtmlCtrl::Load(const TCHAR *szFileName)
{
	Close();

	HANDLE hFile = fs_open(szFileName,_T("rb"));
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// If the application is in an unicode environment we need to check what
	// byte-order us used.
#ifdef UNICODE
	unsigned short usBOM = 0;
	fs_read(&usBOM,2,hFile);

	switch (usBOM)
	{
		// Currently the only supported byte-order.
		case BOM_UTF32BE:
			break;

		case BOM_UTF8:
		case BOM_UTF32LE:
		case BOM_SCSU:
			return false;

		default:
			// If no BOM is found the file pointer has to be re-moved to the beginning.
			fs_seek(hFile,0,FILE_BEGIN);
			break;
	};
#endif

	unsigned long ulFuxx = (unsigned long)fs_tell(hFile);
	unsigned long ulFileSize = (unsigned long)fs_filesize(hFile) - (unsigned long)fs_tell(hFile);
	m_ulDocLength = ulFileSize / sizeof(TCHAR);

	m_pDocBuffer = new TCHAR[m_ulDocLength + 1];
	fs_read(m_pDocBuffer,ulFileSize,hFile);
	m_pDocBuffer[m_ulDocLength] = '\0';

	fs_close(hFile);
	return ParseBuffer();
}

CMiniHtmlCtrl::eTagType CMiniHtmlCtrl::ParseTag(const TCHAR *szTag)
{
	if (!lstrcmp(szTag,_T("h")) || !lstrcmp(szTag,_T("H")))
		return TT_HEADER;
	else if (!lstrcmp(szTag,_T("b")) || !lstrcmp(szTag,_T("B")))
		return TT_BOLD;
	else if (!lstrcmp(szTag,_T("i")) || !lstrcmp(szTag,_T("I")))
		return TT_ITALIC;
	else if (!lstrcmp(szTag,_T("br")) || !lstrcmp(szTag,_T("BR")))
		return TT_BREAK;

	return TT_UNKNOWN;
}

CMiniHtmlCtrl::eAtomAttr CMiniHtmlCtrl::TagToAttr(eTagType TagType)
{
	switch (TagType)
	{
		case TT_BOLD:
			return AA_BOLD;

		case TT_ITALIC:
			return AA_ITALIC;

		case TT_HEADER:
			return AA_HEADER;
	}

	return AA_NORMAL;
}

bool CMiniHtmlCtrl::ParseBuffer()
{
	bool bInTag = false;
	tstring Tag;
	std::vector<eTagType> Tags;
	Tags.push_back(TT_NORMAL);
	int iWordStart = -1;

	for (unsigned int i = 0; i < m_ulDocLength; i++)
	{
		// Ignore some characters.
		if (m_pDocBuffer[i] == '\t' || m_pDocBuffer[i] == '\n' || m_pDocBuffer[i] == '\r')
		{
			if (iWordStart != -1)
			{
				eTagType CurTag = Tags[Tags.size() - 1];
				m_Atoms.push_back(CAtom(m_pDocBuffer + iWordStart,i - iWordStart,
					AT_TEXT,TagToAttr(CurTag)));

				TCHAR szTemp[64];
				lstrncpy(szTemp,m_pDocBuffer + iWordStart,i - iWordStart);
				szTemp[i - iWordStart] = '\0';

				::MessageBox(NULL,szTemp,_T(""),MB_OK);

				iWordStart = -1;
			}
			continue;
		}

		if (bInTag)
		{
			if (m_pDocBuffer[i] == '>')
			{
				// Check if it's an ending tag.
				if (Tag.c_str()[0] == '/')
				{
					if (iWordStart != -1)
					{
						eTagType CurTag = Tags[Tags.size() - 1];
						m_Atoms.push_back(CAtom(m_pDocBuffer + iWordStart,
							i - iWordStart - (unsigned int)Tag.size() - 1,AT_TEXT,TagToAttr(CurTag)));

						TCHAR szTemp[64];
						lstrncpy(szTemp,m_pDocBuffer + iWordStart,i - iWordStart - Tag.size() - 1);
						szTemp[i - iWordStart - Tag.size() - 1] = '\0';

						iWordStart = -1;
					}

					// Pop the tag stack if necessary.
					if (Tags[Tags.size() - 1] == ParseTag(Tag.c_str() + 1))
						Tags.pop_back();

					Tag.erase();
				}
				else if (Tag[Tag.size() - 1] == '/')	// Check if we have a self-terminating tag.
				{
					if (iWordStart != -1)
					{
						eTagType CurTag = Tags[Tags.size() - 1];
						m_Atoms.push_back(CAtom(m_pDocBuffer + iWordStart,
							i - iWordStart - (unsigned int)Tag.size() - 1,AT_TEXT,TagToAttr(CurTag)));

						TCHAR szTemp[64];
						lstrncpy(szTemp,m_pDocBuffer + iWordStart,i - iWordStart - Tag.size() - 1);
						szTemp[i - iWordStart - Tag.size() - 1] = '\0';

						iWordStart = -1;
					}

					Tag.resize(Tag.size() - 1);

					eTagType CurTag = ParseTag(Tag.c_str());
					switch (CurTag)
					{
						case TT_BREAK:
							{
								// Make sure that the break will be of correct height.
								eAtomAttr BreakAttr = Tags.size() > 0 ? TagToAttr(Tags[Tags.size() - 1]) : AA_NORMAL;
								m_Atoms.push_back(CAtom(NULL,NULL,AT_BREAK,BreakAttr));
							}
							break;
					}

					Tag.erase();
				}
				else
				{
					// We have found a new tag and not processed the last atom, process it.
					if (iWordStart != -1)
					{
						eTagType CurTag = Tags[Tags.size() - 1];
						m_Atoms.push_back(CAtom(m_pDocBuffer + iWordStart,
							i - iWordStart - (unsigned int)Tag.size() - 1,AT_TEXT,TagToAttr(CurTag)));

						TCHAR szTemp[64];
						lstrncpy(szTemp,m_pDocBuffer + iWordStart,i - iWordStart - Tag.size() - 1);
						szTemp[i - iWordStart - Tag.size() - 1] = '\0';

						iWordStart = -1;
					}

					eTagType TagType = ParseTag(Tag.c_str());
					Tags.push_back(TagType);
					
					Tag.erase();
				}

				bInTag = false;
			}
			else
			{
				Tag.push_back(m_pDocBuffer[i]);
			}
		}
		else
		{
			if (m_pDocBuffer[i] == '<')
			{
				bInTag = true;
			}
			else
			{
				if (m_pDocBuffer[i] == ' ')
				{
					if (iWordStart != -1)
					{
						eTagType CurTag = Tags[Tags.size() - 1];
						m_Atoms.push_back(CAtom(m_pDocBuffer + iWordStart,i - iWordStart,
							AT_TEXT,TagToAttr(CurTag)));

						TCHAR szTemp[64];
						lstrncpy(szTemp,m_pDocBuffer + iWordStart,i - iWordStart);
						szTemp[i - iWordStart] = '\0';

						iWordStart = -1;
					}
				}
				else
				{
					if (iWordStart == -1)
						iWordStart = i;
				}
			}
		}
	}

	if (iWordStart != -1)
	{
		eTagType CurTag = Tags[Tags.size() - 1];
		m_Atoms.push_back(CAtom(m_pDocBuffer + iWordStart,m_ulDocLength - iWordStart,
			AT_TEXT,TagToAttr(CurTag)));

		TCHAR szTemp[64];
		lstrncpy(szTemp,m_pDocBuffer + iWordStart,m_ulDocLength - iWordStart);
		szTemp[m_ulDocLength - iWordStart] = '\0';
	}

	return true;
}

void CMiniHtmlCtrl::Close()
{
	if (m_pDocBuffer != NULL)
	{
		delete [] m_pDocBuffer;
		m_pDocBuffer = NULL;
	}

	m_ulDocLength = 0;
}

HFONT CMiniHtmlCtrl::GetAtomFont(const CAtom *pAtom)
{
	switch (pAtom->m_Attr)
	{
		case AA_HEADER:
			return m_hHeaderFont;

		case AA_BOLD:
			return m_hBoldFont;

		case AA_ITALIC:
			return m_hItalicFont;

		default:
			return m_hNormalFont;
	}
}

unsigned int CMiniHtmlCtrl::GetAtomWidth(const CAtom *pAtom)
{
	if (pAtom->m_Type == AT_BREAK)
		return 0;

	// Draw the text.
	HDC hDC = GetDC();
	HFONT hOldFont = (HFONT)SelectObject(hDC,GetAtomFont(pAtom));

	RECT rcText = { 0,0,0,0 };
	DrawText(hDC,pAtom->m_szText,pAtom->m_uiTextLen,&rcText,DT_LEFT | DT_CALCRECT);

	SelectObject(hDC,hOldFont);

	return rcText.right;
}

unsigned int CMiniHtmlCtrl::GetAtomSpaceWidth(const CAtom *pAtom)
{
	switch (pAtom->m_Attr)
	{
		case AA_HEADER:
			if (m_uiHeaderSpaceWidth != 0)
				return m_uiHeaderSpaceWidth;
			break;

		case AA_BOLD:
			if (m_uiBoldSpaceWidth != 0)
				return m_uiBoldSpaceWidth;
			break;

		case AA_ITALIC:
			if (m_uiItalicSpaceWidth != 0)
				return m_uiItalicSpaceWidth;
			break;

		default:
			if (m_uiNormalSpaceWidth != 0)
				return m_uiNormalSpaceWidth;
			break;
	}

	HDC hDC = GetDC();
	HFONT hOldFont = (HFONT)SelectObject(hDC,GetAtomFont(pAtom));

	RECT rcText = { 0,0,0,0 };
	DrawText(hDC,_T(" "),1,&rcText,DT_LEFT | DT_CALCRECT);

	SelectObject(hDC,hOldFont);

	switch (pAtom->m_Attr)
	{
		case AA_HEADER:
			m_uiHeaderSpaceWidth = rcText.right;
			break;

		case AA_BOLD:
			m_uiBoldSpaceWidth = rcText.right;
			break;

		case AA_ITALIC:
			m_uiItalicSpaceWidth = rcText.right;
			break;

		default:
			m_uiNormalSpaceWidth = rcText.right;
			break;
	}

	return rcText.right;
}

unsigned int CMiniHtmlCtrl::GetAtomsTotalWidth(unsigned int uiFirstAtom,
											   unsigned int uiLastAtom)
{
	unsigned int uiTotalWidth = 0;
	for (unsigned int uiAtom = uiFirstAtom; uiAtom < uiLastAtom; uiAtom++)
	{
		// Safety precausion.
		if (uiAtom >= m_Atoms.size())
			break;

		CAtom *pAtom = &m_Atoms[uiAtom];
		if (pAtom->m_Type == AT_BREAK)
			continue;

		uiTotalWidth += pAtom->m_uiWidth;
		if (uiAtom != uiFirstAtom)
			uiTotalWidth += GetAtomSpaceWidth(pAtom);
	}

	return uiTotalWidth;
}

int CMiniHtmlCtrl::RenderText(CDCHandle dc,const RECT *pRect,const CAtom *pAtom)
{
	// Draw the text.
	HFONT hOldFont = (HFONT)SelectObject(dc,GetAtomFont(pAtom));

	::SetBkMode(dc,TRANSPARENT);
	::SetTextColor(dc,::GetSysColor(COLOR_WINDOWTEXT));

	RECT rcText = *pRect;
	::DrawText(dc,pAtom->m_szText,pAtom->m_uiTextLen,&rcText,DT_LEFT);

	// Calculate text height.
	int iTextHeight = DrawText(dc,pAtom->m_szText,pAtom->m_uiTextLen,&rcText,DT_LEFT | DT_CALCRECT);

	if (hOldFont != NULL)
		SelectObject(dc,hOldFont);

	return iTextHeight;
}

int CMiniHtmlCtrl::RenderImage(CDCHandle dc,const RECT *pRect,const CAtom *pAtom)
{
	return 0;
}

void CMiniHtmlCtrl::Render(CDCHandle dc)
{
	RECT rcClient;
	GetClientRect(&rcClient);

	rcClient.right -= MINIHTML_HORIZONTAL_INDENT;

	unsigned int uiAtom = 0;
	unsigned int uiMaxHeight = 0,uiCurHeight = 0;

	// Render all lines.
	std::vector<unsigned int>::const_iterator itLine;
	for (itLine = m_LineWordCount.begin(); itLine != m_LineWordCount.end(); itLine++)
	{
		rcClient.left = MINIHTML_HORIZONTAL_INDENT;
		uiMaxHeight = 0;

		// Check if we should center the line.
		if (uiAtom < m_Atoms.size() && m_Atoms[uiAtom].m_Attr == AA_HEADER)
		{
			rcClient.left = MINIHTML_HORIZONTAL_INDENT +
				((rcClient.right - GetAtomsTotalWidth(uiAtom,uiAtom + *itLine)) >> 1);
		}

		// Render all words on the line.
		for (unsigned int i = 0; i < *itLine; i++,uiAtom++)
		{
			// Safety precausion.
			if (uiAtom >= m_Atoms.size())
				break;

			CAtom *pAtom = &m_Atoms[uiAtom];
			if (pAtom->m_Type == AT_BREAK)
			{
				uiMaxHeight = pAtom->m_Attr == AA_HEADER ? MINIHTML_FONTHEIGHT_HEADER :
					MINIHTML_FONTHEIGHT_NORMAL;
				continue;
			}

			switch (pAtom->m_Type)
			{
				case AT_TEXT:
					uiCurHeight = RenderText(dc,&rcClient,pAtom);
					break;

				case AT_IMAGE:
					uiCurHeight = RenderImage(dc,&rcClient,pAtom);
					break;
			}

			if (uiCurHeight > uiMaxHeight)
				uiMaxHeight = uiCurHeight;

			rcClient.left += pAtom->m_uiWidth + GetAtomSpaceWidth(pAtom);
		}

		rcClient.top += uiMaxHeight;
	}
}

void CMiniHtmlCtrl::Layout(unsigned int uiWidth,unsigned int uiHeight)
{
	if (uiWidth == 0)
		return;

	// Remove the indentation and border from the specified width immediately.
	uiWidth -= (MINIHTML_HORIZONTAL_INDENT << 1) + (GetSystemMetrics(SM_CXEDGE) << 1);

	m_LineWordCount.clear();
	unsigned int uiLeft = 0,uiLine = 0;
	unsigned int uiWordCount = 0;
	unsigned int uiSpaceWidth = 0;

	std::vector<CAtom>::iterator itAtom;
	for (itAtom = m_Atoms.begin(); itAtom != m_Atoms.end(); itAtom++)
	{
		if (itAtom->m_Type == AT_BREAK)
		{
			itAtom->m_uiLine = uiLine++;
			uiLeft = 0;

			m_LineWordCount.push_back(uiWordCount + 1);
			uiWordCount = 0;
			continue;
		}

		// Calculate the width of a space character.
		uiSpaceWidth = GetAtomSpaceWidth(&(*itAtom));

		if (itAtom->m_uiWidth == 0)
			itAtom->m_uiWidth = GetAtomWidth(&(*itAtom));

		if ((uiLeft + itAtom->m_uiWidth) <= uiWidth)
		{
			itAtom->m_uiLine = uiLine;

			if (uiLeft != 0)
				uiLeft += uiSpaceWidth;
			uiLeft += itAtom->m_uiWidth;
			uiWordCount++;
		}
		else
		{
			itAtom->m_uiLine = ++uiLine;
			uiLeft = itAtom->m_uiWidth;

			m_LineWordCount.push_back(uiWordCount);
			uiWordCount = 1;
		}
	}

	if (uiLeft != 0)
	{
		m_LineWordCount.push_back(uiWordCount);
	}

	RECT rcClient;
	GetClientRect(&rcClient);
	InvalidateRect(&rcClient);
}

LRESULT CMiniHtmlCtrl::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (wParam != NULL)
	{
		HDC hDC = (HDC)wParam;
		Render(hDC);
		ReleaseDC(hDC);
	}
	else
	{
		CPaintDC dc(m_hWnd);
		Render(dc.m_hDC);
		ReleaseDC(dc);
	}

	return 0;
}

LRESULT CMiniHtmlCtrl::OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	unsigned int uiWidth = GET_X_LPARAM(lParam);
	unsigned int uiHeight = GET_Y_LPARAM(lParam);

	Layout(uiWidth,uiHeight);
	return 0;
}
