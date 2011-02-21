/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include "core2_read.hh"
#include "core2_stream.hh"

CCore2InStream::CCore2InStream(ckcore::Log *pLog,ckmmc::Device &Device,
							   unsigned long ulStartBlock,unsigned long ulEndBlock) :
	m_pLog(pLog),m_Device(Device),m_ReadFunc(Device,&m_Stream),m_ulStartBlock(ulStartBlock),
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

bool CCore2InStream::FillBuffer()
{
	unsigned long ulNumFrames = GetSafeFrameReadCount();
	if (!m_Read.ReadData(m_Device,NULL,&m_ReadFunc,m_ulCurBlock,ulNumFrames,false))
	{
		m_pLog->print_line(_T("  Error: Unable to read user data from disc (%u, %u)."),
			m_ulCurBlock,ulNumFrames);
		return false;
	}

	m_ulBufferData = m_Stream.GetBufferDataSize();
	m_ulBufferPos = 0;

	m_ulCurBlock += ulNumFrames;

	return true;
}

ckcore::tint64 CCore2InStream::read(void *pBuffer,ckcore::tuint32 uiCount)
{
	if (end())
		return -1;	// W00t? This was discovered when switching to ckcore.

	ckcore::tint64 iResult = 0;

	// Check if the requested data can be found in the buffer.
	if (uiCount <= m_ulBufferData - m_ulBufferPos)
	{
		memcpy(pBuffer,m_pBuffer + m_ulBufferPos,uiCount);
		m_ulBufferPos += uiCount;

		iResult = uiCount;
	}
	else
	{
		// First copy everything we have in the buffer.
		unsigned long ulInBuffer = m_ulBufferData - m_ulBufferPos;

		memcpy(pBuffer,m_pBuffer + m_ulBufferPos,ulInBuffer);
		m_ulBufferPos += ulInBuffer;

		unsigned long ulRemain = uiCount - ulInBuffer;

		if (end())
			return ulInBuffer;

		// Check if we can read more from the CD.
		if (!FillBuffer())
			return -1;

		if (ulRemain > m_ulBufferData - m_ulBufferPos)
		{
			// FIXME: Is execution of this block possible?
			unsigned long ulCanRead = m_ulBufferData - m_ulBufferPos;

			memcpy((unsigned char *)pBuffer + ulInBuffer,m_pBuffer + m_ulBufferPos,ulCanRead);
			m_ulBufferPos += ulCanRead;

			iResult = ulInBuffer + ulCanRead;
		}
		else
		{
			memcpy((unsigned char *)pBuffer + ulInBuffer,m_pBuffer + m_ulBufferPos,ulRemain);
			m_ulBufferPos += ulRemain;

			iResult = uiCount;
		}
	}

	return iResult;
}

ckcore::tint64 CCore2InStream::size()
{
	return -1;
}

bool CCore2InStream::end()
{
	return m_ulCurBlock >= m_ulEndBlock && m_ulBufferPos >= m_ulBufferData;
}

bool CCore2InStream::seek(ckcore::tuint32 uiDistance,ckcore::InStream::StreamWhence Whence)
{
	if (Whence == ckcore::InStream::ckSTREAM_BEGIN)
	{
		unsigned long ulFrame = BytesToFrame(uiDistance);

		// Calculate which sector the byte distance corresponds to.
		m_ulCurBlock = m_ulStartBlock + ulFrame;

		if (!FillBuffer())
			return false;

		m_ulBufferPos = (unsigned long)(uiDistance - ulFrame * m_ReadFunc.GetFrameSize());
		return true;
	}
	else
	{
		// Check if we can just move forward in the internal buffer.
		if (uiDistance < m_ulBufferData - m_ulBufferPos)
		{
			m_ulBufferPos += (unsigned long)uiDistance;
		}
		else
		{
			// In which internal frame are we?
			unsigned long ulFrame = BytesToFrame(m_ulBufferPos);

			// Search through the current frame.
			ulFrame++;
			__int64 iRemain = uiDistance - (m_ulBufferData - m_ulBufferPos);

			// How many more frames should we move through.
			ulFrame += BytesToFrame(iRemain);

			m_ulCurBlock = m_ulStartBlock + ulFrame;

			if (!FillBuffer())
				return false;

			m_ulBufferPos = (unsigned long)(iRemain - ulFrame * m_ReadFunc.GetFrameSize());
			return true;
		}
	}

	return true;
}
