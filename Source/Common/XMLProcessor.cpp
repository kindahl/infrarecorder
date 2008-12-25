/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include "XMLProcessor.h"
#include "StringConv.h"

#ifdef UNICODE
const wchar_t CXMLProcessor::m_szXMLHeader[] = _T("<?xml version=\"1.0\" encoding=\"utf-16\" standalone=\"yes\"?>\r\n");
#else
const char CXMLProcessor::m_szXMLHeader[] = "<?xml version=\"1.0\" encoding=\"windows-%i\" standalone=\"yes\"?>\r\n";
#endif

CXMLProcessor::CXMLProcessor(eMode Mode) : m_Mode(Mode)
{
	m_ulBufferSize = 0;
	m_ulBufferPos = 0;

	memset(m_ucBuffer,0,sizeof(m_ucBuffer));

	m_pRoot = new CXMLElement();
	lstrcpy(m_pRoot->m_szName,_T("Root"));

	// Set the current child to the root.
	m_pCurrent = m_pRoot;
}

CXMLProcessor::~CXMLProcessor()
{
	delete m_pRoot;
}

void CXMLProcessor::DumpBuffer()
{
	ckcore::File File(_T("xml_buffer_dump.txt"));
	if (File.open(ckcore::FileBase::ckOPEN_WRITE))
		File.write(m_ucBuffer,sizeof(m_ucBuffer));
}

bool CXMLProcessor::ReadNext(ckcore::File &File,TCHAR &c)
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

		TCHAR sz;
		sz = true;

		if (m_ulBufferSize == 0)
			return false;
	}

	m_iRemainBytes -= sizeof(TCHAR);
	c = m_ucBuffer[m_ulBufferPos++];
	return true;
}

void CXMLProcessor::ReadBack()
{
	if (m_ulBufferPos > 0)
	{
		m_iRemainBytes += sizeof(TCHAR);
		m_ulBufferPos--;
	}
}

/*
	CXMLProcessor::ReadTagAttr
	-----------------------
	Reads the current tag's attributes. It returns false if the element is empty
	and true if it's not. 
*/
bool CXMLProcessor::ReadTagAttr(ckcore::File &File,CXMLElement *pElement)
{
	TCHAR uc;
	if (!ReadNext(File,uc))
		return false;

	CXMLAttribute *pAttr = new CXMLAttribute();

	while (uc != '>')
	{
		// Check if this is an empty element.
		if (uc == '/')
		{
			if (!ReadNext(File,uc))
			{
				delete pAttr;
				return false;
			}

			if (uc == '>')
			{
				delete pAttr;
				return false;
			}

			pAttr->m_szName.Append('/');
			pElement->m_ulAttrLength++;
		}

		// Check if we have reached a ". In that case we expect a value.
		if (uc == '=')
		{
			pAttr->m_szName.Append('\0');

			// Skip the first ".
			ReadNext(File,uc);

			ReadNext(File,uc);
			while (uc != '"')
			{
				pAttr->m_szValue.Append(uc);
				pElement->m_ulAttrLength++;

				if (!ReadNext(File,uc))
					break;
			}

			pAttr->m_szValue.Append('\0');
			pElement->m_Attributes.push_back(pAttr);

			// Allocate memory for the next attribute.
			pAttr = new CXMLAttribute();
		}
		else
		{
			if (uc != ' ')
			{
				pAttr->m_szName.Append(uc);
				pElement->m_ulAttrLength++;
			}
		}

		if (!ReadNext(File,uc))
		{
			delete pAttr;
			return false;
		}
	}

	delete pAttr;
	return true;
}

/*
	CXMLProcessor::ReadNextTag
	-----------------------
	Reads the next tag in the file. It returns false if the element is empty
	and true if it's not.
*/
bool CXMLProcessor::ReadNextTag(ckcore::File &File,CXMLElement *pElement)
{
	TCHAR uc;
	if (!ReadNext(File,uc))
		return false;

	bool bResult = true;

	while (uc != '>')
	{
		if (uc == ' ')
		{
			bResult = ReadTagAttr(File,pElement);
			break;
		}

		pElement->m_szName.Append(uc);
		if (!ReadNext(File,uc))
			return false;
	}

	pElement->m_szName.Append('\0');
	return bResult;
}

