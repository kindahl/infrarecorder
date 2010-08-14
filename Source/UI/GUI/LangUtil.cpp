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

#include "stdafx.h"
#include <ckfilesystem/stringtable.hh>
#include "../../Common/StringUtil.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"

const TCHAR *g_szHelpFile = _T("InfraRecorder.chm");

const TCHAR *lngGetString(unsigned int uiID)
{
	ATLASSERT( uiID < _countof(g_szStringTable) );

	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(uiID,szStrValue))
				return szStrValue;
		}
	}

	// Load internal (English) string.
	return g_szStringTable[uiID];
}

const TCHAR *lngGetManual()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("translation")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(TRANSLATION_ID_MANUAL,szStrValue))
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

void lngTranslateTables()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLngProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLngProcessor->EnterSection(_T("filesystem")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::WARNING_FSDIRLEVEL,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::WARNING_FSDIRLEVEL,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::WARNING_SKIPFILE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::WARNING_SKIPFILE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::WARNING_SKIP4GFILE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::WARNING_SKIP4GFILE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::WARNING_SKIP4GFILEISO,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::WARNING_SKIP4GFILEISO,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::ERROR_PATHTABLESIZE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::ERROR_PATHTABLESIZE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::ERROR_OPENWRITE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::ERROR_OPENWRITE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::ERROR_OPENREAD,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::ERROR_OPENREAD,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::STATUS_BUILDTREE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::STATUS_BUILDTREE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::STATUS_WRITEDATA,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::STATUS_WRITEDATA,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::STATUS_WRITEISOTABLE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::STATUS_WRITEISOTABLE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::STATUS_WRITEJOLIETTABLE,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::STATUS_WRITEJOLIETTABLE,szStrValue);
			if (g_LanguageSettings.m_pLngProcessor->GetValuePtr(ckfilesystem::StringTable::STATUS_WRITEDIRENTRIES,szStrValue))
				ckfilesystem::StringTable::instance().set_string(ckfilesystem::StringTable::STATUS_WRITEDIRENTRIES,szStrValue);
		}
	}
}

ckcore::tstring lngSlowFormatStr(const eStringTable TranslatedFormatStr,...)
{
    ckcore::tstring Result;

	va_list Args;
	va_start(Args,TranslatedFormatStr);

	ckcore::string::vformatstr(Result,lngGetString(TranslatedFormatStr),Args);

	va_end(Args);
    return Result;
}