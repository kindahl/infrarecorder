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
#include "Core.h"
#include "cdrtoolsParseStrings.h"
#include "../../Common/FileManager.h"
#include "../../Common/StringContainer.h"
#include "StringTable.h"
#include "Settings.h"
#include "LogDlg.h"
#include "LangUtil.h"
#include "WinVer.h"
#include "TempManager.h"

// FIXME: How come Windows 95 supports larger command lines than Windows 2000?
//        Windows 2000 seems to be the only OS that is limited to MAX_PATH.
//        http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/createprocess.asp
//        http://support.microsoft.com/default.aspx?scid=kb;en-us;830473
//
//        Possible solution?
//        http://blogs.msdn.com/oldnewthing/archive/2003/12/10/56028.aspx
//        http://blogs.msdn.com/oldnewthing/archive/2003/12/11/56043.aspx

CCore g_Core;

CCore::CCore()
{
	m_iMode = -1;
	m_iStatusMode = SMODE_DEFAULT;

	m_bGraceTimeDone = false;
	m_bDummyMode = false;
	m_bErrorPathMode = true;

	m_uiCDRToolsPathLen = 0;

	m_pProgress = NULL;
}

CCore::~CCore()
{
	m_TrackSize.clear();
}

/*
	CCore::Initialize
	-----------------
	Prepares the object. This function should be called in the beginning of all
	cdrtools related functions.
*/
void CCore::Initialize(int iMode,CAdvancedProgress *pProgress)
{
	m_iMode = iMode;
	m_iStatusMode = SMODE_DEFAULT;
	m_bGraceTimeDone = false;
	m_bErrorPathMode = true;			// By default we're looking for a cygwin path before error messages.

	m_uiCDRToolsPathLen = lstrlen(g_GlobalSettings.m_szCDRToolsPathCyg);

	m_pProgress = pProgress;

	// Remove all previous track information.
	m_TrackSize.clear();
	m_uiCurrentTrack = 0;
}

void CCore::CreateBatchFile(const char *szChangeDir,const char *szCommandLine,TCHAR *szBatchPath)
{
	fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irPathList"),szBatchPath);

	// Delete the generated temporary file since we need a batch file.
	if (fs_fileexists(szBatchPath))
		fs_deletefile(szBatchPath);

	ChangeFileExt(szBatchPath,_T(".bat"));

	CStringContainerA StringContainer;

	if (szChangeDir != NULL)
		StringContainer.m_szStrings.push_back(szChangeDir);

	StringContainer.m_szStrings.push_back(szCommandLine);
	StringContainer.SaveToFile(szBatchPath);
}

bool CCore::SafeLaunch(tstring &CommandLine,bool bWaitForProcess)
{
	// If the command line is longer than 260 characters we need to take special
	// actions since all (only Windows 2000?) Windows versions older than XP are
	// limited to MAX_PATH. Windows XP supports 32K character command lines.
	if (CommandLine.length() > MAX_PATH - 1)
	{
		if (g_WinVer.m_ulMajorVersion < MAJOR_WINXP ||
			(g_WinVer.m_ulMajorVersion == MAJOR_WINXP && g_WinVer.m_ulMinorVersion < MINOR_WINXP))
		{
			// Windows NT4 and 2000 supports 2048 character command lines, I am not
			// sure if Windows 9x do. This needs to be checked.
			if (CommandLine.length() > 2047)
			{
				TCHAR szMessage[256];
				lsnprintf_s(szMessage,256,lngGetString(ERROR_COMMANDLINE),2048);
				MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
				return false;
			}

			if (g_GlobalSettings.m_bLog)
				g_LogDlg.AddLine(_T("  Warning: The command line is %d characters long. Trying to execute through shell."),CommandLine.length());

			TCHAR szBatchPath[MAX_PATH];
#ifdef UNICODE
			char *szCommandLine = new char[CommandLine.length() + 1];
			TCharToChar(CommandLine.c_str(),szCommandLine);

			// Create the batch file.
			CreateBatchFile(NULL,szCommandLine,szBatchPath);

			delete [] szCommandLine;
#else
			CreateBatchFile(NULL,CommandLine.c_str(),szBatchPath);
#endif
			
			TCHAR szBatchCmdLine[MAX_PATH + 2];
			lstrcpy(szBatchCmdLine,_T("\""));
			lstrcat(szBatchCmdLine,szBatchPath);
			lstrcat(szBatchCmdLine,_T("\""));

			if (bWaitForProcess)
			{
				bool bResult = Launch(szBatchCmdLine,bWaitForProcess);
					fs_deletefile(szBatchPath);
				return bResult;
			}
			else
			{
				g_TempManager.AddObject(szBatchPath);
				return Launch(szBatchCmdLine,bWaitForProcess);
			}
		}
		// Windows XP supports maximum 32768 characters.
		else if (CommandLine.length() > 32767)
		{
			TCHAR szMessage[256];
			lsnprintf_s(szMessage,256,lngGetString(ERROR_COMMANDLINE),2048);
			MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
			return false;
		}
	}

	return Launch((TCHAR *)CommandLine.c_str(),bWaitForProcess);
}

bool CCore::CheckGraceTime(const char *szBuffer)
{
	// If the message: Starting to write CD/DVD... is received we know that the 
	// grace count down time is about to start so we change the new line
	// delimiter to '.' since the gracetime is updated using the '\b' character.
	if (!strncmp(szBuffer,CDRTOOLS_STARTCDWRITE,CDRTOOLS_STARTCDWRITE_LENGTH))
	{
		// Sometimes the "Starting to write CD/DVD..." string will reappear after the
		// grace time countdown. Because of that we only allow this delimiter change once.
		if (!m_bGraceTimeDone)
			m_cLineDelimiter = '.';

		return true;
	}
	else if (!strncmp(szBuffer,CDRTOOLS_GRACEBEGIN,CDRTOOLS_GRACEBEGIN_LENGTH))
	{
		m_iStatusMode = SMODE_GRACETIME;

		char szMode[6];	// Can be "dummy" or "real".
		int iTimer = 0;

		sscanf(szBuffer,"Last chance to quit, starting %s write in %d seconds.",szMode,&iTimer);

		m_bDummyMode = szMode[0] == 'd';

		// Update the status.
		m_pProgress->SetStatus(lngGetString(PROGRESS_GRACETIME),iTimer);

		return true;
	}

	return false;
}

bool CCore::CheckProgress(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_STARTTRACK,CDRTOOLS_STARTTRACK_LENGTH))
	{
		m_cLineDelimiter = '.';

		m_iStatusMode = SMODE_PREPROGRESS;

		return true;
	}

	return false;
}

void CCore::ErrorOutputCDRECORD(const char *szBuffer)
{
	if (m_pProgress != NULL)
	{
		if (!strncmp(szBuffer,CDRTOOLS_NOMEDIA,CDRTOOLS_NOMEDIA_LENGTH))
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_NOMEDIA));
		else if (!strncmp(szBuffer,CDRTOOLS_BLANKERROR,CDRTOOLS_BLANKERROR_LENGTH))
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_ERASE));
		else if (!strncmp(szBuffer,CDRTOOLS_BLANKUNSUP,CDRTOOLS_BLANKUNSUP_LENGTH))
			m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(INFO_UNSUPERASEMODE));
		else if (!strncmp(szBuffer,CDRTOOLS_BLANKRETRY,CDRTOOLS_BLANKRETRY_LENGTH))
			m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(INFO_ERASERETRY));
		else if (!strncmp(szBuffer,CDRTOOLS_NODISC,CDRTOOLS_NODISC_LENGTH))
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_NOMEDIA));
		else if (!strncmp(szBuffer,CDRTOOLS_BADAUDIOCODING,CDRTOOLS_BADAUDIOCODING_LENGTH))
		{
			TCHAR szMessage[MAX_PATH + 128];
			lstrcpy(szMessage,lngGetString(FAILURE_AUDIOCODING));

#ifdef UNICODE
			TCHAR szFileName[MAX_PATH + 3];
			CharToTChar(szBuffer + CDRTOOLS_BADAUDIOCODING_LENGTH + 10,szFileName);
			lstrcat(szMessage,szFileName);
#else
			lstrcat(szMessage,szBuffer + CDRTOOLS_BADAUDIOCODING_LENGTH + 10);
#endif

			m_pProgress->AddLogEntry(LOGTYPE_ERROR,szMessage);
		}
		else if (!strncmp(szBuffer,CDRTOOLS_UNSUPPORTED,CDRTOOLS_UNSUPPORTED_LENGTH))
		{
			char *pBuffer = (char *)szBuffer + 12;

			if (!strncmp(pBuffer,CDRTOOLS_SECTOR,CDRTOOLS_SECTOR_LENGTH))
			{
				int iSectorSize = 0;
				sscanf(pBuffer,"sector size %ld for %*[^\0]",&iSectorSize);

				m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_BADSECTORSIZE),iSectorSize);
			}
		}
		else if (!strncmp(szBuffer,CDRTOOLS_WRITEERROR,CDRTOOLS_WRITEERROR_LENGTH))
		{
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_WRITE));
		}
#ifndef CORE_DVD_SUPPORT
		else if (!strncmp(szBuffer,CDRTOOLS_FOUNDDVDMEDIA,CDRTOOLS_FOUNDDVDMEDIA_LENGTH))
		{
			m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(FAILURE_DVDSUPPORT));
		}
#endif
		else if (!strncmp(szBuffer,CDRTOOLS_OPENSESSION,CDRTOOLS_OPENSESSION_LENGTH))
		{
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_OPENSESSION));
		}
		else if (!strncmp(szBuffer,CDRTOOLS_WARNINGCAP,CDRTOOLS_WARNINGCAP_LENGTH))	// "WARNING:" Prefix.
		{
			char *pBuffer = (char *)szBuffer + CDRTOOLS_WARNINGCAP_LENGTH + 1;

			if (!strncmp(pBuffer,CDRTOOLS_DISCSPACEWARNING,CDRTOOLS_DISCSPACEWARNING_LENGTH))
				m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(WARNING_DISCSIZE));
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERRORLEADIN,CDRTOOLS_ERRORLEADIN_LENGTH))
		{
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_WRITELEADIN));

			// When called from BurnTracks/BurnCompilation we need to flag the operation as failed.
			m_bOperationRes = false;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERRORINITDRIVE,CDRTOOLS_ERRORINITDRIVE_LENGTH))
		{
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_INITDRIVE));

			// When called from BurnTracks/BurnCompilation we need to flag the operation as failed.
			m_bOperationRes = false;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_DVDRWDUMMY,CDRTOOLS_DVDRWDUMMY_LENGTH))
		{
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_DVDRWDUMMY));

			// When called from BurnTracks/BurnCompilation we need to flag the operation as failed.
			m_bOperationRes = false;
		}
#ifdef CORE_IGNORE_ERRORINFOMESSAGES		// Ingore error information messages.
		else if (!strncmp(szBuffer,CDRTOOLS_VERSIONINFO,CDRTOOLS_VERSIONINFO_LENGTH))
			return;
		else if (!strncmp(szBuffer,CDRTOOLS_DVDINFO,CDRTOOLS_DVDINFO_LENGTH))
			return;
		else if (!strncmp(szBuffer,CDRTOOLS_DVDGETINFO,CDRTOOLS_DVDGETINFO_LENGTH))
			return;
#endif
#ifdef CORE_PRINT_UNSUPERRORMESSAGES		// Print unhandled messages from cdrecord to the log window.
	#ifdef CORE_DVD_SUPPORT
		else if (!strncmp(szBuffer,CDRTOOLS_FOUNDDVDMEDIA,CDRTOOLS_FOUNDDVDMEDIA_LENGTH))
			return;
	#endif
		else if (!strncmp(szBuffer,CDRTOOLS_TURNINGBFON,CDRTOOLS_TURNINGBFON_LENGTH))
			return;
		else
		{
	#ifdef UNICODE
			TCHAR szWideBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
			CharToTChar(szBuffer,szWideBuffer);
			m_pProgress->AddLogEntry(LOGTYPE_WINLOGO,szWideBuffer);
	#else
			m_pProgress->AddLogEntry(LOGTYPE_WINLOGO,szBuffer);
	#endif
		}
