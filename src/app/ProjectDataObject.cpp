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

#include "stdafx.h"
#include "ProjectDataObject.h"
#include "EnumFmtEtc.h"

CProjectDataObject::CProjectDataObject()
{
	// Reference count must ALWAYS start at 1.
	m_lRefCount = 1;

	memset(&m_FormatEtc,0,sizeof(m_FormatEtc));
	m_FormatEtc.cfFormat = static_cast<CLIPFORMAT>(::RegisterClipboardFormat(_T(PROJECT_CF_NAME)));
	m_FormatEtc.dwAspect = DVASPECT_CONTENT;
	m_FormatEtc.lindex = 0;
	m_FormatEtc.ptd = NULL;
	m_FormatEtc.tymed = TYMED_HGLOBAL;

	memset(&m_StgMedium,0,sizeof(m_StgMedium));
	m_StgMedium.tymed = TYMED_HGLOBAL;

	// No files have yet been extracted.
	m_bOperationPerformed = false;

	m_pParent = NULL;
}

CProjectDataObject::~CProjectDataObject()
{
	ReleaseStgMedium(&m_StgMedium);
}

void CProjectDataObject::Reset()
{
	m_bOperationPerformed = false;
	m_FileItems.clear();
}

void CProjectDataObject::AddFile(CItemData *pFileItem)
{
	m_FileItems.push_back(pFileItem);
}

void CProjectDataObject::SetParent(CProjectNode *pParent)
{
	m_pParent = pParent;
}

bool CProjectDataObject::IsFormatSupported(FORMATETC *pFormatEtc)
{
	if (m_FormatEtc.cfFormat == pFormatEtc->cfFormat &&
		m_FormatEtc.dwAspect == pFormatEtc->dwAspect &&
		m_FormatEtc.tymed & pFormatEtc->tymed)
	{
		return true;
	}

	return false;
}

HRESULT __stdcall CProjectDataObject::QueryInterface(REFIID iid,void **ppvObject)
{
    // Check to see what interface has been requested.
    if (iid == IID_IDataObject || iid == IID_IUnknown)
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

ULONG __stdcall CProjectDataObject::AddRef()
{
    // Increment object reference count.
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CProjectDataObject::Release()
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

HRESULT __stdcall CProjectDataObject::GetData(FORMATETC *pFormatEtc,STGMEDIUM *pStgMedium)
{
	if (!IsFormatSupported(pFormatEtc))
		return DV_E_FORMATETC;

	// Copy the storage medium data.
	pStgMedium->tymed = m_StgMedium.tymed;
	pStgMedium->pUnkForRelease = 0;
	pStgMedium->hGlobal = GlobalAlloc(GMEM_SHARE,sizeof(CProjectDropData) + sizeof(CItemData *) * m_FileItems.size());

	CProjectDropData *pDropData = (CProjectDropData *)GlobalLock(pStgMedium->hGlobal);
	pDropData->m_pParent = m_pParent;
	pDropData->m_ppData = (CItemData **)(pDropData + sizeof(CProjectDropData));

	// Copy the file names into the global memory.
	unsigned int uiPos = 0;
	std::vector<CItemData *>::const_iterator it;
	for (it = m_FileItems.begin(); it != m_FileItems.end(); it++)
		pDropData->m_ppData[uiPos++] = (CItemData *)*it;

	GlobalUnlock(pDropData);
	return S_OK;
}

HRESULT CProjectDataObject::GetDataHere(FORMATETC *pFormatEtc,STGMEDIUM *pMedium)
{
    return DATA_E_FORMATETC;
}

HRESULT __stdcall CProjectDataObject::QueryGetData(FORMATETC *pFormatEtc)
{
	return IsFormatSupported(pFormatEtc) ? S_OK : DV_E_FORMATETC;
}

HRESULT CProjectDataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct,FORMATETC *pFormatEtcOut)
{
	// MUST be set to NULL.
    pFormatEtcOut->ptd = NULL;

    return E_NOTIMPL;
}

HRESULT __stdcall CProjectDataObject::SetData(FORMATETC *pFormatEtc,STGMEDIUM *pMedium,BOOL fRelease)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CProjectDataObject::EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppEnumFormatEtc)
{
	if (dwDirection == DATADIR_GET)
	{
		// Windows 2000 and newer only.
		//return SHCreateStdEnumFmtEtc(1,&m_FormatEtc,ppEnumFormatEtc);
		return CreateEnumFmtEtc(1,&m_FormatEtc,ppEnumFormatEtc);
	}
	else
	{
		return E_NOTIMPL;
	}
}

HRESULT CProjectDataObject::DAdvise(FORMATETC *pFormatEtc,DWORD advf,IAdviseSink *pAdvSink, 
	DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CProjectDataObject::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CProjectDataObject::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
