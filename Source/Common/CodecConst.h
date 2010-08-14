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

// Callback function definitions.
#define IRC_MESSAGE_INFO			0
#define IRC_MESSAGE_WARNING			1
#define IRC_MESSAGE_ERROR			2

typedef void CALLBACK tirc_send_message(int iType,const TCHAR *szMessage);

// Exported codec function types.
typedef int (WINAPI *tirc_capabilities)();
typedef TCHAR *(WINAPI *tirc_string)(unsigned int uiID);
typedef bool (WINAPI *tirc_set_callback)(tirc_send_message *pSendMessage);
typedef bool (WINAPI *tirc_decode_init)(const TCHAR *szFileName,int &iNumChannels,int &iSampleRate,int &iBitRate,unsigned __int64 &uiDuration);
typedef __int64 (WINAPI *tirc_decode_process)(unsigned char *pBuffer,__int64 iBufferSize,unsigned __int64 &uiTime);
typedef bool (WINAPI *tirc_decode_exit)();
typedef bool (WINAPI *tirc_encode_init)(const TCHAR *szFileName,int iNumChannels,int iSampleRate,int iBitRate);
typedef __int64 (WINAPI *tirc_encode_process)(unsigned char *pBuffer,__int64 iDataSize);
typedef __int64 (WINAPI *tirc_encode_flush)();
typedef bool (WINAPI *tirc_encode_exit)();
typedef bool (WINAPI *tirc_encode_config)();

// Capability flags.
#define IRC_HAS_DECODER				0x0001
#define IRC_HAS_ENCODER				0x0002
#define IRC_HAS_CONFIG				0x0004

// String identifiers.
#define IRC_STR_VERSION				0
#define IRC_STR_ABOUT				1
#define IRC_STR_ENCODER				2
#define IRC_STR_FILEEXT				3
