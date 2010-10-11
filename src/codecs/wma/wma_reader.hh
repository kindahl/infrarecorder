/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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
#include <wmsdk.h>
#include "read_stream.hh"

class CWMAReader
{
private:
	CReadStream *m_pStream;

	INSSBuffer *m_pSample;
	unsigned char *m_pSampleBuffer;
	unsigned long m_ulSampleBufferSize;
	unsigned long m_ulSampleBufferPos;

	unsigned __int64 m_uiCurrentTime;

	HRESULT GetStreamNumber(IWMProfile *pProfile,unsigned short &usStreamNumber);

public:
	IWMSyncReader *m_pWMReader;

	CWMAReader();
	~CWMAReader();

	bool Open(const TCHAR *szFileName);
	bool Close();

	HRESULT DecodeSamples(unsigned char *pBuffer,__int64 iBufferSize,
		__int64 &iProcessed,unsigned __int64 &uiTime);
};
