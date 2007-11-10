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
#include "../../Common/CodecConst.h"
#include "../../Common/StringUtil.h"
#include <sndfile.h>
#include "LibraryHelper.h"

tirc_send_message *g_pSendMessage = NULL;

// Capability flags.
int g_iCapabilities = IRC_HAS_DECODER | IRC_HAS_ENCODER;

// Version and about strings.
TCHAR *g_szVersion = _T("0.42.1.0");
TCHAR *g_szAbout = _T("InfraRecorder Wave Codec\n\nCopyright © 2006-2007 Christian Kindahl.\n\nThis codec is based on the libsndfile library, created\nby Erik de Castro Lopo. More information can be\nfound on the libsndfile website:\nhttp://www.mega-nerd.com/libsndfile/");
TCHAR *g_szEncoder = _T("Wave");
TCHAR *g_szFileExt = _T(".wav");

// Global variables.
SNDFILE *g_hInFile = NULL;
SNDFILE *g_hOutFile = NULL;
unsigned int g_uiDecSampleRate = 0;
unsigned int g_uiDecFrameSize = 0;
unsigned __int64 g_uiDecCurrentTime = 0;

// Library helper (since this codec uses dynamic loading of libsndfile.dll).
CLibraryHelper g_LibraryHelper;

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			// Calculate the full library file name.
			TCHAR szFileName[MAX_PATH];
			::GetModuleFileName((HMODULE)hModule,szFileName,MAX_PATH - 1);

			ExtractFilePath(szFileName);
			lstrcat(szFileName,_T("libsndfile.dll"));

			// Tell the library helper to load the library.
			g_LibraryHelper.Load(szFileName);
			break;

		case DLL_PROCESS_DETACH:
		case DLL_THREAD_DETACH:
			// The codec has been detached, unload the libsndfile library.
			g_LibraryHelper.Unload();
			break;
	}

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
	if (!g_LibraryHelper.IsLoaded())
		return false;

	SF_INFO	sfInfo;

#ifdef UNICODE
	char szMultiFileName[MAX_PATH];
	TCharToChar(szFileName,szMultiFileName);
	g_hInFile = g_LibraryHelper.irc_sf_open(szMultiFileName,SFM_READ,&sfInfo);
#else
	g_hInFile = g_LibraryHelper.irc_sf_open(szFileName,SFM_READ,&sfInfo);
#endif

	if (g_hInFile == NULL)
		return false;

	// Get file information.
	iNumChannels = sfInfo.channels;
	iSampleRate = sfInfo.samplerate;

	switch (sfInfo.format & SF_FORMAT_SUBMASK)
	{
		case SF_FORMAT_PCM_S8:
		case SF_FORMAT_PCM_U8:
			iBitRate = sfInfo.samplerate << 3;
			break;

		case SF_FORMAT_PCM_16:
		case SF_FORMAT_DWVW_16:
			iBitRate = sfInfo.samplerate << 4;
			break;

		case SF_FORMAT_PCM_24:
		case SF_FORMAT_DWVW_24:
			iBitRate = sfInfo.samplerate * 24;
			break;

		case SF_FORMAT_PCM_32:
		case SF_FORMAT_FLOAT:
			iBitRate = sfInfo.samplerate << 5;
			break;

		case SF_FORMAT_DOUBLE:
			iBitRate = sfInfo.samplerate << 6;
			break;

		case SF_FORMAT_G721_32:
			iBitRate = 32000;
			break;

		case SF_FORMAT_G723_24:
			iBitRate = 24000;
			break;

		case SF_FORMAT_G723_40:
			iBitRate = 40000;
			break;

		case SF_FORMAT_DWVW_12:
			iBitRate = sfInfo.samplerate * 12;
			break;

		/*case SF_FORMAT_ULAW:
		case SF_FORMAT_ALAW:
		case SF_FORMAT_IMA_ADPCM:
		case SF_FORMAT_MS_ADPCM:
		case SF_FORMAT_GSM610:
		SF_FORMAT_VOX_ADPCM:
		case SF_FORMAT_DWVW_N:
		SF_FORMAT_DPCM_8:
		SF_FORMAT_DPCM_16:*/

		default:
			iBitRate = -1;

			g_LibraryHelper.irc_sf_close(g_hInFile);
			g_hInFile = NULL;
			return false;
			//break;
	}

	// Calculate file duration (in milliseconds).
	uiDuration = 0;
	if (sfInfo.samplerate != 0)
		uiDuration = (sfInfo.frames / sfInfo.samplerate) * 1000;

	g_uiDecCurrentTime = 0;

	// We need to remember the samplerate and frame size so we can calculate
	// the current time when decoding the file.
	g_uiDecSampleRate = iSampleRate;
	g_uiDecFrameSize = ((iBitRate / iSampleRate) >> 3) * iNumChannels;

	return true;
}

