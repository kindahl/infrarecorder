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

#include "stdafx.hh"
#include <ckcore/file.hh>
#include <base/codec_const.hh>
#include <base/string_util.hh>
#include <lame.h>
#include "config_dlg.hh"
#include "lame.hh"

tirc_send_message *g_pSendMessage = NULL;

// Capability flags.
int g_iCapabilities = IRC_HAS_ENCODER | IRC_HAS_CONFIG;

// Version and about strings.
TCHAR *g_szVersion = _T("0.42.1.0");
TCHAR *g_szAbout = _T("InfraRecorder MP3 Encoder\n\nCopyright © 2006-2010 Christian Kindahl.\n\nThis codec is using the libmp3lame (LAME) library.\nVisit: http://lame.sourceforge.net/ for more information.\n\nPlease note that personal and/or commercial use of\ncompiled versions of this codec requires a patent license\nin some countries. Please check before using this codec.");
TCHAR *g_szEncoder = _T("MP3");
TCHAR *g_szFileExt = _T(".mp3");

// Global variables.
ckcore::File *g_pOutFile = NULL;
int g_iNumChannels = -1;
int g_iSampleRate = -1;
int g_iBitRate = -1;

lame_global_flags *gfp;

unsigned char *g_pEncBuffer = NULL;
unsigned int g_uiEncBufferSize = 4096;

// Encoder configuration.
CEncoderConfig g_EncoderConfig;

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
    return TRUE;
}

bool LocalSendMessage(int iType,const TCHAR *szMessage)
{
	if (g_pSendMessage == NULL)
		return false;

	g_pSendMessage(iType,szMessage);
	return true;
}

/*
	LAME error handlers.
*/
void mp3_errorf(const char *szFormat,va_list ap)
{
	// The 64-bit compiler does not support this type of usage of va_start.
#ifdef _M_X64
	LocalSendMessage(IRC_MESSAGE_ERROR,_T("Unknown error in lame.irc (x64 build)."));
#else
	va_start(ap,szFormat);

	int iLength = _vscprintf(szFormat,ap) + 1;
	char *szMessage = new char[iLength];
	vsprintf(szMessage,szFormat,ap);

#ifdef UNICODE
	TCHAR *szWideMessage = new TCHAR[iLength];
	AnsiToUnicode(szWideMessage,szMessage,iLength);

	LocalSendMessage(IRC_MESSAGE_ERROR,szWideMessage);
	delete [] szWideMessage;
#else
	LocalSendMessage(IRC_MESSAGE_ERROR,szMessage);
#endif

	delete [] szMessage;
#endif
}

void mp3_debugf(const char *szFormat,va_list ap)
{
	// Ignore debug messages.
}

void mp3_msgf(const char *szFormat,va_list ap)
{
#ifdef _M_X64
	// Ignore information messages.
#else
	va_start(ap,szFormat);

	int iLength = _vscprintf(szFormat,ap) + 1;
	char *szMessage = new char[iLength];
	vsprintf(szMessage,szFormat,ap);

#ifdef UNICODE
	TCHAR *szWideMessage = new TCHAR[iLength];
	AnsiToUnicode(szWideMessage,szMessage,iLength);

	LocalSendMessage(IRC_MESSAGE_INFO,szWideMessage);
	delete [] szWideMessage;
#else
	LocalSendMessage(IRC_MESSAGE_INFO,szMessage);
#endif

	delete [] szMessage;
#endif
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
	return false;
}

__int64 WINAPI irc_decode_process(unsigned char *pBuffer,__int64 iBufferSize,
								  unsigned __int64 &uiTime)
{
	return -1;
}

bool WINAPI irc_decode_exit()
{
	return false;
}

