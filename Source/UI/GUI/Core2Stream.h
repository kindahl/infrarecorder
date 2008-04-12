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
#include "../../Common/Stream.h"
#include "../../Common/Log.h"
#include "Core2Read.h"

#define CORE2_INSTREAM_FRAMEFACTOR			10	// We will cache 10 frames in the memory.

class CCore2InStream : public CSeekInStream
{
private:
	class CInternalStream : public COutStream
	{
	private:
		unsigned char *m_pBuffer;
		unsigned long m_ulBufferSize;
		unsigned long m_ulBufferData;

	public:
		CInternalStream() : m_pBuffer(NULL),m_ulBufferSize(0),m_ulBufferData(0)
		{
		}

		void SetBuffer(unsigned char *pBuffer,unsigned long ulBufferSize)
		{
			m_pBuffer = pBuffer;
			m_ulBufferSize = ulBufferSize;
		}

		int Write(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize)
		{
			if (m_pBuffer == NULL)
				return STREAM_FAIL;

			if (m_ulBufferSize < ulSize)
				return STREAM_FAIL;

			memcpy(m_pBuffer,pBuffer,ulSize);
			m_ulBufferData = ulSize;

			return STREAM_OK;
		}

		unsigned long GetBufferDataSize()
		{
			return m_ulBufferData;
		}
	};
	CInternalStream m_Stream;

	unsigned char *m_pBuffer;
	unsigned long m_ulBufferSize;
	unsigned long m_ulBufferData;		// The amount of data that's stored in the buffer.
	unsigned long m_ulBufferPos;

	const unsigned long m_ulStartBlock;	// The start sector to use as beginning of the stream.
	const unsigned long m_ulEndBlock;	// The last sector.
	unsigned long m_ulCurBlock;

	CLog *m_pLog;
	CCore2Device *m_pDevice;

	Core2ReadFunction::CReadUserData m_ReadFunc;
	CCore2Read m_Read;

	unsigned long GetSafeFrameReadCount();
	unsigned long BytesToFrame(unsigned __int64 uiBytes);
	int FillBuffer();

public:
	CCore2InStream(CLog *pLog,CCore2Device *pDevice,
		unsigned long ulStartBlock,unsigned long ulEndBlock);
	~CCore2InStream();

	// CInStream.
	int Read(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize);
	bool EOS();

	// CSeekInStream.
	__int64 Seek(__int64 iDistance,eSeekMode SeekMode);
};
