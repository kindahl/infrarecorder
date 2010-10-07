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
#include <ckcore/convert.hh>
#include <base/string_container.hh>
#include "cdrtools_parse_strings.hh"
#include "string_table.hh"
#include "settings.hh"
#include "log_dlg.hh"
#include "lang_util.hh"
#include "version.hh"
#include "temp_manager.hh"
#include "device_util.hh"
#include "core2.hh"
#include "core.hh"

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

	m_lNumCopies = 1;
}

void CCore::Reinitialize()
{
	m_bGraceTimeDone = false;
	m_bErrorPathMode = true;			// By default we're looking for a cygwin path before error messages.

	m_uiCDRToolsPathLen = lstrlen(g_GlobalSettings.m_szCDRToolsPathCyg);

	m_uiCurrentTrack = 0;
}

void CCore::CreateBatchFile(const char *szChangeDir,const char *szCommandLine,TCHAR *szBatchPath)
{
	// FIXME: This is not very nice.
	ckcore::File BatchFile = ckcore::File::temp(g_GlobalSettings.m_szTempPath,
												ckT("InfraRecorder"));
	lstrcpy(szBatchPath,BatchFile.name().c_str());

	// Delete the generated temporary file since we need a batch file.
	ckcore::File::remove(szBatchPath);

	ChangeFileExt(szBatchPath,_T(".bat"));

	CStringContainerA StringContainer;

	if (szChangeDir != NULL)
		StringContainer.m_szStrings.push_back(szChangeDir);

	StringContainer.m_szStrings.push_back(szCommandLine);
	StringContainer.SaveToFile(szBatchPath);
}

bool CCore::SafeLaunch(tstring &CommandLine,bool bWaitForProcess)
{
	if (g_GlobalSettings.m_bLog)
		g_pLogDlg->print_line(_T("  Command line to run: %s"),CommandLine.c_str());

	if (m_lNumCopies > 0)
		m_lNumCopies--;

	m_LastCmdLine = CommandLine;
	m_bLastWaitForProcess = bWaitForProcess;

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
				g_pLogDlg->print_line(_T("  Warning: The command line is %d characters long. Trying to execute through shell."),CommandLine.length());

			TCHAR szBatchPath[MAX_PATH];
#ifdef UNICODE
			char *szCommandLine = new char[CommandLine.length() + 1];
			UnicodeToAnsi(szCommandLine,CommandLine.c_str(),(int)CommandLine.length() + 1);

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
				bool bResult = create(szBatchCmdLine);
				ckcore::File::remove(szBatchPath);

				if (bWaitForProcess)
					wait();

				return bResult;
			}
			else
			{
				g_TempManager.AddObject(szBatchPath);
				bool bResult = create(szBatchCmdLine);

				if (bWaitForProcess)
					wait();

				return bResult;
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

	bool bResult = create((TCHAR *)CommandLine.c_str());

	if (bWaitForProcess)
		wait();

	return bResult;
}

bool CCore::Relaunch()
{
	if (m_lNumCopies <= 0 || m_LastCmdLine.size() == 0)
		return false;

	if (!m_pProgress->RequestNextDisc())
		return false;

	// Launch the process from a separate thread since the current thread belongs
	// to the previous process.
	unsigned long ulThreadID = 0;
	HANDLE hThread = ::CreateThread(NULL,0,NextCopyThread,this,0,&ulThreadID);

	if (hThread != NULL)
	{
		::CloseHandle(hThread);
		return true;
	}
	
	return false;
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
			add_block_delim('.');

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
		m_pProgress->set_status(lngGetString(PROGRESS_GRACETIME),iTimer);

		return true;
	}

	return false;
}

bool CCore::CheckProgress(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_STARTTRACK,CDRTOOLS_STARTTRACK_LENGTH))
	{
		add_block_delim('.');

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
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_NOMEDIA));
		else if (!strncmp(szBuffer,CDRTOOLS_BLANKERROR,CDRTOOLS_BLANKERROR_LENGTH))
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_ERASE));
		else if (!strncmp(szBuffer,CDRTOOLS_BLANKUNSUP,CDRTOOLS_BLANKUNSUP_LENGTH))
			m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(INFO_UNSUPERASEMODE));
		else if (!strncmp(szBuffer,CDRTOOLS_BLANKRETRY,CDRTOOLS_BLANKRETRY_LENGTH))
			m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(INFO_ERASERETRY));
		else if (!strncmp(szBuffer,CDRTOOLS_NODISC,CDRTOOLS_NODISC_LENGTH))
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_NOMEDIA));
		else if (!strncmp(szBuffer,CDRTOOLS_BADAUDIOCODING,CDRTOOLS_BADAUDIOCODING_LENGTH))
		{
			TCHAR szMessage[MAX_PATH + 128];
			lstrcpy(szMessage,lngGetString(FAILURE_AUDIOCODING));

#ifdef UNICODE
			TCHAR szFileName[MAX_PATH + 3];
			AnsiToUnicode(szFileName,szBuffer + CDRTOOLS_BADAUDIOCODING_LENGTH + 10,sizeof(szFileName) / sizeof(wchar_t));
			lstrcat(szMessage,szFileName);
#else
			lstrcat(szMessage,szBuffer + CDRTOOLS_BADAUDIOCODING_LENGTH + 10);
#endif

			m_pProgress->notify(ckcore::Progress::ckERROR,szMessage);
		}
		else if (!strncmp(szBuffer,CDRTOOLS_UNSUPPORTED,CDRTOOLS_UNSUPPORTED_LENGTH))
		{
			char *pBuffer = (char *)szBuffer + 12;

			if (!strncmp(pBuffer,CDRTOOLS_SECTOR,CDRTOOLS_SECTOR_LENGTH))
			{
				int iSectorSize = 0;
				sscanf(pBuffer,"sector size %ld for %*[^\0]",&iSectorSize);

				m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_BADSECTORSIZE),iSectorSize);
			}
		}
		else if (!strncmp(szBuffer,CDRTOOLS_WRITEERROR,CDRTOOLS_WRITEERROR_LENGTH))
		{
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_WRITE));
		}
