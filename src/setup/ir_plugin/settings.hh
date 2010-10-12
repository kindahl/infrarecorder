/*
 * Copyright (C) 2006-2010 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include <list>
#include <base/string_util.hh>
#include <base/xml_processor.hh>
#include <base/lng_processor.hh>

class ISettings
{
public:
	virtual bool Save(CXmlProcessor *pXML) = 0;
	virtual bool Load(CXmlProcessor *pXML) = 0;
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

	bool Save(CXmlProcessor *pXML);
	bool Load(CXmlProcessor *pXML);
};

extern CLanguageSettings g_LanguageSettings;
