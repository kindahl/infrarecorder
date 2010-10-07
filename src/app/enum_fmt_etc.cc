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

#include "stdafx.hh"
#include "enum_fmt_etc.hh"

HRESULT CreateEnumFmtEtc(unsigned int uiNumFormats,FORMATETC *pFormats,
	IEnumFORMATETC **ppEnumFormatEtc)
{
	if (uiNumFormats == 0 || pFormats == NULL || ppEnumFormatEtc == NULL)
		return E_INVALIDARG;

	*ppEnumFormatEtc = new CEnumFmtEtc(pFormats,uiNumFormats);

	return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

CEnumFmtEtc::CEnumFmtEtc(FORMATETC *pFormats,unsigned int uiNumFormats)
{
	m_lRefCount = 1;

	m_uiFormatIndex = 0;
	m_uiNumFormats = uiNumFormats;
	m_pFormats = new FORMATETC[uiNumFormats];

	for (unsigned int i = 0; i < uiNumFormats; i++)
	{
		memcpy(&m_pFormats[i],&pFormats[i],sizeof(FORMATETC));

		if (pFormats->ptd)
		{
			m_pFormats[i].ptd = (DVTARGETDEVICE *)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
			memcpy(&m_pFormats[i].ptd,&pFormats[i].ptd,sizeof(DVTARGETDEVICE));
		}
	}
}

CEnumFmtEtc::~CEnumFmtEtc()
{
	if (m_pFormats)
	{
		for (unsigned int i = 0; i < m_uiNumFormats; i++)
		{
			if (m_pFormats[i].ptd)
				CoTaskMemFree(m_pFormats[i].ptd);
		}

		delete [] m_pFormats;
	}
}

void CEnumFmtEtc::SetIndex(unsigned int uiFormatIndex)
{
	m_uiFormatIndex = uiFormatIndex;
}

ULONG __stdcall CEnumFmtEtc::AddRef()
{
    // Increment object reference count.
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CEnumFmtEtc::Release()
{
    // Decrement object reference count.
	LONG lCount = InterlockedDecrement(&m_lRefCount);
		
	if (lCount == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return lCount;
	}
}

HRESULT __stdcall CEnumFmtEtc::QueryInterface(REFIID iid,void **ppvObject)
{
    // Check to see what interface has been requested.
    if (iid == IID_IEnumFORMATETC || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall CEnumFmtEtc::Next(ULONG celt,FORMATETC *pFormatEtc,ULONG *pceltFetched)
{
	unsigned int uiCopied = 0;

	if (celt == 0 || pFormatEtc == 0)
		return E_INVALIDARG;

	while (m_uiFormatIndex < m_uiNumFormats && uiCopied < celt)
	{
		// Copy the format data.
		memcpy(&pFormatEtc[uiCopied],&m_pFormats[m_uiFormatIndex],sizeof(FORMATETC));

		if (m_pFormats[m_uiFormatIndex].ptd)
		{
			pFormatEtc[uiCopied].ptd = (DVTARGETDEVICE *)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
			memcpy(&pFormatEtc[uiCopied].ptd,&m_pFormats[m_uiFormatIndex].ptd,sizeof(DVTARGETDEVICE));
		}

		uiCopied++;
		m_uiFormatIndex++;
	}

	if (pceltFetched != 0) 
		*pceltFetched = uiCopied;

	// Was all requested data copied?
	return (uiCopied == celt) ? S_OK : S_FALSE;
}

HRESULT __stdcall CEnumFmtEtc::Skip(ULONG celt)
{
	m_uiFormatIndex += celt;
	return (m_uiFormatIndex <= m_uiNumFormats) ? S_OK : S_FALSE;
}

HRESULT __stdcall CEnumFmtEtc::Reset()
{
	m_uiFormatIndex = 0;
	return S_OK;
}

HRESULT __stdcall CEnumFmtEtc::Clone(IEnumFORMATETC **ppEnumFormatEtc)
{
	HRESULT hResult;

	// Make a duplicate enumerator.
	hResult = CreateEnumFmtEtc(m_uiNumFormats,m_pFormats,ppEnumFormatEtc);

	if (hResult == S_OK)
		((CEnumFmtEtc *)*ppEnumFormatEtc)->SetIndex(m_uiFormatIndex);

	return hResult;
}