#ifndef CORE_DVD_SUPPORT
		else if (!strncmp(szBuffer,CDRTOOLS_FOUNDDVDMEDIA,CDRTOOLS_FOUNDDVDMEDIA_LENGTH))
		{
			m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(FAILURE_DVDSUPPORT));
		}
#endif
		else if (!strncmp(szBuffer,CDRTOOLS_OPENSESSION,CDRTOOLS_OPENSESSION_LENGTH))
		{
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_OPENSESSION));
		}
		else if (!strncmp(szBuffer,CDRTOOLS_WARNINGCAP,CDRTOOLS_WARNINGCAP_LENGTH))	// "WARNING:" Prefix.
		{
			char *pBuffer = (char *)szBuffer + CDRTOOLS_WARNINGCAP_LENGTH + 1;

			if (!strncmp(pBuffer,CDRTOOLS_DISCSPACEWARNING,CDRTOOLS_DISCSPACEWARNING_LENGTH))
				m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(WARNING_DISCSIZE));
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERRORLEADIN,CDRTOOLS_ERRORLEADIN_LENGTH))
		{
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_WRITELEADIN));

			// When called from BurnTracks/BurnCompilation we need to flag the operation as failed.
			m_bOperationRes = false;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_ERRORINITDRIVE,CDRTOOLS_ERRORINITDRIVE_LENGTH))
		{
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_INITDRIVE));

			// When called from BurnTracks/BurnCompilation we need to flag the operation as failed.
			m_bOperationRes = false;
		}
		else if (!strncmp(szBuffer,CDRTOOLS_DVDRWDUMMY,CDRTOOLS_DVDRWDUMMY_LENGTH))
		{
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_DVDRWDUMMY));

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
		else if (!strncmp(szBuffer,CDRTOOLS_FIFO,CDRTOOLS_FIFO_LENGTH))
			return;
		else
		{
			m_pProgress->notify(ckcore::Progress::ckEXTERNAL,
								ckcore::string::ansi_to_auto<1024>(szBuffer).c_str());
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
	
				m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(ERROR_SECTOR),ulSector);
			}
		}
		else if (!strncmp(szBuffer,CDRTOOLS_RETRYSECTOR,CDRTOOLS_RETRYSECTOR_LENGTH))
		{
			unsigned long ulSector = atoi(szBuffer + CDRTOOLS_RETRYSECTOR_LENGTH + 1);

			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_READSOURCEDISC),ulSector);
		}
#ifdef CORE_PRINT_UNSUPERRORMESSAGES
		else
		{
			m_pProgress->notify(ckcore::Progress::ckEXTERNAL,
								ckcore::string::ansi_to_auto<1024>(szBuffer).c_str());
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
			m_pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_UNSUPRW));
	}
	else if (CheckGraceTime(szBuffer))
	{
		return;
	}
	else if (!strncmp(szBuffer,CDRTOOLS_BLANKTIME,CDRTOOLS_BLANKTIME_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_ERASE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_RELOADDRIVE,CDRTOOLS_RELOADDRIVE_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(FAILURE_LOADDRIVE));
		m_pProgress->set_status(lngGetString(ERROR_RELOADDRIVE));

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
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_RELOADDRIVE,CDRTOOLS_RELOADDRIVE_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(FAILURE_LOADDRIVE));
		m_pProgress->set_status(lngGetString(ERROR_RELOADDRIVE));

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
		m_pProgress->set_status(szStatus);
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FILLFIFO,CDRTOOLS_FILLFIFO_LENGTH))
	{
		m_pProgress->set_status(lngGetString(STATUS_FILLBUFFER));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_WRITETIME,CDRTOOLS_WRITETIME_LENGTH))
	{
		// Only display the error message if no error occured.
		if (m_bOperationRes)
			m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_WRITE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FIXATE,CDRTOOLS_FIXATE_LENGTH))
	{
		m_pProgress->set_status(lngGetString(STATUS_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_FIXATETIME,CDRTOOLS_FIXATETIME_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_WARNINGCAP,CDRTOOLS_WARNINGCAP_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(WARNING_FIXATE));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_RELOADDRIVE,CDRTOOLS_RELOADDRIVE_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(FAILURE_LOADDRIVE));
		m_pProgress->set_status(lngGetString(ERROR_RELOADDRIVE));

		// Enable the reload button.
		m_pProgress->AllowReload();
	}
}

void CCore::ReadDataTrackOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_END,CDRTOOLS_END_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINREADTRACK),m_TrackSize[1]);
		m_pProgress->set_status(lngGetString(STATUS_READTRACK));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_ADDRESS,CDRTOOLS_ADDRESS_LENGTH))
	{
		unsigned __int64 uiAddress = 0;
		unsigned __int64 uiCount = 0;

		if (sscanf(szBuffer,"addr: %8ld cnt: %ld",&uiAddress,&uiCount) == 2)
			m_pProgress->set_progress((unsigned char)(((double)(uiAddress - m_TrackSize[0])/m_uiTotalSize) * 100));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTIME,CDRTOOLS_TOTALTIME_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_READTRACK),m_TrackSize[1]);
		m_bOperationRes = true;			// Success.
	}
}

void CCore::ReadAudioTrackOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_PERCENTDONE,CDRTOOLS_PERCENTDONE_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINREADTRACK),m_uiTotalSize);
		m_pProgress->set_status(lngGetString(STATUS_READTRACK));

		m_iStatusMode = SMODE_AUDIOPROGRESS;
	}
}

