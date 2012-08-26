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

#pragma once
#include <ckcore/stream.hh>
#include <ckcore/log.hh>
#include "core2_read.hh"

#define CORE2_INSTREAM_FRAMEFACTOR			10	// We will cache 10 frames in the memory.

class CCore2InStream : public ckcore::InStream
{
private:
    class CInternalStream : public ckcore::OutStream
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

        ckcore::tint64 write(const void *pBuffer,ckcore::tuint32 uiCount)
        {
            if (m_pBuffer == NULL)
                return -1;

            if (m_ulBufferSize < uiCount)
                return -1;

            memcpy(m_pBuffer,pBuffer,uiCount);
            m_ulBufferData = uiCount;

            return uiCount;
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

    ckcore::Log *m_pLog;
    ckmmc::Device &m_Device;

    Core2ReadFunction::CReadUserData m_ReadFunc;
    CCore2Read m_Read;

    unsigned long GetSafeFrameReadCount();
    unsigned long BytesToFrame(unsigned __int64 uiBytes);
    bool FillBuffer();

public:
    CCore2InStream(ckcore::Log *pLog,ckmmc::Device &Device,
        unsigned long ulStartBlock,unsigned long ulEndBlock);
    ~CCore2InStream();

    // ckCore::InStream.
    ckcore::tint64 read(void *pBuffer,ckcore::tuint32 uiCount);
    ckcore::tint64 size();
    bool end();
    bool seek(ckcore::tuint32 uiDistnace,ckcore::InStream::StreamWhence Whence);
};