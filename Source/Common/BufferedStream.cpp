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

#include "stdafx.h"
#include "BufferedStream.h"

/*
	CBufferedInStream
*/
CBufferedInStream::CBufferedInStream(CInStream &InStream,unsigned long ulBufferSize) :
	m_InStream(InStream)
{
	m_pBuffer = new unsigned char[ulBufferSize];
	m_ulBufferSize = ulBufferSize;
	m_ulBufferPos = 0;
	m_ulBytesInBuffer = 0;	// = m_ulBufferSize - m_ulBufferPos.

	m_bEOBS = false;
}

CBufferedInStream::~CBufferedInStream()
{
	delete [] m_pBuffer;
}

int CBufferedInStream::Read(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize)
{
	if (m_bEOBS)
		return STREAM_FAIL;

	unsigned long ulResultPos = 0;
	unsigned long ulRequestedSize = ulSize;

	while (ulSize > m_ulBytesInBuffer)
	{
		memcpy((unsigned char *)pBuffer + ulResultPos,m_pBuffer + m_ulBufferPos,m_ulBytesInBuffer);
		ulSize -= m_ulBytesInBuffer;

		m_ulBufferPos = 0;
		m_ulBytesInBuffer = 0;

		unsigned long ulNewBufferSize = 0;
		m_InStream.Read(m_pBuffer,m_ulBufferSize,&ulNewBufferSize);
		
		if (ulNewBufferSize == 0)
		{
			if (pProcessedSize != NULL)
				*pProcessedSize = ulRequestedSize - ulSize;

			m_bEOBS = true;
		}
		else
		{
			m_ulBufferSize = ulNewBufferSize;
			m_ulBytesInBuffer = m_ulBufferSize;
		}
	}

	memcpy((unsigned char *)pBuffer + ulResultPos,m_pBuffer + m_ulBufferPos,ulSize);
	m_ulBufferPos += ulSize;
	m_ulBytesInBuffer = m_ulBufferSize - m_ulBufferPos;

	return STREAM_OK;
}

bool CBufferedInStream::EOS()
{
	return m_bEOBS;
}

/*
	COutBufferedStream
*/
CBufferedOutStream::CBufferedOutStream(COutStream &OutStream,unsigned long ulBufferSize) :
	m_OutStream(OutStream)
{
	m_pBuffer = new unsigned char[ulBufferSize];
	m_ulBufferSize = ulBufferSize;
	m_ulBufferPos = 0;

	m_uiTotalWritten = 0;
}

CBufferedOutStream::~CBufferedOutStream()
{
	delete [] m_pBuffer;
}

int CBufferedOutStream::Write(void *pBuffer,unsigned long ulSize,
	unsigned long *pProcessedSize)
{
	int iResult = STREAM_OK;
	unsigned long ulProcessedSize = ulSize;
	unsigned long ulLocalBufPos = 0;

	while (m_ulBufferPos + ulSize > m_ulBufferSize)
	{
		unsigned long ulRemainLen = m_ulBufferPos + ulSize - m_ulBufferSize;

		memcpy(m_pBuffer + m_ulBufferPos,(unsigned char *)pBuffer + ulLocalBufPos,m_ulBufferSize - m_ulBufferPos);
		ulLocalBufPos += m_ulBufferSize - m_ulBufferPos;

		// Flush.
		iResult = m_OutStream.Write(m_pBuffer,m_ulBufferSize,NULL);
		m_ulBufferPos = 0;

		ulSize = ulRemainLen;
	}

	memcpy(m_pBuffer + m_ulBufferPos,(unsigned char *)pBuffer + ulLocalBufPos,ulSize);
	m_ulBufferPos += ulSize;

	// Update the processed size variable.
	if (pProcessedSize != NULL)
		*pProcessedSize = ulProcessedSize;

	return iResult;
}

int CBufferedOutStream::Flush()
{
	int iResult = m_OutStream.Write(m_pBuffer,m_ulBufferPos,NULL);
	m_ulBufferPos = 0;

	return iResult;
}
