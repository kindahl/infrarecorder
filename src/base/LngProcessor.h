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
#include <ckcore/file.hh>
#include "CustomString.h"

#define BOM_UTF8				0xEFBBBF
#define BOM_UTF32BE				0x0000FEFF
#define BOM_UTF32LE				0xFFFE0000
#define BOM_SCSU				0x0EFEFF

#define LNG_NAMELENGTH			128
#define LNG_VALUELENGTH			128

#define LNG_BUFFER_SIZE			1024

// Return values.
#define LNGRES_FAIL				0x00
#define LNGRES_OK				0x01
#define LNGRES_FILEERROR		0x02
#define LNGRES_UNSUPBOM			0x03

class CLngValue
{
public:
	unsigned long ulName;
	CCustomString m_szValue;

	CLngValue() : m_szValue(LNG_VALUELENGTH)
	{
		ulName = 0;
		m_szValue[0] = '\0';
	}

	CLngValue(unsigned int uiValueLength) :
		m_szValue(uiValueLength)
	{
		ulName = 0;
		m_szValue[0] = '\0';
	}
};

class CLngSection
{
public:
	CCustomString m_szName;
	std::vector<CLngValue *> m_Values;

	CLngSection() : m_szName(LNG_NAMELENGTH)
	{
		m_szName[0] = '\0';
	}

	CLngSection(unsigned int uiNameLength) :
		m_szName(uiNameLength)
	{
		m_szName[0] = '\0';
	}

	~CLngSection()
	{
		Clear();
	}

	void Clear()
	{
		// Free the children.
		for (unsigned int iIndex = 0; iIndex < m_Values.size(); iIndex++)
		{
			// Remove the object from m_Instances.
			std::vector <CLngValue *>::iterator itObject = m_Values.begin() + iIndex;
			delete *itObject;
		}

		m_Values.clear();
	}
};

class CLngProcessor
{
protected:
	ckcore::File m_File;

	TCHAR m_ucBuffer[LNG_BUFFER_SIZE];
	unsigned long m_ulBufferSize;
	unsigned long m_ulBufferPos;

	__int64 m_iRemainBytes;

	std::vector<CLngSection *> m_pSections;
	CLngSection *m_pCurrent;

	void Clear();

	bool ReadNext(TCHAR &c);
	void ReadBack();

public:
	CLngProcessor(const TCHAR *szFullPath);
	~CLngProcessor();

	int Load();

	bool EnterSection(const TCHAR *szSectionName);

	bool GetValue(unsigned long ulName,TCHAR *szValue,unsigned int uiMaxValueLen);
	bool GetValuePtr(unsigned long ulName,TCHAR *&szValue);
};
