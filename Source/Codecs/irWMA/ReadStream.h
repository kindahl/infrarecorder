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
#include <objidl.h>

class CReadStream : public IStream
{
private:
	HANDLE m_hFile;
	LONG m_cRefs;

public:
	CReadStream();
	~CReadStream();

	HRESULT Open(const TCHAR *szFileName);

	// IStream methods.
	HRESULT STDMETHODCALLTYPE Read(void *pv,ULONG cb,ULONG *pcbRead);
    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER *plibNewPosition);
    HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg,DWORD grfStatFlag);

	HRESULT STDMETHODCALLTYPE Write(void const *pv,ULONG cb,ULONG *pcbWritten);
    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
    HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm,ULARGE_INTEGER cb,ULARGE_INTEGER *pcbRead,ULARGE_INTEGER *pcbWritten);
    HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
    HRESULT STDMETHODCALLTYPE Revert();
    HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType);
    HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType);
    HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm);

	// IUnknown methods.
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
};
