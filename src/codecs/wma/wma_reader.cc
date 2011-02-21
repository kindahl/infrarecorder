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
#include "wma_reader.hh"

CWMAReader::CWMAReader()
{
	m_pWMReader = NULL;
	m_pStream = NULL;

	m_pSample = NULL;
	m_pSampleBuffer = NULL;
	m_ulSampleBufferSize = 0;
	m_ulSampleBufferPos = 0;

	m_uiCurrentTime = 0;
}

CWMAReader::~CWMAReader()
{
	if (m_pSample != NULL)
	{
		m_pSample->Release();
		m_pSample = NULL;
	}

	if (m_pWMReader != NULL)
	{
		m_pWMReader->Release();
		m_pWMReader = NULL;
	}

	if (m_pStream != NULL)
	{
		delete m_pStream;
		m_pStream = NULL;
	}
}

HRESULT CWMAReader::GetStreamNumber(IWMProfile *pProfile,unsigned short &usStreamNumber)
{
	HRESULT hResult = S_OK;
    IWMStreamConfig *pStream = NULL;
    unsigned long ulStreamCount = 0;
    GUID pguidStreamType;

    if (pProfile == NULL)
        return E_INVALIDARG;

    hResult = pProfile->GetStreamCount(&ulStreamCount);
    if (FAILED(hResult))
        return hResult;

    usStreamNumber = 0;

	// Make sure that the file only contains audio streams.
	for (unsigned long i = 0; i < ulStreamCount; i++)
    {
        hResult = pProfile->GetStream(i,&pStream);
        if (FAILED(hResult))
            break;

        unsigned short usLocalStreamNumber = 0;

        // Get the stream number of the current stream.
        hResult = pStream->GetStreamNumber(&usLocalStreamNumber);
        if (FAILED(hResult))
            break;

        hResult = pStream->GetStreamType(&pguidStreamType);
        if (FAILED(hResult))
            break ;
        
        if (pguidStreamType == WMMEDIATYPE_Audio)
            usStreamNumber = usLocalStreamNumber;

        if (pStream != NULL)
		{
			pStream->Release();
			pStream = NULL;
		}
    }

	return hResult;
}

bool CWMAReader::Open(const TCHAR *szFileName)
{
	if (szFileName == NULL)
		return false;

	HRESULT hResult = S_OK;

	// Create reader.
	if (m_pWMReader == NULL)
	{
		hResult = g_LibraryHelper.irc_WMCreateSyncReader(NULL,0,&m_pWMReader);
		if (FAILED(hResult))
			return false;
	}

	// Open file stream.
	if (m_pStream == NULL)
	{
		m_pStream = new CReadStream();
		if (m_pStream == NULL)
			return false;
	}

	hResult = m_pStream->Open(szFileName);
	if (FAILED(hResult))
		return false;

	hResult = m_pWMReader->OpenStream(m_pStream);
	if (FAILED(hResult))
		return false;

	// Get profile interface.
	IWMProfile *pProfile = NULL;
	
	hResult = m_pWMReader->QueryInterface(IID_IWMProfile,(void **)&pProfile);
	if (FAILED(hResult))
		return false;

	//return false;

	// Find out the audio stream number.
	unsigned short usStreamNumber = 0;
	hResult = GetStreamNumber(pProfile,usStreamNumber);

	if (pProfile != NULL)
		pProfile->Release();

	if (FAILED(hResult))
		return false;

	if (usStreamNumber != 0)
	{
		WMT_STREAM_SELECTION wmtSS = WMT_ON;
        hResult = m_pWMReader->SetStreamsSelected(1,&usStreamNumber,&wmtSS);
        if (FAILED(hResult))
            return false;

        hResult = m_pWMReader->SetReadStreamSamples(usStreamNumber,FALSE);
        if (FAILED(hResult))
            return false;
    }

	// Set duration.
	hResult = m_pWMReader->SetRange(0,0);
	if (FAILED(hResult))
		return false;

	m_uiCurrentTime = 0;
	return true;
}

bool CWMAReader::Close()
{
	if (m_pWMReader != NULL)
	{
		HRESULT hResult = m_pWMReader->Close();
		if (FAILED(hResult))
			return false;

		m_pWMReader->Release();
		m_pWMReader = NULL;
	}

	return true;
}

HRESULT CWMAReader::DecodeSamples(unsigned char *pBuffer,__int64 iBufferSize,
								  __int64 &iProcessed,unsigned __int64 &uiTime)
{
	iProcessed = 0;

	// If there are any data left in the internal buffer, copy it directly to the
	// specified buffer.
	if (m_ulSampleBufferSize > m_ulSampleBufferPos)
	{
		unsigned long ulInBuffer = m_ulSampleBufferSize - m_ulSampleBufferPos;

		if (ulInBuffer > iBufferSize)
		{
			memcpy(pBuffer,m_pSampleBuffer + m_ulSampleBufferPos,(int)iBufferSize);
			m_ulSampleBufferPos += (unsigned long)iBufferSize;

			iProcessed = iBufferSize;
		}
		else
		{
			memcpy(pBuffer,m_pSampleBuffer + m_ulSampleBufferPos,ulInBuffer);

			m_ulSampleBufferSize = 0;
			m_ulSampleBufferPos = 0;
			m_pSample->Release();
			m_pSample = NULL;

			iProcessed = ulInBuffer;
		}

		uiTime = m_uiCurrentTime;
		return S_OK;
	}

    // Now when the internal buffer is empty, read more samples to the
	// internal buffer.
	HRESULT hResult = S_OK;

	unsigned short usStreamNum = 0;
	unsigned __int64 uiSampleTime = 0;
	unsigned __int64 uiDuration = 0;
	unsigned long ulFlags = 0;
	unsigned long ulOutputNum = 0;

	hResult = m_pWMReader->GetNextSample(0,&m_pSample,&uiSampleTime,&uiDuration,
		&ulFlags,&ulOutputNum,&usStreamNum);
	if (FAILED(hResult))
		return hResult;

	// Buffer management.
	hResult = m_pSample->GetBufferAndLength(&m_pSampleBuffer,&m_ulSampleBufferSize);
	if (FAILED(hResult))
		return hResult;

	if (m_ulSampleBufferSize > iBufferSize)
	{
		memcpy(pBuffer,m_pSampleBuffer,(int)iBufferSize);

		m_ulSampleBufferPos += (unsigned long)iBufferSize;
		iProcessed = iBufferSize;
	}
	else
	{
		memcpy(pBuffer,m_pSampleBuffer,m_ulSampleBufferSize);
		iProcessed = m_ulSampleBufferSize;

		m_ulSampleBufferSize = 0;

		m_pSample->Release();
		m_pSample = NULL;
	}

	// Update the internal time variable.
	m_uiCurrentTime = uiSampleTime;
	uiTime = m_uiCurrentTime;

    return hResult;
}
