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

#include "stdafx.h"
#include "ActionManager.h"
#include "Settings.h"
#include "Core.h"
#include "Core2.h"
#include "InfraRecorder.h"
#include "BurnImageDlg.h"
#include "CopyDiscDlg.h"
#include "CopyImageDlg.h"
#include "InfoDlg.h"
#include "TracksDlg.h"
#include "EraseDlg.h"
#include "FixateDlg.h"
#include "ProgressDlg.h"
#include "SimpleProgressDlg.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "ProjectManager.h"
#include "../../Common/FileManager.h"
#include "../../Core/ckFileSystem/DiscImageWriter.h"
#include "SCSI.h"
#include "LogDlg.h"

CActionManager g_ActionManager;

CActionManager::CActionManager()
{
}

CActionManager::~CActionManager()
{
}

DWORD WINAPI CActionManager::BurnCompilationThread(LPVOID lpThreadParameter)
{
	int iProjectType = g_ProjectManager.GetProjectType();
	int iResult = 0;
	TCHAR szFileName[MAX_PATH];
	TCHAR szPathList[MAX_PATH];

	// Make sure that the disc will not be ejected before beeing verified.
	bool bEject = g_BurnImageSettings.m_bEject;
	g_BurnImageSettings.m_bEject = false;

	if (iProjectType == PROJECTTYPE_DATA ||
		iProjectType == PROJECTTYPE_MIXED ||
		iProjectType == PROJECTTYPE_DVDVIDEO)
	{
		fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irPathList"),szPathList);

		switch (g_ProjectManager.GetProjectType())
		{
			case PROJECTTYPE_DATA:
			case PROJECTTYPE_DVDVIDEO:
				g_TreeManager.SavePathList(szPathList,g_TreeManager.GetRootNode());
				break;

			case PROJECTTYPE_MIXED:
				g_TreeManager.SavePathList(szPathList,g_ProjectManager.GetMixDataRootNode(),
					lstrlen(g_ProjectManager.GetMixDataRootNode()->pItemData->GetFileName()) + 1);
				break;

			default:
				return 0;
		};

		// Create a temporary disc image if not burning on the fly.
		if (!g_BurnImageSettings.m_bOnFly)
		{
			fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irImage"),szFileName);

			// Set the status information.
			g_ProgressDlg.SetWindowText(lngGetString(STITLE_CREATEIMAGE));
			g_ProgressDlg.SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

			iResult = g_Core.CreateImageEx(szFileName,szPathList,&g_ProgressDlg);
			fs_deletefile(szPathList);

			switch (iResult)
			{
				case RESULT_INTERNALERROR:
					g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
					g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
					g_ProgressDlg.NotifyComplteted();

					lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
					fs_deletefile(szFileName);
					return 0;

				case RESULT_EXTERNALERROR:
					fs_deletefile(szFileName);
					return 0;
			};
		}
	}

	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_BurnImageSettings.m_iRecorder);
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(g_BurnImageSettings.m_iRecorder);
	tDeviceInfoEx *pDeviceInfoEx = g_DeviceManager.GetDeviceInfoEx(g_BurnImageSettings.m_iRecorder);

	// Set the status information.
	TCHAR szDeviceName[128];
	g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

	g_ProgressDlg.SetWindowText(lngGetString(STITLE_BURNCOMPILATION));
	g_ProgressDlg.SetDevice(szDeviceName);

	std::vector<TCHAR *> AudioTracks;
	std::vector<TCHAR *> TempTracks;
	TCHAR szAudioText[MAX_PATH];
	TCHAR *pAudioText = NULL;

	// Decode any encoded tracks in audio and mixed-mode projects.
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_DECODETRACKS));
	switch (iProjectType)
	{
		case PROJECTTYPE_MIXED:
		case PROJECTTYPE_AUDIO:
			g_ProjectManager.GetAudioTracks(AudioTracks);

			// Decode any audio tracks that might be encoded.
			if (!g_ProjectManager.DecodeAudioTracks(AudioTracks,TempTracks,&g_ProgressDlg))
			{
				g_ProgressDlg.NotifyComplteted();

				// Remove any temporary tracks.
				for (unsigned int i = 0; i < TempTracks.size(); i++)
				{
					if (fs_fileexists(TempTracks[i]))
						fs_deletefile(TempTracks[i]);

					std::vector <TCHAR *>::iterator itObject = TempTracks.begin() + i;
					delete [] *itObject;
				}

				TempTracks.clear();
				return 0;
			}
			break;
	}

	// Start the burn process.
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));
	switch (iProjectType)
	{
		case PROJECTTYPE_DATA:
		case PROJECTTYPE_DVDVIDEO:
			if (g_BurnImageSettings.m_bOnFly)
			{
				// Try to estimate the file system size before burning the compilation.
				unsigned __int64 uiDataSize = 0;
				g_ProgressDlg.SetStatus(lngGetString(PROGRESS_ESTIMAGESIZE));

				if (!g_Core.EstimateImageSize(szPathList,&g_ProgressDlg,uiDataSize))
				{
					g_ProgressDlg.AddLogEntry(CProgressDlg::LT_ERROR,lngGetString(ERROR_ESTIMAGESIZE));

					fs_deletefile(szPathList);
					iResult = RESULT_EXTERNALERROR;
				}
				else
				{
					// Reset the status.
					g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

					if (g_BurnImageSettings.m_bVerify)
					{
						iResult = (int)g_Core.BurnCompilationEx(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szPathList,
							AudioTracks,NULL,g_ProjectSettings.m_iISOFormat,uiDataSize * 2048);
					}
					else
					{
						iResult = (int)g_Core.BurnCompilation(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szPathList,
							AudioTracks,NULL,g_ProjectSettings.m_iISOFormat,uiDataSize * 2048);
					}

					fs_deletefile(szPathList);
				}
			}
			else
			{
				if (g_BurnImageSettings.m_bVerify)
				{
					iResult = (int)g_Core.BurnTracksEx(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szFileName,
						AudioTracks,NULL,g_ProjectSettings.m_iISOFormat);
				}
				else
				{
					iResult = (int)g_Core.BurnTracks(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szFileName,
						AudioTracks,NULL,g_ProjectSettings.m_iISOFormat);
				}

				fs_deletefile(szFileName);
			}
			break;

		case PROJECTTYPE_MIXED:
			// Save CD-Text information.
			if (AudioTracks.size() > 0)
			{
				// Check if any audio information has been edited.
				if (g_TreeManager.HasExtraAudioData(g_ProjectManager.GetMixAudioRootNode()))
				{
					fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irText"),szAudioText);

					if (g_ProjectManager.SaveCDText(szAudioText))
						pAudioText = szAudioText;
					else
						lngMessageBox(HWND_DESKTOP,FAILURE_CREATECDTEXT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
				}
			}

			if (g_BurnImageSettings.m_bOnFly)
			{
				// Try to estimate the file system size before burning the compilation.
				unsigned __int64 uiDataSize = 0;
				g_ProgressDlg.SetStatus(lngGetString(PROGRESS_ESTIMAGESIZE));

				if (!g_Core.EstimateImageSize(szPathList,&g_ProgressDlg,uiDataSize))
				{
					g_ProgressDlg.AddLogEntry(CProgressDlg::LT_ERROR,lngGetString(ERROR_ESTIMAGESIZE));

					fs_deletefile(szPathList);
					iResult = RESULT_EXTERNALERROR;
				}
				else
				{
					// Reset the status.
					g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

					if (g_BurnImageSettings.m_bVerify)
					{
						iResult = (int)g_Core.BurnCompilationEx(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szPathList,
							AudioTracks,pAudioText,g_ProjectSettings.m_iISOFormat,uiDataSize * 2048);
					}
					else
					{
						iResult = (int)g_Core.BurnCompilation(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szPathList,
							AudioTracks,pAudioText,g_ProjectSettings.m_iISOFormat,uiDataSize * 2048);
					}

					fs_deletefile(szPathList);
				}
			}
			else
			{
				if (g_BurnImageSettings.m_bVerify)
				{
					iResult = (int)g_Core.BurnTracksEx(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szFileName,
						AudioTracks,pAudioText,g_ProjectSettings.m_iISOFormat);
				}
				else
				{
					iResult = (int)g_Core.BurnTracks(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szFileName,
						AudioTracks,pAudioText,g_ProjectSettings.m_iISOFormat);
				}

				fs_deletefile(szFileName);
			}

			// Remove any temporary tracks.
			for (unsigned int i = 0; i < TempTracks.size(); i++)
			{
				fs_deletefile(TempTracks[i]);

				std::vector <TCHAR *>::iterator itObject = TempTracks.begin() + i;
				delete [] *itObject;
			}

			TempTracks.clear();

			// Remove the CD-Text file.
			if (pAudioText != NULL)
				fs_deletefile(pAudioText);
			break;

		case PROJECTTYPE_AUDIO:
			// Save CD-Text information.
			if (AudioTracks.size() > 0)
			{
				// Check if any audio information has been edited.
				if (g_TreeManager.HasExtraAudioData(g_TreeManager.GetRootNode()))
				{
					fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irText"),szAudioText);

					if (g_ProjectManager.SaveCDText(szAudioText))
						pAudioText = szAudioText;
					else
						lngMessageBox(HWND_DESKTOP,FAILURE_CREATECDTEXT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
				}
			}

			iResult = (int)g_Core.BurnTracks(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,NULL,
				AudioTracks,pAudioText,0);

			// Remove any temporary tracks.
			for (unsigned int i = 0; i < TempTracks.size(); i++)
			{
				fs_deletefile(TempTracks[i]);

				std::vector <TCHAR *>::iterator itObject = TempTracks.begin() + i;
				delete [] *itObject;
			}

			TempTracks.clear();

			// Remove the CD-Text file.
			if (pAudioText != NULL)
				fs_deletefile(pAudioText);
			return 0;
	};

	// Handle the result.
	if (!iResult)
	{
		g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
		g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
		g_ProgressDlg.NotifyComplteted();

		lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		return 0;
	}
	else if (iResult == RESULT_OK)
	{
		// If the recording was successfull, verify the disc.
		if (g_BurnImageSettings.m_bVerify)
		{
			// We need to reload the drive media.
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_RELOADMEDIA));
			//g_Core.EjectDisc(pDeviceInfo,true);
			//g_Core.LoadDisc(pDeviceInfo,true);
			CCore2Device Device;
			Device.Open(&pDeviceInfo->Address);

			g_Core2.StartStopUnit(&Device,CCore2::LOADMEDIA_EJECT,false);
			if (!g_Core2.StartStopUnit(&Device,CCore2::LOADMEDIA_LOAD,false))
				lngMessageBox(g_ProgressDlg,INFO_RELOAD,GENERAL_INFORMATION,MB_OK | MB_ICONINFORMATION);

			// Set the device information.
			TCHAR szDeviceName[128];
			g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

			g_ProgressDlg.SetDevice(szDeviceName);
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

			// Get the device drive letter.
			TCHAR szDriveLetter[3];
			szDriveLetter[0] = pDeviceInfo->Address.m_cDriveLetter;
			szDriveLetter[1] = ':';
			szDriveLetter[2] = '\0';

			// Validate the project files.
			g_ProjectManager.VerifyCompilation(&g_ProgressDlg,szDriveLetter);

			// We're done.
			g_ProgressDlg.SetProgress(100);
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_DONE));
			g_ProgressDlg.NotifyComplteted();
		}

		// Eject the disc if requested.
		if (bEject)
			g_Core.EjectDisc(pDeviceInfo,false);
	}

	return 0;
}

