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
#include <ckcore/file.hh>
#include <base/codec_const.hh>
#include <stdlib.h>
#include <time.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include "config_dlg.hh"
#include "vorbis.hh"

/*#ifdef _M_X64
#pragma comment(lib,"libogg/win32/Static_Release/ogg_static_x64.lib")
#else
#pragma comment(lib,"libogg/win32/Static_Release/ogg_static.lib")
#endif

#pragma comment(lib,"libvorbis/win32/Vorbis_Static_Release/vorbis_static.lib")
#pragma comment(lib,"libvorbis/win32/VorbisEnc_Static_Release/vorbisenc_static.lib")
#pragma comment(lib,"libvorbis/win32/VorbisFile_Static_Release/vorbisfile_static.lib")*/

tirc_send_message *g_pSendMessage = NULL;

// Capability flags.
int g_iCapabilities = IRC_HAS_DECODER | IRC_HAS_ENCODER | IRC_HAS_CONFIG;

// Version and about strings.
TCHAR *g_szVersion = _T("0.42.1.0");
TCHAR *g_szAbout = _T("InfraRecorder Ogg Vorbis Codec\n\nCopyright © 2006-2010 Christian Kindahl.\n\nThis codec is using the following 3rd party libraries:\n - libogg: Copyright © 2002, Xiph.org Foundation.\n - libvorbis: Copyright © 2002-2004 Xiph.org Foundation.");
TCHAR *g_szEncoder = _T("Ogg Vorbis");
TCHAR *g_szFileExt = _T(".ogg");

// Global variables.
ckcore::File *g_pFile = NULL;
FILE *g_hInFile = NULL;
int g_iNumChannels = -1;
int g_iSampleRate = -1;
int g_iBitRate = -1;

// Encoding.
ogg_page og;
ogg_stream_state os;
vorbis_dsp_state vd;
vorbis_block vb;
vorbis_info vi;

// Decoding.
OggVorbis_File vf;

// Encoder configuration.
CEncoderConfig g_EncoderConfig;

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
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
	// Get file handle.
#ifdef UNICODE
	g_hInFile = _wfopen(szFileName,_T("rb"));
#else
	g_hInFile = fopen(szFileName,"rb");
#endif

	if (g_hInFile == NULL)
		return false;

	// Open Vorbis bitstream.
	if (ov_open(g_hInFile,&vf,NULL,0) < 0)
		return false;

	// Throw the comments plus a few lines about the bitstream we're decoding.
	{
		char **ppTemp = ov_comment(&vf,-1)->user_comments;
		vorbis_info *vi = ov_info(&vf,-1);

		while (*ppTemp)
			++ppTemp;

		iNumChannels = vi->channels;
		iSampleRate = vi->rate;
		iBitRate = iSampleRate << 4;
	}

	// Get file duration (in milliseconds).
	uiDuration = (unsigned __int64)ov_time_total(&vf,-1) * 1000;

	return true;
}

__int64 WINAPI irc_decode_process(unsigned char *pBuffer,__int64 iBufferSize,
								  unsigned __int64 &uiTime)
{
	static int iCurrentSection;

	if (iBufferSize > 0xFFFFFFFF)
		return -1;

	unsigned long ulRead = ov_read(&vf,(char *)pBuffer,(unsigned int)iBufferSize,0,2,1,&iCurrentSection);
	if (ulRead < 0)
		return -1;

	// Current time (in milliseconds).
	uiTime = (unsigned __int64)ov_time_tell(&vf) * 1000;

	return ulRead;
}

bool WINAPI irc_decode_exit()
{
	// Close the file handle.
	if (g_hInFile != NULL)
	{
		fclose(g_hInFile);
		g_hInFile = NULL;
	}

	// Exit the decoder.
	ov_clear(&vf);

	return true;
}

