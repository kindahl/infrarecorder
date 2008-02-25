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
#include "StringContainer.h"
#include "FileManager.h"
#include "CustomString.h"

CStringContainer::CStringContainer()
{
	m_ulBufferSize = 0;
	m_ulBufferPos = 0;

	memset(m_ucBuffer,0,sizeof(m_ucBuffer));
}

CStringContainer::~CStringContainer()
{
	m_szStrings.clear();
}

bool CStringContainer::ReadNext(HANDLE hFile,TCHAR &c)
{
	if (m_ulBufferPos == m_ulBufferSize)
	{
		memset(m_ucBuffer,0,sizeof(m_ucBuffer));

		m_ulBufferSize = fs_read(m_ucBuffer,sizeof(m_ucBuffer),hFile);
		m_ulBufferSize /= sizeof(TCHAR);
		m_ulBufferPos = 0;

		if (m_ulBufferSize == 0)
			return false;
	}

	m_iRemainBytes -= sizeof(TCHAR);
	c = m_ucBuffer[m_ulBufferPos++];
	return true;
}

/*
	CStringContainer::SaveToFile
	----------------------------
	Saves the container data to the file with the specified file name. If bCRLF
	is set each line will be terminated with <Carriage-Return><Line-Feed>,
	otherwise only a new line (Line-Feed) will be used.
*/
int CStringContainer::SaveToFile(const TCHAR *szFileName,bool bCRLF)
{
	HANDLE hFile = fs_open(szFileName,TEXT("wb"));
	if (!hFile)
		return SCRES_FAIL;

	// Write byte order mark.
#ifdef UNICODE
	unsigned short usBOM = BOM_UTF32BE;
	fs_write(&usBOM,2,hFile);
#endif

	// Find the longest string.
	unsigned int uiLongestString = 0;
	for (unsigned int i = 0; i < m_szStrings.size(); i++)
	{
		if (m_szStrings[i].length() > uiLongestString)
			uiLongestString = (unsigned int)m_szStrings[i].length();
	}

	// Write the strings to the file.
	TCHAR *szBuffer = new TCHAR[uiLongestString + 3];

	for (unsigned int i = 0; i < m_szStrings.size(); i++)
	{
		lstrcpy(szBuffer,m_szStrings[i].c_str());

		if (bCRLF)
			lstrcat(szBuffer,TEXT("\r\n"));
		else
			lstrcat(szBuffer,TEXT("\n"));

		fs_write((void *)szBuffer,lstrlen(szBuffer) * sizeof(TCHAR),hFile);
	}

	delete [] szBuffer;

	fs_close(hFile);
	return SCRES_OK;
}

int CStringContainer::LoadFromFile(const TCHAR *szFileName)
{
	HANDLE hFile = fs_open(szFileName,TEXT("rb"));
	if (!hFile)
		return SCRES_FAIL;

	// Read byte order mark.
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
			return SCRES_UNSUPBOM;

		default:
			// If no BOM is found the file pointer has to be re-moved to the beginning.
			fs_seek(hFile,0,FILE_BEGIN);
			break;
	};
#endif

	// Read the file data.
	CCustomString Buffer(256);
	m_iRemainBytes = fs_filesize(hFile);

	while (m_iRemainBytes)
	{
		TCHAR uc;
		if (!ReadNext(hFile,uc))
			break;

		if (uc == '\r')
		{
			continue;
		}
		else if (uc == '\n')
		{
			Buffer.Append('\0');
			m_szStrings.push_back((TCHAR *)Buffer);
			Buffer.Reset();
			continue;
		}

		Buffer.Append(uc);
	}

	fs_close(hFile);
	return SCRES_OK;
}

CStringContainerA::CStringContainerA()
{
	m_ulBufferSize = 0;
	m_ulBufferPos = 0;

	memset(m_ucBuffer,0,sizeof(m_ucBuffer));
}

CStringContainerA::~CStringContainerA()
{
	m_szStrings.clear();
}

bool CStringContainerA::ReadNext(HANDLE hFile,char &c)
{
	if (m_ulBufferPos == m_ulBufferSize)
	{
		memset(m_ucBuffer,0,sizeof(m_ucBuffer));

		m_ulBufferSize = fs_read(m_ucBuffer,sizeof(m_ucBuffer),hFile);
		m_ulBufferPos = 0;

		if (m_ulBufferSize == 0)
			return false;
	}

	m_iRemainBytes--;
	c = m_ucBuffer[m_ulBufferPos++];
	return true;
}

/*
	CStringContainer::SaveToFile
	----------------------------
	Saves the container data to the file with the specified file name. If bCRLF
	is set each line will be terminated with <Carriage-Return><Line-Feed>,
	otherwise only a new line (Line-Feed) will be used.
*/
int CStringContainerA::SaveToFile(const TCHAR *szFileName,bool bCRLF)
{
	HANDLE hFile = fs_open(szFileName,TEXT("wb"));
	if (!hFile)
		return SCRES_FAIL;

	// Find the longest string.
	unsigned int uiLongestString = 0;
	for (unsigned int i = 0; i < m_szStrings.size(); i++)
	{
		if (m_szStrings[i].length() > uiLongestString)
			uiLongestString = (unsigned int)m_szStrings[i].length();
	}

	// Write the strings to the file.
	char *szBuffer = new char[uiLongestString + 3];

	for (unsigned int i = 0; i < m_szStrings.size(); i++)
	{
		strcpy(szBuffer,m_szStrings[i].c_str());

		if (bCRLF)
			strcat(szBuffer,"\r\n");
		else
			strcat(szBuffer,"\n");

		fs_write((void *)szBuffer,(unsigned long)strlen(szBuffer),hFile);
	}

	delete [] szBuffer;

	fs_close(hFile);
	return SCRES_OK;
}

int CStringContainerA::LoadFromFile(const TCHAR *szFileName)
{
	HANDLE hFile = fs_open(szFileName,TEXT("rb"));
	if (!hFile)
		return SCRES_FAIL;

	// Read the file data.
	CCustomStringA Buffer(256);
	m_iRemainBytes = fs_filesize(hFile);

	while (m_iRemainBytes)
	{
		char uc;
		if (!ReadNext(hFile,uc))
			break;

		if (uc == '\r')
		{
			continue;
		}
		else if (uc == '\n')
		{
			Buffer.Append('\0');
			m_szStrings.push_back((char *)Buffer);
			Buffer.Reset();
			continue;
		}

		Buffer.Append(uc);
	}

	fs_close(hFile);
	return SCRES_OK;
}