/*
	CXMLProcessor::Load
	-------------------
	Loads the specified XML into a tree structure in memory. Varius return values.
*/
int CXMLProcessor::Load(const TCHAR *szFullPath)
{
	// Open the file.
	ckcore::File File(szFullPath);
	if (!File.open(ckcore::FileBase::ckOPEN_READ))
		return XMLRES_FILEERROR;

	// If the application is in an unicode environment we need to check what
	// byte-order us used.
#ifdef UNICODE
	unsigned short usBOM = 0;
	if (File.read(&usBOM,2) == -1)
		return XMLRES_FILEERROR;

	switch (usBOM)
	{
		// Currently the only supported byte-order.
		case BOM_UTF32BE:
			break;

		case BOM_UTF8:
		case BOM_UTF32LE:
		case BOM_SCSU:
			return XMLRES_UNSUPBOM;

		default:
			// If no BOM is found the file pointer has to be re-moved to the beginning.
			if (File.seek(0,ckcore::FileBase::ckFILE_BEGIN) == -1)
				return XMLRES_FILEERROR;

			break;
	};
#endif

	// Clear the root.
	m_pRoot->Clear();
	CXMLElement *pCurElem = m_pRoot;

	// Set the current child to the root.
	m_pCurrent = m_pRoot;

	m_iRemainBytes = File.size();

	unsigned int uiDataPos = 0;

	bool bHandleData = false;

	while (m_iRemainBytes)
	{
		TCHAR uc;
		if (!ReadNext(File,uc))
			break;

		// We have found the beginning of a tag.
		if (uc == '<')
		{
			if (!ReadNext(File,uc))
				break;

			// Check if we have reached the end of a section.
			if (uc == '/')
			{
				CCustomString szTemp(XML_NAMELENGTH);
				
				if (!ReadNext(File,uc))
					break;

				while (uc != '>')
				{
					szTemp.Append(uc);
					if (!ReadNext(File,uc))
						break;
				}

				szTemp.Append('\0');

				if (lstrcmp(pCurElem->m_szName,szTemp))
				{
					return XMLRES_BADSYNC;
				}
				else
				{
					bHandleData = false;

					if (uiDataPos != 0)
					{
						pCurElem->m_szData.Append('\0');
						uiDataPos = 0;
					}

					pCurElem = pCurElem->m_pParent;
				}
			}
			else if (uc == '?')	// Check if we have reached a processing unstruction.
			{
				TCHAR cTemp;
				if (!ReadNext(File,cTemp))
					break;

				while (cTemp != '>')
				{
					if (!ReadNext(File,cTemp))
						break;
				}
			}
			else
			{
				ReadBack();

				if (uiDataPos != 0)
				{
					pCurElem->m_szData.Append('\0');
					uiDataPos = 0;
				}

				// Add the new element.
				CXMLElement *pNewElem = new CXMLElement();	
				pNewElem->m_pParent = pCurElem;

				bHandleData = ReadNextTag(File,pNewElem);

				if (pCurElem != NULL)
					pCurElem->m_Children.push_back(pNewElem);

				if (bHandleData)
					pCurElem = pNewElem;
			}
		}
		else
		{
			// Filter some characters.
			if (uc == '\t' || uc == '\n' || uc == '\r')
				continue;

			pCurElem->m_szData.Append(uc);
			uiDataPos++;
		}
	}

	return XMLRES_OK;
}

