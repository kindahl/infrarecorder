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

#pragma once
#include <vector>
#include <map>
#include "../../Common/StringUtil.h"
#include "../../Core/ckFileSystem/SectorStream.h"
#include "../../Core/ckFileSystem/FileSet.h"
#include "SCSI.h"
#include "AdvancedProgress.h"
#include "Core2Device.h"

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

	enum eWriteMode
	{
		WRITEMODE_DONTCHANGE = 0x00,
		WRITEMODE_PACKET = 0x01,
		WRITEMODE_TAO = 0x02,
		WRITEMODE_SAO = 0x04,
		WRITEMODE_RAW96R = 0x08,
		WRITEMODE_RAW16 = 0x10,
		WRITEMODE_RAW96P = 0x20,
		WRITEMODE_LAYERJUMP = 0x40
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

	bool HandleEvents(CCore2Device *pDevice,CAdvancedProgress *pProgress,
		unsigned char &ucHandledEvents);
	bool WaitForUnit(CCore2Device *pDevice,CAdvancedProgress *pProgress);
	//bool UnitReady(CCore2Device *pDevice);
	eMediaChange CheckMediaChange(CCore2Device *pDevice);

	bool LockMedia(CCore2Device *pDevice,bool bLock);
	bool StartStopUnit(CCore2Device *pDevice,eLoadMedia Action,bool bImmed);
	bool CloseTrackSession(CCore2Device *pDevice,unsigned char ucCloseFunction,
		unsigned short usTrackNumber,bool bImmed);

	bool GetProfile(CCore2Device *pDevice,unsigned short &usProfile);
	bool GetFeatureSupport(CCore2Device *pDevice,unsigned short usFeature,
		bool &bSupportFeature);
	bool GetMediaWriteSpeeds(CCore2Device *pDevice,std::vector<unsigned int> &Speeds);
	bool GetMaxReadSpeed(CCore2Device *pDevice,unsigned short &usSpeed);
	bool GetMaxSpeeds(CCore2Device *pDevice,unsigned short &usReadSpeed,
		unsigned short &usWriteSpeed);
	bool GetMediaWriteModes(CCore2Device *pDevice,unsigned char &ucWriteModes);

	bool SetDiscSpeeds(CCore2Device *pDevice,unsigned short usReadSpeed,
		unsigned short usWriteSpeed);

	bool UpdateModePage5(CCore2Device *pDevice,bool bTestWrite,
		eWriteMode WriteMode = WRITEMODE_DONTCHANGE,bool bSilent = false);

	// Primary functions.
	bool EraseDisc(CCore2Device *pDevice,CAdvancedProgress *pProgress,int iMethod,
		bool bForce,bool bEject,bool bSimulate,unsigned int uiSpeed);
	bool ReadDataTrack(CCore2Device *pDevice,CAdvancedProgress *pProgress,
		unsigned char ucTrackNumber,bool bIgnoreErr,const TCHAR *szFilePath);
	bool ReadFullTOC(CCore2Device *pDevice,const TCHAR *szFileName);
	int CreateImage(COutStream &OutStream,ckFileSystem::CFileSet &Files,
		CProgressEx &Progress,std::map<tstring,tstring> *pFilePathMap = NULL);
	int CreateImage(const TCHAR *szFullPath,ckFileSystem::CFileSet &Files,		// Wrapper.
		CProgressEx &Progress,std::map<tstring,tstring> *pFilePathMap = NULL);
	int EstimateImageSize(ckFileSystem::CFileSet &Files,CProgressEx &Progress,	// Wrapper.
		unsigned __int64 &uiImageSize);
};

extern CCore2 g_Core2;