bool WINAPI irc_encode_init(const TCHAR *szFileName,int iNumChannels,
							int iSampleRate,int iBitRate)
{
	// Currently we only support a maximum of six channels.
	if (iNumChannels > 6)
		return false;

	if (g_pFile != NULL)
		return false;

	g_iNumChannels = iNumChannels;
	g_iSampleRate = iSampleRate;
	g_iBitRate = iBitRate;

	// Open output file.
	g_pFile = new ckcore::File(szFileName);
	if (!g_pFile->open(ckcore::File::ckOPEN_WRITE))
	{
		delete g_pFile;
		g_pFile = NULL;

		return false;
	}

	// Initialize encoder.
	vorbis_info_init(&vi);

	// Variable bit rate.
	/*if (vorbis_encode_init_vbr(&vi,iNumChannels,iSampleRate,0.1f))
		return false;*/

	// Setup configuration.
	switch (g_EncoderConfig.m_iMode)
	{
		case CONFIG_MODE_QUALITY:
			if (vorbis_encode_init_vbr(&vi,iNumChannels,iSampleRate,
				(float)g_EncoderConfig.m_iQuality/100.0f) < 0)
				return false;
			break;

		case CONFIG_MODE_BITRATE:
			if (vorbis_encode_init(&vi,iNumChannels,iSampleRate,g_EncoderConfig.m_iBitrate * 1000,
				g_EncoderConfig.m_iBitrate * 1000,g_EncoderConfig.m_iBitrate * 1000) < 0)
				return false;
			break;

		case CONFIG_MODE_VARBITRATE:
			if (vorbis_encode_init(&vi,iNumChannels,iSampleRate,g_EncoderConfig.m_iMaxBitrate * 1000,
				-1,g_EncoderConfig.m_iMinBitrate * 1000) < 0)
				return false;
			break;

		case CONFIG_MODE_AVBITRATE:
			if (vorbis_encode_init(&vi,iNumChannels,iSampleRate,-1,
				g_EncoderConfig.m_iAvBitrate * 1000,-1) < 0)
				return false;
			break;
	}

	// Add a comment.
	vorbis_comment vc;
	vorbis_comment_init(&vc);
	vorbis_comment_add_tag(&vc,"ENCODER","irVorbis.irc");

	// Set up the analysis state and auxiliary encoding storage.
	vorbis_analysis_init(&vd,&vi);
	vorbis_block_init(&vd,&vb);

	// Pick a random serial number; that way we can more likely build chained streams just by concatenation.
	srand((int)time(NULL));
	ogg_stream_init(&os,rand());

	// Write the three header packets.
	{
		ogg_packet opHeader;
		ogg_packet opHeaderComment;
		ogg_packet opHeaderCode;

		vorbis_analysis_headerout(&vd,&vc,&opHeader,&opHeaderComment,&opHeaderCode);
		ogg_stream_packetin(&os,&opHeader);
		ogg_stream_packetin(&os,&opHeaderComment);
		ogg_stream_packetin(&os,&opHeaderCode);

		// This ensures the actual audio data will start on a new page, as per spec.
		int iResult;

		while ((iResult = ogg_stream_flush(&os,&og)))
		{
			if (iResult == 0)
				break;

			g_pFile->write(og.header,og.header_len);
			g_pFile->write(og.body,og.body_len);
		}
	}

	vorbis_comment_clear(&vc);
	return true;
}