DWORD WINAPI CActionManager::CreateImageThread(LPVOID lpThreadParameter)
{
	TCHAR *szFileName = (TCHAR *)lpThreadParameter;

	TCHAR szPathList[MAX_PATH];
	fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irPathList"),szPathList);

	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_DATA:
		case PROJECTTYPE_DVDVIDEO:
			g_TreeManager.SavePathList(szPathList,g_TreeManager.GetRootNode());
			break;

		case PROJECTTYPE_MIXED:
			g_TreeManager.SavePathList(szPathList,g_ProjectManager.GetMixDataRootNode(),lstrlen(g_ProjectManager.GetMixDataRootNode()->pItemData->GetFileName()) + 1);
			break;

		default:
			delete [] szFileName;
			return 0;
	};

	// Set the status information.
	g_ProgressDlg.SetWindowText(lngGetString(STITLE_CREATEIMAGE));
	g_ProgressDlg.SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

	int iResult = g_Core.CreateImage(szFileName,szPathList,&g_ProgressDlg);
	fs_deletefile(szPathList);

	switch (iResult)
	{
		case RESULT_INTERNALERROR:
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.NotifyComplteted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
			fs_deletefile(szFileName);

			delete [] szFileName;
			return 0;

		case RESULT_EXTERNALERROR:
			fs_deletefile(szFileName);

			delete [] szFileName;
			return 0;
	};

	delete [] szFileName;
	return 0;
}

