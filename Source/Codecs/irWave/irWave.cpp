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
#include <vfw.h>
#include "../../Common/CodecConst.h"
#include "../../Common/StringUtil.h"
#include "WaveWriter.h"

#pragma comment(lib,"vfw32.lib")

tirc_send_message *g_pSendMessage = NULL;

// Capability flags.
int g_iCapabilities = IRC_HAS_DECODER | IRC_HAS_ENCODER;

// Version and about strings.
TCHAR *g_szVersion = _T("0.42.1.0");
TCHAR *g_szAbout = _T("InfraRecorder Wave Codec\n\nCopyright © 2006-2008 Christian Kindahl.");
TCHAR *g_szEncoder = _T("Wave");
TCHAR *g_szFileExt = _T(".wav");

// Global variables.
PAVIFILE g_pAVIFile = NULL;
PAVISTREAM g_pAVIStream = NULL;
long g_lCurSample = 0;
CWaveWriter g_WaveWriter;

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		// Initialize COM for the current thread if it hasn't already been initialized.
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			AVIFileInit();
			break;

		case DLL_PROCESS_DETACH:
		case DLL_THREAD_DETACH:
			AVIFileExit();
			break;
	};

    return TRUE;
}

/*
	irc_capabilities
	----------------
	Returns bit information on what operations that are supported by the codec.
*/
int WINAPI irc_capabilities()
{
	return g_iCapabilities;
}

TCHAR *WINAPI irc_string(unsigned int uiID)
{
	switch (uiID)
	{
		case IRC_STR_VERSION:
			return g_szVersion;

		case IRC_STR_ABOUT:
			return g_szAbout;

		case IRC_STR_ENCODER:
			return g_szEncoder;

		case IRC_STR_FILEEXT:
			return g_szFileExt;
	}

	return NULL;
}

bool WINAPI irc_set_callback(tirc_send_message *pSendMessage)
{
	if (pSendMessage == NULL)
		return false;

	g_pSendMessage = pSendMessage;
	return true;
}

bool WINAPI irc_decode_init(const TCHAR *szFileName,int &iNumChannels,
							int &iSampleRate,int &iBitRate,unsigned __int64 &uiDuration)
{
	// Open the file.
	HRESULT hResult = AVIFileOpen(&g_pAVIFile,szFileName,OF_SHARE_DENY_NONE,NULL);
	if (FAILED(hResult))
		return false;

	// Get the first stream in the file (we assume only one audio stream).
	hResult = AVIFileGetStream(g_pAVIFile,&g_pAVIStream,streamtypeAUDIO,0);
	if (FAILED(hResult))
		return false;

	// Gather stream information.
	long lAudioInfoSize = 0;
	hResult = AVIStreamFormatSize(g_pAVIStream,0,&lAudioInfoSize);
	if (FAILED(hResult))
		return false;

	if (lAudioInfoSize < sizeof(WAVEFORMATEX))
		lAudioInfoSize = sizeof(WAVEFORMATEX);

	WAVEFORMATEX *pAudioInfo = (WAVEFORMATEX *)new unsigned char[lAudioInfoSize];
	if (pAudioInfo == NULL)
		return false;

	hResult = AVIStreamReadFormat(g_pAVIStream,0,pAudioInfo,&lAudioInfoSize);
	if (FAILED(hResult))
	{
		delete [] pAudioInfo;
		return false;
	}

	if (pAudioInfo->wFormatTag == WAVE_FORMAT_PCM)
		pAudioInfo->cbSize = 0;

    iNumChannels = pAudioInfo->nChannels;
	iSampleRate = pAudioInfo->nSamplesPerSec;
	iBitRate = iSampleRate * pAudioInfo->wBitsPerSample;

	// Get stream duration in milliseconds.
	long lLength = AVIStreamLength(g_pAVIStream);
	uiDuration = AVIStreamSampleToTime(g_pAVIStream,lLength);

	g_lCurSample = AVIStreamStart(g_pAVIStream);

	delete [] pAudioInfo;
	return true;
}

__int64 WINAPI irc_decode_process(unsigned char *pBuffer,__int64 iBufferSize,
								  unsigned __int64 &uiTime)
{
	if (iBufferSize > 0x7FFFFFFF)
		return -1;

	long lProcessed = 0;
	long lProcessedSamples = 0;

	HRESULT hResult = AVIStreamRead(g_pAVIStream,g_lCurSample,AVISTREAMREAD_CONVENIENT,
		pBuffer,(long)iBufferSize,&lProcessed,&lProcessedSamples);
	if (FAILED(hResult))
		return -1;

	g_lCurSample += lProcessedSamples;
	uiTime = AVIStreamSampleToTime(g_pAVIStream,g_lCurSample);
	return lProcessed;
}

bool WINAPI irc_decode_exit()
{
	HRESULT hResult = AVIStreamRelease(g_pAVIStream);
	if (FAILED(hResult))
		return false;
	g_pAVIStream = NULL;

	hResult = AVIFileRelease(g_pAVIFile);
	if (FAILED(hResult))
		return false;
	g_pAVIFile = NULL;

	return true;
}

bool WINAPI irc_encode_init(const TCHAR *szFileName,int iNumChannels,
							int iSampleRate,int iBitRate)
{
	return g_WaveWriter.Open(szFileName,iNumChannels,iSampleRate,iBitRate);
}

__int64 WINAPI irc_encode_process(unsigned char *pBuffer,__int64 iDataSize)
{
	return g_WaveWriter.Write(pBuffer,iDataSize);
}

__int64 WINAPI irc_encode_flush()
{
	return 0;
}

bool WINAPI irc_encode_exit()
{
	return g_WaveWriter.Close();
}

bool WINAPI irc_encode_config()
{
	return false;
}
