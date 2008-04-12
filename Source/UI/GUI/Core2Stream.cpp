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
#include "Core2Read.h"
#include "Core2Stream.h"

CCore2InStream::CCore2InStream(CLog *pLog,CCore2Device *pDevice,
							   unsigned long ulStartBlock,unsigned long ulEndBlock) :
	m_pLog(pLog),m_pDevice(pDevice),m_ReadFunc(pDevice,&m_Stream),m_ulStartBlock(ulStartBlock),
	m_ulEndBlock(ulEndBlock),m_ulCurBlock(ulStartBlock)
{
	m_ulBufferData = 0;
	m_ulBufferPos = 0;
	m_ulBufferSize = m_ReadFunc.GetFrameSize() * CORE2_INSTREAM_FRAMEFACTOR;
	m_pBuffer = new unsigned char[m_ulBufferSize];

	m_Stream.SetBuffer(m_pBuffer,m_ulBufferSize);
}

CCore2InStream::~CCore2InStream()
{
	if (m_pBuffer != NULL)
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;

		m_ulBufferSize = 0;
	}
}

unsigned long CCore2InStream::GetSafeFrameReadCount()
{
	if (m_ulEndBlock - m_ulCurBlock < CORE2_INSTREAM_FRAMEFACTOR)
		return m_ulEndBlock - m_ulCurBlock;
	else
		return CORE2_INSTREAM_FRAMEFACTOR;
}

/*
	Not to be confused with other BytesToSector functions that calculates
	The number of sectors to store the specified number of bytes. This function
	calculates in which frame the last byte is stored.
*/
unsigned long CCore2InStream::BytesToFrame(unsigned __int64 uiBytes)
{
	if (uiBytes == 0)
		return 0;

	unsigned long ulSectors = 0;

	while (uiBytes > m_ReadFunc.GetFrameSize())
	{
		uiBytes -= m_ReadFunc.GetFrameSize();
		ulSectors++;
	}

	return ulSectors;
}

int CCore2InStream::FillBuffer()
{
	unsigned long ulNumFrames = GetSafeFrameReadCount();
	if (!m_Read.ReadData(m_pDevice,NULL,&m_ReadFunc,m_ulCurBlock,ulNumFrames,false))
	{
		m_pLog->AddLine(_T("  Error: Unable to read user data from disc (%u, %u)."),
			m_ulCurBlock,ulNumFrames);
		return STREAM_FAIL;
	}

	m_ulBufferData = m_Stream.GetBufferDataSize();
	m_ulBufferPos = 0;

	m_ulCurBlock += ulNumFrames;

	return STREAM_OK;
}

int CCore2InStream::Read(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize)
{
	if (EOS())
		return STREAM_FAIL;

	// Check if the requested data can be found in the buffer.
	if (ulSize <= m_ulBufferData - m_ulBufferPos)
	{
		memcpy(pBuffer,m_pBuffer + m_ulBufferPos,ulSize);
		m_ulBufferPos += ulSize;

		if (pProcessedSize != NULL)
			*pProcessedSize = ulSize;
	}
	else
	{
		// First copy everything we have in the buffer.
		unsigned long ulInBuffer = m_ulBufferData - m_ulBufferPos;

		memcpy(pBuffer,m_pBuffer + m_ulBufferPos,ulInBuffer);
		m_ulBufferPos += ulInBuffer;

		unsigned long ulRemain = ulSize - ulInBuffer;

		if (EOS())
		{
			if (pProcessedSize != NULL)
				*pProcessedSize = ulInBuffer;

			return STREAM_OK;
		}

		// Check if we can read more from the CD.
		int iResult = FillBuffer();
		if (iResult != STREAM_OK)
			return iResult;

		if (ulRemain > m_ulBufferData - m_ulBufferPos)
		{
			// FIXME: Is execution of this block possible?
			unsigned long ulCanRead = m_ulBufferData - m_ulBufferPos;

			memcpy((unsigned char *)pBuffer + ulInBuffer,m_pBuffer + m_ulBufferPos,ulCanRead);
			m_ulBufferPos += ulCanRead;

			if (pProcessedSize != NULL)
				*pProcessedSize = ulInBuffer + ulCanRead;
		}
		else
		{
			memcpy((unsigned char *)pBuffer + ulInBuffer,m_pBuffer + m_ulBufferPos,ulRemain);
			m_ulBufferPos += ulRemain;

			if (pProcessedSize != NULL)
				*pProcessedSize = ulSize;
		}
	}

	return STREAM_OK;
}

bool CCore2InStream::EOS()
{
	return m_ulCurBlock >= m_ulEndBlock && m_ulBufferPos >= m_ulBufferData;
}

__int64 CCore2InStream::Seek(__int64 iDistance,eSeekMode SeekMode)
{
	// Assure that the type cast will be safe.
	if (iDistance > 0xFFFFFFFF *  m_ReadFunc.GetFrameSize())
		return -1;

	if (SeekMode == SM_BEGIN)
	{
		unsigned long ulFrame = BytesToFrame(iDistance);

		// Calculate which sector the byte distance corresponds to.
		m_ulCurBlock = m_ulStartBlock + ulFrame;

		int iResult = FillBuffer();
		if (iResult != STREAM_OK)
			return -1;

		m_ulBufferPos = (unsigned long)(iDistance - ulFrame * m_ReadFunc.GetFrameSize());
		return iDistance;
	}
	else
	{
		// Check if we can just move forward in the internal buffer.
		if (iDistance < m_ulBufferData - m_ulBufferPos)
		{
			m_ulBufferPos += (unsigned long)iDistance;
		}
		else
		{
			// In which internal frame are we?
			unsigned long ulFrame = BytesToFrame(m_ulBufferPos);

			// Search through the current frame.
			ulFrame++;
			__int64 iRemain = iDistance - (m_ulBufferData - m_ulBufferPos);

			// How many more frames should we move through.
			ulFrame += BytesToFrame(iRemain);

			m_ulCurBlock = m_ulStartBlock + ulFrame;

			int iResult = FillBuffer();
			if (iResult != STREAM_OK)
				return -1;

			m_ulBufferPos = (unsigned long)(iRemain - ulFrame * m_ReadFunc.GetFrameSize());
			return iDistance;
		}
	}

	return true;
}