void CCore::ScanTrackOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_END,CDRTOOLS_END_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINSCANTRACK),m_TrackSize[1]);
		m_pProgress->set_status(lngGetString(STATUS_SCANTRACK));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_ADDRESS,CDRTOOLS_ADDRESS_LENGTH))
	{
		unsigned __int64 uiAddress = 0;
		unsigned __int64 uiCount = 0;

		if (sscanf(szBuffer,"addr: %8ld cnt: %ld",&uiAddress,&uiCount) == 2)
			m_pProgress->set_progress((unsigned char)(((double)(uiAddress - m_TrackSize[0])/m_uiTotalSize) * 100));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTIME,CDRTOOLS_TOTALTIME_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_SCANTRACK),m_TrackSize[1]);
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
				m_pProgress->notify(ulSectors > 0 ? ckcore::Progress::ckWARNING : ckcore::Progress::ckINFORMATION,
					lngGetString(STATUS_C2TOTAL),ulBytes,ulBytes);
			}
		}
		else if (!strncmp(pBuffer,"rate",4))
		{
			float fRate = (float)atof(pBuffer + 6);

			m_pProgress->notify(fRate > 0 ? ckcore::Progress::ckWARNING : ckcore::Progress::ckINFORMATION,
				lngGetString(STATUS_C2RATE),fRate);
		}
	}
}

void CCore::ReadDiscOutput(const char *szBuffer)
{
	if (!strncmp(szBuffer,CDRTOOLS_END,CDRTOOLS_END_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINREADDISC));
		m_pProgress->set_status(lngGetString(STATUS_READDISC));

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
			m_pProgress->set_progress((unsigned char)(((double)uiAddress/m_uiTotalSize) * 100));
	}
	else if (!strncmp(szBuffer,CDRTOOLS_TOTALTIME,CDRTOOLS_TOTALTIME_LENGTH))
	{
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_READDISC));

		m_bOperationRes = true;			// Success.
	}
}

void CCore::event_output(const std::string &block)
{
	// Write to the log.
	if (g_GlobalSettings.m_bLog && m_iStatusMode == SMODE_DEFAULT)
	{
		g_pLogDlg->print(_T("   > "));
		g_pLogDlg->print_line(ckcore::string::ansi_to_auto<1024>(block.c_str()).c_str());
	}

	// Always skip the copyright line.
	if (!strncmp(block.c_str(),CDRTOOLS_COPYRIGHT,CDRTOOLS_COPYRIGHT_LENGTH))
		return;

	// Check for a cygwin path.
	if (m_bErrorPathMode)
	{
		if (!strncmp(block.c_str(),CDRTOOLS_CYGWINPATH,CDRTOOLS_CYGWINPATH_LENGTH))
		{
			// An error message has been found.
			if (!strncmp(block.c_str() + m_uiCDRToolsPathLen,CDRTOOLS_ERROR,CDRTOOLS_ERROR_LENGTH))
			{
				ErrorOutputCDRECORD(block.c_str() + m_uiCDRToolsPathLen + CDRTOOLS_ERROR_LENGTH);
				return;
			}
			else if (!strncmp(block.c_str() + m_uiCDRToolsPathLen,CDRTOOLS_ERROR3,CDRTOOLS_ERROR3_LENGTH))
			{
				ErrorOutputREADCD(block.c_str() + m_uiCDRToolsPathLen + CDRTOOLS_ERROR3_LENGTH);
				return;
			}
			else if (!strncmp(block.c_str() + m_uiCDRToolsPathLen,CDRTOOLS_ERROR4,CDRTOOLS_ERROR4_LENGTH))
			{
				ErrorOutputCDDA2WAV(block.c_str() + m_uiCDRToolsPathLen + CDRTOOLS_ERROR4_LENGTH);
				return;
			}
		}
	}
	else
	{
		// An error message has been found.
		if (!strncmp(block.c_str(),CDRTOOLS_ERROR,CDRTOOLS_ERROR_LENGTH))
		{
			ErrorOutputCDRECORD(block.c_str() + CDRTOOLS_ERROR_LENGTH);
			return;
		}
		else if (!strncmp(block.c_str(),CDRTOOLS_ERROR3,CDRTOOLS_ERROR3_LENGTH))
		{
			ErrorOutputREADCD(block.c_str() + CDRTOOLS_ERROR3_LENGTH);
			return;
		}
		else if (!strncmp(block.c_str(),CDRTOOLS_ERROR4,CDRTOOLS_ERROR4_LENGTH))
		{
			ErrorOutputCDDA2WAV(block.c_str() + CDRTOOLS_ERROR4_LENGTH);
			return;
		}
	}

	// If we are in grace time mode we only look for timer updates.
	if (m_iStatusMode == SMODE_GRACETIME)
	{
		int iTimer = 0;
		sscanf(block.c_str(),"\b\b\b\b\b\b\b\b\b\b\b\b\b%4d seconds",&iTimer);

		// Update the status.
		m_pProgress->set_status(lngGetString(PROGRESS_GRACETIME),iTimer);

		// Leave grace time mode if the timer is 0.
		if (iTimer == 0)
		{
			m_iStatusMode = SMODE_DEFAULT;
			m_bGraceTimeDone = true;

			// We reset the line delimiter to the new line character.
			remove_block_delim('.');

			// Add a new message to the progress window (and update its staus).
			switch (m_iMode)
			{
				case MODE_ERASE:
					m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINERASE),
						m_bDummyMode ? lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
					m_pProgress->set_status(lngGetString(STATUS_ERASE));
					break;

				case MODE_FIXATE:
					m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINFIXATE),
						m_bDummyMode ? lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
					m_pProgress->set_status(lngGetString(STATUS_FIXATE));
					break;

				case MODE_BURNIMAGE:
				case MODE_BURNIMAGEEX:
					m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINWRITE),
						m_bDummyMode ? lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
					m_pProgress->set_status(lngGetString(STATUS_WRITEDATA));
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
		add_block_delim('x');

		// Change the mode to progress mode.
		m_iStatusMode = SMODE_PROGRESS;

		// notify the status window that we're starting to write a new track.
		m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINTRACK),
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

		char *pBuffer = (char *)block.c_str();
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
		m_pProgress->set_status(lngGetString(STATUS_WRITE),m_uiCurrentTrack + 1,(int)m_TrackSize.size(),fSpeed);

		m_pProgress->set_progress((unsigned char)(((double)(iWritten + m_uiProcessedSize)/m_uiTotalSize) * 100));
		m_pProgress->SetBuffer(iBuffer);

		// Check if we're done writing the track.
		if (iWritten != 0 && iWritten == static_cast<__int64>(m_TrackSize[m_uiCurrentTrack]))
		{
			m_uiCurrentTrack++;

			remove_block_delim('x');

			// Leave the progress mode.
			m_iStatusMode = SMODE_DEFAULT;

			// Update the totoal processed size.
			m_uiProcessedSize += iWritten;
		}

		return;
	}
	else if (m_iStatusMode == SMODE_AUDIOPROGRESS)
	{
		int iProgress = atoi(block.c_str());
		m_pProgress->set_progress((unsigned char)iProgress);

		if (iProgress == 100)
		{
			m_iStatusMode = SMODE_DEFAULT;

			m_pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_READTRACK),m_uiTotalSize);
			m_bOperationRes = true;		// Success.
		}
	}

	switch (m_iMode)
	{
		case MODE_EJECT:
			break;

		case MODE_ERASE:
			EraseOutput(block.c_str());
			break;

		case MODE_FIXATE:
			FixateOutput(block.c_str());
			break;

		case MODE_BURNIMAGE:
		case MODE_BURNIMAGEEX:
			BurnImageOutput(block.c_str());
			break;

		case MODE_READDATATRACK:
		case MODE_READDATATRACKEX:
			ReadDataTrackOutput(block.c_str());
			break;

		case MODE_READAUDIOTRACK:
		case MODE_READAUDIOTRACKEX:
			ReadAudioTrackOutput(block.c_str());
			break;

		case MODE_SCANTRACK:
		case MODE_SCANTRACKEX:
			ScanTrackOutput(block.c_str());
			break;

		case MODE_READDISC:
		case MODE_READDISCEX:
			ReadDiscOutput(block.c_str());
			break;
	};
}

