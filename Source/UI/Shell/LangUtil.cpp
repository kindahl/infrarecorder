/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

#include "stdafx.h"
#include "LangUtil.h"
#include "Settings.h"
#include "StringTable.h"

TCHAR *lngGetString(unsigned int uiID)
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a main translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("shell")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(uiID,szStrValue))
				return szStrValue;
		}
	}

	// Load internal (English) string.
	return g_szStringTable[uiID];
}

int lngMessageBox(HWND hWnd,unsigned int uiTextID,unsigned int uiCaptionID,unsigned int uiType)
{
	return MessageBox(hWnd,lngGetString(uiTextID),lngGetString(uiCaptionID),uiType);
}
