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
#include <base/codec_const.hh>
#include "config_dlg.hh"
#include "lame_encoder.hh"

// Capability flags.
int capabilities = IRC_HAS_ENCODER | IRC_HAS_CONFIG;

// Version and about strings.
TCHAR *str_version = _T("0.53.0.0");
TCHAR *str_about = _T("InfraRecorder MP3 Encoder\n\nCopyright © 2006-2011 Christian Kindahl.\n\nThis codec is using the libmp3lame (LAME) library.\nVisit: http://lame.sourceforge.net/ for more information.\n\nPlease note that personal and/or commercial use of\ncompiled versions of this codec requires a patent license\nin some countries. Please check before using this codec.");
TCHAR *str_encoder = _T("MP3");
TCHAR *str_file_ext = _T(".mp3");

// Global variables.
LameEncoder *lame_encoder = NULL;

// Encoder configuration.
CEncoderConfig encoder_cfg;

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
    return TRUE;
}

/**
 * Returns bit information on what operations that are supported by the codec.
 * @return Bit information on what operations that are supported by the codec.
 */
int WINAPI irc_capabilities()
{
	return capabilities;
}

TCHAR *WINAPI irc_string(unsigned int uiID)
{
	switch (uiID)
	{
		case IRC_STR_VERSION:
			return str_version;

		case IRC_STR_ABOUT:
			return str_about;

		case IRC_STR_ENCODER:
			return str_encoder;

		case IRC_STR_FILEEXT:
			return str_file_ext;
	}

	return NULL;
}

bool WINAPI irc_set_callback(tirc_send_message *pSendMessage)
{
	if (pSendMessage == NULL)
		return false;

    LameBase::set_callback(pSendMessage);
	return true;
}

bool WINAPI irc_decode_init(const TCHAR *szFileName,int &iNumChannels,
							int &iSampleRate,int &iBitRate,
                            unsigned __int64 &uiDuration)
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
    if (lame_encoder != NULL)
        return false;

    lame_encoder = new LameEncoder(szFileName,iNumChannels,iSampleRate,iBitRate,encoder_cfg);
    if (!lame_encoder->initialize())
    {
        delete lame_encoder;
        lame_encoder = NULL;

        return false;
    }

	return true;
}

__int64 WINAPI irc_encode_process(unsigned char *pBuffer,__int64 iDataSize)
{
    if (lame_encoder == NULL)
        return -1;

    return lame_encoder->encode(pBuffer,iDataSize);
}

__int64 WINAPI irc_encode_flush()
{
    if (lame_encoder == NULL)
        return -1;

    return lame_encoder->flush();
}

bool WINAPI irc_encode_exit()
{
	if (lame_encoder == NULL)
        return false;

    delete lame_encoder;
    lame_encoder = NULL;

	return true;
}

bool WINAPI irc_encode_config()
{
	CConfigDlg ConfigDlg(&encoder_cfg);
	ConfigDlg.DoModal();

	return true;
}