#endif
	}
}

void CCore::ErrorOutputMKISOFS(const char *szBuffer)
{
	if (m_pProgress != NULL)
	{
		if (!strncmp(szBuffer,CDRTOOLS_FILENOTFOUND,CDRTOOLS_FILENOTFOUND_LENGTH))
		{
			TCHAR szMessage[MAX_PATH + 128];
			lstrcpy(szMessage,lngGetString(FAILURE_FILENOTFOUND));

#ifdef UNICODE
			TCHAR szFileName[MAX_PATH + 3];
			CharToTChar(szBuffer + CDRTOOLS_FILENOTFOUND_LENGTH + 15,szFileName);
			lstrcat(szMessage,szFileName);
#else
			lstrcat(szMessage,szBuffer + CDRTOOLS_FILENOTFOUND_LENGTH + 15);
#endif

			m_pProgress->AddLogEntry(LOGTYPE_ERROR,szMessage);
		}
		else if (!strncmp(szBuffer,CDRTOOLS_DEPPDIR,CDRTOOLS_DEEPDIR_LENGTH))
		{
			char szFileName[MAX_PATH];
			int iDepth = 0;
			int iMaxDepth = 0;

			sscanf(szBuffer,"Directories too deep for '%[^']' (%d) max is %d.",szFileName,&iDepth,&iMaxDepth);
#ifdef UNICODE
			TCHAR szWideFileName[MAX_PATH];
			CharToTChar(szFileName,szWideFileName);

			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_DEEPDIR),
				szWideFileName,iDepth,iMaxDepth);
#else
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_DEEPDIR),
				szFileName,iDepth,iMaxDepth);
#endif
		}
#ifdef CORE_PRINT_UNSUPERRORMESSAGES
		else
		{
#ifdef UNICODE
			TCHAR szWideBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
			CharToTChar(szBuffer,szWideBuffer);
			m_pProgress->AddLogEntry(LOGTYPE_WINLOGO,szWideBuffer);
#else
			m_pProgress->AddLogEntry(LOGTYPE_WINLOGO,szBuffer);
#endif
		}
#endif
	}
}

void CCore::ErrorOutputREADCD(const char *szBuffer)
{
	if (m_pProgress != NULL)
	{
		if (!strncmp(szBuffer,CDRTOOLS_IOERROR,CDRTOOLS_IOERROR_LENGTH))
		{
			char *pBuffer = (char *)szBuffer + CDRTOOLS_IOERROR_LENGTH;

			if (!strncmp(pBuffer,CDRTOOLS_SECTORERROR,CDRTOOLS_SECTORERROR_LENGTH))
			{
				unsigned long ulSector = atoi(pBuffer + CDRTOOLS_SECTORERROR_LENGTH + 1);
	
				m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(ERROR_SECTOR),ulSector);
			}
		}
		else if (!strncmp(szBuffer,CDRTOOLS_RETRYSECTOR,CDRTOOLS_RETRYSECTOR_LENGTH))
		{
			unsigned long ulSector = atoi(szBuffer + CDRTOOLS_RETRYSECTOR_LENGTH + 1);

			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_READSOURCEDISC),ulSector);
		}
#ifdef CORE_PRINT_UNSUPERRORMESSAGES
		else
		{
#ifdef UNICODE
			TCHAR szWideBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
			CharToTChar(szBuffer,szWideBuffer);
			m_pProgress->AddLogEntry(LOGTYPE_WINLOGO,szWideBuffer);
#else
			m_pProgress->AddLogEntry(LOGTYPE_WINLOGO,szBuffer);
#endif
		}
#endif
	}
}

void CCore::ErrorOutputCDDA2WAV(const char *szBuffer)
{
}

void CCore::EraseOutput(const char *szBuffer)
{
	// Check if the media or command is not supported by the drive.
	if (!strncmp(szBuffer,CDRTOOLS_NOSUPPORT,CDRTOOLS_NOSUPPORT_LENGTH))
	{
		char *pBuffer = (char *)szBuffer + 42;

		if (!strncmp(pBuffer,CDRTOOLS_BLANK,CDRTOOLS_BLANK_LENGTH))
			m_pProgress->AddLogEntry(LOGTYPE_ERROR,lngGetString(FAILURE_UNSUPRW));
	}
	else if (CheckGraceTime(szBuffer))
	{
		return;
	}
	else if (!strncmp(szBuffer,CDRTOOLS_BLANKTIME,CDRTOOLS_BLANKTIME_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_ERASE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_RELOADDRIVE,CDRTOOLS_RELOADDRIVE_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(FAILURE_LOADDRIVE));
		m_pProgress->SetStatus(lngGetString(ERROR_RELOADDRIVE));

		// Enable the reload button.
		m_pProgress->AllowReload();
	}
}

void CCore::FixateOutput(const char *szBuffer)
{
	if (CheckGraceTime(szBuffer))
	{
		return;
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FIXATETIME,CDRTOOLS_FIXATETIME_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_RELOADDRIVE,CDRTOOLS_RELOADDRIVE_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(FAILURE_LOADDRIVE));
		m_pProgress->SetStatus(lngGetString(ERROR_RELOADDRIVE));

		// Enable the reload button.
		m_pProgress->AllowReload();
	}
}

void CCore::BurnImageOutput(const char *szBuffer)
{
	if (CheckGraceTime(szBuffer))
		return;

	if (CheckProgress(szBuffer))
		return;

	if (!strncmp(szBuffer,CDRTOOLS_WRITEPREGAP,CDRTOOLS_WRITEPREGAP_LENGTH))
	{
		int iTrack = 0;
		long lPos = 0;
		sscanf(szBuffer,"Writing pregap for track %d at %ld",&iTrack,&lPos);

		TCHAR szStatus[64];
		lsnprintf_s(szStatus,64,lngGetString(STATUS_WRITEPREGAP),iTrack,lPos);
		m_pProgress->SetStatus(szStatus);
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FILLFIFO,CDRTOOLS_FILLFIFO_LENGTH))
	{
		m_pProgress->SetStatus(lngGetString(STATUS_FILLBUFFER));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_WRITETIME,CDRTOOLS_WRITETIME_LENGTH))
	{
		// Only display the error message if no error occured.
		if (m_bOperationRes)
			m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_WRITE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FIXATE,CDRTOOLS_FIXATE_LENGTH))
	{
		m_pProgress->SetStatus(lngGetString(STATUS_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FIXATETIME,CDRTOOLS_FIXATETIME_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_WARNINGCAP,CDRTOOLS_WARNINGCAP_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(WARNING_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_RELOADDRIVE,CDRTOOLS_RELOADDRIVE_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_WARNING,lngGetString(FAILURE_LOADDRIVE));
		m_pProgress->SetStatus(lngGetString(ERROR_RELOADDRIVE));

		// Enable the reload button.
		m_pProgress->AllowReload();
	}
}

void CCore::CreateImageOutput(const char *szBuffer)
{
	// Look for progress information.
	float fProgress = 0.0f;
	char szTime[64];

	if (sscanf(szBuffer,"%f%% done, estimate finish %*s %*s %*s %s %*[^\0]",&fProgress,szTime) == 2)
	{
		m_pProgress->SetProgress((int)fProgress);

		// Update the status.
#ifdef UNICODE
		TCHAR szWideTime[64];
		CharToTChar(szTime,szWideTime);

		TCHAR szStatus[128];
		lsnprintf_s(szStatus,128,lngGetString(STATUS_WRITEIMAGE),szWideTime);
		m_pProgress->SetStatus(szStatus);
#else
		char szStatus[128];
		sprintf(szStatus,lngGetString(STATUS_WRITEIMAGE),szTime);
		m_pProgress->SetStatus(szStatus);
#endif
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTTSIZE,CDRTOOLS_TOTALTTSIZE_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_CREATEIMAGE));
		m_bOperationRes = true;		// Success.
	}
	// This is a special case. mkisofs likes to print a boot image error on a weird place.
	else if (!strncmp(szBuffer,CDRTOOLS_SIZEOFBOOT,CDRTOOLS_SIZEOFBOOT_LENGTH))
	{
		// Typical string: Size of boot image is 24 sectors -> /cygdrive/...
		const char *pBuffer = szBuffer;
		while (*pBuffer != '>')
			pBuffer++;

		pBuffer += 2;

		if (!strncmp(pBuffer,CDRTOOLS_CYGWINPATH,CDRTOOLS_CYGWINPATH_LENGTH))
			ErrorOutputMKISOFS(pBuffer + m_uiCDRToolsPathLen + CDRTOOLS_ERROR2_LENGTH);
	}
}

void CCore::ReadDataTrackOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_END,CDRTOOLS_END_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINREADTRACK),m_TrackSize[1]);
		m_pProgress->SetStatus(lngGetString(STATUS_READTRACK));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_ADDRESS,CDRTOOLS_ADDRESS_LENGTH))
	{
		unsigned __int64 uiAddress = 0;
		unsigned __int64 uiCount = 0;

		if (sscanf(szBuffer,"addr: %8ld cnt: %ld",&uiAddress,&uiCount) == 2)
			m_pProgress->SetProgress((int)(((double)(uiAddress - m_TrackSize[0])/m_uiTotalSize) * 100));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTIME,CDRTOOLS_TOTALTIME_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_READTRACK),m_TrackSize[1]);
		m_bOperationRes = true;			// Success.
	}
}

void CCore::ReadAudioTrackOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_PERCENTDONE,CDRTOOLS_PERCENTDONE_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINREADTRACK),m_uiTotalSize);
		m_pProgress->SetStatus(lngGetString(STATUS_READTRACK));

		m_iStatusMode = SMODE_AUDIOPROGRESS;
	}
}

void CCore::ScanTrackOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_END,CDRTOOLS_END_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINSCANTRACK),m_TrackSize[1]);
		m_pProgress->SetStatus(lngGetString(STATUS_SCANTRACK));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_ADDRESS,CDRTOOLS_ADDRESS_LENGTH))
	{
		unsigned __int64 uiAddress = 0;
		unsigned __int64 uiCount = 0;

		if (sscanf(szBuffer,"addr: %8ld cnt: %ld",&uiAddress,&uiCount) == 2)
			m_pProgress->SetProgress((int)(((double)(uiAddress - m_TrackSize[0])/m_uiTotalSize) * 100));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTIME,CDRTOOLS_TOTALTIME_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_SCANTRACK),m_TrackSize[1]);
		m_bOperationRes = true;			// Success.
	}
	else if (!strncmp(szBuffer,CDRTOOLS_C2ERRORS,CDRTOOLS_C2ERRORS_LENGTH))
	{
		char *pBuffer = (char *)szBuffer + CDRTOOLS_C2ERRORS_LENGTH;

		if (!strncmp(pBuffer,"total",5))
		{
			unsigned long ulBytes = 0;
			unsigned long ulSectors = 0;

			if (sscanf(pBuffer + 7,"%ld bytes in %d sectors on disk",&ulBytes,&ulSectors) == 2)
			{
				m_pProgress->AddLogEntry(ulSectors > 0 ? LOGTYPE_WARNING : LOGTYPE_INFORMATION,
					lngGetString(STATUS_C2TOTAL),ulBytes,ulBytes);
			}
		}
		else if (!strncmp(pBuffer,"rate",4))
		{
			float fRate = (float)atof(pBuffer + 6);

			m_pProgress->AddLogEntry(fRate > 0 ? LOGTYPE_WARNING : LOGTYPE_INFORMATION,
				lngGetString(STATUS_C2RATE),fRate);
		}
	}
}