/*
	FIXME: Clean up when done.
	Yeah, this is a dirty test method for the new file system routines.
*/
DWORD WINAPI CActionManager::CreateImageThread42(LPVOID lpThreadParameter)
{
	TCHAR *szFileName = (TCHAR *)lpThreadParameter;

	bool bDvdVideo = true;

	ckFileSystem::CFileComparator FileComp(bDvdVideo);
	ckFileSystem::CFileSet Files(FileComp);
	
	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_DATA:
		case PROJECTTYPE_DVDVIDEO:
			g_TreeManager.GetPathList(Files,g_TreeManager.GetRootNode());
			break;

		case PROJECTTYPE_MIXED:
			g_TreeManager.GetPathList(Files,g_ProjectManager.GetMixDataRootNode(),
				lstrlen(g_ProjectManager.GetMixDataRootNode()->pItemData->GetFileName()) + 1);
			break;

		default:
			delete [] szFileName;
			return 0;
	};

	// Set the status information.
	g_ProgressDlg.SetWindowText(lngGetString(STITLE_CREATEIMAGE));
	g_ProgressDlg.SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

	ckFileSystem::CDiscImageWriter DiscImageWriter(&g_LogDlg,ckFileSystem::CDiscImageWriter::FS_ISO9660_UDF_JOLIET);
	DiscImageWriter.SetVolumeLabel(_T("080101_2159"));
	DiscImageWriter.SetTextFields(_T("System."),_T("Volume set."),_T("Publisher."),_T("Preparer."));
	DiscImageWriter.SetFileFields(_T("Copyright."),_T("Abstract."),_T("Bibliographic."));

	//DiscImageWriter.SetInterchangeLevel(ckFileSystem::CIso9660::LEVEL_3);
	DiscImageWriter.SetInterchangeLevel(ckFileSystem::CIso9660::LEVEL_1);

	/*DiscImageWriter.AddBootImageNoEmu(_T("C:\\Users\\Christian Kindahl\\Desktop\\isolinux.bin"),true,0,4);
	DiscImageWriter.AddBootImageNoEmu(_T("C:\\Users\\Christian Kindahl\\Desktop\\isolinux.bin"),false,0x42,0x42);
	DiscImageWriter.AddBootImageNoEmu(_T("C:\\Users\\Christian Kindahl\\Desktop\\isolinux.bin"),true,0x84,0x42);*/

	/*g_LogDlg.AddLine(_T("Dumping file set:"));

	ckFileSystem::CFileSet::const_iterator itFile;
	for (itFile = Files.begin(); itFile != Files.end(); itFile++)
		g_LogDlg.AddLine(_T("  %s"),itFile->m_InternalPath.c_str());*/

	g_ProgressDlg.AddLogEntry(CProgressDlg::LT_INFORMATION,lngGetString(PROGRESS_BEGINDISCIMAGE));

	int iResult = DiscImageWriter.Create(szFileName,Files,g_ProgressDlg);

	g_ProgressDlg.SetProgress(100);
	g_ProgressDlg.NotifyComplteted();

	switch (iResult)
	{
		case RESULT_OK:
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_DONE));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_INFORMATION,lngGetString(SUCCESS_CREATEIMAGE));
			break;

		case RESULT_FAIL:
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_FAILED));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_ERROR,lngGetString(FAILURE_CREATEIMAGE));
			break;
	}

	delete [] szFileName;
	return 0;
}