void CXMLProcessor::SaveEntity(ckcore::File &File,unsigned int uiIndent,CXMLElement *pElement)
{
	// If the element contains no information we skip it.
	if (pElement->m_Children.size() == 0 &&
		/*pElement->m_szAttr[0] == '\0' &&*/
		pElement->m_Attributes.size() == 0 &&
		pElement->m_szData[0] == '\0')
	{
		return;
	}

	// The length of the complete attributes string when printed to the buffer.
	unsigned long ulAttrLength = pElement->m_ulAttrLength +
		(unsigned long)pElement->m_Attributes.size() * 4;
	
	// The epsilon value compensates for some <, > and / characters.
	TCHAR *szOutBuf = new TCHAR[
		pElement->m_szName.Length() * 2 +
		pElement->m_szData.Length() +
		ulAttrLength +
		uiIndent +
		XML_OUTBUF_EPSILON];
	unsigned int i = 0;

	// Indent.
	for (i = 0; i < uiIndent; i++)
		szOutBuf[i] = '\t';
	
	szOutBuf[i++] = '<';
	szOutBuf[i] = '\0';

	// Name.
	lstrcat(szOutBuf,pElement->m_szName);

	// Attributes.
	for (i = 0; i < pElement->m_Attributes.size(); i++)
	{
		lstrcat(szOutBuf,_T(" "));
		lstrcat(szOutBuf,pElement->m_Attributes[i]->m_szName);
		lstrcat(szOutBuf,_T("=\""));
		lstrcat(szOutBuf,pElement->m_Attributes[i]->m_szValue);
		lstrcat(szOutBuf,_T("\""));
	}

	// End of the tag.
	if (pElement->m_Children.size() != 0)
	{
		if (pElement->m_szData[0] == '\0')
			lstrcat(szOutBuf,_T(">\r\n"));
		else
			lstrcat(szOutBuf,_T(">"));
	}
	else
	{
		if (pElement->m_szData[0] == '\0')
			lstrcat(szOutBuf,_T("/>\r\n"));
		else
			lstrcat(szOutBuf,_T(">"));
	}

	// Write to the file.
	File.write(szOutBuf,lstrlen(szOutBuf) * sizeof(TCHAR));

	// Write the data.
	if (pElement->m_szData[0] != '\0')
	{
		lstrcpy(szOutBuf,pElement->m_szData);

		// ...
		if (pElement->m_Children.size() != 0)
			lstrcat(szOutBuf,_T("\r\n"));
		// ...

		File.write(szOutBuf,lstrlen(szOutBuf) * sizeof(TCHAR));
	}

	// Traverse the children.
	for (i = 0; i < pElement->m_Children.size(); i++)
		SaveEntity(File,uiIndent + 1,pElement->m_Children[i]);

	// Re-indent if necessary.
	if (pElement->m_Children.size() != 0)
	{
		for (i = 0; i < uiIndent; i++)
			szOutBuf[i] = '\t';
	}
	else
	{
		 i = 0;
	}
	
	// If the element has data or has children we should include a full termination tag.
	if (pElement->m_szData[0] != '\0' ||
		pElement->m_Children.size() != 0)
	{
		szOutBuf[i++] = '<';
		szOutBuf[i++] = '/';
		szOutBuf[i] = '\0';

		lstrcat(szOutBuf,pElement->m_szName);
		lstrcat(szOutBuf,_T(">\r\n"));

		File.write(szOutBuf,lstrlen(szOutBuf) * sizeof(TCHAR));
	}

	delete [] szOutBuf;
}

