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
#include <map>
#include <ckcore/stream.hh>
#include <ckfilesystem/sectorstream.hh>
#include <ckfilesystem/fileset.hh>
#include <ckmmc/device.hh>
#include <base/StringUtil.h>
#include "scsi.hh"
#include "advanced_progress.hh"

class CCore2
{
public:
	CCore2();
	~CCore2();

	enum eLoadMedia
	{
		LOADMEDIA_STOP = 0x00,
		LOADMEDIA_START = 0x01,
		LOADMEDIA_EJECT = 0x02,
		LOADMEDIA_LOAD = 0x03
	};

	enum eMediaChange
	{
		MEDIACHANGE_NOCHANGE,
		MEDIACHANGE_EJECTREQUEST,
		MEDIACHANGE_NEWMEDIA,
		MEDIACHANGE_MEDIAREMOVAL,
		MEDIACHANGE_MEDIACHANGED,
		MEDIACHANGE_BGFORMAT_COMPLETED,
		MEDIACHANGE_BGFORMAT_RESTARTED
	};

	enum
	{
		ERASE_FORMAT_QUICK,
		ERASE_FORMAT_FULL,
		ERASE_BLANK_FULL,
		ERASE_BLANK_MINIMAL,
		ERASE_BLANK_UNCLOSE,
		ERASE_BLANK_SESSION
	};

	bool HandleEvents(ckmmc::Device &Device,CAdvancedProgress *pProgress,
		unsigned char &ucHandledEvents);
	bool WaitForUnit(ckmmc::Device &Device,CAdvancedProgress *pProgress);
	eMediaChange CheckMediaChange(ckmmc::Device &Device);

	bool LockMedia(ckmmc::Device &Device,bool bLock);
	bool StartStopUnit(ckmmc::Device &Device,eLoadMedia Action,bool bImmed);
	bool CloseTrackSession(ckmmc::Device &Device,unsigned char ucCloseFunction,
		unsigned short usTrackNumber,bool bImmed);

	bool SetDiscSpeeds(ckmmc::Device &Device,unsigned short usReadSpeed,
		unsigned short usWriteSpeed);

	bool UpdateModePage5(ckmmc::Device &Device,bool bTestWrite,bool bSilent = false);

	// Primary functions.
	bool EraseDisc(ckmmc::Device &Device,CAdvancedProgress *pProgress,int iMethod,
		bool bForce,bool bEject,bool bSimulate,unsigned int uiSpeed);
	bool ReadDataTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
		unsigned char ucTrackNumber,bool bIgnoreErr,const TCHAR *szFilePath);
	bool ReadFullTOC(ckmmc::Device &Device,const TCHAR *szFileName);
	int CreateImage(ckcore::OutStream &OutStream,const ckfilesystem::FileSet &Files,
		ckcore::Progress &Progress,bool bFailOnError,
		std::map<tstring,tstring> *pFilePathMap = NULL);
	int CreateImage(const TCHAR *szFullPath,const ckfilesystem::FileSet &Files,		// Wrapper.
		ckcore::Progress &Progress,bool bFailOnError,
		std::map<tstring,tstring> *pFilePathMap = NULL);
	int EstimateImageSize(const ckfilesystem::FileSet &Files,ckcore::Progress &Progress,	// Wrapper.
		unsigned __int64 &uiImageSize);
};

extern CCore2 g_Core2;