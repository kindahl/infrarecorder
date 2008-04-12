/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "Stream.h"

class CInFileStream : public CSeekInStream
{
private:
	HANDLE m_hFile;

public:
	CInFileStream();
	~CInFileStream();

	bool Open(const TCHAR *szFileName);
	bool EOS();
	bool Close();
	int Read(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize);
	__int64 Seek(__int64 iDistance,int iMode);	// FIXME: Remove this.
	__int64 Seek(__int64 iDistance,eSeekMode SeekMode);
	__int64 Tell();
	__int64 GetSize();
	bool GetModifiedData(unsigned short &usFileDate,unsigned short &usFileTime);
};

class COutFileStream : public COutStream
{
private:
	HANDLE m_hFile;

public:
	COutFileStream();
	~COutFileStream();

	bool Open(const TCHAR *szFileName);
	bool Close();
	int Write(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize);
	__int64 Seek(__int64 iDistance,int iMode);
	__int64 Tell();
};