/*
	CXMLProcessor::Save
	-------------------
	Saves the current loaded XML-structure to a file with the specified filename.
	Various return values.
*/
int CXMLProcessor::Save(const TCHAR *szFullPath)
{
	// Open the file.
	ckcore::File File(szFullPath);
	if (!File.open(ckcore::FileBase::ckOPEN_WRITE))
		return XMLRES_FILEERROR;

	// Write byte-order mark.
#ifdef UNICODE
#ifdef XML_SAVE_BOM
	unsigned short usBOM = BOM_UTF32BE;
	if (File.write(&usBOM,2) == -1)
	{
		File.remove();
		return XMLRES_FILEERROR;
	}
#endif
#endif

	// Write the header.
#ifndef UNICODE
	char szTemp[70];
	sprintf(szTemp,m_szXMLHeader,GetACP());

	if (File.Write((void *)szTemp,(unsigned long)strlen(szTemp)) == -1)
	{
		File.Remove();
		return XMLRES_FILEERROR;
	}
#else
	if (File.write((void *)m_szXMLHeader,lstrlen(m_szXMLHeader) * sizeof(TCHAR)) == -1)
	{
		File.remove();
		return XMLRES_FILEERROR;
	}
#endif

	for (unsigned int i = 0; i < m_pRoot->m_Children.size(); i++)
		SaveEntity(File,0,m_pRoot->m_Children[i]);

	return XMLRES_OK;
}

/*
	XMLProcessor::EnterElement
	--------------------------
	Enters the specified element if it exists. True is returned upon success,
	false if the specified element was not found.
*/
bool CXMLProcessor::EnterElement(const TCHAR *szName)
{
	for (unsigned int i = 0; i < m_pCurrent->m_Children.size(); i++)
	{
		if (!lstrcmp(m_pCurrent->m_Children[i]->m_szName,szName))
		{
			m_pCurrent = m_pCurrent->m_Children[i];
			return true;
		}
	}

	return false;
}

bool CXMLProcessor::EnterElement(unsigned int uiIndex)
{
	if (uiIndex >= m_pCurrent->m_Children.size())
		return false;

	m_pCurrent = m_pCurrent->m_Children[uiIndex];
	return true;
}

/*
	CXMLProcessor::LeaveElement
	---------------------------
	Leaves the current element. True is returned if the operation was successful,
	otherwise false is returned.
*/
bool CXMLProcessor::LeaveElement()
{
	if (m_pCurrent->m_pParent != NULL)
	{
		m_pCurrent = m_pCurrent->m_pParent;
		return true;
	}

	return false;
}

unsigned int CXMLProcessor::GetElementChildCount()
{
	return (unsigned int)m_pCurrent->m_Children.size();
}

bool CXMLProcessor::GetElementAttrValue(const TCHAR *szAttrName,TCHAR *&szValue)
{
	for (unsigned int i = 0; i < m_pCurrent->m_Attributes.size(); i++)
	{
		if (!lstrcmp(m_pCurrent->m_Attributes[i]->m_szName,szAttrName))
		{
			szValue = m_pCurrent->m_Attributes[i]->m_szValue;
			return true;
		}
	}

	return false;
}

void CXMLProcessor::GetSafeElementAttrValue(const TCHAR *szAttrName,TCHAR *szValue,int iMaxLength)
{
	TCHAR *szData = NULL;
	if (GetElementAttrValue(szAttrName,szData) && lstrlen(szData) < iMaxLength)
		lstrcpy(szValue,szData);
}

void CXMLProcessor::GetSafeElementAttrValue(const TCHAR *szAttrName,bool *pValue)
{
	TCHAR *szData = NULL;
	if (GetElementAttrValue(szAttrName,szData))
		*pValue = StringToInt(szData) != 0;
}

void CXMLProcessor::GetSafeElementAttrValue(const TCHAR *szAttrName,int *pValue)
{
	TCHAR *szData = NULL;
	if (GetElementAttrValue(szAttrName,szData))
		*pValue = StringToInt(szData);
}

void CXMLProcessor::GetSafeElementAttrValue(const TCHAR *szAttrName,__int64 *pValue)
{
	TCHAR *szData = NULL;
	if (GetElementAttrValue(szAttrName,szData))
		*pValue = StringToInt64(szData);
}

void CXMLProcessor::GetSafeElementAttrValue(const TCHAR *szAttrName,double *pValue)
{
	TCHAR *szData = NULL;
	if (GetElementAttrValue(szAttrName,szData))
		*pValue = StringToDouble(szData);
}

