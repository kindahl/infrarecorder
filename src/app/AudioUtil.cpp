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

#include "stdafx.h"
#ifndef UNICODE
#include <stdio.h>
#endif
#include <ckcore/file.hh>
#include <base/StringUtil.h>
#include "AudioUtil.h"

bool IsWave(const TCHAR *szFileName)
{
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_READ))
		return false;

	// Validate ID.
	char szBuffer[4];
	if (File.read(szBuffer,4) == -1)
		return false;

	if (strncmp(szBuffer,"RIFF",4))
		return false;

	// Ignore size.
	File.seek(4,ckcore::File::ckFILE_CURRENT);

	// Validate type.
	if (File.read(szBuffer,4) == -1)
		return false;

	if (strncmp(szBuffer,"WAVE",4))
		return false;

	return true;
}

int GetAudioFormat(const TCHAR *szFileName)
{
	if (IsWave(szFileName))
		return AUDIOFORMAT_WAVE;

	return AUDIOFORMAT_UNKNOWN;
}

int GetAudioLength(const TCHAR *szFileName)
{
	TCHAR szCommand[MAX_PATH + 33],szResult[256],szLength[256];
	int iLength = 0;

	lsprintf(szCommand,_T("open \"%s\" type waveaudio alias seq"),szFileName);
	mciSendString(szCommand,szResult,256,NULL);
	mciSendString(_T("status seq length"),szLength,256,NULL);
	mciSendString(_T("close seq"),szResult,255,NULL);

#ifdef UNICODE
	iLength = _wtoi(szLength);
#else
	iLength = atoi(szLength);
#endif

	return iLength;
}

/*int GetAudioInfo(const TCHAR *szFileName)
{
	HANDLE hFile = fs_open(szFileName,_T("rb"));
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// Header chunk.
	char szBuffer[4];
	fs_read(szBuffer,4,hFile);

	if (strncmp(szBuffer,"RIFF",4))
	{
		fs_close(hFile);
		return AUDIOLENGTH_BADFORMAT;
	}

	fs_seek(hFile,4,FILE_CURRENT);
	fs_read(szBuffer,4,hFile);

	if (strncmp(szBuffer,"WAVE",4))
	{
		fs_close(hFile);
		return AUDIOLENGTH_BADFORMAT;
	}

	// Format chunk.
	fs_read(szBuffer,4,hFile);

	if (strncmp(szBuffer,"fmt ",4))
	{
		fs_close(hFile);
		return AUDIOLENGTH_BADFORMAT;
	}

	unsigned long ulSize = 0;
	fs_read(&ulSize,sizeof(unsigned long),hFile);

	unsigned short usCompCode = 0;
	unsigned short usNumChannels = 0;
	unsigned long ulSampleRate = 0;
	unsigned long ulBytesPerSec = 0;
	unsigned short usBlockAlign = 0;
	unsigned short usBitsPerSample = 0;

	fs_read(&usCompCode,sizeof(unsigned short),hFile);
	fs_read(&usNumChannels,sizeof(unsigned short),hFile);
	fs_read(&ulSampleRate,sizeof(unsigned long),hFile);
	fs_read(&ulBytesPerSec,sizeof(unsigned long),hFile);
	fs_read(&usBlockAlign,sizeof(unsigned short),hFile);
	fs_read(&usBitsPerSample,sizeof(unsigned short),hFile);

	fs_close(hFile);
	return AUDIOLENGTH_BADFORMAT;
}*/
