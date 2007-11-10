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

#pragma once
#include <vector>
#include "ConsolePipe.h"
#include "DeviceManager.h"
#include "AdvancedProgress.h"
#include "../../Common/StringUtil.h"

#define CORE_IGNORE_ERRORINFOMESSAGES		// Should we ignore error information message (copyright etc.)?
#define CORE_PRINT_UNSUPERRORMESSAGES		// Should we print unhandled/unsupported messages to the log window?
#define CORE_DVD_SUPPORT					// Is DVD recording supported in this version?

#define RESULT_INTERNALERROR		0
#define RESULT_OK					1
#define RESULT_EXTERNALERROR		2

class CCore : public CConsolePipe
{
private:
	int m_iMode;
	int m_iStatusMode;

	bool m_bGraceTimeDone;
	bool m_bDummyMode;
	bool m_bErrorPathMode;					// Is set to true when we're expecting a cygwin path in the beginning of each error message.

	bool m_bOperationRes;

	unsigned __int64 m_uiProcessedSize;
	unsigned __int64 m_uiTotalSize;
	unsigned __int64 m_uiEstimatedSize;		// Used when estimating file system size.
	std::vector<unsigned __int64> m_TrackSize;
	unsigned int m_uiCurrentTrack;

	// Holds the length of the path to the cdrtools directory (cygwin path).
	// This is used to remove the cygwin path to cdrtools on every stderr message.
	// It's important that the length is updated when the path changes.
	unsigned int m_uiCDRToolsPathLen;

	// Object that can receive messages while receiving output from a console
	// application.
	CAdvancedProgress *m_pProgress;

	void Initialize(int iMode,CAdvancedProgress *pProgress = NULL);
	void CreateBatchFile(const char *szChangeDir,const char *szCommandLine,TCHAR *szBatchPath);
	bool SafeLaunch(tstring &CommandLine,bool bWaitForProcess);

	bool CheckGraceTime(const char *szBuffer);
	bool CheckProgress(const char *szBuffer);

	void ErrorOutputCDRECORD(const char *szBuffer);
	void ErrorOutputMKISOFS(const char *szBuffer);
	void ErrorOutputREADCD(const char *szBuffer);
	void ErrorOutputCDDA2WAV(const char *szBuffer);

	void EraseOutput(const char *szBuffer);
	void FixateOutput(const char *szBuffer);
	void BurnImageOutput(const char *szBuffer);
	void CreateImageOutput(const char *szBuffer);
	void ReadDataTrackOutput(const char *szBuffer);
	void ReadAudioTrackOutput(const char *szBuffer);
	void ScanTrackOutput(const char *szBuffer);
	void ReadDiscOutput(const char *szBuffer);
	void EstimateSizeOutput(const char *szBuffer);

	// Inherited.
	void FlushOutput(const char *szBuffer);
	void ProcessEnded();

	bool BurnImage(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szFileName,bool bWaitForProcess,bool bCloneMode);
	bool BurnTracks(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iDataMode,int iMode,bool bWaitForProcess);
	bool CreateImage(const TCHAR *szFileName,const TCHAR *szPathList,
		CAdvancedProgress *pProgress,int iMode,bool bEstimateSize,bool bWaitForProcess);
	bool ReadDataTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		unsigned int uiTrackNumber,unsigned long ulStartSector,unsigned long ulEndSector,
		int iMode,bool bWaitForProcess);
	bool ReadAudioTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		unsigned int uiTrackNumber,int iMode,bool bWaitForProcess);
	bool ScanTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
		unsigned long ulStartSector,unsigned long ulEndSector,int iMode,bool bWaitForProcess);
	bool ReadDisc(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		int iMode,bool bWaitForProcess);
	bool BurnCompilation(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szPathList,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iDataMode,unsigned __int64 uiDataBytes,int iMode);

	enum eMode
	{
		MODE_EJECT = 0,
		MODE_ERASE,
		MODE_FIXATE,
		MODE_BURNIMAGE,
		MODE_BURNIMAGEEX,
		MODE_CREATEIMAGE,
		MODE_CREATEIMAGEEX,
		MODE_READDATATRACK,
		MODE_READDATATRACKEX,
		MODE_READAUDIOTRACK,
		MODE_READAUDIOTRACKEX,
		MODE_SCANTRACK,
		MODE_SCANTRACKEX,
		MODE_READDISC,
		MODE_READDISCEX,
		MODE_ESTIMATESIZE
	};

	enum eStatusMode
	{
		SMODE_DEFAULT = 0,
		SMODE_GRACETIME,
		SMODE_PREPROGRESS,
		SMODE_PROGRESS,
		SMODE_AUDIOPROGRESS
	};

public:
	CCore();
	~CCore();

	bool EjectDisc(tDeviceInfo *pDeviceInfo,bool bWaitForProcess);
	bool LoadDisc(tDeviceInfo *pDeviceInfo,bool bWaitForProcess);
	bool EraseDisc(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		CAdvancedProgress *pProgress,int iMode,bool bForce,bool bEject,bool bSimulate);
	bool FixateDisc(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		CAdvancedProgress *pProgress,bool bEject,bool bSimulate);
	bool BurnImage(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		bool bCloneMode);
	bool BurnImageEx(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		bool bCloneMode);
	bool BurnTracks(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iDataMode);
	int BurnTracksEx(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iDataMode);
	bool CreateImage(const TCHAR *szFileName,const TCHAR *szPathList,
		CAdvancedProgress *pProgress);
	int CreateImageEx(const TCHAR *szFileName,const TCHAR *szPathList,
		CAdvancedProgress *pProgress);
	bool ReadDataTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		unsigned int uiTrackNumber,unsigned long ulStartSector,unsigned long ulEndSector);
	int ReadDataTrackEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		unsigned int uiTrackNumber,unsigned long ulStartSector,unsigned long ulEndSector);
	bool ReadAudioTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		unsigned int uiTrackNumber);
	int ReadAudioTrackEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
		unsigned int uiTrackNumber);
	bool ScanTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
		unsigned long ulStartSector,unsigned long ulEndSector);
	int ScanTrackEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
		unsigned long ulStartSector,unsigned long ulEndSector);
	bool CopyDisc(tDeviceInfo *pSourceDeviceInfo,tDeviceInfo *pTargetDeviceInfo,
		tDeviceCap *pTargetDeviceCap,tDeviceInfoEx *pTargetDeviceInfoEx,CAdvancedProgress *pProgress);
	bool ReadDisc(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName);
	int ReadDiscEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName);
	bool BurnCompilation(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szPathList,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iMode,unsigned __int64 uiDataBytes);
	int BurnCompilationEx(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szPathList,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iMode,unsigned __int64 uiDataBytes);
	bool EstimateImageSize(const TCHAR *szPathList,CAdvancedProgress *pProgress,unsigned __int64 &uiSize);
};

extern CCore g_Core;
