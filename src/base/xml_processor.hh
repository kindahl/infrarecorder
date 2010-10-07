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
#include "custom_string.hh"

#define BOM_UTF8				0xEFBBBF
#define BOM_UTF32BE				0x0000FEFF
#define BOM_UTF32LE				0xFFFE0000
#define BOM_SCSU				0x0EFEFF

#define XML_NAMELENGTH			128
//#define XML_ATTRLENGTH			256
#define XML_DATALENGTH			256

// ...
#define XML_ATTR_NAMELENGTH		32
#define XML_ATTR_VALUELENGTH	16
// ...

#define XML_OUTBUF_EPSILON		16
#define XML_BUFFER_SIZE			1024
#define XML_SAVE_BOM

// Return values.
#define XMLRES_FAIL				0x00
#define XMLRES_OK				0x01
#define XMLRES_BADSYNC			0x02
#define XMLRES_FILEERROR		0x03
#define XMLRES_UNSUPBOM			0x04

class CXmlAttribute
{
public:
	CCustomString m_szName;
	CCustomString m_szValue;

	CXmlAttribute() : m_szName(XML_ATTR_NAMELENGTH), m_szValue(XML_ATTR_VALUELENGTH)
	{
	}

	CXmlAttribute(unsigned int uiNameLength,unsigned int uiValueLength) :
		m_szName(uiNameLength), m_szValue(uiValueLength)
	{
	}
};

class CXmlElement
{
public:
	CCustomString m_szName;
	CCustomString m_szData;

	unsigned long m_ulAttrLength;

	std::vector<CXmlAttribute *> m_Attributes;
	std::vector<CXmlElement *> m_Children;
	CXmlElement *m_pParent;

	CXmlElement() : m_szName(XML_NAMELENGTH), m_szData(XML_DATALENGTH)
	{
		m_szName[0] = '\0';
		m_szData[0] = '\0';

		m_ulAttrLength = 0;
	}

	CXmlElement(unsigned int uiNameLength,unsigned int uiDataLength) :
		m_szName(uiNameLength), m_szData(uiDataLength)
	{
		m_szName[0] = '\0';
		m_szData[0] = '\0';

		m_ulAttrLength = 0;
	}

	~CXmlElement()
	{
		Clear();
	}

	void Clear()
	{
		// Free the children.
		for (unsigned int iIndex = 0; iIndex < m_Children.size(); iIndex++)
		{
			// Remove the object from m_Instances.
			std::vector <CXmlElement *>::iterator itObject = m_Children.begin() + iIndex;
			delete *itObject;
		}

		m_Children.clear();

		// ...
		for (unsigned int iIndex = 0; iIndex < m_Attributes.size(); iIndex++)
		{
			// Remove the object from m_Instances.
			std::vector <CXmlAttribute *>::iterator itObject = m_Attributes.begin() + iIndex;
			delete *itObject;
		}

		m_Children.clear();
	}
};

class CXmlProcessor
{
public:
	enum eMode
	{
		MODE_NORMAL,
		MODE_HTML
	};

private:
	eMode m_Mode;

	TCHAR m_ucBuffer[XML_BUFFER_SIZE];
	unsigned long m_ulBufferSize;
	unsigned long m_ulBufferPos;

	__int64 m_iRemainBytes;

	CXmlElement *m_pRoot;
	CXmlElement *m_pCurrent;

	void DumpBuffer();

	bool ReadNext(ckcore::File &File,TCHAR &c);
	void ReadBack();

	bool ReadTagAttr(ckcore::File &File,CXmlElement *pElement);
	bool ReadNextTag(ckcore::File &File,CXmlElement *pElement);

	void SaveEntity(ckcore::File &File,unsigned int uiIndent,CXmlElement *pElement);

#ifdef UNICODE
	static const wchar_t m_szXMLHeader[];
#else
	static const char m_szXMLHeader[];
#endif

public:
	CXmlProcessor(eMode Mode = MODE_NORMAL);
	~CXmlProcessor();

	int Load(const TCHAR *szFullPath);
	int Save(const TCHAR *szFullPath);

	bool EnterElement(const TCHAR *szName);
	bool EnterElement(unsigned int uiIndex);
	bool LeaveElement();

	unsigned int GetElementChildCount();

	bool GetElementAttrValue(const TCHAR *szAttrName,TCHAR *&szValue);
	void GetSafeElementAttrValue(const TCHAR *szAttrName,TCHAR *szValue,int iMaxLength);
	void GetSafeElementAttrValue(const TCHAR *szAttrName,bool *pValue);
	void GetSafeElementAttrValue(const TCHAR *szAttrName,int *pValue);
	void GetSafeElementAttrValue(const TCHAR *szAttrName,__int64 *pValue);
	void GetSafeElementAttrValue(const TCHAR *szAttrName,double *pValue);
	void GetSafeElementAttrValue(const TCHAR *szAttrName,long *pValue);

	bool GetElementData(TCHAR *&szData);
	void GetSafeElementData(const TCHAR *szName,TCHAR *pData,int iMaxLength);
	void GetSafeElementData(const TCHAR *szName,bool *pData);
	void GetSafeElementData(const TCHAR *szName,int *pData);
	void GetSafeElementData(const TCHAR *szName,__int64 *pData);
	void GetSafeElementData(const TCHAR *szName,double *pData);
	void GetSafeElementData(const TCHAR *szName,long *pData);

	void GetSafeElementData(TCHAR *pData);
	void GetSafeElementData(bool *pData);
	void GetSafeElementData(int *pData);
	void GetSafeElementData(__int64 *pData);
	void GetSafeElementData(double *pData);
	void GetSafeElementData(long *pData);

	bool AddElement(const TCHAR *szName,const TCHAR *szData,bool bEnter = false);
	bool AddElement(const TCHAR *szName,bool bData,bool bEnter = false);
	bool AddElement(const TCHAR *szName,int iData,bool bEnter = false);
	bool AddElement(const TCHAR *szName,__int64 iData,bool bEnter = false);
	bool AddElement(const TCHAR *szName,double dData,bool bEnter = false);
	bool AddElement(const TCHAR *szName,long uData,bool bEnter = false);
	bool AddElementAttr(const TCHAR *szAttrName,const TCHAR *szValue);
	bool AddElementAttr(const TCHAR *szAttrName,bool bValue);
	bool AddElementAttr(const TCHAR *szAttrName,int iValue);
	bool AddElementAttr(const TCHAR *szAttrName,__int64 iValue);
	bool AddElementAttr(const TCHAR *szAttrName,double dValue);
	bool AddElementAttr(const TCHAR *szAttrName,long lValue);
};