__int64 WINAPI irc_decode_process(unsigned char *pBuffer,__int64 iBufferSize,
								  unsigned __int64 &uiTime)
{
	if (!g_LibraryHelper.IsLoaded())
		return -1;

	if (iBufferSize < 0)
		return -1;

	if (g_hInFile == NULL)
		return -1;

	__int64 iRead = g_LibraryHelper.irc_sf_read_raw(g_hInFile,pBuffer,iBufferSize);

	// Current time (in milliseconds).
	g_uiDecCurrentTime += ((iRead / g_uiDecFrameSize) * 1000) / g_uiDecSampleRate;
	uiTime = g_uiDecCurrentTime;

	return iRead;
}

bool WINAPI irc_decode_exit()
{
	if (!g_LibraryHelper.IsLoaded())
		return false;

	if (g_hInFile == NULL)
		return false;

	g_LibraryHelper.irc_sf_close(g_hInFile);
	g_hInFile = NULL;

	return true;
}

bool WINAPI irc_encode_init(const TCHAR *szFileName,int iNumChannels,
							int iSampleRate,int iBitRate)
{
	if (!g_LibraryHelper.IsLoaded())
		return false;

	SF_INFO sfInfo;
	sfInfo.channels = iNumChannels;
	sfInfo.samplerate = iSampleRate;
	sfInfo.format = SF_FORMAT_WAV;

	switch (iBitRate / iSampleRate)
	{
		case 8:
			sfInfo.format |= SF_FORMAT_PCM_S8;
			break;

		case 16:
			sfInfo.format |= SF_FORMAT_PCM_16;
			break;

		case 24:
			sfInfo.format |= SF_FORMAT_PCM_24;
			break;

		case 32:
			sfInfo.format |= SF_FORMAT_PCM_32;
			break;

		default:
			return false;
	}

	if (!g_LibraryHelper.irc_sf_format_check(&sfInfo))
		return false;

#ifdef UNICODE
	char szMultiFileName[MAX_PATH];
	TCharToChar(szFileName,szMultiFileName);
	g_hOutFile = g_LibraryHelper.irc_sf_open(szMultiFileName,SFM_WRITE,&sfInfo);
#else
	g_hOutFile = g_LibraryHelper.irc_sf_open(szFileName,SFM_WRITE,&sfInfo);
#endif

	return true;
}

__int64 WINAPI irc_encode_process(unsigned char *pBuffer,__int64 iDataSize)
{
	if (!g_LibraryHelper.IsLoaded())
		return -1;

	if (iDataSize < 0)
		return -1;

	if (g_hOutFile == NULL)
		return -1;

	return g_LibraryHelper.irc_sf_write_raw(g_hOutFile,pBuffer,iDataSize);
}

__int64 WINAPI irc_encode_flush()
{
	return 0;
}

bool WINAPI irc_encode_exit()
{
	if (!g_LibraryHelper.IsLoaded())
		return false;

	if (g_hOutFile == NULL)
		return false;

	g_LibraryHelper.irc_sf_close(g_hOutFile);
	g_hOutFile = NULL;

	return true;
}

bool WINAPI irc_encode_config()
{
	return false;
}
