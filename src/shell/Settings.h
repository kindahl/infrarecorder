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
#include <base/xml_processor.hh>
#include <base/lng_processor.hh>

class ISettings
{
public:
	virtual bool Load(CXmlProcessor *pXml) = 0;
};

class CLanguageSettings : public ISettings
{
public:
	TCHAR m_szLanguageFile[MAX_PATH];
	CLngProcessor *m_pLngProcessor;

	CLanguageSettings()
	{
		m_szLanguageFile[0] = '\0';
		m_pLngProcessor = NULL;
	}

	~CLanguageSettings()
	{
		if (m_pLngProcessor != NULL)
		{
			delete m_pLngProcessor;
			m_pLngProcessor = NULL;
		}
	}

	bool Load(CXmlProcessor *pXml);
};

class CGlobalSettings : public ISettings
{
public:
	// Shell extension.
	bool m_bShellExtSubMenu;
	bool m_bShellExtIcon;

	CGlobalSettings()
	{
		// Shell extension.
		m_bShellExtSubMenu = false;
		m_bShellExtIcon = true;
	}

	bool Load(CXmlProcessor *pXml);
};

extern CLanguageSettings g_LanguageSettings;
extern CGlobalSettings g_GlobalSettings;