void CCore::ReadDiscOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_END,CDRTOOLS_END_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINREADDISC));
		m_pProgress->SetStatus(lngGetString(STATUS_READDISC));

		// Update the total number of sectors to process.
		m_uiTotalSize = atoi(szBuffer + CDRTOOLS_END_LENGTH);

		// Prevent a crash if the above function fails.
		if (m_uiTotalSize == 0)
			m_uiTotalSize = 1;
	}
	else if (!strncmp(szBuffer,CDRTOOLS_ADDRESS,CDRTOOLS_ADDRESS_LENGTH))
	{
		unsigned __int64 uiAddress = 0;
		unsigned __int64 uiCount = 0;

		if (sscanf(szBuffer,"addr: %8ld cnt: %ld",&uiAddress,&uiCount) == 2)
			m_pProgress->SetProgress((int)(((double)uiAddress/m_uiTotalSize) * 100));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTIME,CDRTOOLS_TOTALTIME_LENGTH))
	{
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_READDISC));

		m_bOperationRes = true;			// Success.
	}
}

void CCore::EstimateSizeOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_TOTALEXTENT,CDRTOOLS_TOTALEXTENT_LENGTH))
	{
		sscanf(szBuffer,"Total extents scheduled to be written = %I64d",&m_uiEstimatedSize);

		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_ESTIMAGESIZE),m_uiEstimatedSize);

		m_bOperationRes = true;
	}
}

void CCore::FlushOutput(const char *szBuffer)
{
	// Write to the log.
	if (g_GlobalSettings.m_bLog && m_iStatusMode == SMODE_DEFAULT)
	{
		g_LogDlg.AddString(_T("   > "));
#ifdef UNICODE
		TCHAR szWideBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
		CharToTChar(szBuffer,szWideBuffer);
		g_LogDlg.AddLine(szWideBuffer);
#else
		g_LogDlg.AddLine(szBuffer);
#endif
	}

	// Always skip the copyright line.
	if (!strncmp(szBuffer,CDRTOOLS_COPYRIGHT,CDRTOOLS_COPYRIGHT_LENGTH))
		return;

	// Check for a cygwin path.
	if (m_bErrorPathMode)
	{
		if (!strncmp(szBuffer,CDRTOOLS_CYGWINPATH,CDRTOOLS_CYGWINPATH_LENGTH))
		{
			// An error message has been found.
			if (!strncmp(szBuffer + m_uiCDRToolsPathLen,CDRTOOLS_ERROR,CDRTOOLS_ERROR_LENGTH))
			{
				ErrorOutputCDRECORD(szBuffer + m_uiCDRToolsPathLen + CDRTOOLS_ERROR_LENGTH);
				return;
			}
			else if (!strncmp(szBuffer + m_uiCDRToolsPathLen,CDRTOOLS_ERROR2,CDRTOOLS_ERROR2_LENGTH))
			{
				ErrorOutputMKISOFS(szBuffer + m_uiCDRToolsPathLen + CDRTOOLS_ERROR2_LENGTH);
				return;
			}
			else if (!strncmp(szBuffer + m_uiCDRToolsPathLen,CDRTOOLS_ERROR3,CDRTOOLS_ERROR3_LENGTH))
			{
				ErrorOutputREADCD(szBuffer + m_uiCDRToolsPathLen + CDRTOOLS_ERROR3_LENGTH);
				return;
			}
			else if (!strncmp(szBuffer + m_uiCDRToolsPathLen,CDRTOOLS_ERROR4,CDRTOOLS_ERROR4_LENGTH))
			{
				ErrorOutputCDDA2WAV(szBuffer + m_uiCDRToolsPathLen + CDRTOOLS_ERROR4_LENGTH);
				return;
			}
		}
	}
	else
	{
		// An error message has been found.
		if (!strncmp(szBuffer,CDRTOOLS_ERROR,CDRTOOLS_ERROR_LENGTH))
		{
			ErrorOutputCDRECORD(szBuffer + CDRTOOLS_ERROR_LENGTH);
			return;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERROR2,CDRTOOLS_ERROR2_LENGTH))
		{
			ErrorOutputMKISOFS(szBuffer + CDRTOOLS_ERROR2_LENGTH);
			return;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERROR3,CDRTOOLS_ERROR3_LENGTH))
		{
			ErrorOutputREADCD(szBuffer + CDRTOOLS_ERROR3_LENGTH);
			return;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERROR4,CDRTOOLS_ERROR4_LENGTH))
		{
			ErrorOutputCDDA2WAV(szBuffer + CDRTOOLS_ERROR4_LENGTH);
			return;
		}
	}

	// If we are in grace time mode we only look for timer updates.
	if (m_iStatusMode == SMODE_GRACETIME)
	{
		int iTimer = 0;
		sscanf(szBuffer,"\b\b\b\b\b\b\b\b\b\b\b\b\b%4d seconds",&iTimer);

		// Update the status.
		m_pProgress->SetStatus(lngGetString(PROGRESS_GRACETIME),iTimer);

		// Leave grace time mode if the timer is 0.
		if (iTimer == 0)
		{
			m_iStatusMode = SMODE_DEFAULT;
			m_bGraceTimeDone = true;

			// We reset the line delimiter to the new line character.
			m_cLineDelimiter = '\n';

			// Add a new message to the progress window (and update its staus).
			switch (m_iMode)
			{
				case MODE_ERASE:
					m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINERASE),
						m_bDummyMode ? lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
					m_pProgress->SetStatus(lngGetString(STATUS_ERASE));
					break;

				case MODE_FIXATE:
					m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINFIXATE),
						m_bDummyMode ? lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
					m_pProgress->SetStatus(lngGetString(STATUS_FIXATE));
					break;

				case MODE_BURNIMAGE:
				case MODE_BURNIMAGEEX:
					m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINWRITE),
						m_bDummyMode ? lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
					m_pProgress->SetStatus(lngGetString(STATUS_WRITEDATA));
					break;
			};

			// Start the smoke.
			//if (!m_bDummyMode)
				m_pProgress->StartSmoke();

			m_pProgress->SetRealMode(true);
		}

		return;
	}
	else if (m_iStatusMode == SMODE_PREPROGRESS)
	{
		// Now we want to terminate the strings by the x (speed).
		m_cLineDelimiter = 'x';

		// Change the mode to progress mode.
		m_iStatusMode = SMODE_PROGRESS;

		// Notify the status window that we're starting to write a new track.
		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINTRACK),
			m_uiCurrentTrack + 1);

		return;
	}
	else if (m_iStatusMode == SMODE_PROGRESS)
	{
		// "Track 01:    1 of  473 MB written (fifo 100%) [buf 100%]   0.3x"
		//int iTrack = 0;
		int iBuffer = 0;
		__int64 iWritten = 0;
		float fSpeed = 0.0f;

		char *pBuffer = (char *)szBuffer;
		if (pBuffer[0] == '.')
			*pBuffer++;

		// Special parsing, cdrtools has a bad habit of only writing output that is
		// not zero. This only affects the FIFO and buffer size. We know that the
		// track number, written size (in MB) and the write speed is always included
		// (cdrecord.c).
		pBuffer += 6;
		//iTrack = atoi(pBuffer);

		// Skip the integer.
		pBuffer = SkipInteger(pBuffer);

		// Skip the ':' character.
		*pBuffer++;

		iWritten = _atoi64(pBuffer);

		// Skip the integer.
		pBuffer = SkipInteger(pBuffer);

		// Skip trailing whitespace.
		*pBuffer++;

		// Look for total size (currently ignored).
		if (!strncmp(pBuffer,"of ",3))
		{
			pBuffer += 3;
			pBuffer = SkipInteger(pBuffer);

			// Skip trailing whitespace.
			*pBuffer++;
		}

		pBuffer += 11;

		// Look for FIFO (currently ignored).
		if (*pBuffer == '(')
		{
			pBuffer += 6;
			pBuffer = SkipInteger(pBuffer);

			// Skip the '%', ')' and whitespace character.
			pBuffer += 3;
		}

		// Look for buffer.
		if (*pBuffer == '[')
		{
			pBuffer += 5;
			iBuffer = atoi(pBuffer);
			pBuffer = SkipInteger(pBuffer);

			// Skip the '%', ']' and whitespace character.
			pBuffer += 3;
		}

		// Update: 2007-02-10. Skip BCAP information.
		if (*pBuffer == '|')
			pBuffer += 12;

		// Write speed.
		fSpeed = (float)atof(pBuffer);

		// Update the status.
		m_pProgress->SetStatus(lngGetString(STATUS_WRITE),m_uiCurrentTrack + 1,(int)m_TrackSize.size(),fSpeed);

		m_pProgress->SetProgress((int)(((double)(iWritten + m_uiProcessedSize)/m_uiTotalSize) * 100));
		m_pProgress->SetBuffer(iBuffer);

		// Check if we're done writing the track.
		if (iWritten != 0 && iWritten == m_TrackSize[m_uiCurrentTrack/*iTrack - 1*/])
		{
			m_uiCurrentTrack++;

			m_cLineDelimiter = '\n';

			// Leave the progress mode.
			m_iStatusMode = SMODE_DEFAULT;

			// Update the totoal processed size.
			m_uiProcessedSize += iWritten;
		}

		return;
	}
	else if (m_iStatusMode == SMODE_AUDIOPROGRESS)
	{
		int iProgress = atoi(szBuffer);
		m_pProgress->SetProgress(iProgress);

		if (iProgress == 100)
		{
			m_iStatusMode = SMODE_DEFAULT;

			m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(SUCCESS_READTRACK),m_uiTotalSize);
			m_bOperationRes = true;		// Success.
		}
	}

	switch (m_iMode)
	{
		case MODE_EJECT:
			break;

		case MODE_ERASE:
			EraseOutput(szBuffer);
			break;

		case MODE_FIXATE:
			FixateOutput(szBuffer);
			break;

		case MODE_BURNIMAGE:
		case MODE_BURNIMAGEEX:
			BurnImageOutput(szBuffer);
			break;

		case MODE_CREATEIMAGE:
		case MODE_CREATEIMAGEEX:
			CreateImageOutput(szBuffer);

		case MODE_READDATATRACK:
		case MODE_READDATATRACKEX:
			ReadDataTrackOutput(szBuffer);
			break;

		case MODE_READAUDIOTRACK:
		case MODE_READAUDIOTRACKEX:
			ReadAudioTrackOutput(szBuffer);
			break;

		case MODE_SCANTRACK:
		case MODE_SCANTRACKEX:
			ScanTrackOutput(szBuffer);
			break;

		case MODE_READDISC:
		case MODE_READDISCEX:
			ReadDiscOutput(szBuffer);
			break;

		case MODE_ESTIMATESIZE:
			EstimateSizeOutput(szBuffer);
			break;
	};
}

void CCore::ProcessEnded()
{
	if (g_GlobalSettings.m_bLog)
		g_LogDlg.AddLine(_T("CCore::ProcessEnded"));

	switch (m_iMode)
	{
		case MODE_EJECT:
			break;

		case MODE_ERASE:
		case MODE_FIXATE:
		case MODE_BURNIMAGE:
		case MODE_CREATEIMAGE:
		case MODE_READDATATRACK:
		case MODE_READAUDIOTRACK:
		case MODE_SCANTRACK:
		case MODE_READDISC:
			m_pProgress->SetProgress(100);
			m_pProgress->SetStatus(lngGetString(PROGRESS_DONE));
			m_pProgress->NotifyComplteted();
			break;

		case MODE_BURNIMAGEEX:
		case MODE_CREATEIMAGEEX:
		case MODE_READDATATRACKEX:
		case MODE_READAUDIOTRACKEX:
		case MODE_SCANTRACKEX:
		case MODE_READDISCEX:
		case MODE_ESTIMATESIZE:
			if (m_bOperationRes)
			{
				m_pProgress->SetProgress(0);
			}
			else
			{
				m_pProgress->SetProgress(100);
				m_pProgress->SetStatus(lngGetString(PROGRESS_DONE));
				m_pProgress->NotifyComplteted();
			}
			break;
	};
}

