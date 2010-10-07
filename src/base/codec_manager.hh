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
#include <vector>
#include "codec_const.hh"

// Local structures.
class CCodec
{
private:
	HINSTANCE m_hInstance;

public:
	CCodec();
	~CCodec();

	bool Load(const TCHAR *szFileName);
	bool GetFileName(TCHAR *szFileName,unsigned long ulBufSize);

	tirc_capabilities irc_capabilities;
	tirc_string irc_string;
	tirc_set_callback irc_set_callback;
	tirc_decode_init irc_decode_init;
	tirc_decode_process irc_decode_process;
	tirc_decode_exit irc_decode_exit;
	tirc_encode_init irc_encode_init;
	tirc_encode_process irc_encode_process;
	tirc_encode_flush irc_encode_flush;
	tirc_encode_exit irc_encode_exit;
	tirc_encode_config irc_encode_config;
};

class CCodecManager
{
private:
	bool m_bIsLoaded;

	bool LoadCodec(const TCHAR *szFileName);

public:
	CCodecManager();
	~CCodecManager();

	std::vector<CCodec *> m_Codecs;

	bool LoadCodecs(const TCHAR *szCodecPath);
	bool IsLoaded();
};