void CXMLProcessor::GetSafeElementAttrValue(const TCHAR *szAttrName,long *pValue)
{
	TCHAR *szData = NULL;
	if (GetElementAttrValue(szAttrName,szData))
		*pValue = StringToLong(szData);
}

bool CXMLProcessor::GetElementData(TCHAR *&szData)
{
	if (m_pCurrent->m_szData[0] == '\0')
		return false;

	szData = m_pCurrent->m_szData;
	return true;
}

void CXMLProcessor::GetSafeElementData(const TCHAR *szName,TCHAR *pData,int iMaxLength)
{
	if (EnterElement(szName))
	{
		TCHAR *szData = NULL;
		if (GetElementData(szData))
		{
			if (lstrlen(szData) < iMaxLength)
				lstrcpy(pData,szData);
		}

		LeaveElement();
	}
}

void CXMLProcessor::GetSafeElementData(const TCHAR *szName,bool *pData)
{
	if (EnterElement(szName))
	{
		TCHAR *szData = NULL;
		if (GetElementData(szData))
			*pData = StringToInt(szData) != 0;

		LeaveElement();
	}
}

void CXMLProcessor::GetSafeElementData(const TCHAR *szName,int *pData)
{
	if (EnterElement(szName))
	{
		TCHAR *szData = NULL;
		if (GetElementData(szData))
			*pData = StringToInt(szData);

		LeaveElement();
	}
}

void CXMLProcessor::GetSafeElementData(const TCHAR *szName,__int64 *pData)
{
	if (EnterElement(szName))
	{
		TCHAR *szData = NULL;
		if (GetElementData(szData))
			*pData = StringToInt64(szData);

		LeaveElement();
	}
}

void CXMLProcessor::GetSafeElementData(const TCHAR *szName,double *pData)
{
	if (EnterElement(szName))
	{
		TCHAR *szData = NULL;
		if (GetElementData(szData))
			*pData = StringToDouble(szData);

		LeaveElement();
	}
}

void CXMLProcessor::GetSafeElementData(const TCHAR *szName,long *pData)
{
	if (EnterElement(szName))
	{
		TCHAR *szData = NULL;
		if (GetElementData(szData))
			*pData = StringToLong(szData);

		LeaveElement();
	}
}

void CXMLProcessor::GetSafeElementData(TCHAR *pData)
{
	TCHAR *szData = NULL;
	if (GetElementData(szData))
		lstrcpy(pData,szData);
}

void CXMLProcessor::GetSafeElementData(bool *pData)
{
	TCHAR *szData = NULL;
	if (GetElementData(szData))
		*pData = StringToInt(szData) != 0;
}

void CXMLProcessor::GetSafeElementData(int *pData)
{
	TCHAR *szData = NULL;
	if (GetElementData(szData))
		*pData = StringToInt(szData);
}

void CXMLProcessor::GetSafeElementData(__int64 *pData)
{
	TCHAR *szData = NULL;
	if (GetElementData(szData))
		*pData = StringToInt64(szData);
}

void CXMLProcessor::GetSafeElementData(double *pData)
{
	TCHAR *szData = NULL;
	if (GetElementData(szData))
		*pData = StringToDouble(szData);
}

void CXMLProcessor::GetSafeElementData(long *pData)
{
	TCHAR *szData = NULL;
	if (GetElementData(szData))
		*pData = StringToLong(szData);
}

bool CXMLProcessor::AddElement(const TCHAR *szName,const TCHAR *szData,bool bEnter)
{
	CXMLElement *pNewElem = new CXMLElement(lstrlen(szName) + 1,lstrlen(szData) + 1);
	if (!pNewElem)
		return false;

	pNewElem->m_pParent = m_pCurrent;
	pNewElem->m_szName.CopyFrom(szName);

	if (m_Mode == MODE_HTML)
		pNewElem->m_szData.CopyFromHtml(szData);
	else
		pNewElem->m_szData.CopyFrom(szData);

	m_pCurrent->m_Children.push_back(pNewElem);

	if (bEnter)
		m_pCurrent = pNewElem;

	return true;
}