void CCore::event_finished()
{
	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::ProcessEnded"));
		g_pLogDlg->print_line(_T("  Process exited with code: %d."),uiExitCode);
	}

	switch (m_iMode)
	{
		case MODE_EJECT:
			break;

		case MODE_ERASE:
		case MODE_FIXATE:
		//case MODE_BURNIMAGE:
		case MODE_READDATATRACK:
		case MODE_READAUDIOTRACK:
		case MODE_SCANTRACK:
		case MODE_READDISC:
			m_pProgress->set_progress(100);
			m_pProgress->set_status(lngGetString(uiExitCode == 0 ? PROGRESS_DONE : PROGRESS_FAILED));
			m_pProgress->NotifyCompleted();
			break;

		case MODE_BURNIMAGE:
			m_pProgress->set_progress(100);
			m_pProgress->set_status(lngGetString(uiExitCode == 0 ? PROGRESS_DONE : PROGRESS_FAILED));

			if (m_lNumCopies <= 0 || !Relaunch())
				m_pProgress->NotifyCompleted();
			break;

		//case MODE_BURNIMAGEEX:
		case MODE_READDATATRACKEX:
		case MODE_READAUDIOTRACKEX:
		case MODE_SCANTRACKEX:
		case MODE_READDISCEX:
			if (m_bOperationRes)
			{
				m_pProgress->set_progress(0);
			}
			else
			{
				m_pProgress->set_progress(100);
				m_pProgress->set_status(lngGetString(uiExitCode == 0 ? PROGRESS_DONE : PROGRESS_FAILED));
				m_pProgress->NotifyCompleted();
			}
			break;

		case MODE_BURNIMAGEEX:
			if (m_lNumCopies <= 0 || !Relaunch())
			{
				if (m_bOperationRes)
				{
					m_pProgress->set_progress(0);
				}
				else
				{
					m_pProgress->set_progress(100);
					m_pProgress->set_status(lngGetString(uiExitCode == 0 ? PROGRESS_DONE : PROGRESS_FAILED));
					m_pProgress->NotifyCompleted();
				}
			}
			break;
	};
}

bool CCore::EjectDisc(ckmmc::Device &Device,bool bWaitForProcess)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::EjectDisc"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
	}

	// Initialize this object.
	Initialize(MODE_EJECT);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -eject dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::LoadDisc(ckmmc::Device &Device,bool bWaitForProcess)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::LoadDisc"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
	}

	// Initialize this object.
	Initialize(MODE_EJECT);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -load dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::EraseDisc(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					  int iMode,bool bForce,bool bEject,bool bSimulate)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::EraseDisc"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  Mode = %d, Force = %d, Eject = %d, Simulate = %d."),
							  iMode,(int)bForce,(int)bEject,(int)bSimulate);
	}

	// Initialize this object.
	Initialize(MODE_ERASE,pProgress);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -v -blank=");

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

	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	TCHAR szBuffer[64];
	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	if (bForce)
		CommandLine += _T(" -force");

	if (Device.support(ckmmc::Device::ckDEVICE_EJECT) && bEject)
		CommandLine += _T(" -eject");

	if (Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE) && bSimulate)
		CommandLine += _T(" -dummy");

	return SafeLaunch(CommandLine,false);
}

bool CCore::FixateDisc(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					   bool bEject,bool bSimulate)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::FixateDisc"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  Eject = %d, Simulate = %d."),
							  (int)bEject,(int)bSimulate);
	}

	// Initialize this object.
	Initialize(MODE_FIXATE,pProgress);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -v -fix dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	TCHAR szBuffer[64];
	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	if (Device.support(ckmmc::Device::ckDEVICE_EJECT) && bEject)
		CommandLine += _T(" -eject");

	if (Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE) && bSimulate)
		CommandLine += _T(" -dummy");

	return SafeLaunch(CommandLine,false);
}