INT_PTR CActionManager::BurnCompilation(HWND hWndParent,bool bAppMode)
{
	bool bAudioProject = g_ProjectManager.GetProjectType() == PROJECTTYPE_AUDIO;
	// Display the burn image dialog.
	CBurnImageDlg BurnImageDlg(lngGetString(MISC_BURNCOMPILATION),false,
		!bAudioProject,!bAudioProject);
	INT_PTR iResult = BurnImageDlg.DoModal();

	if (iResult == IDOK)
	{
		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_ProgressDlg.SetAppMode(bAppMode);
		if (!g_ProgressDlg.IsWindow())
			g_ProgressDlg.Create(hWndParent);

		g_ProgressDlg.ShowWindow(true);
		g_ProgressDlg.Reset();
		g_ProgressDlg.AttachConsolePipe(&g_Core);
		g_ProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// Create the new thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,BurnCompilationThread,this,0,&ulThreadID);
		::CloseHandle(hThread);

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

INT_PTR CActionManager::CreateImage(HWND hWndParent,bool bAppMode)
{
	WTL::CFileDialog FileDialog(false,_T("iso"),_T("Untitled"),OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("Disc Images (*.iso)\0*.iso\0\0"),hWndParent);
	INT_PTR iResult = FileDialog.DoModal();

	if (iResult == IDOK)
	{
		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_ProgressDlg.SetAppMode(bAppMode);
		if (!g_ProgressDlg.IsWindow())
			g_ProgressDlg.Create(hWndParent);

		g_ProgressDlg.ShowWindow(true);
		g_ProgressDlg.Reset();
		g_ProgressDlg.AttachConsolePipe(&g_Core);
		g_ProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// WARNING: Heap allocation, this memory memory must be freed when the thread exit.
		TCHAR *szFileName = new TCHAR[MAX_PATH];
		lstrcpy(szFileName,FileDialog.m_szFileName);

		// Create the new thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,CreateImageThread,szFileName,0,&ulThreadID);
		::CloseHandle(hThread);

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

INT_PTR CActionManager::BurnImage(HWND hWndParent,bool bAppMode)
{
	WTL::CFileDialog FileDialog(true,0,0,OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("Disc Images (*.iso, *.cue, *.img)\0*.iso;*.cue;*.img\0Raw Images (*.bin, *.raw)\0*.bin;*.raw\0All Files (*.*)\0*.*\0\0"),hWndParent);
	INT_PTR iResult = FileDialog.DoModal();

	if (iResult == IDOK)
		iResult = BurnImageEx(hWndParent,bAppMode,FileDialog.m_szFileName);

	return iResult;
}

INT_PTR CActionManager::BurnImageEx(HWND hWndParent,bool bAppMode,const TCHAR *szFilePath)
{
	// Dialog title.
	TCHAR szFileName[MAX_PATH];
	lstrcpy(szFileName,szFilePath);
	ExtractFileName(szFileName);

	TCHAR szTitle[MAX_PATH];
	lstrcpy(szTitle,lngGetString(MISC_BURN));
	lstrcat(szTitle,szFileName);

	// Look for a TOC file.
	TCHAR szTOCFilePath[MAX_PATH];
	lstrcpy(szTOCFilePath,szFilePath);
	lstrcat(szTOCFilePath,_T(".toc"));

	bool bImageHasTOC = false;
	if (fs_fileexists(szTOCFilePath))
		bImageHasTOC = true;

	// Display the burn image dialog.
	CBurnImageDlg BurnImageDlg(szTitle,bImageHasTOC,false,false);
	INT_PTR iResult = BurnImageDlg.DoModal();

	if (iResult == IDOK)
	{
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_BurnImageSettings.m_iRecorder);
		tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(g_BurnImageSettings.m_iRecorder);
		tDeviceInfoEx *pDeviceInfoEx = g_DeviceManager.GetDeviceInfoEx(g_BurnImageSettings.m_iRecorder);

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_ProgressDlg.SetAppMode(bAppMode);

		if (!g_ProgressDlg.IsWindow())
			g_ProgressDlg.Create(hWndParent);

		g_ProgressDlg.ShowWindow(true);
		g_ProgressDlg.SetWindowText(lngGetString(STITLE_BURNIMAGE));
		g_ProgressDlg.Reset();
		g_ProgressDlg.AttachConsolePipe(&g_Core);
		g_ProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		TCHAR szDeviceName[128];
		g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

		g_ProgressDlg.SetDevice(szDeviceName);
		g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

		// Begin burning the image.
		if (!g_Core.BurnImage(pDeviceInfo,pDeviceCap,pDeviceInfoEx,&g_ProgressDlg,szFilePath,bImageHasTOC))
		{
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.NotifyComplteted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		}

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

DWORD WINAPI CActionManager::CopyDiscOnFlyThread(LPVOID lpThreadParameter)
{
	// Get device information.
	tDeviceInfo *pSourceDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iSource);

	tDeviceInfo *pTargetDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iTarget);
	tDeviceCap *pTargetDeviceCap = g_DeviceManager.GetDeviceCap(g_CopyDiscSettings.m_iTarget);
	tDeviceInfoEx *pTargetDeviceInfoEx = g_DeviceManager.GetDeviceInfoEx(g_CopyDiscSettings.m_iTarget);

	// Set the status information.
	TCHAR szDeviceName[128];
	g_DeviceManager.GetDeviceName(pTargetDeviceInfo,szDeviceName);

	g_ProgressDlg.SetWindowText(lngGetString(STITLE_COPYDISC));
	g_ProgressDlg.SetDevice(szDeviceName);
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

	// Start the operation.
	if (!g_Core.CopyDisc(pSourceDeviceInfo,pTargetDeviceInfo,pTargetDeviceCap,pTargetDeviceInfoEx,&g_ProgressDlg))
	{
		g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
		g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
		g_ProgressDlg.NotifyComplteted();

		lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		return 0;
	}

	return 0;
}

DWORD WINAPI CActionManager::CopyDiscThread(LPVOID lpThreadParameter)
{
	TCHAR szFileName[MAX_PATH];
	fs_createtempfilepath(g_GlobalSettings.m_szTempPath,_T("irImage"),szFileName);

	// Set the status information.
	g_ProgressDlg.SetWindowText(lngGetString(STITLE_CREATEIMAGE));
	g_ProgressDlg.SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

	// Get device information.
	tDeviceInfo *pSourceDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iSource);

	// Override the read sub-channel data setting.
	g_ReadSettings.m_bClone = g_CopyDiscSettings.m_bClone;

	int iResult = g_Core.ReadDiscEx(pSourceDeviceInfo,&g_ProgressDlg,szFileName);

	switch (iResult)
	{
		case RESULT_INTERNALERROR:
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.NotifyComplteted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);

			// Remove any temporary files.
			fs_deletefile(szFileName);
			if (g_CopyDiscSettings.m_bClone)
			{
				lstrcat(szFileName,_T(".toc"));
				fs_deletefile(szFileName);
			}
			return 0;

		case RESULT_EXTERNALERROR:
			// Remove any temporary files.
			fs_deletefile(szFileName);
			if (g_CopyDiscSettings.m_bClone)
			{
				lstrcat(szFileName,_T(".toc"));
				fs_deletefile(szFileName);
			}
			return 0;
	}

	// Ask the user to switch discs if the target is the same as the source.
	if (g_CopyDiscSettings.m_iSource == g_CopyDiscSettings.m_iTarget)
	{
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iSource);

		// Open and eject the new device.
		CCore2Device Device;
		if (!Device.Open(&pDeviceInfo->Address))
		{
			g_ProgressDlg.SetStatus(lngGetString(ERROR_OPENDEVICE));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(ERROR_OPENDEVICE));
			g_ProgressDlg.NotifyComplteted();
			return 0;
		}

		g_Core2.StartStopUnit(&Device,CCore2::LOADMEDIA_EJECT,true);
		Device.Close();

		
		if (lngMessageBox(g_ProgressDlg,INFO_INSERTBLANK,GENERAL_INFORMATION,
			MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
		{
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.NotifyComplteted();

			// Remove any temporary files.
			fs_deletefile(szFileName);
			if (g_CopyDiscSettings.m_bClone)
			{
				lstrcat(szFileName,_T(".toc"));
				fs_deletefile(szFileName);
			}

			return 0;
		}
	}

	// Get device information.
	tDeviceInfo *pTargetDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iTarget);
	tDeviceCap *pTargetDeviceCap = g_DeviceManager.GetDeviceCap(g_CopyDiscSettings.m_iTarget);
	tDeviceInfoEx *pTargetDeviceInfoEx = g_DeviceManager.GetDeviceInfoEx(g_CopyDiscSettings.m_iTarget);

	// Set the status information.
	TCHAR szDeviceName[128];
	g_DeviceManager.GetDeviceName(pTargetDeviceInfo,szDeviceName);

	g_ProgressDlg.SetWindowText(lngGetString(STITLE_BURNIMAGE));
	g_ProgressDlg.SetDevice(szDeviceName);
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

	if (!g_Core.BurnImageEx(pTargetDeviceInfo,pTargetDeviceCap,pTargetDeviceInfoEx,&g_ProgressDlg,szFileName,
		g_CopyDiscSettings.m_bClone))
	{
		g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
		g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
		g_ProgressDlg.NotifyComplteted();

		lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	}

	// Remove any temporary files.
	fs_deletefile(szFileName);
	if (g_CopyDiscSettings.m_bClone)
	{
		lstrcat(szFileName,_T(".toc"));
		fs_deletefile(szFileName);
	}

	return 0;
}