bool CCore::EjectDisc(tDeviceInfo *pDeviceInfo,bool bWaitForProcess)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::EjectDisc"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
	}

	// Initialize this object.
	Initialize(MODE_EJECT);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdrecord.exe\" -eject dev=");

	TCHAR szDeviceAdr[32];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szDeviceAdr);
	CommandLine += szDeviceAdr;

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::LoadDisc(tDeviceInfo *pDeviceInfo,bool bWaitForProcess)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::LoadDisc"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
	}

	// Initialize this object.
	Initialize(MODE_EJECT);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdrecord.exe\" -load dev=");

	TCHAR szDeviceAdr[32];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szDeviceAdr);
	CommandLine += szDeviceAdr;

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::EraseDisc(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,CAdvancedProgress *pProgress,
					  int iMode,bool bForce,bool bEject,bool bSimulate)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::EraseDisc"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Mode = %d, Force = %d, Eject = %d, Simulate = %d."),
			iMode,(int)bForce,(int)bEject,(int)bSimulate);
	}

	// Initialize this object.
	Initialize(MODE_ERASE,pProgress);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdrecord.exe\" -v -blank=");

	switch (iMode)
	{
		case 0:
			CommandLine += _T("all dev=");
			break;

		case 2:
			CommandLine += _T("unclose dev=");
			break;

		case 3:
			CommandLine += _T("session dev=");
			break;

		default:
			CommandLine += _T("fast dev=");
			break;
	}

	TCHAR szBuffer[32];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	if (bForce)
		CommandLine += _T(" -force");

	if (bEject)
		CommandLine += _T(" -eject");

	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING)
	{
		if (bSimulate)
			CommandLine += _T(" -dummy");
	}

	return SafeLaunch(CommandLine,false);
}

bool CCore::FixateDisc(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,CAdvancedProgress *pProgress,
					   bool bEject,bool bSimulate)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::FixateDisc"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Eject = %d, Simulate = %d."),(int)bEject,(int)bSimulate);
	}

	// Initialize this object.
	Initialize(MODE_FIXATE,pProgress);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdrecord.exe\" -v -fix dev=");

	TCHAR szBuffer[32];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	if (bEject)
		CommandLine += _T(" -eject");

	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING)
	{
		if (bSimulate)
			CommandLine += _T(" -dummy");
	}

	return SafeLaunch(CommandLine,false);
}

bool CCore::BurnImage(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
					  tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
					  const TCHAR *szFileName,bool bWaitForProcess,bool bCloneMode)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::BurnImage"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  File = %s."),szFileName);
		g_LogDlg.AddLine(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Fixate = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Clone = %d."),
			(int)g_BurnImageSettings.m_bEject,
			(int)g_BurnImageSettings.m_bSimulate,
			(int)g_BurnImageSettings.m_bBUP,
			(int)g_BurnImageSettings.m_bPadTracks,
			(int)g_BurnImageSettings.m_bFixate,
			(int)g_BurnAdvancedSettings.m_bOverburn,
			(int)g_BurnAdvancedSettings.m_bSwab,
			(int)g_BurnAdvancedSettings.m_bIgnoreSize,
			(int)g_BurnAdvancedSettings.m_bImmed,
			(int)g_BurnAdvancedSettings.m_bAudioMaster,
			(int)g_BurnAdvancedSettings.m_bForceSpeed,
			(int)g_BurnAdvancedSettings.m_bVariRec,
			g_BurnAdvancedSettings.m_iVariRec,
			(int)bCloneMode);
	}

	// Initialize this object.
	Initialize(MODE_BURNIMAGE,pProgress);

	// We need to specify the total size that we should record.
	m_uiProcessedSize = 0;
	m_uiTotalSize = fs_filesize(szFileName) / (1024 * 1024);		// MB.
	m_TrackSize.push_back(m_uiTotalSize);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdrecord.exe\" -v dev=");

	TCHAR szBuffer[32];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	// Clone.
	if (bCloneMode)
		CommandLine += _T(" -clone");

	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		lsprintf(szBuffer,_T(" fs=%dm"),g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING)
	{
		if (g_BurnImageSettings.m_bEject)
			CommandLine += _T(" -eject");
	}

	// Simulation.
	if (g_BurnImageSettings.m_bSimulate)
		CommandLine += _T(" -dummy");

	// Write method.
	switch (g_BurnImageSettings.m_iWriteMethod)
	{
		case WRITEMETHOD_SAO:
			CommandLine += _T(" -sao");
			break;

		case WRITEMETHOD_TAO:
			CommandLine += _T(" -tao");
			break;

		case WRITEMETHOD_TAONOPREGAP:
			CommandLine += _T(" -tao pregap=0");
			break;

		case WRITEMETHOD_RAW96R:
			CommandLine += _T(" -raw96r");
			break;

		case WRITEMETHOD_RAW16:
			CommandLine += _T(" -raw16");
			break;

		case WRITEMETHOD_RAW96P:
			CommandLine += _T(" -raw96p");
			break;
	};

	TCHAR szDriverOpts[128];
	lstrcpy(szDriverOpts,_T(" driveropts="));
	bool bUseDriverOpts = false;

	// Buffer underrun protection.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_BUFRECORDING)
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			lstrcat(szDriverOpts,_T("burnfree,"));
		else
			lstrcat(szDriverOpts,_T("noburnfree,"));
	}

	// Audio master.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_AUDIOMASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			lstrcat(szDriverOpts,_T("audiomaster,"));
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_FORCESPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			lstrcat(szDriverOpts,_T("noforcespeed,"));
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_VARIREC))
	{
		if (g_BurnAdvancedSettings.m_bVariRec)
		{
			TCHAR szVariRec[32];
			lsprintf(szVariRec,_T("varirec=%d,"),g_BurnAdvancedSettings.m_iVariRec);
			lstrcat(szDriverOpts,szVariRec);
			bUseDriverOpts = true;
		}
	}

	if (bUseDriverOpts)
	{
		szDriverOpts[lstrlen(szDriverOpts) - 1] = '\0';
		CommandLine += szDriverOpts;
	}

	// Pad tracks.
	if (g_BurnImageSettings.m_bPadTracks)
		CommandLine += _T(" -pad");

	// Fixate.
	if (!g_BurnImageSettings.m_bFixate)
		CommandLine += _T(" -nofix");

	// Overburning.
	if (g_BurnAdvancedSettings.m_bOverburn)
		CommandLine += _T(" -overburn");

	// Swap audio byte order.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_SWABAUDIO))
	{
		if (g_BurnAdvancedSettings.m_bSwab)
			CommandLine += _T(" -swab");
	}

	// Ignore size.
	if (g_BurnAdvancedSettings.m_bIgnoreSize)
		CommandLine += _T(" -ignsize");

	// SCSI IMMED flag.
	if (g_BurnAdvancedSettings.m_bImmed)
		CommandLine += _T(" -immed");

	// Speed.
	if (g_BurnImageSettings.m_uiWriteSpeed != -1)
	{
		lsprintf(szBuffer,_T(" speed=%d"),g_BurnImageSettings.m_uiWriteSpeed);
		CommandLine += szBuffer;
	}

	// File name.
	TCHAR szCygwinFileName[MAX_PATH + 16];
	GetCygwinFileName(szFileName,szCygwinFileName);

	TCHAR szFileExt[MAX_PATH];
	ExtractFileExt(szFileName,szFileExt);
	if (!lstrcmp(szFileExt,_T(".cue")))				// .cue file.
	{
		CommandLine += _T(" cuefile=\"");
		CommandLine += szCygwinFileName;
		CommandLine += _T("\"");
	}
	else											// any other.
	{
		CommandLine += _T(" \"");
		CommandLine += szCygwinFileName;
		CommandLine += _T("\"");
	}

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::BurnTracks(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
					  tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
					  const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
					  const TCHAR *szAudioText,int iDataMode,int iMode,bool bWaitForProcess)
{
	// This function behaves different from almost all the others using
	// m_bOperationRes. It actually assumes a successfull writing until proved
	// otherwise.
	m_bOperationRes = true;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::BurnTracks"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);

		if (szDataTrack != NULL)
			g_LogDlg.AddLine(_T("  File = %s."),szDataTrack);

		g_LogDlg.AddLine(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Fixate = %d, Method = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Mode = %d."),
			(int)g_BurnImageSettings.m_bEject,
			(int)g_BurnImageSettings.m_bSimulate,
			(int)g_BurnImageSettings.m_bBUP,
			(int)g_BurnImageSettings.m_bPadTracks,
			(int)g_BurnImageSettings.m_bFixate,
			g_BurnImageSettings.m_iWriteMethod,
			(int)g_BurnAdvancedSettings.m_bOverburn,
			(int)g_BurnAdvancedSettings.m_bSwab,
			(int)g_BurnAdvancedSettings.m_bIgnoreSize,
			(int)g_BurnAdvancedSettings.m_bImmed,
			(int)g_BurnAdvancedSettings.m_bAudioMaster,
			(int)g_BurnAdvancedSettings.m_bForceSpeed,
			(int)g_BurnAdvancedSettings.m_bVariRec,
			g_BurnAdvancedSettings.m_iVariRec,
			iDataMode);
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// We need to specify the total size that we should record.
	m_uiProcessedSize = 0;

	if (szDataTrack != NULL)
	{
		m_uiTotalSize = fs_filesize(szDataTrack) / (1024 * 1024);		// MB.
		m_TrackSize.push_back(m_uiTotalSize);
	}
	else
	{
		m_uiTotalSize = 0;
	}

	for (unsigned int i = 0; i < AudioTracks.size(); i++)
	{
		unsigned __int64 uiTrackSize = fs_filesize(AudioTracks[i]) / (1024 * 1024);
		m_TrackSize.push_back(uiTrackSize);

		m_uiTotalSize += uiTrackSize;
	}

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdrecord.exe\" -v dev=");

	TCHAR szBuffer[32];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		lsprintf(szBuffer,_T(" fs=%dm"),g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING)
	{
		if (g_BurnImageSettings.m_bEject)
			CommandLine += _T(" -eject");
	}

	// Simulation.
	if (g_BurnImageSettings.m_bSimulate)
		CommandLine += _T(" -dummy");

	// Write method.
	switch (g_BurnImageSettings.m_iWriteMethod)
	{
		case WRITEMETHOD_SAO:
			CommandLine += _T(" -sao");
			break;

		case WRITEMETHOD_TAO:
			CommandLine += _T(" -tao");
			break;

		case WRITEMETHOD_TAONOPREGAP:
			CommandLine += _T(" -tao pregap=0");
			break;

		case WRITEMETHOD_RAW96R:
			CommandLine += _T(" -raw96r");
			break;

		case WRITEMETHOD_RAW16:
			CommandLine += _T(" -raw16");
			break;

		case WRITEMETHOD_RAW96P:
			CommandLine += _T(" -raw96p");
			break;
	};

	TCHAR szDriverOpts[128];
	lstrcpy(szDriverOpts,_T(" driveropts="));
	bool bUseDriverOpts = false;

	// Buffer underrun protection.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_BUFRECORDING)
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			lstrcat(szDriverOpts,_T("burnfree,"));
		else
			lstrcat(szDriverOpts,_T("noburnfree,"));
	}

	// Audio master.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_AUDIOMASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			lstrcat(szDriverOpts,_T("audiomaster,"));
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_FORCESPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			lstrcat(szDriverOpts,_T("noforcespeed,"));
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_VARIREC))
	{
		if (g_BurnAdvancedSettings.m_bVariRec)
		{
			TCHAR szVariRec[32];
			lsprintf(szVariRec,_T("varirec=%d,"),g_BurnAdvancedSettings.m_iVariRec);
			lstrcat(szDriverOpts,szVariRec);
			bUseDriverOpts = true;
		}
	}

	if (bUseDriverOpts)
	{
		szDriverOpts[lstrlen(szDriverOpts) - 1] = '\0';
		CommandLine += szDriverOpts;
	}

	// Pad tracks.
	if (g_BurnImageSettings.m_bPadTracks)
		CommandLine += _T(" -pad");

	// Fixate.
	if (!g_BurnImageSettings.m_bFixate)
		CommandLine += _T(" -nofix");

	// Overburning.
	if (g_BurnAdvancedSettings.m_bOverburn)
		CommandLine += _T(" -overburn");

	// Swap audio byte order.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_SWABAUDIO))
	{
		if (g_BurnAdvancedSettings.m_bSwab)
			CommandLine += _T(" -swab");
	}

	// Ignore size.
	if (g_BurnAdvancedSettings.m_bIgnoreSize)
		CommandLine += _T(" -ignsize");

	// SCSI IMMED flag.
	if (g_BurnAdvancedSettings.m_bImmed)
		CommandLine += _T(" -immed");

	// Speed.
	if (g_BurnImageSettings.m_uiWriteSpeed != -1)
	{
		lsprintf(szBuffer,_T(" speed=%d"),g_BurnImageSettings.m_uiWriteSpeed);
		CommandLine += szBuffer;
	}

	// File name.
	TCHAR szCygwinFileName[MAX_PATH + 16];

	if (szDataTrack != NULL)
	{
		// Mode.
		switch (iDataMode)
		{
			case 0:		// Mode 1
				CommandLine += _T(" -data");
				break;

			case 1:		// Mode 2 XA (multisession)
				CommandLine += _T(" -multi");
				break;
		};

		GetCygwinFileName(szDataTrack,szCygwinFileName);

		TCHAR szFileExt[MAX_PATH];
		ExtractFileExt(szDataTrack,szFileExt);
		if (!lstrcmp(szFileExt,_T(".cue")))				// .cue file.
		{
			CommandLine += _T(" cuefile=\"");
			CommandLine += szCygwinFileName;
			CommandLine += _T("\"");
		}
		else											// any other.
		{
			CommandLine += _T(" \"");
			CommandLine += szCygwinFileName;
			CommandLine += _T("\"");
		}
	}

	// Audio tracks.
	if (AudioTracks.size() > 0)
		CommandLine += _T(" -audio");

	for (unsigned int i = 0; i < AudioTracks.size(); i++)
	{
		GetCygwinFileName(AudioTracks[i],szCygwinFileName);

		CommandLine += _T(" \"");
		CommandLine += szCygwinFileName;
		CommandLine += _T("\"");
	}

	// Audio text.
	if (szAudioText != NULL)
	{
		CommandLine += _T(" textfile=\"");
		CommandLine += szAudioText;
		CommandLine += _T("\"");
	}

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::BurnImage(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
					  tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
					  const TCHAR *szFileName,bool bCloneMode)
{
	return BurnImage(pDeviceInfo,pDeviceCap,pDeviceInfoEx,pProgress,szFileName,false,bCloneMode);
}

