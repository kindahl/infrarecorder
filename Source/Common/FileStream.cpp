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
#include "FileStream.h"
#include "StringUtil.h"
#include "FileManager.h"

/*
	CInFileStream
*/
CInFileStream::CInFileStream()
{
	m_hFile = NULL;
}

CInFileStream::~CInFileStream()
{
	Close();
}

bool CInFileStream::Open(const TCHAR *szFileName)
{
	if (m_hFile != NULL)
		Close();

	m_hFile = fs_open(szFileName,TEXT("rb"));

	return (m_hFile != NULL);
}

bool CInFileStream::EOS()
{
	return (GetSize() == Tell());
}

bool CInFileStream::Close()
{
	if (m_hFile == NULL)
		return false;

	return fs_close(m_hFile);
}

int CInFileStream::Read(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize)
{	
	unsigned long ulReadSize = fs_read(pBuffer,ulSize,m_hFile);
	
	if (pProcessedSize != NULL)
		*pProcessedSize = ulReadSize;

	return STREAM_OK;
}

__int64 CInFileStream::Seek(__int64 iDistance,int iMode)
{
	return fs_seek(m_hFile,iDistance,iMode);
}

__int64 CInFileStream::Tell()
{
	return fs_tell(m_hFile);
}

__int64 CInFileStream::GetSize()
{
	return fs_filesize(m_hFile);
}

bool CInFileStream::GetModifiedData(unsigned short &usFileDate,unsigned short &usFileTime)
{
	return fs_getmodtime(m_hFile,usFileDate,usFileTime);
}

/*
	COutFileStream
*/
COutFileStream::COutFileStream()
{
	m_hFile = NULL;
}

COutFileStream::~COutFileStream()
{
	Close();
}

bool COutFileStream::Open(const TCHAR *szFileName)
{
	if (m_hFile != NULL)
		Close();

	m_hFile = fs_open(szFileName,TEXT("wb"));

	return (m_hFile != NULL);
}

bool COutFileStream::Close()
{
	if (m_hFile == NULL)
		return false;

	return fs_close(m_hFile);
}

int COutFileStream::Write(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize)
{
	unsigned long ulWriteSize = fs_write(pBuffer,ulSize,m_hFile);

	if (pProcessedSize != NULL)
		*pProcessedSize = ulWriteSize;

	if (ulWriteSize != ulSize)
		return STREAM_FAIL;

	return STREAM_OK;
}

__int64 COutFileStream::Seek(__int64 iDistance,int iMode)
{
	return fs_seek(m_hFile,iDistance,iMode);
}

__int64 COutFileStream::Tell()
{
	return fs_tell(m_hFile);
}