bool CCore::BurnImage(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					  const TCHAR *szFileName,bool bWaitForProcess,bool bCloneMode)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::BurnImage"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  File = %s."),szFileName);
		g_pLogDlg->print_line(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Close = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Clone = %d."),
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

	// For creating multiple copies.
	m_lNumCopies = g_BurnImageSettings.m_lNumCopies;

	// We need to specify the total size that we should record.
	m_uiProcessedSize = 0;
	m_uiTotalSize = ckcore::File::size(szFileName) / (1024 * 1024);		// MB.
	m_TrackSize.push_back(m_uiTotalSize);

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -v dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	// Clone.
	if (bCloneMode)
		CommandLine += _T(" -clone");

	TCHAR szBuffer[64];
	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		lsprintf(szBuffer,_T(" fs=%dm"),g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (Device.support(ckmmc::Device::ckDEVICE_EJECT) &&
		g_BurnImageSettings.m_bEject)
	{
		CommandLine += _T(" -eject");
	}

	// Simulation.
	if (Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE) &&
		g_BurnImageSettings.m_bSimulate)
	{
		CommandLine += _T(" -dummy");
	}

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
	if (Device.support(ckmmc::Device::ckDEVICE_BUP))
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			lstrcat(szDriverOpts,_T("burnfree,"));
		else
			lstrcat(szDriverOpts,_T("noburnfree,"));
	}

	// Audio master.
	if (Device.support(ckmmc::Device::ckDEVICE_AUDIO_MASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			lstrcat(szDriverOpts,_T("audiomaster,"));
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (Device.support(ckmmc::Device::ckDEVICE_FORCE_SPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			lstrcat(szDriverOpts,_T("noforcespeed,"));
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (Device.support(ckmmc::Device::ckDEVICE_VARIREC))
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

	// Swap audio byte order. FIXME: Should possibly check for support before selecting.
	if (g_BurnAdvancedSettings.m_bSwab)
		CommandLine += _T(" -swab");

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

	ckcore::Path FilePath(szFileName);
	if (!ckcore::string::astrcmpi(FilePath.ext_name().c_str(),ckT("cue")))
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

bool CCore::BurnTracks(ckmmc::Device &Device,CAdvancedProgress *pProgress,
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
		g_pLogDlg->print_line(_T("CCore::BurnTracks"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());

		if (szDataTrack != NULL)
			g_pLogDlg->print_line(_T("  File = %s."),szDataTrack);

		g_pLogDlg->print_line(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Close = %d, Method = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Mode = %d."),
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
		m_uiTotalSize = ckcore::File::size(szDataTrack) / (1024 * 1024);		// MB.
		m_TrackSize.push_back(m_uiTotalSize);
	}
	else
	{
		m_uiTotalSize = 0;
	}

	for (unsigned int i = 0; i < AudioTracks.size(); i++)
	{
		unsigned __int64 uiTrackSize = ckcore::File::size(AudioTracks[i]) / (1024 * 1024);
		m_TrackSize.push_back(uiTrackSize);

		m_uiTotalSize += uiTrackSize;
	}

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -v dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	TCHAR szBuffer[64];
	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		lsprintf(szBuffer,_T(" fs=%dm"),g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (Device.support(ckmmc::Device::ckDEVICE_EJECT) &&
		g_BurnImageSettings.m_bEject)
	{
		CommandLine += _T(" -eject");
	}

	// Simulation.
	if (Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE) &&
		g_BurnImageSettings.m_bSimulate)
	{
		CommandLine += _T(" -dummy");
	}

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
	if (Device.support(ckmmc::Device::ckDEVICE_BUP))
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			lstrcat(szDriverOpts,_T("burnfree,"));
		else
			lstrcat(szDriverOpts,_T("noburnfree,"));
	}

	// Audio master.
	if (Device.support(ckmmc::Device::ckDEVICE_AUDIO_MASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			lstrcat(szDriverOpts,_T("audiomaster,"));
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (Device.support(ckmmc::Device::ckDEVICE_FORCE_SPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			lstrcat(szDriverOpts,_T("noforcespeed,"));
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (Device.support(ckmmc::Device::ckDEVICE_VARIREC))
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

	// Swap audio byte order. // FIXME: Should probably check for support before selecting.
	if (g_BurnAdvancedSettings.m_bSwab)
		CommandLine += _T(" -swab");

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

		ckcore::Path FilePath(szDataTrack);
		if (!ckcore::string::astrcmpi(FilePath.ext_name().c_str(),ckT("cue")))
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

bool CCore::BurnImage(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					  const TCHAR *szFileName,bool bCloneMode)
{
	return BurnImage(Device,pProgress,szFileName,false,bCloneMode);
}

/*
	CCore::BurnImageEx
	------------------
	Same as the function above except that it will not return untill the process
	has ended.
*/
bool CCore::BurnImageEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						const TCHAR *szFileName,bool bCloneMode)
{
	return BurnImage(Device,pProgress,szFileName,true,bCloneMode);
}

bool CCore::BurnTracks(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					   const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
					   const TCHAR *szAudioText,int iDataMode)
{
	return BurnTracks(Device,pProgress,szDataTrack,AudioTracks,szAudioText,
					  iDataMode,MODE_BURNIMAGE,true);
}

/*
	CCore::BurnTracksEx
	-------------------
	Works like CCore::BurnTracks but it does not end the progress when done. It
	allows for more operations to be performed in the same progress window. It
	also has extra return values.
*/
eBurnResult CCore::BurnTracksEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						const TCHAR *szDataTrack,std::vector<TCHAR *> &AudioTracks,
						const TCHAR *szAudioText,int iDataMode)
{
	if (!BurnTracks(Device,pProgress,szDataTrack,AudioTracks,szAudioText,
					iDataMode,MODE_BURNIMAGEEX,true))
	{
		return BURNRESULT_INTERNALERROR;
	}

	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	return uiExitCode == 0 ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
	//return m_bOperationRes ? RESULT_OK : RESULT_EXTERNALERROR;
}

/*
	CCore::ReadDataTrack
	--------------------
	Reads a track from the CD and stores the raw binary content in the file
	named szFileName.
*/
bool CCore::ReadDataTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						  const TCHAR *szFileName,unsigned int uiTrackNumber,
						  unsigned long ulStartSector,unsigned long ulEndSector,
						  int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::ReadDataTrack"));
		g_pLogDlg->print_line(_T("  File = %s."),szFileName);
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  Start = %d, End = %d."),ulStartSector,ulEndSector);
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
	CommandLine += _T(CORE_READAPP);
	CommandLine += _T("\" dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

    // Sector range.
	TCHAR szBuffer[128];
	lsprintf(szBuffer,_T(" sectors=%d-%d"),ulStartSector,ulEndSector);
	CommandLine += szBuffer;

	// File name.
	CommandLine += _T(" f=\"");
	CommandLine += szFileName;
	CommandLine += _T("\"");

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ReadDataTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						  const TCHAR *szFileName,unsigned int uiTrackNumber,
						  unsigned long ulStartSector,unsigned long ulEndSector)
{
	return ReadDataTrack(Device,pProgress,szFileName,uiTrackNumber,
						 ulStartSector,ulEndSector,MODE_READDATATRACK,true);
}

eBurnResult CCore::ReadDataTrackEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						   const TCHAR *szFileName,unsigned int uiTrackNumber,
						   unsigned long ulStartSector,unsigned long ulEndSector)
{
	if (!ReadDataTrack(Device,pProgress,szFileName,uiTrackNumber,ulStartSector,
					   ulEndSector,MODE_READDATATRACKEX,true))
	{
		return BURNRESULT_INTERNALERROR;
	}

	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	return uiExitCode == 0 ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
	//return m_bOperationRes ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
}

/*
*/
bool CCore::ReadAudioTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						   const TCHAR *szFileName,unsigned int uiTrackNumber,
						   int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::ReadAudioTrack"));
		g_pLogDlg->print_line(_T("  File = %s."),szFileName);
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
	}

	// Initialize this object.
	Initialize(iMode,pProgress);

	// Remember what track we are working with.
	m_uiTotalSize = uiTrackNumber;

	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_AUDIOAPP);
	CommandLine += _T("\" -D ");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	// Miscellaneous.
	CommandLine += _T(" -I generic_scsi -x -B -O wav -g -H");

	// Track.
	TCHAR szBuffer[128];
	lsprintf(szBuffer,_T(" -t %d+%d"),uiTrackNumber,uiTrackNumber);
	CommandLine += szBuffer;

	// File name.
	CommandLine += _T(" \"");
	CommandLine += szFileName;
	CommandLine += _T("\"");

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ReadAudioTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						   const TCHAR *szFileName,unsigned int uiTrackNumber)
{
	return ReadAudioTrack(Device,pProgress,szFileName,uiTrackNumber,
						  MODE_READAUDIOTRACK,true);
}
eBurnResult CCore::ReadAudioTrackEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,const TCHAR *szFileName,
							unsigned int uiTrackNumber)
{
	if (!ReadAudioTrack(Device,pProgress,szFileName,uiTrackNumber,
						MODE_READAUDIOTRACKEX,true))
	{
		return BURNRESULT_INTERNALERROR;
	}

	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	return uiExitCode == 0 ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
	//return m_bOperationRes ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
}

/*
	CCore::ScanTrack
	----------------
	Scans the selected track for CRC and C2 errors.
*/
bool CCore::ScanTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					  unsigned int uiTrackNumber,unsigned long ulStartSector,
					  unsigned long ulEndSector,int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::ScanTrack"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  Start = %d, End = %d."),ulStartSector,ulEndSector);
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
	CommandLine += _T(CORE_READAPP);
	CommandLine += _T("\" -c2scan dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

    // Sector range.
	TCHAR szBuffer[128];
	lsprintf(szBuffer,_T(" sectors=%d-%d"),ulStartSector,ulEndSector);
	CommandLine += szBuffer;

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ScanTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
					  unsigned long ulStartSector,unsigned long ulEndSector)
{
	return ScanTrack(Device,pProgress,uiTrackNumber,ulStartSector,ulEndSector,
					 MODE_SCANTRACK,true);
}

