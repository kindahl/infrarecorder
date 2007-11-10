/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "LangUtil.h"
#include "Settings.h"
#include "StringTable.h"

const TCHAR *g_szHelpFile = _T("InfraRecorder.chm");

const TCHAR *lngGetString(unsigned int uiID)
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(uiID,szStrValue))
				return szStrValue;
		}
	}

	// Load internal (English) string.
	return g_szStringTable[uiID];
}

const TCHAR *lngGetManual()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("translation")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TRANSLATION_ID_MANUAL,szStrValue))
				return szStrValue;
		}
	}

	// Return default file name.
	return g_szHelpFile;
}

int lngMessageBox(HWND hWnd,unsigned int uiTextID,unsigned int uiCaptionID,unsigned int uiType)
{
	return MessageBox(hWnd,lngGetString(uiTextID),lngGetString(uiCaptionID),uiType);
}