/*
	CCore::BurnImageEx
	------------------
	Same as the function above except that it will not return untill the process
	has ended.
*/
bool CCore::BurnImageEx(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
						tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
						const TCHAR *szFileName,bool bCloneMode)
{
	return BurnImage(pDeviceInfo,pDeviceCap,pDeviceInfoEx,pProgress,szFileName,true,bCloneMode);
}

bool CCore::BurnTracks(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
					   tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
					   const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
					   const TCHAR *szAudioText,int iDataMode)
{
	return BurnTracks(pDeviceInfo,pDeviceCap,pDeviceInfoEx,pProgress,szDataTrack,
		AudioTracks,szAudioText,iDataMode,MODE_BURNIMAGE,true);
}

/*
	CCore::BurnTracksEx
	-------------------
	Works like CCore::BurnTracks but it does not end the progress when done. It
	allows for more operations to be performed in the same progress window. It
	also has extra return values.
*/
int CCore::BurnTracksEx(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
						 tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
						 const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
						 const TCHAR *szAudioText,int iDataMode)
{
	if (!BurnTracks(pDeviceInfo,pDeviceCap,pDeviceInfoEx,pProgress,szDataTrack,
		AudioTracks,szAudioText,iDataMode,MODE_BURNIMAGEEX,true))
		return RESULT_INTERNALERROR;

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

bool CCore::CreateImage(const TCHAR *szFileName,const TCHAR *szPathList,
						CAdvancedProgress *pProgress,int iMode,bool bEstimateSize,
						bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		if (bEstimateSize)
			g_LogDlg.AddLine(_T("CCore::CreateImage (estimate file system size only)"));
		else
			g_LogDlg.AddLine(_T("CCore::CreateImage"));

		g_LogDlg.AddLine(_T("  File = %s."),szFileName);

		g_LogDlg.AddLine(_T("  ISOLevel = %d, ISOCharSet = %d, Joliet = %d, JolietLongNames = %d, UDF = %d, RR = %d, OmitVN = %d, DVD-Video = %d, BootImages = %d."),
			g_ProjectSettings.m_iISOLevel,
			g_ProjectSettings.m_iISOCharSet,
			(int)g_ProjectSettings.m_bJoliet,
			(int)g_ProjectSettings.m_bJolietLongNames,
			(int)g_ProjectSettings.m_bUDF,
			(int)g_ProjectSettings.m_bRockRidge,
			(int)g_ProjectSettings.m_bOmitVN,
			(int)g_ProjectSettings.m_bDVDVideo,
			(int)g_ProjectSettings.m_BootImages.size());
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("mkisofs.exe\" -V \"");
	CommandLine += g_ProjectSettings.m_szLabel;
	CommandLine += _T("\"");

	// First we try to save a file containing the volume information to we
	// don't have to waste command line space.
	CStringContainerA StringContainer;
	char szLargeBuffer[MAX_PATH + 32];

	// Publiser.
	strcpy(szLargeBuffer,"PUBL=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szPublisher,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szPublisher);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Preparer.
	strcpy(szLargeBuffer,"PREP=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szPreparer,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szPreparer);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Application (*).
	StringContainer.m_szStrings.push_back("APPI=InfraRecorder (C) 2006-2007 Christian Kindahl");

	// System.
	strcpy(szLargeBuffer,"SYSI=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szSystem,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szSystem);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Volume set.
	strcpy(szLargeBuffer,"VOLS=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szVolumeSet,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szVolumeSet);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Copyright.
	strcpy(szLargeBuffer,"COPY=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szCopyright,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szCopyright);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Abstract.
	strcpy(szLargeBuffer,"ABST=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szAbstract,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szAbstract);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Bibliographic.
	strcpy(szLargeBuffer,"BIBL=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szBibliographic,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szBibliographic);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Calculate the file name and try to save the file.
	TCHAR szMkisofsFileName[MAX_PATH];
	lstrcpy(szMkisofsFileName,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szMkisofsFileName,_T(".mkisofsrc"));

	if (StringContainer.SaveToFile(szMkisofsFileName) == SCRES_OK)
	{
		// If the file was successfully created, make sure that it's removed
		// when InfraRecorder closes.
		g_TempManager.AddObject(szMkisofsFileName);
	}
	else
	{
		if (g_GlobalSettings.m_bLog)
			g_LogDlg.AddLine(_T("  Warning: Could write .mkisofs file, using the command line instead."));

		// Publisher.
		if (g_ProjectSettings.m_szPublisher[0] != '\0')
		{
			CommandLine += _T(" -publisher \"");
			CommandLine += g_ProjectSettings.m_szPublisher;
			CommandLine += _T("\"");
		}

		// Preparer.
		if (g_ProjectSettings.m_szPreparer[0] != '\0')
		{
			CommandLine += _T(" -p \"");
			CommandLine += g_ProjectSettings.m_szPreparer;
			CommandLine += _T("\"");
		}

		// Application (*)
		CommandLine += _T(" -A \"InfraRecorder\"");

		// System.
		if (g_ProjectSettings.m_szSystem[0] != '\0')
		{
			CommandLine += _T(" -sysid \"");
			CommandLine += g_ProjectSettings.m_szSystem;
			CommandLine += _T("\"");
		}

		// Volume set.
		if (g_ProjectSettings.m_szVolumeSet[0] != '\0')
		{
			CommandLine += _T(" -volset \"");
			CommandLine += g_ProjectSettings.m_szVolumeSet;
			CommandLine += _T("\"");
		}

		// Copyright.
		if (g_ProjectSettings.m_szCopyright[0] != '\0')
		{
			CommandLine += _T(" -copyright \"");
			CommandLine += g_ProjectSettings.m_szCopyright;
			CommandLine += _T("\"");
		}

		// Abstract.
		if (g_ProjectSettings.m_szAbstract[0] != '\0')
		{
			CommandLine += _T(" -abstract \"");
			CommandLine += g_ProjectSettings.m_szAbstract;
			CommandLine += _T("\"");
		}

		// Bibliographic.
		if (g_ProjectSettings.m_szBibliographic[0] != '\0')
		{
			CommandLine += _T(" -biblio \"");
			CommandLine += g_ProjectSettings.m_szBibliographic;
			CommandLine += _T("\"");
		}
	}

	// ISO level.
	TCHAR szBuffer[64];
	lsprintf(szBuffer,_T(" -iso-level %d"),g_ProjectSettings.m_iISOLevel + 1);
	CommandLine += szBuffer;

	// ISO character set.
	CommandLine += _T(" -input-charset ");
	CommandLine += g_szCharacterSets[g_ProjectSettings.m_iISOCharSet];

	// Joliet.
	if (g_ProjectSettings.m_bJoliet)
	{
		CommandLine += _T(" -r -J");

		if (g_ProjectSettings.m_bJolietLongNames)
			CommandLine += _T(" -joliet-long");
	}
	else
	{
		// Rock Ridge (automatically enabled when using Joliet).
		if (g_ProjectSettings.m_bRockRidge)
			CommandLine += _T(" -r");
	}

	// UDF.
	if (g_ProjectSettings.m_bUDF)
		CommandLine += _T(" -udf");

	// Omit version numbers.
	if (g_ProjectSettings.m_bOmitVN)
		CommandLine += _T(" -N");

	if (g_ProjectSettings.m_bDVDVideo)
		CommandLine += _T(" -dvd-video");

	// Boot information.
	bool bFirst = true;

	std::list <CProjectBootImage *>::iterator itImageObject;
	for (itImageObject = g_ProjectSettings.m_BootImages.begin(); itImageObject != g_ProjectSettings.m_BootImages.end(); itImageObject++)
	{
		if (!bFirst)
			CommandLine += _T(" -eltorito-alt-boot");

		CommandLine += _T(" -b \"");

		// Prepare the full relative image name.
		TCHAR szImageFullPath[MAX_PATH];
		if ((*itImageObject)->m_LocalPath.c_str()[0] == '/' ||
			(*itImageObject)->m_LocalPath.c_str()[0] == '\\')
		{
			lstrcpy(szImageFullPath,(*itImageObject)->m_LocalPath.c_str() + 1);
		}
		else
		{
			lstrcpy(szImageFullPath,(*itImageObject)->m_LocalPath.c_str());
		}

		lstrcat(szImageFullPath,(*itImageObject)->m_LocalName.c_str());
		ForceSlashDelimiters(szImageFullPath);

		CommandLine += szImageFullPath;
		CommandLine += _T("\"");

		switch ((*itImageObject)->m_iEmulation)
		{
			case PROJECTBI_BOOTEMU_NONE:
				CommandLine += _T(" -no-emul-boot");

				CommandLine += _T(" -boot-load-seg ");
				lsnprintf_s(szBuffer,64,_T("%d"),(*itImageObject)->m_iLoadSegment);
				CommandLine += szBuffer;

				CommandLine += _T(" -boot-load-size ");
				lsnprintf_s(szBuffer,64,_T("%d"),(*itImageObject)->m_iLoadSize);
				CommandLine += szBuffer;
				break;

			case PROJECTBI_BOOTEMU_FLOPPY:
				if ((*itImageObject)->m_bNoBoot)
					CommandLine += _T(" -no-boot");
				break;

			case PROJECTBI_BOOTEMU_HARDDISK:
				CommandLine += _T(" -hard-disk-boot");

				if ((*itImageObject)->m_bNoBoot)
					CommandLine += _T(" -no-boot");
				break;
		}

		if ((*itImageObject)->m_bBootInfoTable)
			CommandLine += _T(" -boot-info-table");

		bFirst = false;
	}

	if (g_ProjectSettings.m_BootImages.size() > 0)
	{
		CommandLine += _T(" -c ");
		CommandLine += g_ProjectSettings.m_szBootCatalog;
	}

	// Multisession.
	if (g_ProjectSettings.m_bMultiSession)
	{
		lsprintf(szBuffer,_T(" -C %I64d,%I64d"),g_ProjectSettings.m_uiLastSession,g_ProjectSettings.m_uiNextSession);
		CommandLine += szBuffer;

		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_ProjectSettings.m_uiDeviceIndex);
		
		if (pDeviceInfo != NULL)
		{
			g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);

			CommandLine += _T(" -M ");
			CommandLine += szBuffer;
		}
	}

	// Paths.
	CommandLine += _T(" -graft-points -path-list \"");
	CommandLine += szPathList;

	if (bEstimateSize)
	{
		CommandLine += _T("\" -print-size");

		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINESTIMAGESIZE));
	}
	else
	{
		CommandLine += _T("\" -gui -o \"");
		CommandLine += szFileName;
		CommandLine += _T("\"");

		m_pProgress->AddLogEntry(LOGTYPE_INFORMATION,lngGetString(PROGRESS_BEGINDISCIMAGE));
	}

	return SafeLaunch(CommandLine,bWaitForProcess);
}

/*
	CCore::CreateImage
	------------------
	Creates a disc image to szFileName containing the files in the szPathList file.
*/
bool CCore::CreateImage(const TCHAR *szFileName,const TCHAR *szPathList,
						CAdvancedProgress *pProgress)
{
	return CreateImage(szFileName,szPathList,pProgress,MODE_CREATEIMAGE,false,true);
}

/*
	CCore::CreateImageEx
	--------------------
	Same as CreateImage but does not tell the progress window when the process
	has completed so multiple actions can be performed using the same progress
	window. It also has extra return values.
*/
int CCore::CreateImageEx(const TCHAR *szFileName,const TCHAR *szPathList,
						CAdvancedProgress *pProgress)
{
	if (!CreateImage(szFileName,szPathList,pProgress,MODE_CREATEIMAGEEX,false,true))
		return RESULT_INTERNALERROR;

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
	CCore::ReadDataTrack
	--------------------
	Reads a track from the CD and stores the raw binary content in the file
	named szFileName.
*/
bool CCore::ReadDataTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,
						   const TCHAR *szFileName,unsigned int uiTrackNumber,
						   unsigned long ulStartSector,unsigned long ulEndSector,
						   int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::ReadDataTrack"));
		g_LogDlg.AddLine(_T("  File = %s."),szFileName);
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Start = %d, End = %d."),ulStartSector,ulEndSector);
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// We need to specify the end adresses (in sectors) that we should read.
	m_uiProcessedSize = 0;
	m_uiTotalSize = ulEndSector - ulStartSector;	// Track length (in sectors).
	m_TrackSize.push_back(ulStartSector);			// Start address of the track.
	m_TrackSize.push_back(uiTrackNumber);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("readcd.exe\" dev=");

	// Device address.
	TCHAR szBuffer[64];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

    // Sector range.
	lsprintf(szBuffer,_T(" sectors=%d-%d"),ulStartSector,ulEndSector);
	CommandLine += szBuffer;

	// File name.
	CommandLine += _T(" f=\"");
	CommandLine += szFileName;
	CommandLine += _T("\"");

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ReadDataTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,
						 const TCHAR *szFileName,unsigned int uiTrackNumber,
						 unsigned long ulStartSector,unsigned long ulEndSector)
{
	return ReadDataTrack(pDeviceInfo,pProgress,szFileName,uiTrackNumber,ulStartSector,ulEndSector,MODE_READDATATRACK,true);
}

int CCore::ReadDataTrackEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,
						   const TCHAR *szFileName,unsigned int uiTrackNumber,
						   unsigned long ulStartSector,unsigned long ulEndSector)
{
	if (!ReadDataTrack(pDeviceInfo,pProgress,szFileName,uiTrackNumber,ulStartSector,ulEndSector,MODE_READDATATRACKEX,true))
		return RESULT_INTERNALERROR;

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
*/
bool CCore::ReadAudioTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,
						   const TCHAR *szFileName,unsigned int uiTrackNumber,
						   int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::ReadAudioTrack"));
		g_LogDlg.AddLine(_T("  File = %s."),szFileName);
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// Remember what track we are working with.
	m_uiTotalSize = uiTrackNumber;

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("cdda2wav.exe\" -D ");

	// Device address.
	TCHAR szBuffer[64];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	// Miscellaneous.
	CommandLine += _T(" -I generic_scsi -x -B -O wav -g -H");

	// Track.
	lsprintf(szBuffer,_T(" -t %d+%d"),uiTrackNumber,uiTrackNumber);
	CommandLine += szBuffer;

	// File name.
	CommandLine += _T(" \"");
	CommandLine += szFileName;
	CommandLine += _T("\"");

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ReadAudioTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
						   unsigned int uiTrackNumber)
{
	return ReadAudioTrack(pDeviceInfo,pProgress,szFileName,uiTrackNumber,MODE_READAUDIOTRACK,true);
}
int CCore::ReadAudioTrackEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
							unsigned int uiTrackNumber)
{
	if (!ReadAudioTrack(pDeviceInfo,pProgress,szFileName,uiTrackNumber,MODE_READAUDIOTRACKEX,true))
		return RESULT_INTERNALERROR;

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
	CCore::ScanTrack
	----------------
	Scans the selected track for CRC and C2 errors.
*/
bool CCore::ScanTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,
					  unsigned int uiTrackNumber,unsigned long ulStartSector,
					  unsigned long ulEndSector,int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::ScanTrack"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Start = %d, End = %d."),ulStartSector,ulEndSector);
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// We need to specify the end adresses (in sectors) that we should read.
	m_uiProcessedSize = 0;
	m_uiTotalSize = ulEndSector - ulStartSector;	// Track length (in sectors).
	m_TrackSize.push_back(ulStartSector);			// Start address of the track.
	m_TrackSize.push_back(uiTrackNumber);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("readcd.exe\" -c2scan dev=");

	// Device address.
	TCHAR szBuffer[64];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

    // Sector range.
	lsprintf(szBuffer,_T(" sectors=%d-%d"),ulStartSector,ulEndSector);
	CommandLine += szBuffer;

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ScanTrack(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
					  unsigned long ulStartSector,unsigned long ulEndSector)
{
	return ScanTrack(pDeviceInfo,pProgress,uiTrackNumber,ulStartSector,ulEndSector,MODE_SCANTRACK,true);
}

int CCore::ScanTrackEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
					   unsigned long ulStartSector,unsigned long ulEndSector)
{
	if (!ScanTrack(pDeviceInfo,pProgress,uiTrackNumber,ulStartSector,ulEndSector,MODE_SCANTRACKEX,true))
		return RESULT_INTERNALERROR;

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
	CCore::CopyDisc
	---------------
	Performs an on-the-fly copy of a disc. This function is configured through the
	g_BurnImageSettings, g_BurnAdvancedSettings and g_ReadSettings (m_bIgnoreErr
	only) objects.
*/
bool CCore::CopyDisc(tDeviceInfo *pSourceDeviceInfo,tDeviceInfo *pTargetDeviceInfo,
					 tDeviceCap *pTargetDeviceCap,tDeviceInfoEx *pTargetDeviceInfoEx,
					 CAdvancedProgress *pProgress)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::CopyDisc"));
		g_LogDlg.AddLine(_T("  Source: [%d,%d,%d] %s %s %s"),pSourceDeviceInfo->Address.m_iBus,pSourceDeviceInfo->Address.m_iTarget,
			pSourceDeviceInfo->Address.m_iLun,pSourceDeviceInfo->szVendor,pSourceDeviceInfo->szIdentification,pSourceDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Target: [%d,%d,%d] %s %s %s"),pTargetDeviceInfo->Address.m_iBus,pTargetDeviceInfo->Address.m_iTarget,
			pTargetDeviceInfo->Address.m_iLun,pTargetDeviceInfo->szVendor,pTargetDeviceInfo->szIdentification,pTargetDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Fixate = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Ignore read errors = %d."),
			(int)g_BurnImageSettings.m_bEject,
			(int)g_BurnImageSettings.m_bSimulate,
			(int)g_BurnImageSettings.m_bBUP,
			(int)g_BurnImageSettings.m_bPadTracks,
			(int)g_BurnImageSettings.m_bFixate,
			(int)g_BurnAdvancedSettings.m_bOverburn,
			(int)g_BurnAdvancedSettings.m_bSwab,
			(int)g_BurnAdvancedSettings.m_bIgnoreSize,
			(int)g_BurnAdvancedSettings.m_bImmed,
			(int)g_BurnAdvancedSettings.m_bAudioMaster,
			(int)g_BurnAdvancedSettings.m_bForceSpeed,
			(int)g_BurnAdvancedSettings.m_bVariRec,
			g_BurnAdvancedSettings.m_iVariRec,
			(int)g_ReadSettings.m_bIgnoreErr);
	}

	// Initialize this object.
	Initialize(MODE_BURNIMAGE,pProgress);

	// Since this function uses a batch file for execution no cygwin paths are included in
	// stderr messages.
	m_bErrorPathMode = false;

	// We need to specify the total size that we should record.
	m_uiProcessedSize = 0;
	m_uiTotalSize = g_CopyDiscSettings.m_uiSourceSize / (1024 * 1024);		// MB.
	m_TrackSize.push_back(m_uiTotalSize);

	std::string CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = "readcd.exe -v dev=";

	// Source device.
	char szBuffer[32];
	g_DeviceManager.GetDeviceAddrA(pSourceDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	// Speed.
	if (g_BurnImageSettings.m_uiWriteSpeed != -1)
	{
		sprintf(szBuffer," speed=%d",g_BurnImageSettings.m_uiWriteSpeed);
		CommandLine += szBuffer;
	}

	// Ignore read errors.
	if (g_ReadSettings.m_bIgnoreErr)
		CommandLine += " -noerror -nocorr";

	// Redirection.
	CommandLine += " f=- 2> NUL: | ";

	// cdrecord.exe related.
	CommandLine += "cdrecord.exe -v dev=";

	g_DeviceManager.GetDeviceAddrA(pTargetDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	sprintf(szBuffer," gracetime=%d",g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		sprintf(szBuffer," fs=%dm",g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (pTargetDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING)
	{
		if (g_BurnImageSettings.m_bEject)
			CommandLine += " -eject";
	}

	// Simulation.
	if (g_BurnImageSettings.m_bSimulate)
		CommandLine += " -dummy";

	// Write method.
	switch (g_BurnImageSettings.m_iWriteMethod)
	{
		case WRITEMETHOD_SAO:
			CommandLine += " -sao";
			break;

		case WRITEMETHOD_TAO:
			CommandLine += " -tao";
			break;

		case WRITEMETHOD_TAONOPREGAP:
			CommandLine += " -tao pregap=0";
			break;

		case WRITEMETHOD_RAW96R:
			CommandLine += " -raw96r";
			break;

		case WRITEMETHOD_RAW16:
			CommandLine += " -raw16";
			break;

		case WRITEMETHOD_RAW96P:
			CommandLine += " -raw96p";
			break;
	};

	char szDriverOpts[128];
	strcpy(szDriverOpts," driveropts=");
	bool bUseDriverOpts = false;

	// Buffer underrun protection.
	if (pTargetDeviceCap->uiGeneral & DEVICEMANAGER_CAP_BUFRECORDING)
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			strcat(szDriverOpts,"burnfree,");
		else
			strcat(szDriverOpts,"noburnfree,");
	}

	// Audio master.
	if (strstr(pTargetDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_AUDIOMASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			strcat(szDriverOpts,"audiomaster,");
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (strstr(pTargetDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_FORCESPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			strcat(szDriverOpts,"noforcespeed,");
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (strstr(pTargetDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_VARIREC))
	{
		if (g_BurnAdvancedSettings.m_bVariRec)
		{
			char szVariRec[16];
			sprintf(szVariRec,"varirec=%d,",g_BurnAdvancedSettings.m_iVariRec);
			strcat(szDriverOpts,szVariRec);
			bUseDriverOpts = true;
		}
	}

	if (bUseDriverOpts)
	{
		szDriverOpts[strlen(szDriverOpts) - 1] = '\0';
		CommandLine += szDriverOpts;
	}

	// Pad tracks.
	if (g_BurnImageSettings.m_bPadTracks)
		CommandLine += " -pad";

	// Fixate.
	if (!g_BurnImageSettings.m_bFixate)
		CommandLine += " -nofix";

	// Overburning.
	if (g_BurnAdvancedSettings.m_bOverburn)
		CommandLine += " -overburn";

	// Swap audio byte order.
	if (strstr(pTargetDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_SWABAUDIO))
	{
		if (g_BurnAdvancedSettings.m_bSwab)
			CommandLine += " -swab";
	}

	// Ignore size.
	if (g_BurnAdvancedSettings.m_bIgnoreSize)
		CommandLine += " -ignsize";

	// SCSI IMMED flag.
	if (g_BurnAdvancedSettings.m_bImmed)
		CommandLine += " -immed";

	// Speed.
	if (g_BurnImageSettings.m_uiWriteSpeed != -1)
	{
		sprintf(szBuffer," speed=%d",g_BurnImageSettings.m_uiWriteSpeed);
		CommandLine += szBuffer;
	}

	// Redirection.
	CommandLine += " -";

	char szChangeDir[MAX_PATH + 3];
	strcpy(szChangeDir,"cd ");

#ifdef UNICODE
	char szFolderPath[MAX_PATH];
	TCharToChar(g_GlobalSettings.m_szCDRToolsPath,szFolderPath);

	strcat(szChangeDir,szFolderPath);
#else
	strcat(szChangeDir,g_GlobalSettings.m_szCDRToolsPath);
#endif

	if (CommandLine.length() > 2047)
	{
		// All versions below Windows XP only supports 2047 character command lines (using the shell).
		if (g_WinVer.m_ulMajorVersion < MAJOR_WINXP ||
			(g_WinVer.m_ulMajorVersion == MAJOR_WINXP && g_WinVer.m_ulMinorVersion < MINOR_WINXP))
		{
			TCHAR szMessage[256];
			lsnprintf_s(szMessage,256,lngGetString(ERROR_COMMANDLINE),2048);
			MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
			return false;
		}
		// Windows XP supports 8192 character command lines (using the shell).
		else if (CommandLine.length() > 8191)
		{
			TCHAR szMessage[256];
			lsnprintf_s(szMessage,256,lngGetString(ERROR_COMMANDLINE),8192);
			MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
			return false;
		}
	}

	// Create the batch file.
	TCHAR szBatchPath[MAX_PATH];
	CreateBatchFile(szChangeDir,CommandLine.c_str(),szBatchPath);

	TCHAR szBatchCmdLine[MAX_PATH + 2];
	lstrcpy(szBatchCmdLine,_T("\""));
	lstrcat(szBatchCmdLine,szBatchPath);
	lstrcat(szBatchCmdLine,_T("\""));

	bool bResult = Launch(szBatchCmdLine,true);
		fs_deletefile(szBatchPath);
	return bResult;
}

/*
	CCore::ReadDisc
	---------------
	Reads a disc to a disc image. This function is configured through the
	g_ReadSettings object.
*/
bool CCore::ReadDisc(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName,
					 int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::ReadDisc"));
		g_LogDlg.AddLine(_T("  File = %s."),szFileName);
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Ignore read errors = %d, Clone = %d, Speed = %d."),
			(int)g_ReadSettings.m_bIgnoreErr,
			(int)g_ReadSettings.m_bClone,
			g_ReadSettings.m_iReadSpeed);
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// We need to specify the end adresses (in sectors) that we should read.
	m_uiProcessedSize = 0;
	m_uiTotalSize = 0;

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T("readcd.exe\" dev=");

	// Device address.
	TCHAR szBuffer[64];
	g_DeviceManager.GetDeviceAddr(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	// Ignore read errors.
	if (g_ReadSettings.m_bIgnoreErr)
		CommandLine += _T(" -noerror -nocorr");

	// Clone.
	if (g_ReadSettings.m_bClone)
		CommandLine += _T(" -clone");

	// Speed.
	if (g_ReadSettings.m_iReadSpeed != -1)
		lsprintf(szBuffer,_T(" speed=%d"),g_ReadSettings.m_iReadSpeed);

	// File name.
	CommandLine += _T(" f=\"");
	CommandLine += szFileName;
	CommandLine += _T("\"");

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ReadDisc(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName)
{
	return ReadDisc(pDeviceInfo,pProgress,szFileName,MODE_READDISC,false);
}

int CCore::ReadDiscEx(tDeviceInfo *pDeviceInfo,CAdvancedProgress *pProgress,const TCHAR *szFileName)
{
	if (!ReadDisc(pDeviceInfo,pProgress,szFileName,MODE_READDISCEX,true))
		return RESULT_INTERNALERROR;

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
	CCore::BurnCompilation
	----------------------
	Burns a compilation on the fly to a disc. This function is configured through
	the g_BurnImageSettings and g_ProjectSettings object.
*/
bool CCore::BurnCompilation(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szPathList,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iDataMode,unsigned __int64 uiDataBytes,int iMode)
{
	m_bOperationRes = true;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_LogDlg.AddLine(_T("CCore::BurnCompilation"));
		g_LogDlg.AddLine(_T("  [%d,%d,%d] %s %s %s"),pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun,pDeviceInfo->szVendor,pDeviceInfo->szIdentification,pDeviceInfo->szRevision);
		g_LogDlg.AddLine(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Fixate = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d."),
			(int)g_BurnImageSettings.m_bEject,
			(int)g_BurnImageSettings.m_bSimulate,
			(int)g_BurnImageSettings.m_bBUP,
			(int)g_BurnImageSettings.m_bPadTracks,
			(int)g_BurnImageSettings.m_bFixate,
			(int)g_BurnAdvancedSettings.m_bOverburn,
			(int)g_BurnAdvancedSettings.m_bSwab,
			(int)g_BurnAdvancedSettings.m_bIgnoreSize,
			(int)g_BurnAdvancedSettings.m_bImmed,
			(int)g_BurnAdvancedSettings.m_bAudioMaster,
			(int)g_BurnAdvancedSettings.m_bForceSpeed,
			(int)g_BurnAdvancedSettings.m_bVariRec,
			g_BurnAdvancedSettings.m_iVariRec);
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// Since this function uses a batch file for execution no cygwin paths are included in
	// stderr messages.
	m_bErrorPathMode = false;

	// We need to specify the total size that we should record.
	m_uiProcessedSize = 0;

	m_uiTotalSize = uiDataBytes / (1024 * 1024);			// MiB.
	m_TrackSize.push_back(m_uiTotalSize);

	for (unsigned int i = 0; i < AudioTracks.size(); i++)
	{
		unsigned __int64 uiTrackSize = fs_filesize(AudioTracks[i]) / (1024 * 1024);
		m_TrackSize.push_back(uiTrackSize);

		m_uiTotalSize += uiTrackSize;
	}

	// Setup the command line.
	std::string CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = "mkisofs.exe -gui -V \"";

#ifdef UNICODE
	char szMultiBuffer[MAX_PATH];
	TCharToChar(g_ProjectSettings.m_szLabel,szMultiBuffer);
	CommandLine += szMultiBuffer;
#else
	CommandLine += g_ProjectSettings.m_szLabel;
#endif
	CommandLine += "\"";

	// First we try to save a file containing the volume information to we
	// don't have to waste command line space.
	CStringContainerA StringContainer;
	char szLargeBuffer[MAX_PATH + 32];

	// Publiser.
	strcpy(szLargeBuffer,"PUBL=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szPublisher,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szPublisher);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Preparer.
	strcpy(szLargeBuffer,"PREP=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szPreparer,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szPreparer);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Application (*).
	StringContainer.m_szStrings.push_back("APPI=InfraRecorder (C) 2006-2007 Christian Kindahl");

	// System.
	strcpy(szLargeBuffer,"SYSI=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szSystem,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szSystem);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Volume set.
	strcpy(szLargeBuffer,"VOLS=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szVolumeSet,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szVolumeSet);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Copyright.
	strcpy(szLargeBuffer,"COPY=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szCopyright,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szCopyright);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Abstract.
	strcpy(szLargeBuffer,"ABST=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szAbstract,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szAbstract);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Bibliographic.
	strcpy(szLargeBuffer,"BIBL=");
#ifdef UNICODE
	TCharToChar(g_ProjectSettings.m_szBibliographic,szLargeBuffer + 5);
#else
	lstrcat(szLargeBuffer,g_ProjectSettings.m_szBibliographic);
#endif
	StringContainer.m_szStrings.push_back(szLargeBuffer);

	// Calculate the file name and try to save the file.
	TCHAR szMkisofsFileName[MAX_PATH];
	lstrcpy(szMkisofsFileName,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szMkisofsFileName,_T(".mkisofsrc"));

	if (StringContainer.SaveToFile(szMkisofsFileName) == SCRES_OK)
	{
		// If the file was successfully created, make sure that it's removed
		// when InfraRecorder closes.
		g_TempManager.AddObject(szMkisofsFileName);
	}
	else
	{
		if (g_GlobalSettings.m_bLog)
			g_LogDlg.AddLine(_T("  Warning: Could write .mkisofs file, using the command line instead."));

		// Publisher.
		if (g_ProjectSettings.m_szPublisher[0] != '\0')
		{
			CommandLine += " -publisher \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szPublisher,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szPublisher;
#endif
			CommandLine += "\"";
		}

		// Preparer.
		if (g_ProjectSettings.m_szPreparer[0] != '\0')
		{
			CommandLine += " -p \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szPreparer,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szPreparer;
#endif
			CommandLine += "\"";
		}

		// Application (*)
		CommandLine += " -A \"InfraRecorder\"";

		// System.
		if (g_ProjectSettings.m_szSystem[0] != '\0')
		{
			CommandLine += " -sysid \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szSystem,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szSystem;
#endif
			CommandLine += "\"";
		}

		// Volume set.
		if (g_ProjectSettings.m_szVolumeSet[0] != '\0')
		{
			CommandLine += " -volset \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szVolumeSet,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szVolumeSet;
#endif
			CommandLine += "\"";
		}

		// Copyright.
		if (g_ProjectSettings.m_szCopyright[0] != '\0')
		{
			CommandLine += " -copyright \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szCopyright,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szCopyright;
#endif
			CommandLine += "\"";
		}

		// Abstract.
		if (g_ProjectSettings.m_szAbstract[0] != '\0')
		{
			CommandLine += " -abstract \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szAbstract,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szAbstract;
#endif
			CommandLine += "\"";
		}

		// Bibliographic.
		if (g_ProjectSettings.m_szBibliographic[0] != '\0')
		{
			CommandLine += " -biblio \"";
#ifdef UNICODE
			TCharToChar(g_ProjectSettings.m_szBibliographic,szMultiBuffer);
			CommandLine += szMultiBuffer;
#else
			CommandLine += g_ProjectSettings.m_szBibliographic;
#endif
			CommandLine += "\"";
		}
	}

	// ISO level.
	char szBuffer[64];
	sprintf(szBuffer," -iso-level %d",g_ProjectSettings.m_iISOLevel + 1);
	CommandLine += szBuffer;

	// ISO character set.
	CommandLine += " -input-charset ";
#ifdef UNICODE
	TCharToChar(g_szCharacterSets[g_ProjectSettings.m_iISOCharSet],szMultiBuffer);
	CommandLine += szMultiBuffer;
#else
	CommandLine += g_szCharacterSets[g_ProjectSettings.m_iISOCharSet];
#endif

	// Joliet.
	if (g_ProjectSettings.m_bJoliet)
	{
		CommandLine += " -r -J";

		if (g_ProjectSettings.m_bJolietLongNames)
			CommandLine += " -joliet-long";
	}
	else
	{
		// Rock Ridge (automatically enabled when using Joliet).
		if (g_ProjectSettings.m_bRockRidge)
			CommandLine += " -r";
	}

	// UDF.
	if (g_ProjectSettings.m_bUDF)
		CommandLine += " -udf";

	// Omit version numbers.
	if (g_ProjectSettings.m_bOmitVN)
		CommandLine += " -N";

	if (g_ProjectSettings.m_bDVDVideo)
		CommandLine += " -dvd-video";

	// Boot information.
	bool bFirst = true;

	std::list <CProjectBootImage *>::iterator itImageObject;
	for (itImageObject = g_ProjectSettings.m_BootImages.begin(); itImageObject != g_ProjectSettings.m_BootImages.end(); itImageObject++)
	{
		if (!bFirst)
			CommandLine += " -eltorito-alt-boot";

		CommandLine += " -b \"";

		// Prepare the full relative image name.
		TCHAR szImageFullPath[MAX_PATH];
		if ((*itImageObject)->m_LocalPath.c_str()[0] == '/' ||
			(*itImageObject)->m_LocalPath.c_str()[0] == '\\')
		{
			lstrcpy(szImageFullPath,(*itImageObject)->m_LocalPath.c_str() + 1);
		}
		else
		{
			lstrcpy(szImageFullPath,(*itImageObject)->m_LocalPath.c_str());
		}

		lstrcat(szImageFullPath,(*itImageObject)->m_LocalName.c_str());
		ForceSlashDelimiters(szImageFullPath);

#ifdef UNICODE
		TCharToChar(szImageFullPath,szMultiBuffer);
		CommandLine += szMultiBuffer;
#else
		CommandLine += szImageFullPath;
#endif
		CommandLine += "\"";

		switch ((*itImageObject)->m_iEmulation)
		{
			case PROJECTBI_BOOTEMU_NONE:
				CommandLine += " -no-emul-boot";

				CommandLine += " -boot-load-seg ";
				sprintf(szBuffer,"%d",(*itImageObject)->m_iLoadSegment);
				CommandLine += szBuffer;

				CommandLine += " -boot-load-size ";
				sprintf(szBuffer,"%d",(*itImageObject)->m_iLoadSize);
				CommandLine += szBuffer;
				break;

			case PROJECTBI_BOOTEMU_FLOPPY:
				if ((*itImageObject)->m_bNoBoot)
					CommandLine += " -no-boot";
				break;

			case PROJECTBI_BOOTEMU_HARDDISK:
				CommandLine += " -hard-disk-boot";

				if ((*itImageObject)->m_bNoBoot)
					CommandLine += " -no-boot";
				break;
		}

		if ((*itImageObject)->m_bBootInfoTable)
			CommandLine += " -boot-info-table";

		bFirst = false;
	}

	if (g_ProjectSettings.m_BootImages.size() > 0)
	{
		CommandLine += " -c ";
#ifdef UNICODE
		TCharToChar(g_ProjectSettings.m_szBootCatalog,szMultiBuffer);
		CommandLine += szMultiBuffer;
#else
		CommandLine += g_ProjectSettings.m_szBootCatalog;
#endif
	}

	// Multisession.
	if (g_ProjectSettings.m_bMultiSession)
	{
		sprintf(szBuffer," -C %I64d,%I64d",g_ProjectSettings.m_uiLastSession,g_ProjectSettings.m_uiNextSession);
		CommandLine += szBuffer;

		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_ProjectSettings.m_uiDeviceIndex);
		
		if (pDeviceInfo != NULL)
		{
			g_DeviceManager.GetDeviceAddrA(pDeviceInfo,szBuffer);

			CommandLine += " -M ";
			CommandLine += szBuffer;
		}
	}

	// Path list.
	CommandLine += " -graft-points -path-list \"";
#ifdef UNICODE
	TCharToChar(szPathList,szMultiBuffer);
	CommandLine += szMultiBuffer;
#else
	CommandLine += szPathList;
#endif
	CommandLine += "\" 2> NUL: | ";

	// cdrecord part of the command line.
	CommandLine += "cdrecord.exe -v dev=";

	g_DeviceManager.GetDeviceAddrA(pDeviceInfo,szBuffer);
	CommandLine += szBuffer;

	sprintf(szBuffer," gracetime=%d",g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		sprintf(szBuffer," fs=%dm",g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_TESTWRITING)
	{
		if (g_BurnImageSettings.m_bEject)
			CommandLine += " -eject";
	}

	// Simulation.
	if (g_BurnImageSettings.m_bSimulate)
		CommandLine += " -dummy";

	// Write method.
	switch (g_BurnImageSettings.m_iWriteMethod)
	{
		case WRITEMETHOD_SAO:
			CommandLine += " -sao";

			// The SAO method needs to know the size of the file system beforehand (specified in sectors).
			sprintf(szBuffer," tsize=%I64ds",uiDataBytes/2048);
			CommandLine += szBuffer;
			break;

		case WRITEMETHOD_TAO:
			CommandLine += " -tao";
			break;

		case WRITEMETHOD_TAONOPREGAP:
			CommandLine += " -tao pregap=0";
			break;

		case WRITEMETHOD_RAW96R:
			CommandLine += " -raw96r";
			break;

		case WRITEMETHOD_RAW16:
			CommandLine += " -raw16";
			break;

		case WRITEMETHOD_RAW96P:
			CommandLine += " -raw96p";
			break;
	};

	char szDriverOpts[128];
	strcpy(szDriverOpts," driveropts=");
	bool bUseDriverOpts = false;

	// Buffer underrun protection.
	if (pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_BUFRECORDING)
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			strcat(szDriverOpts,"burnfree,");
		else
			strcat(szDriverOpts,"noburnfree,");
	}

	// Audio master.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_AUDIOMASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			strcat(szDriverOpts,"audiomaster,");
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_FORCESPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			strcat(szDriverOpts,"noforcespeed,");
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_VARIREC))
	{
		if (g_BurnAdvancedSettings.m_bVariRec)
		{
			char szVariRec[32];
			sprintf(szVariRec,"varirec=%d,",g_BurnAdvancedSettings.m_iVariRec);
			strcat(szDriverOpts,szVariRec);
			bUseDriverOpts = true;
		}
	}

	if (bUseDriverOpts)
	{
		szDriverOpts[strlen(szDriverOpts) - 1] = '\0';
		CommandLine += szDriverOpts;
	}

	// Pad tracks.
	if (g_BurnImageSettings.m_bPadTracks)
		CommandLine += " -pad";

	// Fixate.
	if (!g_BurnImageSettings.m_bFixate)
		CommandLine += " -nofix";

	// Overburning.
	if (g_BurnAdvancedSettings.m_bOverburn)
		CommandLine += " -overburn";

	// Swap audio byte order.
	if (strstr(pDeviceInfoEx->szWriteFlags,CDRTOOLS_WRITEFLAGS_SWABAUDIO))
	{
		if (g_BurnAdvancedSettings.m_bSwab)
			CommandLine += " -swab";
	}

	// Ignore size.
	if (g_BurnAdvancedSettings.m_bIgnoreSize)
		CommandLine += " -ignsize";

	// SCSI IMMED flag.
	if (g_BurnAdvancedSettings.m_bImmed)
		CommandLine += " -immed";

	// Speed.
	if (g_BurnImageSettings.m_uiWriteSpeed != -1)
	{
		sprintf(szBuffer," speed=%d",g_BurnImageSettings.m_uiWriteSpeed);
		CommandLine += szBuffer;
	}

	// Mode.
	switch (iDataMode)
	{
		case 0:		// Mode 1
			CommandLine += " -data -";
			break;

		case 1:		// Mode 2 XA (multisession)
			CommandLine += " -multi -";
			break;
	};

	// Audio tracks.
	if (AudioTracks.size() > 0)
		CommandLine += " -audio";

	TCHAR szCygwinFileName[MAX_PATH + 16];

	for (unsigned int i = 0; i < AudioTracks.size(); i++)
	{
		GetCygwinFileName(AudioTracks[i],szCygwinFileName);

		CommandLine += " \"";
#ifdef UNICODE
		TCharToChar(szCygwinFileName,szMultiBuffer);
		CommandLine += szMultiBuffer;
#else
		CommandLine += szCygwinFileName;
#endif
		CommandLine += "\"";
	}

	// Audio text.
	if (szAudioText != NULL)
	{
		CommandLine += " textfile=\"";
#ifdef UNICODE
		TCharToChar(szCygwinFileName,szMultiBuffer);
		CommandLine += szMultiBuffer;
#else
		CommandLine += szAudioText;
#endif
		CommandLine += "\"";
	}

	// Change directory command.
	char szChangeDir[MAX_PATH + 3];
	strcpy(szChangeDir,"cd ");

#ifdef UNICODE
	char szFolderPath[MAX_PATH];
	TCharToChar(g_GlobalSettings.m_szCDRToolsPath,szFolderPath);

	strcat(szChangeDir,szFolderPath);
#else
	strcat(szChangeDir,g_GlobalSettings.m_szCDRToolsPath);
#endif

	if (CommandLine.length() > 2047)
	{
		// All versions below Windows XP only supports 2047 character command lines (using the shell).
		if (g_WinVer.m_ulMajorVersion < MAJOR_WINXP ||
			(g_WinVer.m_ulMajorVersion == MAJOR_WINXP && g_WinVer.m_ulMinorVersion < MINOR_WINXP))
		{
			TCHAR szMessage[256];
			lsnprintf_s(szMessage,256,lngGetString(ERROR_COMMANDLINE),2048);
			MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
			return false;
		}
		// Windows XP supports 8192 character command lines (using the shell).
		else if (CommandLine.length() > 8191)
		{
			TCHAR szMessage[256];
			lsnprintf_s(szMessage,256,lngGetString(ERROR_COMMANDLINE),8192);
			MessageBox(HWND_DESKTOP,szMessage,lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
			return false;
		}
	}

	// Create the batch file.
	TCHAR szBatchPath[MAX_PATH];
	CreateBatchFile(szChangeDir,CommandLine.c_str(),szBatchPath);

	TCHAR szBatchCmdLine[MAX_PATH + 2];
	lstrcpy(szBatchCmdLine,_T("\""));
	lstrcat(szBatchCmdLine,szBatchPath);
	lstrcat(szBatchCmdLine,_T("\""));

	bool bResult = Launch(szBatchCmdLine,true);
		fs_deletefile(szBatchPath);
	return bResult;
}

bool CCore::BurnCompilation(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szPathList,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iMode,unsigned __int64 uiDataBytes)
{
	return BurnCompilation(pDeviceInfo,pDeviceCap,pDeviceInfoEx,pProgress,szPathList,
		AudioTracks,szAudioText,iMode,uiDataBytes,MODE_BURNIMAGE);
}

int CCore::BurnCompilationEx(tDeviceInfo *pDeviceInfo,tDeviceCap *pDeviceCap,
		tDeviceInfoEx *pDeviceInfoEx,CAdvancedProgress *pProgress,
		const TCHAR *szPathList,std::vector<TCHAR *> &AudioTracks,
		const TCHAR *szAudioText,int iMode,unsigned __int64 uiDataBytes)
{
	if (!BurnCompilation(pDeviceInfo,pDeviceCap,pDeviceInfoEx,pProgress,szPathList,
		AudioTracks,szAudioText,iMode,uiDataBytes,MODE_BURNIMAGEEX))
		return RESULT_INTERNALERROR;		

	return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
	CCore::EstimateImageSize
	------------------------
	Estimates the file system size in multiples of sectors (2048 bytes). This
	is needed when burning discs in SAO (DAO) mode when the file system size
	needs to be known beforehand.
*/
bool CCore::EstimateImageSize(const TCHAR *szPathList,CAdvancedProgress *pProgress,unsigned __int64 &uiSize)
{
	bool bResult = CreateImage(NULL,szPathList,pProgress,MODE_ESTIMATESIZE,true,true);
		uiSize = m_uiEstimatedSize;
	return bResult;
}