eBurnResult CCore::ScanTrackEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,unsigned int uiTrackNumber,
					   unsigned long ulStartSector,unsigned long ulEndSector)
{
	if (!ScanTrack(Device,pProgress,uiTrackNumber,ulStartSector,ulEndSector,
				   MODE_SCANTRACKEX,true))
	{
		return BURNRESULT_INTERNALERROR;
	}

	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	return uiExitCode == 0 ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
	//return m_bOperationRes ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
}

/*
	CCore::CopyDisc
	---------------
	Performs an on-the-fly copy of a disc. This function is configured through the
	g_BurnImageSettings, g_BurnAdvancedSettings and g_ReadSettings (m_bIgnoreErr
	only) objects.
*/
bool CCore::CopyDisc(ckmmc::Device &SrcDevice,ckmmc::Device &DstDevice,
					 CAdvancedProgress *pProgress)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::CopyDisc"));
		g_pLogDlg->print_line(_T("  Source: [%d,%d,%d] %s"),SrcDevice.address().bus_,
							  SrcDevice.address().target_,SrcDevice.address().lun_,
							  SrcDevice.name());
		g_pLogDlg->print_line(_T("  Target: [%d,%d,%d] %s"),DstDevice.address().bus_,
							  DstDevice.address().target_,DstDevice.address().lun_,
							  DstDevice.name());
		g_pLogDlg->print_line(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Close = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Ignore read errors = %d."),
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

	CommandLine = CORE_READAPP;
	CommandLine += " -v dev=";

	// Source device.
	CommandLine += NDeviceUtil::GetDeviceAddrA(SrcDevice);

	// Speed.
	char szBuffer[64];
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

	// Write app related.
	CommandLine += CORE_WRITEAPP;
	CommandLine += " -v dev=";
	CommandLine += NDeviceUtil::GetDeviceAddrA(DstDevice);

	sprintf(szBuffer," gracetime=%d",g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		sprintf(szBuffer," fs=%dm",g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (DstDevice.support(ckmmc::Device::ckDEVICE_EJECT) &&
		g_BurnImageSettings.m_bEject)
	{
		CommandLine += " -eject";
	}

	// Simulation.
	if (DstDevice.support(ckmmc::Device::ckDEVICE_TEST_WRITE) &&
		g_BurnImageSettings.m_bSimulate)
	{
		CommandLine += " -dummy";
	}

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
	if (DstDevice.support(ckmmc::Device::ckDEVICE_BUP))
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			strcat(szDriverOpts,"burnfree,");
		else
			strcat(szDriverOpts,"noburnfree,");
	}

	// Audio master.
	if (DstDevice.support(ckmmc::Device::ckDEVICE_AUDIO_MASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			strcat(szDriverOpts,"audiomaster,");
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (DstDevice.support(ckmmc::Device::ckDEVICE_FORCE_SPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			strcat(szDriverOpts,"noforcespeed,");
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (DstDevice.support(ckmmc::Device::ckDEVICE_VARIREC))
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

	// Swap audio byte order. // FIXME: Should probably check for support before selecting.
	if (g_BurnAdvancedSettings.m_bSwab)
		CommandLine += " -swab";

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
	UnicodeToAnsi(szFolderPath,g_GlobalSettings.m_szCDRToolsPath,sizeof(szFolderPath));

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

	bool bResult = create(szBatchCmdLine);
	wait();
	ckcore::File::remove(szBatchPath);
	return bResult;
}

/*
	CCore::ReadDisc
	---------------
	Reads a disc to a disc image. This function is configured through the
	g_ReadSettings object.
*/
bool CCore::ReadDisc(ckmmc::Device &Device,CAdvancedProgress *pProgress,const TCHAR *szFileName,
					 int iMode,bool bWaitForProcess)
{
	m_bOperationRes = false;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::ReadDisc"));
		g_pLogDlg->print_line(_T("  File = %s."),szFileName);
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  Ignore read errors = %d, Clone = %d, Speed = %d."),
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
	CommandLine += _T(CORE_READAPP);
	CommandLine += _T("\" dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	// Ignore read errors.
	if (g_ReadSettings.m_bIgnoreErr)
		CommandLine += _T(" -noerror -nocorr");

	// Clone.
	if (g_ReadSettings.m_bClone)
		CommandLine += _T(" -clone");

	// Speed.
	if (g_ReadSettings.m_iReadSpeed != -1)
	{
		TCHAR szBuffer[64];
		lsprintf(szBuffer,_T(" speed=%d"),g_ReadSettings.m_iReadSpeed);
	}

	// File name.
	CommandLine += _T(" f=\"");
	CommandLine += szFileName;
	CommandLine += _T("\"");

	return SafeLaunch(CommandLine,bWaitForProcess);
}

bool CCore::ReadDisc(ckmmc::Device &Device,CAdvancedProgress *pProgress,const TCHAR *szFileName)
{
	return ReadDisc(Device,pProgress,szFileName,MODE_READDISC,false);
}

eBurnResult CCore::ReadDiscEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,const TCHAR *szFileName)
{
	if (!ReadDisc(Device,pProgress,szFileName,MODE_READDISCEX,true))
		return BURNRESULT_INTERNALERROR;

	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	return uiExitCode == 0 ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
	//return m_bOperationRes ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
}

DWORD WINAPI CCore::CreateCompImageThread(LPVOID lpThreadParameter)
{
	CCompImageParams *pParams = (CCompImageParams *)lpThreadParameter;

	g_Core2.CreateImage(pParams->m_Process,pParams->m_Files,
						pParams->m_Progress,false);

	return 0;
}

DWORD WINAPI CCore::NextCopyThread(LPVOID lpThreadParameter)
{
	CCore *pCore = (CCore *)lpThreadParameter;
	pCore->Reinitialize();

	pCore->m_uiProcessedSize = 0;
	pCore->m_pProgress->set_progress(0);

	TCHAR szBuffer[128];
	lsprintf(szBuffer,lngGetString(INFO_CREATECOPY),
		g_BurnImageSettings.m_lNumCopies - pCore->m_lNumCopies + 1,
		g_BurnImageSettings.m_lNumCopies);
	pCore->m_pProgress->notify(ckcore::Progress::ckINFORMATION,szBuffer);

	pCore->SafeLaunch(pCore->m_LastCmdLine,pCore->m_bLastWaitForProcess);
	return 0;
}

/*
	CCore::BurnCompilation
	----------------------
	Burns a compilation on the fly to a disc. This function is configured through
	the g_BurnImageSettings and g_ProjectSettings object.
*/
bool CCore::BurnCompilation(ckmmc::Device &Device,CAdvancedProgress *pProgress,
							ckcore::Progress &Progress,const ckfilesystem::FileSet &Files,
							std::vector<TCHAR *> &AudioTracks,const TCHAR *szAudioText,
							int iDataMode,unsigned __int64 uiDataBytes,int iMode)
{
	m_bOperationRes = true;

	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore::BurnCompilation"));
		g_pLogDlg->print_line(_T("  [%d,%d,%d] %s"),Device.address().bus_,
							  Device.address().target_,Device.address().lun_,
							  Device.name());
		g_pLogDlg->print_line(_T("  Eject = %d, Simulate = %d, BUP = %d, Pad tracks = %d, Close = %d, Overburn = %d, Swab = %d, Ignore size = %d, Immed = %d, Audio master = %d, Forcespeed = %d, VariRec (enabled) = %d, VariRec (value) = %d, Data bytes %I64d."),
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
			uiDataBytes);
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
		unsigned __int64 uiTrackSize = ckcore::File::size(AudioTracks[i]) / (1024 * 1024);
		m_TrackSize.push_back(uiTrackSize);

		m_uiTotalSize += uiTrackSize;
	}

	// Setup the command line.
	tstring CommandLine;
	CommandLine.reserve(MAX_PATH);

	// Write application part of the command line.
	CommandLine = _T("\"");
	CommandLine += g_GlobalSettings.m_szCDRToolsPath;
	CommandLine += _T(CORE_WRITEAPP);
	CommandLine += _T("\" -v dev=");
	CommandLine += NDeviceUtil::GetDeviceAddr(Device);

	TCHAR szBuffer[64];
	lsprintf(szBuffer,_T(" gracetime=%d"),g_GlobalSettings.m_iGraceTime);
	CommandLine += szBuffer;

	// FIFO.
	if (g_GlobalSettings.m_iFIFOSize != 4)
	{
		lsprintf(szBuffer,_T(" fs=%dm"),g_GlobalSettings.m_iFIFOSize);
		CommandLine += szBuffer;
	}

	// Eject.
	if (Device.support(ckmmc::Device::ckDEVICE_EJECT) &&
		g_BurnImageSettings.m_bEject)
	{
			CommandLine += _T(" -eject");
	}

	// Simulation.
	if (Device.support(ckmmc::Device::ckDEVICE_TEST_WRITE) &&
		g_BurnImageSettings.m_bSimulate)
	{
		CommandLine += _T(" -dummy");
	}

	// Write method.
	switch (g_BurnImageSettings.m_iWriteMethod)
	{
		case WRITEMETHOD_SAO:
			CommandLine += _T(" -sao");

			// The SAO method needs to know the size of the file system beforehand (specified in sectors).
			lsprintf(szBuffer,_T(" tsize=%I64ds"),uiDataBytes/2048);
			CommandLine += szBuffer;
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
	if (Device.support(ckmmc::Device::ckDEVICE_BUP))
	{
		bUseDriverOpts = true;

		if (g_BurnImageSettings.m_bBUP)
			lstrcat(szDriverOpts,_T("burnfree,"));
		else
			lstrcat(szDriverOpts,_T("noburnfree,"));
	}

	// Audio master.
	if (Device.support(ckmmc::Device::ckDEVICE_AUDIO_MASTER))
	{
		if (g_BurnAdvancedSettings.m_bAudioMaster)
		{
			lstrcat(szDriverOpts,_T("audiomaster,"));
			bUseDriverOpts = true;
		}
	}

	// Forcespeed.
	if (Device.support(ckmmc::Device::ckDEVICE_FORCE_SPEED))
	{
		if (!g_BurnAdvancedSettings.m_bForceSpeed)
		{
			lstrcat(szDriverOpts,_T("noforcespeed,"));
			bUseDriverOpts = true;
		}
	}

	// VariRec.
	if (Device.support(ckmmc::Device::ckDEVICE_VARIREC))
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

	// Swap audio byte order. // FIXME: Should probably check for support before selecting.
	if (g_BurnAdvancedSettings.m_bSwab)
		CommandLine += _T(" -swab");

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

	// Mode.
	switch (iDataMode)
	{
		case 0:		// Mode 1
			CommandLine += _T(" -data -");
			break;

		case 1:		// Mode 2 XA (multisession)
			CommandLine += _T(" -multi -");
			break;
	};

	// Audio tracks.
	if (AudioTracks.size() > 0)
		CommandLine += _T(" -audio");

	TCHAR szCygwinFileName[MAX_PATH + 16];

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
		CommandLine += szCygwinFileName;
		CommandLine += _T("\"");
	}

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

	// Create a separate thread for writing the file system to the process.
	CCompImageParams CompImageParams(*this,Progress,Files);

	unsigned long ulThreadID = 0;
	HANDLE hThread = ::CreateThread(NULL,0,CreateCompImageThread,&CompImageParams,0,&ulThreadID);
	::CloseHandle(hThread);

	//return Launch((TCHAR *)CommandLine.c_str(),true);
	return SafeLaunch(CommandLine,true);
}

bool CCore::BurnCompilation(ckmmc::Device &Device,CAdvancedProgress *pProgress,
							ckcore::Progress &Progress,const ckfilesystem::FileSet &Files,
							std::vector<TCHAR *> &AudioTracks,const TCHAR *szAudioText,
							int iMode,unsigned __int64 uiDataBytes)
{
	return BurnCompilation(Device,pProgress,Progress,Files,AudioTracks,
						   szAudioText,iMode,uiDataBytes,MODE_BURNIMAGE);
}

eBurnResult CCore::BurnCompilationEx(ckmmc::Device &Device,CAdvancedProgress *pProgress,
							 ckcore::Progress &Progress,const ckfilesystem::FileSet &Files,
							 std::vector<TCHAR *> &AudioTracks,const TCHAR *szAudioText,
							 int iMode,unsigned __int64 uiDataBytes)
{
	if (!BurnCompilation(Device,pProgress,Progress,Files,AudioTracks,szAudioText,
						 iMode,uiDataBytes,MODE_BURNIMAGEEX))
	{
		return BURNRESULT_INTERNALERROR;		
	}

	ckcore::tuint32 uiExitCode = 0;
	exit_code(uiExitCode);

	return uiExitCode == 0 ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
	//return m_bOperationRes ? BURNRESULT_OK : BURNRESULT_EXTERNALERROR;
}