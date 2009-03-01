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
#include "StringContainer.h"
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

bool CStringContainer::ReadNext(ckcore::File &File,TCHAR &c)
{
	if (m_ulBufferPos == m_ulBufferSize)
	{
		memset(m_ucBuffer,0,sizeof(m_ucBuffer));

		ckcore::tint64 iRead = File.read(m_ucBuffer,sizeof(m_ucBuffer));
		if (iRead == -1)
			return false;

		m_ulBufferSize = (unsigned long)iRead;
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
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_WRITE))
		return SCRES_FAIL;

	// Write byte order mark.
#ifdef UNICODE
	unsigned short usBOM = BOM_UTF32BE;
	if (File.write(&usBOM,2) == -1)
		return SCRES_FAIL;
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

		if (File.write((void *)szBuffer,lstrlen(szBuffer) * sizeof(TCHAR)) == -1)
		{
			delete [] szBuffer;
			return SCRES_FAIL;
		}
	}

	delete [] szBuffer;
	return SCRES_OK;
}

int CStringContainer::LoadFromFile(const TCHAR *szFileName)
{
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_READ))
		return SCRES_FAIL;

	// Read byte order mark.
#ifdef UNICODE
	unsigned short usBOM = 0;
	if (File.read(&usBOM,2) == -1)
		return SCRES_FAIL;

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
			if (File.seek(0,ckcore::File::ckFILE_BEGIN) == -1)
				return SCRES_FAIL;

			break;
	};
#endif

	// Read the file data.
	CCustomString Buffer(256);
	m_iRemainBytes = File.size();

	while (m_iRemainBytes)
	{
		TCHAR uc;
		if (!ReadNext(File,uc))
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

bool CStringContainerA::ReadNext(ckcore::File &File,char &c)
{
	if (m_ulBufferPos == m_ulBufferSize)
	{
		memset(m_ucBuffer,0,sizeof(m_ucBuffer));

		ckcore::tint64 iRead = File.read(m_ucBuffer,sizeof(m_ucBuffer));
		if (iRead == -1)
			return false;

		m_ulBufferSize = (unsigned long)iRead;
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
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_WRITE))
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

		if (File.write((void *)szBuffer,(unsigned long)strlen(szBuffer)) == -1)
		{
			delete [] szBuffer;
			return SCRES_FAIL;
		}
	}

	delete [] szBuffer;
	return SCRES_OK;
}

int CStringContainerA::LoadFromFile(const TCHAR *szFileName)
{
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_READ))
		return SCRES_FAIL;

	// Read the file data.
	CCustomStringA Buffer(256);
	m_iRemainBytes = File.size();

	while (m_iRemainBytes)
	{
		char uc;
		if (!ReadNext(File,uc))
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

	return SCRES_OK;
}
