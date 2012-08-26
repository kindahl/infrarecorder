/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "read_stream.hh"

CReadStream::CReadStream()
{
    m_cRefs = 1;
    m_hFile = INVALID_HANDLE_VALUE;
}

CReadStream::~CReadStream()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);
}

// IStream methods.
HRESULT CReadStream::Open(const TCHAR *szFileName)
{
    HRESULT hResult = S_OK;

    m_hFile = CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
        OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if (m_hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    if (GetFileType(m_hFile) != FILE_TYPE_DISK)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;

        return E_FAIL;
    }

    return hResult;
}

HRESULT CReadStream::Read(void *pv,ULONG cb,ULONG *pcbRead)
{
    if (!ReadFile(m_hFile,pv,cb,pcbRead,NULL))
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}

HRESULT CReadStream::Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,
                          ULARGE_INTEGER *plibNewPosition)
{
    unsigned long ulMoveMethod;

    switch (dwOrigin)
    {
        case STREAM_SEEK_SET:
            ulMoveMethod = FILE_BEGIN;
            break;

        case STREAM_SEEK_CUR:
            ulMoveMethod = FILE_CURRENT;
            break;

        case STREAM_SEEK_END:
            ulMoveMethod = FILE_END;
            break;

        default:
            return E_INVALIDARG;
    };

    unsigned long ulPos = SetFilePointer(m_hFile,dlibMove.LowPart,NULL,ulMoveMethod);

    if (ulPos == 0xFFFFFFFF)
        return HRESULT_FROM_WIN32(GetLastError());

    if (plibNewPosition != NULL)
    {
        plibNewPosition->LowPart = ulPos;
        plibNewPosition->HighPart = 0;
    }

    return S_OK;
}

HRESULT CReadStream::Stat(STATSTG *pstatstg,DWORD grfStatFlag)
{
    if (pstatstg == NULL || grfStatFlag != STATFLAG_NONAME)
        return E_INVALIDARG;

    unsigned long ulFileSize = GetFileSize(m_hFile,NULL);

    if (ulFileSize == 0xFFFFFFFF)
        return HRESULT_FROM_WIN32(GetLastError());

    memset(pstatstg,0,sizeof(STATSTG));

    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.LowPart = ulFileSize;

    return S_OK;
}

// IStream unimplemented methods.
HRESULT STDMETHODCALLTYPE CReadStream::Write(void const *pv,ULONG cb,ULONG *pcbWritten)
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::SetSize(ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::CopyTo(IStream *pstm,ULARGE_INTEGER cb,ULARGE_INTEGER *pcbRead,ULARGE_INTEGER *pcbWritten)
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::Commit(DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::Revert()
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::LockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType)
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::UnlockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType)
{
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CReadStream::Clone(IStream **ppstm)
{
    return E_NOTIMPL;
}

// IUnknown methods.
HRESULT CReadStream::QueryInterface(REFIID riid,void **ppv)
{
    if (riid == IID_IUnknown || riid == IID_IStream)
    {
        *ppv = this;
        AddRef();

        return S_OK;
    }
        
    return E_NOINTERFACE;
}

ULONG CReadStream::AddRef()
{
    return InterlockedIncrement(&m_cRefs);
}

ULONG CReadStream::Release()
{
    if (InterlockedDecrement(&m_cRefs) == 0)
    {
        delete this;
        return 0;
    }
    
    return 0xBAD;
}
