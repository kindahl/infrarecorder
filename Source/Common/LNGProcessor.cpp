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
#include "LNGProcessor.h"
#include "StringConv.h"
#include "StringUtil.h"

CLNGProcessor::CLNGProcessor(const TCHAR *szFullPath) : m_File(szFullPath)
{
	m_ulBufferSize = 0;
	m_ulBufferPos = 0;

	memset(m_ucBuffer,0,sizeof(m_ucBuffer));

	// Set the current child to the root.
	m_pCurrent = NULL;
}

CLNGProcessor::~CLNGProcessor()
{
	Clear();
}

void CLNGProcessor::Clear()
{
	// Free the children.
	for (unsigned int iIndex = 0; iIndex < m_pSections.size(); iIndex++)
	{
		// Remove the object from m_Instances.
		std::vector <CLNGSection *>::iterator itObject = m_pSections.begin() + iIndex;
		delete *itObject;
	}

	m_pSections.clear();
}

bool CLNGProcessor::ReadNext(TCHAR &c)
{
	if (m_ulBufferPos == m_ulBufferSize)
	{
		memset(m_ucBuffer,0,sizeof(m_ucBuffer));

		ckcore::tint64 iRead = m_File.read(m_ucBuffer,sizeof(m_ucBuffer));
		if (iRead == -1)
			return false;

		m_ulBufferSize = (unsigned long)iRead;
		m_ulBufferSize /= sizeof(TCHAR);
		m_ulBufferPos = 0;

		TCHAR sz;
		sz = true;

		if (m_ulBufferSize == 0)
			return false;
	}

	m_iRemainBytes -= sizeof(TCHAR);
	c = m_ucBuffer[m_ulBufferPos++];
	return true;
}

void CLNGProcessor::ReadBack()
{
	if (m_ulBufferPos > 0)
	{
		m_iRemainBytes += sizeof(TCHAR);
		m_ulBufferPos--;
	}
}

/*
	CLNGProcessor::Load
	-------------------
	Loads the specified XML into a tree structure in memory. Varius return values.
*/
int CLNGProcessor::Load()
{
	// Open the file.
	if (!m_File.open(ckcore::File::ckOPEN_READ))
		return LNGRES_FILEERROR;

	// If the application is in an unicode environment we need to check what
	// byte-order us used.
#ifdef UNICODE
	unsigned short usBOM = 0;
	if (m_File.read(&usBOM,2) == -1)
		return LNGRES_FILEERROR;

	switch (usBOM)
	{
		// Currently the only supported byte-order.
		case BOM_UTF32BE:
			break;

		case BOM_UTF8:
		case BOM_UTF32LE:
		case BOM_SCSU:
			return LNGRES_UNSUPBOM;

		default:
			// If no BOM is found the file pointer has to be re-moved to the beginning.
			if (m_File.seek(0,ckcore::File::ckFILE_BEGIN) == -1)
				return LNGRES_FILEERROR;

			break;
	};
#endif

	// Clear all current sections.
	Clear();

	// Set the current section to NULL.
	m_pCurrent = NULL;

	// The current section (when parsing).
	CLNGSection *pCurSection = NULL;

	CCustomString szNameBuffer(11);

	m_iRemainBytes = m_File.size() - m_File.tell();

	bool bBreak = false;

	int iCount = 0;

	while (m_iRemainBytes)
	{
		iCount++;

		TCHAR uc;
		if (!ReadNext(uc))
			break;

		// Trim.
		while (uc == ' ' || uc == '\t' || uc == '\r' || uc == '\n')
		{
			if (!ReadNext(uc))
			{
				bBreak = true;
				break;
			}
		}

		if (bBreak)
			break;

		// We have found the beginning of a section.
		if (uc == '[')
		{
			if (!ReadNext(uc))
				break;

			pCurSection = new CLNGSection();
			m_pSections.push_back(pCurSection);

			while (uc != ']')
			{
				pCurSection->m_szName.Append(uc);
				if (!ReadNext(uc))
					break;
			}

			pCurSection->m_szName.Append('\0');
		}
		else
		{
			// Ignore all values that are not associated with a section.
			if (pCurSection == NULL)
				continue;

			// If we didn't find a section we have found a value.
			CLNGValue *pNewValue = new CLNGValue();

			while (uc != '=')
			{
				szNameBuffer.Append(uc);
				if (!ReadNext(uc))
					break;
			}

			szNameBuffer.Append('\0');
			lsscanf(szNameBuffer,_T("%x"),&pNewValue->ulName);
			szNameBuffer.Reset();

			// Skip the '='.
			if (!ReadNext(uc))
				break;

			while (uc != '\n' && uc != '\r')
			{
				pNewValue->m_szValue.Append(uc);
				if (!ReadNext(uc))
					break;
			}

			pNewValue->m_szValue.Append('\0');

			pCurSection->m_Values.push_back(pNewValue);
		}
	}

	return LNGRES_OK;
}

bool CLNGProcessor::EnterSection(const TCHAR *szSectionName)
{
	// First, check if we're already in the requested section.
	if (m_pCurrent != NULL && !lstrcmp(szSectionName,m_pCurrent->m_szName))
		return true;

	for (unsigned int i = 0; i < m_pSections.size(); i++)
	{
		if (!lstrcmp(szSectionName,m_pSections[i]->m_szName))
		{
			m_pCurrent = m_pSections[i];
			return true;
		}
	}

	return false;
}

bool CLNGProcessor::GetValue(unsigned long ulName,TCHAR *szValue,unsigned int uiMaxValueLen)
{
	if (m_pCurrent == NULL)
		return false;
	
	for (unsigned int i = 0; i < m_pCurrent->m_Values.size(); i++)
	{
		if (m_pCurrent->m_Values[i]->ulName == ulName)
		{
			if ((unsigned int)lstrlen(m_pCurrent->m_Values[i]->m_szValue) > uiMaxValueLen)
				lstrncpy(szValue,m_pCurrent->m_Values[i]->m_szValue,uiMaxValueLen);
			else
				lstrcpy(szValue,m_pCurrent->m_Values[i]->m_szValue);

			return true;
		}
	}

	return false;
}

bool CLNGProcessor::GetValuePtr(unsigned long ulName,TCHAR *&szValue)
{
	if (m_pCurrent == NULL)
		return false;
	
	for (unsigned int i = 0; i < m_pCurrent->m_Values.size(); i++)
	{
		if (m_pCurrent->m_Values[i]->ulName == ulName)
		{
			szValue = m_pCurrent->m_Values[i]->m_szValue;
			return true;
		}
	}

	return false;
}
