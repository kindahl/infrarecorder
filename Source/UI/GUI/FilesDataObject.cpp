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

#include "stdafx.h"
#include "FilesDataObject.h"
#include "EnumFmtEtc.h"

CFilesDataObject::CFilesDataObject()
{
	// Reference count must ALWAYS start at 1.
	m_lRefCount = 1;

	memset(&m_FormatEtc,0,sizeof(m_FormatEtc));
	m_FormatEtc.cfFormat = CF_HDROP;
	m_FormatEtc.dwAspect = DVASPECT_CONTENT;
	m_FormatEtc.lindex = 0;
	m_FormatEtc.ptd = NULL;
	m_FormatEtc.tymed = TYMED_HGLOBAL;

	memset(&m_StgMedium,0,sizeof(m_StgMedium));
	m_StgMedium.tymed = TYMED_HGLOBAL;
}

CFilesDataObject::~CFilesDataObject()
{
	ReleaseStgMedium(&m_StgMedium);
}

bool CFilesDataObject::IsFormatSupported(FORMATETC *pFormatEtc)
{
	if (m_FormatEtc.cfFormat == pFormatEtc->cfFormat &&
		m_FormatEtc.dwAspect == pFormatEtc->dwAspect &&
		m_FormatEtc.tymed & pFormatEtc->tymed)
	{
		return true;
	}

	return false;
}

void CFilesDataObject::Reset()
{
	m_Files.clear();
}

void CFilesDataObject::AddFile(const TCHAR *szFileName)
{
	m_Files.push_back(szFileName);
}

HRESULT __stdcall CFilesDataObject::QueryInterface(REFIID iid,void **ppvObject)
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

ULONG __stdcall CFilesDataObject::AddRef()
{
    // Increment object reference count.
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CFilesDataObject::Release()
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

HRESULT __stdcall CFilesDataObject::GetData(FORMATETC *pFormatEtc,STGMEDIUM *pStgMedium)
{
	if (!IsFormatSupported(pFormatEtc))
		return DV_E_FORMATETC;

	// Calculate the memory needed for the file names.
	size_t iTotalNameLength = 0;
	std::vector<tstring>::const_iterator itFile;
	for (itFile = m_Files.begin(); itFile != m_Files.end(); itFile++)
		iTotalNameLength += (unsigned int)(*itFile).length() + 1;

	// Copy the file name data into global memory buffer.
	pStgMedium->tymed = m_StgMedium.tymed;
	pStgMedium->pUnkForRelease = 0;
	pStgMedium->hGlobal = GlobalAlloc(GMEM_SHARE,sizeof(DROPFILES) + (iTotalNameLength + 1) * sizeof(TCHAR));
	DROPFILES *pDropFiles = (DROPFILES *)GlobalLock(pStgMedium->hGlobal);

	pDropFiles->pFiles = sizeof(DROPFILES);
#ifdef UNICODE
	pDropFiles->fWide = TRUE;
#else
	pDropFiles->fWide = FALSE;
#endif

	TCHAR *szFiles = (TCHAR *)((unsigned char *)pDropFiles + sizeof(DROPFILES));
	size_t iPos = 0;

	// Copy the file names into the global memory.
	for (itFile = m_Files.begin(); itFile != m_Files.end(); itFile++)
	{
		size_t iPathLength = (*itFile).length();

		memcpy(szFiles + iPos,(*itFile).c_str(),iPathLength * sizeof(TCHAR) + sizeof(TCHAR));
		iPos += iPathLength + 1;
	}

	szFiles[iTotalNameLength] = '\0';

	GlobalUnlock(pDropFiles);
	return S_OK;
}

HRESULT CFilesDataObject::GetDataHere(FORMATETC *pFormatEtc,STGMEDIUM *pMedium)
{
    return DATA_E_FORMATETC;
}

HRESULT __stdcall CFilesDataObject::QueryGetData(FORMATETC *pFormatEtc)
{
	return IsFormatSupported(pFormatEtc) ? S_OK : DV_E_FORMATETC;
}

HRESULT CFilesDataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct,FORMATETC *pFormatEtcOut)
{
	// MUST be set to NULL.
    pFormatEtcOut->ptd = NULL;

    return E_NOTIMPL;
}

HRESULT __stdcall CFilesDataObject::SetData(FORMATETC *pFormatEtc,STGMEDIUM *pMedium,BOOL fRelease)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CFilesDataObject::EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppEnumFormatEtc)
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

HRESULT CFilesDataObject::DAdvise(FORMATETC *pFormatEtc,DWORD advf,IAdviseSink *pAdvSink, 
	DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CFilesDataObject::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT CFilesDataObject::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