bool CXMLProcessor::AddElement(const TCHAR *szName,bool bData,bool bEnter)
{
	return AddElement(szName,(int)bData,bEnter);
}

bool CXMLProcessor::AddElement(const TCHAR *szName,int iData,bool bEnter)
{
	TCHAR szTemp[16];

#ifdef UNICODE
	_itow(iData,szTemp,10);
#else
	_itoa(iData,szTemp,10);
#endif

	return AddElement(szName,szTemp,bEnter);
}

bool CXMLProcessor::AddElement(const TCHAR *szName,__int64 iData,bool bEnter)
{
	TCHAR szTemp[32];

#ifdef UNICODE
	_i64tow(iData,szTemp,10);
#else
	_i64toa(iData,szTemp,10);
#endif

	return AddElement(szName,szTemp,bEnter);
}

bool CXMLProcessor::AddElement(const TCHAR *szName,double dData,bool bEnter)
{
	TCHAR szTemp[32];

#ifdef UNICODE
	swprintf(szTemp,_T("%f"),dData);
#else
	sprintf(szTemp,_T("%f"),dData);
#endif

	return AddElement(szName,szTemp,bEnter);
}

bool CXMLProcessor::AddElement(const TCHAR *szName,long lData,bool bEnter)
{
	TCHAR szTemp[16];

#ifdef UNICODE
	_ltow(lData,szTemp,10);
#else
	_ltoa(lData,szTemp,10);
#endif

	return AddElement(szName,szTemp,bEnter);
}

bool CXMLProcessor::AddElementAttr(const TCHAR *szAttrName,const TCHAR *szValue)
{
	CXMLAttribute *pNewAttr = new CXMLAttribute(lstrlen(szAttrName) + 1,lstrlen(szValue) + 1);
	if (!pNewAttr)
		return false;

	pNewAttr->m_szName.CopyFrom(szAttrName);

	if (m_Mode == MODE_HTML)
		pNewAttr->m_szValue.CopyFromHtml(szValue);
	else
		pNewAttr->m_szValue.CopyFrom(szValue);

	m_pCurrent->m_Attributes.push_back(pNewAttr);
	m_pCurrent->m_ulAttrLength += lstrlen(szAttrName) + lstrlen(szValue);
	return true;
}

bool CXMLProcessor::AddElementAttr(const TCHAR *szAttrName,bool bValue)
{
	return AddElementAttr(szAttrName,(int)bValue);
}

bool CXMLProcessor::AddElementAttr(const TCHAR *szAttrName,int iValue)
{
	TCHAR szTemp[16];

#ifdef UNICODE
	_itow(iValue,szTemp,10);
#else
	_itoa(iValue,szTemp,10);
#endif

	return AddElementAttr(szAttrName,szTemp);
}

bool CXMLProcessor::AddElementAttr(const TCHAR *szAttrName,__int64 iValue)
{
	TCHAR szTemp[32];

#ifdef UNICODE
	_i64tow(iValue,szTemp,10);
#else
	_i64toa(iValue,szTemp,10);
#endif

	return AddElementAttr(szAttrName,szTemp);
}

bool CXMLProcessor::AddElementAttr(const TCHAR *szAttrName,double dValue)
{
	TCHAR szTemp[32];

#ifdef UNICODE
	swprintf(szTemp,_T("%f"),dValue);
#else
	sprintf(szTemp,_T("%f"),dValue);
#endif

	return AddElementAttr(szAttrName,szTemp);
}

bool CXMLProcessor::AddElementAttr(const TCHAR *szAttrName,long lValue)
{
	TCHAR szTemp[16];

#ifdef UNICODE
	_ltow(lValue,szTemp,10);
#else
	_ltoa(lValue,szTemp,10);
#endif

	return AddElementAttr(szAttrName,szTemp);
}