/*
	irc_encode_flush_ex
	-------------------
	Internal function. Flushes the vorbis buffer and writes the output to the
	file.
*/
__int64 irc_encode_flush_ex()
{
	__int64 iWritten = 0;

	// Vorbis does some data preanalysis, then divvies up blocks for
	// more involved (potentially parallel) processing. Get a single
	// block for encoding now.
	ogg_packet op;

	while (vorbis_analysis_blockout(&vd,&vb) == 1)
	{
		// Analysis, assume we want to use bitrate management.
		vorbis_analysis(&vb,NULL);
		vorbis_bitrate_addblock(&vb);

		while (vorbis_bitrate_flushpacket(&vd,&op))
		{
			// Weld the packet into the bitstream.
			ogg_stream_packetin(&os,&op);
	
			// Write out pages (if any).
			while (true)
			{
				if (ogg_stream_pageout(&os,&og) == 0)
					break;

				g_pFile->write(og.header,og.header_len);
				g_pFile->write(og.body,og.body_len);

				iWritten += og.header_len + og.body_len;
	  
				// This could be set above, but for illustrative purposes, I do
				// it here (to show that vorbis does know where the stream ends).
				if (ogg_page_eos(&og))
					return iWritten;
			}
		}
	}

	return iWritten;
}

__int64 WINAPI irc_encode_process(unsigned char *pBuffer,__int64 iDataSize)
{
	// The Vorbis encoder can only support 0xFFFFFFFF samples per analysis.
	if (iDataSize > 0xFFFFFFFF)
		return -1;

	// Deinterleave.
	unsigned int uiSampleSize = (g_iBitRate / g_iSampleRate) >> 3;
	unsigned int uiNumSamples = ((int)iDataSize / uiSampleSize) / g_iNumChannels;

	float **ppBuffer = vorbis_analysis_buffer(&vd,uiNumSamples);

	// The following code is in serious need of optimization.
	unsigned int uiSampleBitSize = g_iBitRate / g_iSampleRate;
	float fScaler = (float)((int)1 << (uiSampleBitSize - 1));

	switch (g_iNumChannels)
	{
		// Three channels.
		case 3:
			for (unsigned int i = 0; i < uiNumSamples; i++)
			{
				int iTemp1 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize]);
				int iTemp2 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize]);
				int iTemp3 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize]);

				unsigned int uiLastJ = uiSampleSize - 1;
				unsigned int uiShift = 8;

				for (unsigned int j = 1; j < uiSampleSize; j++)
				{
					if (j != uiLastJ)
					{
						iTemp1 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize + j] << uiShift);
						iTemp2 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize + j] << uiShift);
						iTemp3 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize + j] << uiShift);
					}
					else
					{
						iTemp1 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize + j] << uiShift);
						iTemp2 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize + j] << uiShift);
						iTemp3 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize + j] << uiShift);
					}

					uiShift += 8;
				}

				ppBuffer[0][i] = iTemp1/fScaler;
				ppBuffer[1][i] = iTemp2/fScaler;
				ppBuffer[2][i] = iTemp3/fScaler;
			}
			break;

		// Five channels.
		case 5:
			for (unsigned int i = 0; i < uiNumSamples; i++)
			{
				// 6-channels.
				int iTemp1 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize]);
				int iTemp2 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize]);
				int iTemp3 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize]);
				int iTemp4 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 3 * uiSampleSize]);
				int iTemp5 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 4 * uiSampleSize]);
	
				unsigned int uiLastJ = uiSampleSize - 1;
				unsigned int uiShift = 8;

				for (unsigned int j = 1; j < uiSampleSize; j++)
				{
					if (j != uiLastJ)
					{
						iTemp1 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize + j] << uiShift);
						iTemp2 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize + j] << uiShift);
						iTemp3 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize + j] << uiShift);
						iTemp4 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 3 * uiSampleSize + j] << uiShift);
						iTemp5 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 4 * uiSampleSize + j] << uiShift);
					}
					else
					{
						iTemp1 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize + j] << uiShift);
						iTemp2 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize + j] << uiShift);
						iTemp3 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize + j] << uiShift);
						iTemp4 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 3 * uiSampleSize + j] << uiShift);
						iTemp5 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 4 * uiSampleSize + j] << uiShift);
					}

					uiShift += 8;
				}

				ppBuffer[0][i] = iTemp1/fScaler;
				ppBuffer[1][i] = iTemp2/fScaler;
				ppBuffer[2][i] = iTemp3/fScaler;
				ppBuffer[3][i] = iTemp4/fScaler;
				ppBuffer[4][i] = iTemp5/fScaler;
			}
			break;

		// Six channels.
		case 6:
			for (unsigned int i = 0; i < uiNumSamples; i++)
			{
				// 6-channels.
				int iTemp1 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize]);
				int iTemp2 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize]);
				int iTemp3 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize]);
				int iTemp4 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 4 * uiSampleSize]);
				int iTemp5 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 5 * uiSampleSize]);
				int iTemp6 = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + 3 * uiSampleSize]);

				unsigned int uiLastJ = uiSampleSize - 1;
				unsigned int uiShift = 8;

				for (unsigned int j = 1; j < uiSampleSize; j++)
				{
					if (j != uiLastJ)
					{
						iTemp1 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize + j] << uiShift);
						iTemp2 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize + j] << uiShift);
						iTemp3 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize + j] << uiShift);
						iTemp4 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 4 * uiSampleSize + j] << uiShift);
						iTemp5 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 5 * uiSampleSize + j] << uiShift);
						iTemp6 |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 3 * uiSampleSize + j] << uiShift);
					}
					else
					{
						iTemp1 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 0 * uiSampleSize + j] << uiShift);
						iTemp2 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 2 * uiSampleSize + j] << uiShift);
						iTemp3 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 1 * uiSampleSize + j] << uiShift);
						iTemp4 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 4 * uiSampleSize + j] << uiShift);
						iTemp5 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 5 * uiSampleSize + j] << uiShift);
						iTemp6 |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + 3 * uiSampleSize + j] << uiShift);
					}

					uiShift += 8;
				}

				ppBuffer[0][i] = iTemp1/fScaler;
				ppBuffer[1][i] = iTemp2/fScaler;
				ppBuffer[2][i] = iTemp3/fScaler;
				ppBuffer[3][i] = iTemp4/fScaler;
				ppBuffer[4][i] = iTemp5/fScaler;
				ppBuffer[5][i] = iTemp6/fScaler;
			}
			break;

		// One, two and four channels.
		case 1:
		case 2:
		case 4:
			for (unsigned int i = 0; i < uiNumSamples; i++)
			{
				for (int k = 0; k < g_iNumChannels; k++)
				{
					int iTemp = (0xFF & (int)pBuffer[i * (uiSampleSize * g_iNumChannels) + k * uiSampleSize]);

					unsigned int uiLastJ = uiSampleSize - 1;
					unsigned int uiShift = 8;

					for (unsigned int j = 1; j < uiSampleSize; j++)
					{
						if (j != uiLastJ)
							iTemp |= ((unsigned char)pBuffer[i * (uiSampleSize * g_iNumChannels) + k * uiSampleSize + j] << uiShift);
						else
							iTemp |= ((signed char)pBuffer[i * (uiSampleSize * g_iNumChannels) + k * uiSampleSize + j] << uiShift);

						uiShift += 8;
					}

					ppBuffer[k][i] = iTemp/fScaler;
				}
			}
			break;

		default:
			return -1;
	}
    
	// Tell the library how much we actually submitted.
	vorbis_analysis_wrote(&vd,uiNumSamples);

	return irc_encode_flush_ex();
}

__int64 WINAPI irc_encode_flush()
{
	vorbis_analysis_wrote(&vd,0);

	return irc_encode_flush_ex();
}

bool WINAPI irc_encode_exit()
{
	// Close the out file.
	if (g_pFile == NULL)
		return false;

	delete g_pFile;
	g_pFile = NULL;

	// Destroy the encoder.
	ogg_stream_clear(&os);
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_info_clear(&vi);

	return true;
}

bool WINAPI irc_encode_config()
{
	CConfigDlg ConfigDlg(&g_EncoderConfig);
	ConfigDlg.DoModal();

	return true;
}
