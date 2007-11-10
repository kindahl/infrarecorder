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

#pragma once
#include <vector>

#define BOM_UTF8				0xEFBBBF
#define BOM_UTF32BE				0x0000FEFF
#define BOM_UTF32LE				0xFFFE0000
#define BOM_SCSU				0x0EFEFF

#define SCRES_FAIL				0x00
#define SCRES_OK				0x01
#define SCRES_UNSUPBOM			0x02

#define SC_BUFFER_SIZE			1024

class CStringContainer
{
private:
	TCHAR m_ucBuffer[SC_BUFFER_SIZE];
	unsigned long m_ulBufferSize;
	unsigned long m_ulBufferPos;

	__int64 m_iRemainBytes;

	bool ReadNext(HANDLE hFile,TCHAR &c);

public:
#ifdef UNICODE
	std::vector<std::wstring> m_szStrings;
#else
	std::vector<std::string> m_szStrings;
#endif

	CStringContainer();
	~CStringContainer();

	int SaveToFile(const TCHAR *szFileName,bool bCRLF = true);
	int LoadFromFile(const TCHAR *szFileName);
};

class CStringContainerA
{
private:
	char m_ucBuffer[SC_BUFFER_SIZE];
	unsigned long m_ulBufferSize;
	unsigned long m_ulBufferPos;

	__int64 m_iRemainBytes;

	bool ReadNext(HANDLE hFile,char &c);

public:
	std::vector<std::string> m_szStrings;

	CStringContainerA();
	~CStringContainerA();

	int SaveToFile(const TCHAR *szFileName,bool bCRLF = true);
	int LoadFromFile(const TCHAR *szFileName);
};
