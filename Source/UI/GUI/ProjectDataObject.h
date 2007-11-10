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
#include "TreeManager.h"

#define PROJECT_CF_NAME			"IRPROJECT"

class CProjectDropData
{
public:
	CProjectNode *m_pParent;
	CItemData **m_ppData;
};

class CProjectDataObject : public IDataObject
{
private:
	bool m_bOperationPerformed;
	long m_lRefCount;
	FORMATETC m_FormatEtc;
	STGMEDIUM m_StgMedium;

	std::vector<CItemData *> m_FileItems;
	CProjectNode *m_pParent;

	bool IsFormatSupported(FORMATETC *pFormatEtc);

public:
	CProjectDataObject();
	~CProjectDataObject();

	void Reset();
	void AddFile(CItemData *pFileItem);
	void SetParent(CProjectNode *pParent);

	// IUnknown members.
    HRESULT __stdcall QueryInterface(REFIID iid,void **ppvObject);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
        
    // IDataObject members.
    HRESULT __stdcall GetData(FORMATETC *pFormatEtc,STGMEDIUM *pmedium);
    HRESULT __stdcall GetDataHere(FORMATETC *pFormatEtc,STGMEDIUM *pmedium);
    HRESULT __stdcall QueryGetData(FORMATETC *pFormatEtc);
    HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatEct,FORMATETC *pFormatEtcOut);
    HRESULT __stdcall SetData(FORMATETC *pFormatEtc,STGMEDIUM *pMedium,BOOL fRelease);
    HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
    HRESULT __stdcall DAdvise(FORMATETC *pFormatEtc,DWORD advf,IAdviseSink *,DWORD *);
    HRESULT __stdcall DUnadvise(DWORD dwConnection);
    HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppEnumAdvise);
};