bool WINAPI irc_encode_init(const TCHAR *szFileName,int iNumChannels,
							int iSampleRate,int iBitRate)
{
	g_iNumChannels = iNumChannels;
	g_iSampleRate = iSampleRate;
	g_iBitRate = iBitRate;

	// Initialize LAME.
	gfp = lame_init();

	// Setup format settings.
	lame_set_num_channels(gfp,iNumChannels);
	lame_set_in_samplerate(gfp,iSampleRate);

	// Configure the encoder.
	switch (g_EncoderConfig.m_iPreset)
	{
		case 0:		// Custom.
			{
				if (g_EncoderConfig.m_iEncodeQuality == CONFIG_EQ_FAST)
					lame_set_quality(gfp,7);
				else if (g_EncoderConfig.m_iEncodeQuality == CONFIG_EQ_HIGH)
					lame_set_quality(gfp,2);

				if (g_EncoderConfig.m_bMono)
					lame_set_mode(gfp,MONO);

				if (g_EncoderConfig.m_bBitrateMode)
				{
					// Bitrate mode.
					if (g_EncoderConfig.m_bConstantBitrate)
					{
						// CBR.
						lame_set_VBR(gfp,vbr_off);
						lame_set_brate(gfp,g_EncoderConfig.m_iBitrate);
					}
					else
					{
						// ARB.
						lame_set_VBR(gfp,vbr_abr);
						lame_set_VBR_mean_bitrate_kbps(gfp,g_EncoderConfig.m_iBitrate);
					}
				}
				else
				{
					// Quality mode.
					lame_set_VBR(gfp,g_EncoderConfig.m_bFastVBR ? vbr_mtrh : vbr_rh);
					lame_set_VBR_q(gfp,g_EncoderConfig.m_iQuality);
				}
			}
			break;

		case 1:		// Medium.
			lame_set_preset(gfp,MEDIUM);
			break;

		case 2:		// Standard.
			lame_set_preset(gfp,STANDARD);
			break;

		case 3:		// Extreme.
			lame_set_preset(gfp,EXTREME);
			break;

		case 4:		// Insane.
			lame_set_preset(gfp,INSANE);
			break;
	}

	// Change error, debug and message handlers.
	lame_set_errorf(gfp,mp3_errorf);
	lame_set_debugf(gfp,mp3_debugf);
	lame_set_msgf(gfp,mp3_msgf);

	// Initialize parameters.
	if (lame_init_params(gfp) < 0)
	{
		lame_close(gfp); 
		return false;
	}

	// Allocate encode buffer.
	if (g_pEncBuffer != NULL)
		delete [] g_pEncBuffer;

	g_uiEncBufferSize = iNumChannels * ((g_iBitRate / g_iSampleRate) >> 3) * BUFFER_FACTOR;
	g_pEncBuffer = new unsigned char[g_uiEncBufferSize];

	// Open file.
	if (g_pOutFile != NULL)
	{
		delete g_pOutFile;
		g_pOutFile = NULL;
	}

	g_pOutFile = new ckcore::File(szFileName);
	if (!g_pOutFile->open(ckcore::File::ckOPEN_WRITE))
	{
		delete g_pOutFile;
		g_pOutFile = NULL;

		return false;
	}

	return true;
}

__int64 WINAPI irc_encode_process(unsigned char *pBuffer,__int64 iDataSize)
{
	if (iDataSize > 0xFFFFFFFF)
		return false;

	if (g_pOutFile == NULL)
		return false;

	unsigned int uiSampleSize = (g_iBitRate / g_iSampleRate) >> 3;
	unsigned int uiNumSamples = ((unsigned int)iDataSize / uiSampleSize) / g_iNumChannels;

	int iWritten = lame_encode_buffer_interleaved(gfp,(short int *)pBuffer,uiNumSamples,g_pEncBuffer,g_uiEncBufferSize);

	if (iWritten > 0)
	{
		if (g_pOutFile->write(g_pEncBuffer,iWritten) == -1)
			return -1;
	}
	else if (iWritten < 0)
	{
		return -1;
	}

	return (__int64)iWritten;
}

__int64 WINAPI irc_encode_flush()
{
	return lame_encode_flush(gfp,g_pEncBuffer,g_uiEncBufferSize);
}

bool WINAPI irc_encode_exit()
{
	// Free the encode buffer.
	if (g_pEncBuffer != NULL)
	{
		delete [] g_pEncBuffer;
		g_pEncBuffer = NULL;
	}

	// Close file handle.
	if (g_pOutFile != NULL)
	{
		delete g_pOutFile;
		g_pOutFile = NULL;
	}

	// Close the LAME encoder.
	lame_close(gfp);

	return false;
}

bool WINAPI irc_encode_config()
{
	CConfigDlg ConfigDlg(&g_EncoderConfig);
	ConfigDlg.DoModal();

	return true;
}