DWORD WINAPI CActionManager::EraseThread(LPVOID lpThreadParameter)
{
	// Get device information.
	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_EraseSettings.m_iRecorder);

	CCore2Device Device;
	if (Device.Open(&pDeviceInfo->Address))
	{
		if (!g_Core2.EraseDisc(&Device,&g_ProgressDlg,g_EraseSettings.m_iMode,
			g_EraseSettings.m_bForce,g_EraseSettings.m_bEject,g_EraseSettings.m_bSimulate,
			g_EraseSettings.m_uiSpeed))
		{
			//MessageBox(HWND_DESKTOP,_T("An error occured while erasing the disc."),_T("Error"),MB_OK | MB_ICONERROR);
		}

		Device.Close();
	}
	else
	{
		lngMessageBox(HWND_DESKTOP,ERROR_OPENDEVICE,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	}

	g_ProgressDlg.SetProgress(100);
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_DONE));
	g_ProgressDlg.NotifyComplteted();

	return true;
}

INT_PTR CActionManager::CopyDisc(HWND hWndParent,bool bAppMode)
{
	// Display the information dialog.
	if (g_GlobalSettings.m_bCopyWarning)
	{
		CInfoDlg InfoDlg(&g_GlobalSettings.m_bCopyWarning,lngGetString(INFO_COPYDISC));
		if (InfoDlg.DoModal() == IDCANCEL)
			return IDCANCEL;
	}

	// Display the burn image dialog.
	CCopyDiscDlg CopyDiscDlg;
	INT_PTR iResult = CopyDiscDlg.DoModal();

	if (iResult == IDOK)
	{
		if (g_CopyDiscSettings.m_iSource == g_CopyDiscSettings.m_iTarget)
		{
			tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iSource);

			// Open and eject the new device.
			CCore2Device Device;
			if (!Device.Open(&pDeviceInfo->Address))
				return 0;

			g_Core2.StartStopUnit(&Device,CCore2::LOADMEDIA_EJECT,true);
			Device.Close();

			if (lngMessageBox(hWndParent,INFO_INSERTSOURCE,GENERAL_INFORMATION,
				MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
			{
				return IDCANCEL;
			}
		}

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_ProgressDlg.SetAppMode(bAppMode);
		if (!g_ProgressDlg.IsWindow())
			g_ProgressDlg.Create(hWndParent);

		g_ProgressDlg.ShowWindow(true);
		g_ProgressDlg.Reset();
		g_ProgressDlg.AttachConsolePipe(&g_Core);
		g_ProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// Create the new thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,g_CopyDiscSettings.m_bOnFly ? CopyDiscOnFlyThread : CopyDiscThread,NULL,0,&ulThreadID);
		::CloseHandle(hThread);

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

/*
	CActionManager::CopyImage
	-------------------------
	Performs the copy disc to a disc image action.
*/
INT_PTR CActionManager::CopyImage(HWND hWndParent,bool bAppMode)
{
	// Display the copy image dialog.
	CCopyImageDlg CopyImageDlg;
	INT_PTR iResult = CopyImageDlg.DoModal();

	if (iResult == IDOK)
	{
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_CopyDiscSettings.m_iSource);

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_ProgressDlg.SetAppMode(bAppMode);
		if (!g_ProgressDlg.IsWindow())
			g_ProgressDlg.Create(hWndParent);

		g_ProgressDlg.ShowWindow(true);
		g_ProgressDlg.SetWindowText(lngGetString(STITLE_CREATEIMAGE));
		g_ProgressDlg.Reset();
		g_ProgressDlg.AttachConsolePipe(&g_Core);
		g_ProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		TCHAR szDeviceName[128];
		g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

		g_ProgressDlg.SetDevice(szDeviceName);
		g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

		// Begin reading the disc.
		if (!g_Core.ReadDisc(pDeviceInfo,&g_ProgressDlg,CopyImageDlg.GetFileName()))
		{
			g_ProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.AddLogEntry(CProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
			g_ProgressDlg.NotifyComplteted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		}

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

INT_PTR CActionManager::ManageTracks(bool bAppMode)
{
	CTracksDlg TracksDlg(bAppMode);
	return TracksDlg.DoModal();
}

INT_PTR CActionManager::Erase(HWND hWndParent,bool bAppMode)
{
	CEraseDlg EraseDlg(bAppMode);
	INT_PTR iResult = EraseDlg.DoModal();

	if (iResult == IDOK)
	{
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_EraseSettings.m_iRecorder);
		tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(g_EraseSettings.m_iRecorder);

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_ProgressDlg.SetAppMode(bAppMode);

		if (!g_ProgressDlg.IsWindow())
			g_ProgressDlg.Create(hWndParent);

		g_ProgressDlg.ShowWindow(true);
		g_ProgressDlg.SetWindowText(lngGetString(STITLE_ERASE));
		g_ProgressDlg.Reset();
		g_ProgressDlg.AttachConsolePipe(&g_Core);
		g_ProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		TCHAR szDeviceName[128];
		g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

		g_ProgressDlg.SetDevice(szDeviceName);
		g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

		// Create the new thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,EraseThread,NULL,0,&ulThreadID);
		::CloseHandle(hThread);

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

INT_PTR CActionManager::Fixate(HWND hWndParent,bool bAppMode)
{
	CFixateDlg FixateDlg(bAppMode);
	INT_PTR iResult = FixateDlg.DoModal();

	if (iResult == IDOK)
	{
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_FixateSettings.m_iRecorder);
		tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(g_FixateSettings.m_iRecorder);

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_SimpleProgressDlg.SetAppMode(bAppMode);

		if (!g_SimpleProgressDlg.IsWindow())
			g_SimpleProgressDlg.Create(hWndParent);

		g_SimpleProgressDlg.ShowWindow(true);
		g_SimpleProgressDlg.SetWindowText(lngGetString(STITLE_FIXATE));
		g_SimpleProgressDlg.Reset();
		g_SimpleProgressDlg.AttachConsolePipe(&g_Core);
		g_SimpleProgressDlg.AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		TCHAR szDeviceName[128];
		g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

		g_SimpleProgressDlg.SetDevice(szDeviceName);
		g_SimpleProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

		// Begin erasing the disc.
		if (!g_Core.FixateDisc(pDeviceInfo,pDeviceCap,&g_SimpleProgressDlg,
			g_FixateSettings.m_bEject,
			g_FixateSettings.m_bSimulate))
		{
			g_SimpleProgressDlg.SetStatus(lngGetString(PROGRESS_CANCELED));
			g_SimpleProgressDlg.AddLogEntry(CSimpleProgressDlg::LT_WARNING,lngGetString(PROGRESS_CANCELED));
			g_SimpleProgressDlg.NotifyComplteted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		}

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}
	}

	return iResult;
}

// For testing purposes only.
/*int CActionManager::VerifyCompilation(HWND hWndParent)
{
	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_BurnImageSettings.m_iRecorder);
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(g_BurnImageSettings.m_iRecorder);
	tDeviceInfoEx *pDeviceInfoEx = g_DeviceManager.GetDeviceInfoEx(g_BurnImageSettings.m_iRecorder);

	// Disable the parent window.
	if (hWndParent != NULL)
		::EnableWindow(hWndParent,false);

	// Create and display the progress dialog.
	g_ProgressDlg.SetAppMode(false);

	if (!g_ProgressDlg.IsWindow())
		g_ProgressDlg.Create(hWndParent);

	g_ProgressDlg.ShowWindow(true);
	g_ProgressDlg.SetWindowText(lngGetString(STITLE_BURNIMAGE));
	g_ProgressDlg.Reset();
	g_ProgressDlg.AttachConsolePipe(&g_Core);
	g_ProgressDlg.AttachHost(hWndParent);
	ProcessMessages();

	// Set the device information.
	TCHAR szDeviceName[128];
	g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

	g_ProgressDlg.SetDevice(szDeviceName);
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_INIT));

	// Get the device drive letter.
	TCHAR szDriveLetter[3];
	szDriveLetter[1] = ':';
	szDriveLetter[2] = '\0';

	bool bFoundDriveLetter = true;
	if (!SCSIGetDriveLetter(2,1,0,szDriveLetter[0]))
	{
		// This is not water proof. External USB and FireWire devices will fail the above
		// function call because USB and FireWire devices can't return an address on the
		// requested form since the USB and FireWire bus can contain multiple devices.
		// In a second attempt to locate the drive a search is performed by the device name.
		// If two identical devices are connected (same revision) to the system there will
		// be a conflict. Maybe I should solve this by using the ASPI driver?
		if (!SCSIGetDriveLetter(pDeviceInfo->szVendor,pDeviceInfo->szIdentification,
			pDeviceInfo->szRevision,szDriveLetter[0]))
		{
			bFoundDriveLetter = false;
		}
	}

	if (bFoundDriveLetter)
	{
		// Validate the project files.
		g_ProjectManager.VerifyCompilation(&g_ProgressDlg,szDriveLetter);
	}
	else
	{
		// Add to progress dialog instead?
		MessageBox(hWndParent,_T("InfraRecorder was unable to determine the drive letter of your recorder. The disc can not be verified."),lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
	}

	// We're done.
	g_ProgressDlg.SetProgress(100);
	g_ProgressDlg.SetStatus(lngGetString(PROGRESS_DONE));
	g_ProgressDlg.NotifyComplteted();

	return 0;
}*/
