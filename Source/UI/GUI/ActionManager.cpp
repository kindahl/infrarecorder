/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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
#include <ckfilesystem/filesystemwriter.hh>
#include <ckfilesystem/sectorstream.hh>
#include "Settings.h"
#include "Core.h"
#include "Core2.h"
#include "Core2Info.h"
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
#include "Scsi.h"
#include "LogDlg.h"
#include "ActionManager.h"

CActionManager g_ActionManager;

CActionManager::CActionManager()
{
}

CActionManager::~CActionManager()
{
}

DWORD WINAPI CActionManager::BurnCompilationThread(LPVOID lpThreadParameter)
{
	int iProjectType = g_ProjectManager.GetProjectType(),iResult = 0;
	ckcore::File ImageFile = ckcore::File::temp(g_GlobalSettings.m_szTempPath);

	// Make sure that the disc will not be ejected before beeing verified.
	bool bEject = g_BurnImageSettings.m_bEject;
	g_BurnImageSettings.m_bEject = false;

	// Used for locating the files on the disc when verifying.
	std::map<tstring,tstring> FilePathMap;

	if (iProjectType == PROJECTTYPE_DATA ||
		iProjectType == PROJECTTYPE_MIXED)
	{
		ckfilesystem::FileComparator FileComp(g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO);
		ckfilesystem::FileSet Files(FileComp);
		
		switch (iProjectType)
		{
			case PROJECTTYPE_DATA:
				g_TreeManager.GetPathList(Files,g_TreeManager.GetRootNode());
				break;

			case PROJECTTYPE_MIXED:
				g_TreeManager.GetPathList(Files,g_ProjectManager.GetMixDataRootNode(),
					lstrlen(g_ProjectManager.GetMixDataRootNode()->pItemData->GetFileName()) + 1);
				break;

			default:
				return 0;
		};

		// Create a temporary disc image if not burning on the fly.
		if (!g_BurnImageSettings.m_bOnFly)
		{
			// Set the status information.
			g_pProgressDlg->SetWindowText(lngGetString(STITLE_CREATEIMAGE));
			g_pProgressDlg->SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
			g_pProgressDlg->set_status(lngGetString(STATUS_WRITEIMAGE));

			g_pProgressDlg->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINDISCIMAGE));

			iResult = g_Core2.CreateImage(ImageFile.name().c_str(),Files,*g_pProgressDlg,
										  true,g_BurnImageSettings.m_bVerify ? &FilePathMap : NULL);
			g_pProgressDlg->set_progress(100);

			switch (iResult)
			{
				case RESULT_OK:
					g_pProgressDlg->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_CREATEIMAGE));
					break;

				case RESULT_CANCEL:
					g_pProgressDlg->NotifyCompleted();

					ImageFile.remove();
					return 0;

				case RESULT_FAIL:
					g_pProgressDlg->set_status(lngGetString(PROGRESS_FAILED));
					g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_CREATEIMAGE));
					g_pProgressDlg->NotifyCompleted();

					ImageFile.remove();
					return 0;
			};
		}
	}

	g_pProgressDlg->set_progress(0);

	ckmmc::Device &Device = *g_BurnImageSettings.m_pRecorder;

	// Set the status information.
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_BURNCOMPILATION));
	g_pProgressDlg->SetDevice(Device);

	std::vector<TCHAR *> AudioTracks;
	std::vector<TCHAR *> TempTracks;
	const TCHAR *pAudioText = NULL;

	// Decode any encoded tracks in audio and mixed-mode projects.
	g_pProgressDlg->set_status(lngGetString(PROGRESS_DECODETRACKS));
	switch (iProjectType)
	{
		case PROJECTTYPE_MIXED:
		case PROJECTTYPE_AUDIO:
			g_ProjectManager.GetAudioTracks(AudioTracks);

			// Decode any audio tracks that might be encoded.
			if (!g_ProjectManager.DecodeAudioTracks(AudioTracks,TempTracks,g_pProgressDlg))
			{
				g_pProgressDlg->NotifyCompleted();

				// Remove any temporary tracks.
				for (unsigned int i = 0; i < TempTracks.size(); i++)
				{
					ckcore::File::remove(TempTracks[i]);

					std::vector <TCHAR *>::iterator itObject = TempTracks.begin() + i;
					delete [] *itObject;
				}

				TempTracks.clear();
				return 0;
			}
			break;
	}

	// Start the burn process.
	for (long i = 0; i < g_BurnImageSettings.m_lNumCopies; i++)
	{
		bool bLast = i == (g_BurnImageSettings.m_lNumCopies - 1);
		g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));
		switch (iProjectType)
		{
			case PROJECTTYPE_DATA:
				if (g_BurnImageSettings.m_bOnFly)
				{
					// Try to estimate the file system size before burning the compilation.
					unsigned __int64 uiDataSize = 0;
					g_pProgressDlg->set_status(lngGetString(PROGRESS_ESTIMAGESIZE));

					ckfilesystem::FileComparator FileComp(g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO);
					ckfilesystem::FileSet Files(FileComp);

					g_TreeManager.GetPathList(Files,g_TreeManager.GetRootNode());

					iResult = g_Core2.EstimateImageSize(Files,*g_pProgressDlg,uiDataSize);

					g_pProgressDlg->set_progress(100);

					if (iResult == RESULT_OK)
					{
						// Reset the status.
						g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

						if (g_BurnImageSettings.m_bVerify || !bLast)
						{
							iResult = (int)g_Core.BurnCompilationEx(Device,g_pProgressDlg,*g_pProgressDlg,Files,
								AudioTracks,NULL,g_ProjectSettings.m_iIsoFormat,uiDataSize);
						}
						else
						{
							iResult = (int)g_Core.BurnCompilation(Device,g_pProgressDlg,*g_pProgressDlg,Files,
								AudioTracks,NULL,g_ProjectSettings.m_iIsoFormat,uiDataSize);
						}
					}
					else if (iResult == RESULT_FAIL)
					{
						g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_ESTIMAGESIZE));
					}
				}
				else
				{
					if (g_BurnImageSettings.m_bVerify || !bLast)
					{
						iResult = (int)g_Core.BurnTracksEx(Device,g_pProgressDlg,ImageFile.name().c_str(),
							AudioTracks,NULL,g_ProjectSettings.m_iIsoFormat);
					}
					else
					{
						iResult = (int)g_Core.BurnTracks(Device,g_pProgressDlg,ImageFile.name().c_str(),
							AudioTracks,NULL,g_ProjectSettings.m_iIsoFormat);
					}
				}
				break;

			case PROJECTTYPE_MIXED:
				// Save CD-Text information.
				if (AudioTracks.size() > 0)
				{
					// Check if any audio information has been edited.
					if (g_TreeManager.HasExtraAudioData(g_ProjectManager.GetMixAudioRootNode()))
					{
						ckcore::File AudioTextFile = ckcore::File::temp(g_GlobalSettings.m_szTempPath);

						if (g_ProjectManager.SaveCDText(AudioTextFile.name().c_str()))
							pAudioText = AudioTextFile.name().c_str();
						else
							lngMessageBox(HWND_DESKTOP,FAILURE_CREATECDTEXT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
					}
				}

				if (g_BurnImageSettings.m_bOnFly)
				{
					// Try to estimate the file system size before burning the compilation.
					unsigned __int64 uiDataSize = 0;
					g_pProgressDlg->set_status(lngGetString(PROGRESS_ESTIMAGESIZE));

					ckfilesystem::FileComparator FileComp(g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO);
					ckfilesystem::FileSet Files(FileComp);

					g_TreeManager.GetPathList(Files,g_ProjectManager.GetMixDataRootNode(),
						lstrlen(g_ProjectManager.GetMixDataRootNode()->pItemData->GetFileName()) + 1);

					iResult = g_Core2.EstimateImageSize(Files,*g_pProgressDlg,uiDataSize);

					g_pProgressDlg->set_progress(100);

					if (iResult == RESULT_OK)
					{
						// Reset the status.
						g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

						if (g_BurnImageSettings.m_bVerify || !bLast)
						{
							iResult = (int)g_Core.BurnCompilationEx(Device,g_pProgressDlg,*g_pProgressDlg,Files,
								AudioTracks,pAudioText,g_ProjectSettings.m_iIsoFormat,uiDataSize);
						}
						else
						{
							iResult = (int)g_Core.BurnCompilation(Device,g_pProgressDlg,*g_pProgressDlg,Files,
								AudioTracks,pAudioText,g_ProjectSettings.m_iIsoFormat,uiDataSize);
						}
					}
					else if (iResult == RESULT_FAIL)
					{
						g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_ESTIMAGESIZE));
					}
				}
				else
				{
					if (g_BurnImageSettings.m_bVerify || !bLast)
					{
						iResult = (int)g_Core.BurnTracksEx(Device,g_pProgressDlg,ImageFile.name().c_str(),
							AudioTracks,pAudioText,g_ProjectSettings.m_iIsoFormat);
					}
					else
					{
						iResult = (int)g_Core.BurnTracks(Device,g_pProgressDlg,ImageFile.name().c_str(),
							AudioTracks,pAudioText,g_ProjectSettings.m_iIsoFormat);
					}
				}
				break;

			case PROJECTTYPE_AUDIO:
				// Save CD-Text information.
				if (AudioTracks.size() > 0)
				{
					// Check if any audio information has been edited.
					if (g_TreeManager.HasExtraAudioData(g_TreeManager.GetRootNode()))
					{
						ckcore::File AudioTextFile = ckcore::File::temp(g_GlobalSettings.m_szTempPath);

						if (g_ProjectManager.SaveCDText(AudioTextFile.name().c_str()))
							pAudioText = AudioTextFile.name().c_str();
						else
							lngMessageBox(HWND_DESKTOP,FAILURE_CREATECDTEXT,GENERAL_ERROR,MB_OK | MB_ICONERROR);
					}
				}

				if (bLast)
				{
					iResult = (int)g_Core.BurnTracks(Device,g_pProgressDlg,NULL,
						AudioTracks,pAudioText,0);
				}
				else
				{
					iResult = (int)g_Core.BurnTracksEx(Device,g_pProgressDlg,NULL,
						AudioTracks,pAudioText,0);
				}
				break;
		};

		// Handle the result.
		if (!iResult)
		{
			g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->NotifyCompleted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
			return 0;
		}
		else if (iResult == RESULT_OK)
		{
			// If the recording was successfull, verify the disc.
			if (g_BurnImageSettings.m_bVerify)
			{
				// We need to reload the drive media.
				g_pProgressDlg->set_status(lngGetString(PROGRESS_RELOADMEDIA));

				g_Core2.StartStopUnit(Device,CCore2::LOADMEDIA_EJECT,false);
				if (!g_Core2.StartStopUnit(Device,CCore2::LOADMEDIA_LOAD,false))
					lngMessageBox(*g_pProgressDlg,INFO_RELOAD,GENERAL_INFORMATION,MB_OK | MB_ICONINFORMATION);

				// Set the device information.
				g_pProgressDlg->SetDevice(Device);
				g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));
				g_pProgressDlg->SetWindowText(lngGetString(STITLE_VERIFYDISC));

				// Get the device drive letter.
				TCHAR szDriveLetter[3];
				szDriveLetter[0] = Device.address().device_[0];
				szDriveLetter[1] = ':';
				szDriveLetter[2] = '\0';

				// Validate the project files.
				g_ProjectManager.VerifyCompilation(g_pProgressDlg,szDriveLetter,FilePathMap);

				// We're done.
				g_pProgressDlg->set_progress(100);
				g_pProgressDlg->set_status(lngGetString(PROGRESS_DONE));
				g_pProgressDlg->NotifyCompleted();
			}

			// Eject the disc if requested.
			if (bEject)
				g_Core.EjectDisc(Device,false);
		}

		if (!bLast)
		{
			g_pProgressDlg->set_progress(0);

			if (!g_pProgressDlg->RequestNextDisc())
			{
				g_pProgressDlg->NotifyCompleted();
				break;
			}

			TCHAR szBuffer[128];
			lsprintf(szBuffer,lngGetString(INFO_CREATECOPY),
				i + 2,
				g_BurnImageSettings.m_lNumCopies);
			g_pProgressDlg->notify(ckcore::Progress::ckINFORMATION,szBuffer);
		}
	}

	// Remove temporary data.
	switch (iProjectType)
	{
		case PROJECTTYPE_DATA:
			if (!g_BurnImageSettings.m_bOnFly)
				ImageFile.remove();
			break;

		case PROJECTTYPE_MIXED:
			if (!g_BurnImageSettings.m_bOnFly)
				ImageFile.remove();

			// Remove any temporary tracks.
			for (unsigned int i = 0; i < TempTracks.size(); i++)
			{
				ckcore::File::remove(TempTracks[i]);

				std::vector <TCHAR *>::iterator itObject = TempTracks.begin() + i;
				delete [] *itObject;
			}

			TempTracks.clear();

			// Remove the CD-Text file.
			if (pAudioText != NULL)
				ckcore::File::remove(pAudioText);
			break;

		case PROJECTTYPE_AUDIO:
			// Remove any temporary tracks.
			for (unsigned int i = 0; i < TempTracks.size(); i++)
			{
				ckcore::File::remove(TempTracks[i]);

				std::vector <TCHAR *>::iterator itObject = TempTracks.begin() + i;
				delete [] *itObject;
			}

			TempTracks.clear();

			// Remove the CD-Text file.
			if (pAudioText != NULL)
				ckcore::File::remove(pAudioText);
			break;
	};

	return 0;
}

DWORD WINAPI CActionManager::CreateImageThread(LPVOID lpThreadParameter)
{
	TCHAR *szFileName = (TCHAR *)lpThreadParameter;

	ckfilesystem::FileComparator FileComp(g_ProjectSettings.m_iFileSystem == FILESYSTEM_DVDVIDEO);
	ckfilesystem::FileSet Files(FileComp);
	
	switch (g_ProjectManager.GetProjectType())
	{
		case PROJECTTYPE_DATA:
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
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_CREATEIMAGE));
	g_pProgressDlg->SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
	g_pProgressDlg->set_status(lngGetString(STATUS_WRITEIMAGE));

	g_pProgressDlg->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINDISCIMAGE));

	int iResult = g_Core2.CreateImage(szFileName,Files,*g_pProgressDlg,true);
	g_pProgressDlg->set_progress(100);
	g_pProgressDlg->NotifyCompleted();

	switch (iResult)
	{
		case RESULT_OK:
			g_pProgressDlg->set_status(lngGetString(PROGRESS_DONE));
			g_pProgressDlg->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_CREATEIMAGE));
			break;

		case RESULT_FAIL:
			g_pProgressDlg->set_status(lngGetString(PROGRESS_FAILED));
			g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_CREATEIMAGE));

			ckcore::File::remove(szFileName);
			break;

		case RESULT_CANCEL:
			ckcore::File::remove(szFileName);
			break;
	};

	delete [] szFileName;
	return 0;
}

/**
	Tries to erase the disc inserted in the specified recorder using a fast
	method. The function will not let you know if it fails.
*/
void CActionManager::QuickErase(ckmmc::Device &Device)
{
	// Get current profile.
	unsigned short usProfile = PROFILE_NONE;
	if (g_Core2.GetProfile(Device,usProfile))
	{
		int iMode = g_EraseSettings.m_iMode;
		bool bForce = g_EraseSettings.m_bForce;
		bool bEject = g_EraseSettings.m_bEject;
		bool bSimulate = g_EraseSettings.m_bSimulate;

		switch (usProfile)
		{
			case PROFILE_DVDPLUSRW:
			case PROFILE_DVDPLUSRW_DL:
				g_EraseSettings.m_iMode = CCore2::ERASE_FORMAT_QUICK;
				break;

			case PROFILE_DVDRAM:
				g_EraseSettings.m_iMode = CCore2::ERASE_FORMAT_FULL;
				break;

			//case PROFILE_CDRW:
			//case PROFILE_DVDMINUSRW_RESTOV:
			//case PROFILE_DVDMINUSRW_SEQ:
			default:
				g_EraseSettings.m_iMode = CCore2::ERASE_BLANK_MINIMAL;
				break;
		}

		g_EraseSettings.m_iMode = CCore2::ERASE_BLANK_MINIMAL;
		g_EraseSettings.m_bForce = true;
		g_EraseSettings.m_bEject = false;
		g_EraseSettings.m_bSimulate = false;
		g_EraseSettings.m_pRecorder = &Device;
		g_EraseSettings.m_uiSpeed = 0xFFFFFFFF;	// Maximum.

		g_pProgressDlg->SetWindowText(lngGetString(STITLE_ERASE));
		g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

		// Set the device information.
		g_pProgressDlg->SetDevice(Device);

		// Create the new erase thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,EraseThread,new CEraseParam(false),0,&ulThreadID);

		// Wait for the thread to finish.
		while (true)
		{
			if (WaitForSingleObject(hThread,100) == WAIT_TIMEOUT)
			{
				ProcessMessages();
				Sleep(100);
			}
			else
			{
				break;
			}
		}

		::CloseHandle(hThread);

		g_pProgressDlg->Reset();
		g_pProgressDlg->AllowCancel(true);

		// Restore settings.
		g_EraseSettings.m_iMode = iMode;
		g_EraseSettings.m_bForce = bForce;
		g_EraseSettings.m_bEject = bEject;
		g_EraseSettings.m_bSimulate = bSimulate;
	}
}

/**
	Depending on the media inserted into the disc this function will try to
	determine if the media should be erased. This includes asking the user if
	necessary.
*/
bool CActionManager::QuickEraseQuery(ckmmc::Device &Device,HWND hWndParent)
{
	if (!g_ProjectSettings.m_bMultiSession)
	{
		CCore2Info Info;
		CCore2DiscInfo DiscInfo;
		if (Info.ReadDiscInformation(Device,&DiscInfo))
		{
			if (DiscInfo.m_ucDiscStatus != CCore2DiscInfo::DS_EMTPY &&
				DiscInfo.m_ucFlags & CCore2DiscInfo::FLAG_ERASABLE)
			{
				if (lngMessageBox(hWndParent,MISC_ERASENONEMPTY,GENERAL_QUESTION,
					MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					return true;
				}
			}
		}
	}

	return false;
}

INT_PTR CActionManager::BurnCompilation(HWND hWndParent,bool bAppMode)
{
	bool bAudioProject = g_ProjectManager.GetProjectType() == PROJECTTYPE_AUDIO;

	// Display the burn image dialog.
	CBurnImageDlg BurnImageDlg(lngGetString(MISC_BURNCOMPILATION),false,
		!bAudioProject,!bAudioProject,bAppMode);
	INT_PTR iResult = BurnImageDlg.DoModal();

	if (iResult == IDOK)
	{
		// Open the device to see if the disc is rewritable and not blank.
		/*bool bErase = false;
		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(g_BurnImageSettings.m_iRecorder);
		if (!g_ProjectSettings.m_bMultiSession)
		{
			CCore2Device Device;
			if (Device.Open(&pDeviceInfo->Address))
			{
				CCore2Info Info;
				CCore2DiscInfo DiscInfo;
				if (Info.ReadDiscInformation(&Device,&DiscInfo))
				{
					if (DiscInfo.m_ucDiscStatus != CCore2DiscInfo::DS_EMTPY &&
						DiscInfo.m_ucFlags & CCore2DiscInfo::FLAG_ERASABLE)
					{
						if (lngMessageBox(hWndParent,MISC_ERASENONEMPTY,GENERAL_QUESTION,
							MB_YESNO | MB_ICONQUESTION) == IDYES)
						{
							bErase = true;
						}
					}
				}
			}
		}*/
		// Check if we should erase the disc.
		bool bErase = QuickEraseQuery(*g_BurnImageSettings.m_pRecorder,hWndParent);

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_pProgressDlg->SetAppMode(bAppMode);
		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(hWndParent);

		g_pProgressDlg->ShowWindow(true);
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(hWndParent);
		ProcessMessages();

		if (bErase)
			QuickErase(*g_BurnImageSettings.m_pRecorder);

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
		g_pProgressDlg->SetAppMode(bAppMode);
		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(hWndParent);

		g_pProgressDlg->ShowWindow(true);
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(hWndParent);
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
	if (ckcore::File::exist(szTOCFilePath))
		bImageHasTOC = true;

	// Display the burn image dialog.
	CBurnImageDlg BurnImageDlg(szTitle,bImageHasTOC,false,false,bAppMode);
	INT_PTR iResult = BurnImageDlg.DoModal();

	if (iResult == IDOK)
	{
		// Check if we should erase the disc.
		bool bErase = QuickEraseQuery(*g_BurnImageSettings.m_pRecorder,hWndParent);

		ckmmc::Device &Device = *g_BurnImageSettings.m_pRecorder;

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_pProgressDlg->SetAppMode(bAppMode);

		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(hWndParent);

		g_pProgressDlg->ShowWindow(true);
		//g_pProgressDlg->SetWindowText(lngGetString(STITLE_BURNIMAGE));
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(hWndParent);
		ProcessMessages();

		// Erase the disc if necessary.
		if (bErase)
			QuickErase(Device);

		// Set the device information.
		g_pProgressDlg->SetWindowText(lngGetString(STITLE_BURNIMAGE));
		g_pProgressDlg->SetDevice(Device);
		g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

		// Begin burning the image.
		if (!g_Core.BurnImage(Device,g_pProgressDlg,szFilePath,bImageHasTOC))
		{
			g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->NotifyCompleted();

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
	ckmmc::Device &SrcDevice = *g_CopyDiscSettings.m_pSource;
	ckmmc::Device &DstDevice = *g_CopyDiscSettings.m_pTarget;

	// Set the status information.
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_COPYDISC));
	g_pProgressDlg->SetDevice(DstDevice);
	g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

	// Start the operation.
	if (!g_Core.CopyDisc(SrcDevice,DstDevice,g_pProgressDlg))
	{
		g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
		g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
		g_pProgressDlg->NotifyCompleted();

		lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
		return 0;
	}

	return 0;
}

DWORD WINAPI CActionManager::CopyDiscThread(LPVOID lpThreadParameter)
{
	ckcore::File ImageFile = ckcore::File::temp(g_GlobalSettings.m_szTempPath);

	// Set the status information.
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_CREATEIMAGE));
	g_pProgressDlg->SetDevice(lngGetString(PROGRESS_IMAGEDEVICE));
	g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

	// Get device information.
	ckmmc::Device &SrcDevice = *g_CopyDiscSettings.m_pSource;

	// Override the read sub-channel data setting.
	g_ReadSettings.m_bClone = g_CopyDiscSettings.m_bClone;

	int iResult = g_Core.ReadDiscEx(SrcDevice,g_pProgressDlg,ImageFile.name().c_str());

	switch (iResult)
	{
		case RESULT_INTERNALERROR:
			g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->NotifyCompleted();

			lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);

			// Remove any temporary files.
			ImageFile.remove();
			if (g_CopyDiscSettings.m_bClone)
			{
				ckcore::tstring TocName = ImageFile.name();
				TocName += ckT(".toc");
				ckcore::File::remove(TocName.c_str());
			}
			return 0;

		case RESULT_EXTERNALERROR:
			// Remove any temporary files.
			ImageFile.remove();
			if (g_CopyDiscSettings.m_bClone)
			{
				ckcore::tstring TocName = ImageFile.name();
				TocName += ckT(".toc");
				ckcore::File::remove(TocName.c_str());
			}
			return 0;
	}

	// Ask the user to switch discs if the target is the same as the source.
	if (g_CopyDiscSettings.m_pSource == g_CopyDiscSettings.m_pTarget)
	{
		g_Core2.StartStopUnit(SrcDevice,CCore2::LOADMEDIA_EJECT,true);

		if (lngMessageBox(*g_pProgressDlg,INFO_INSERTBLANK,GENERAL_INFORMATION,
			MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
		{
			g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->NotifyCompleted();

			// Remove any temporary files.
			ImageFile.remove();
			if (g_CopyDiscSettings.m_bClone)
			{
				ckcore::tstring TocName = ImageFile.name();
				TocName += ckT(".toc");
				ckcore::File::remove(TocName.c_str());
			}

			return 0;
		}
	}

	// Get device information.
	ckmmc::Device &DstDevice = *g_CopyDiscSettings.m_pTarget;

	// Set the status information.
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_BURNIMAGE));
	g_pProgressDlg->SetDevice(DstDevice);
	g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

	if (!g_Core.BurnImageEx(DstDevice,g_pProgressDlg,ImageFile.name().c_str(),
		g_CopyDiscSettings.m_bClone))
	{
		g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
		g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
		g_pProgressDlg->NotifyCompleted();

		lngMessageBox(HWND_DESKTOP,FAILURE_CDRTOOLS,GENERAL_ERROR,MB_OK | MB_ICONERROR);
	}

	// Remove any temporary files.
	ImageFile.remove();
	if (g_CopyDiscSettings.m_bClone)
	{
		ckcore::tstring TocName = ImageFile.name();
		TocName += ckT(".toc");
		ckcore::File::remove(TocName.c_str());
	}

	return 0;
}

DWORD WINAPI CActionManager::EraseThread(LPVOID lpThreadParameter)
{
	std::auto_ptr<CEraseParam> Param((CEraseParam *)lpThreadParameter);

	// Get device information.
	ckmmc::Device &Device = *g_EraseSettings.m_pRecorder;

	bool bResult = g_Core2.EraseDisc(Device,g_pProgressDlg,g_EraseSettings.m_iMode,
		g_EraseSettings.m_bForce,g_EraseSettings.m_bEject,g_EraseSettings.m_bSimulate,
		g_EraseSettings.m_uiSpeed);

	g_pProgressDlg->set_progress(100);

	if (Param->m_bNotifyCompleted)
		g_pProgressDlg->NotifyCompleted();

	if (bResult)
	{
		g_pProgressDlg->set_status(lngGetString(PROGRESS_DONE));
	}
	else
	{
		g_pProgressDlg->set_status(lngGetString(PROGRESS_FAILED));
		g_pProgressDlg->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_ERASE));
	}

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
	CCopyDiscDlg CopyDiscDlg(bAppMode);
	INT_PTR iResult = CopyDiscDlg.DoModal();

	if (iResult == IDOK)
	{
		if (g_CopyDiscSettings.m_pSource == g_CopyDiscSettings.m_pTarget)
		{
			ckmmc::Device &Device = *g_CopyDiscSettings.m_pSource;
			g_Core2.StartStopUnit(Device,CCore2::LOADMEDIA_EJECT,true);

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
		g_pProgressDlg->SetAppMode(bAppMode);
		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(hWndParent);

		g_pProgressDlg->ShowWindow(true);
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(hWndParent);
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
	CCopyImageDlg CopyImageDlg(bAppMode);
	INT_PTR iResult = CopyImageDlg.DoModal();

	if (iResult == IDOK)
	{
		ckmmc::Device &Device = *g_CopyDiscSettings.m_pSource;

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_pProgressDlg->SetAppMode(bAppMode);
		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(hWndParent);

		g_pProgressDlg->ShowWindow(true);
		g_pProgressDlg->SetWindowText(lngGetString(STITLE_CREATEIMAGE));
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		g_pProgressDlg->SetDevice(Device);
		g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

		// Begin reading the disc.
		if (!g_Core.ReadDisc(Device,g_pProgressDlg,CopyImageDlg.GetFileName()))
		{
			g_pProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
			g_pProgressDlg->NotifyCompleted();

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
		ckmmc::Device &Device = *g_EraseSettings.m_pRecorder;

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_pProgressDlg->SetAppMode(bAppMode);

		if (!g_pProgressDlg->IsWindow())
			g_pProgressDlg->Create(hWndParent);

		g_pProgressDlg->ShowWindow(true);
		g_pProgressDlg->SetWindowText(lngGetString(STITLE_ERASE));
		g_pProgressDlg->Reset();
		g_pProgressDlg->AttachProcess(&g_Core);
		g_pProgressDlg->AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		g_pProgressDlg->SetDevice(Device);
		g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

		// Create the new thread.
		unsigned long ulThreadID = 0;
		HANDLE hThread = ::CreateThread(NULL,0,EraseThread,new CEraseParam(true),0,&ulThreadID);

		// Run the message loop if we're in application mode.
		if (bAppMode)
		{
			iResult = MainLoop.Run();
				_Module.RemoveMessageLoop();
			return iResult;
		}

		::CloseHandle(hThread);
	}

	return iResult;
}

INT_PTR CActionManager::Fixate(HWND hWndParent,bool bAppMode)
{
	CFixateDlg FixateDlg(bAppMode);
	INT_PTR iResult = FixateDlg.DoModal();

	if (iResult == IDOK)
	{
		ckmmc::Device &Device = *g_FixateSettings.m_pRecorder;

		// Disable the parent window.
		if (hWndParent != NULL)
			::EnableWindow(hWndParent,false);

		// If we're in application mode we need to create a message loop since
		// the progress window must run independently.
		CMessageLoop MainLoop;

		if (bAppMode)
			_Module.AddMessageLoop(&MainLoop);

		// Create and display the progress dialog.
		g_pSimpleProgressDlg->SetAppMode(bAppMode);

		if (!g_pSimpleProgressDlg->IsWindow())
			g_pSimpleProgressDlg->Create(hWndParent);

		g_pSimpleProgressDlg->ShowWindow(true);
		g_pSimpleProgressDlg->SetWindowText(lngGetString(STITLE_FIXATE));
		g_pSimpleProgressDlg->Reset();
		g_pSimpleProgressDlg->AttachProcess(&g_Core);
		g_pSimpleProgressDlg->AttachHost(hWndParent);
		ProcessMessages();

		// Set the device information.
		g_pSimpleProgressDlg->SetDevice(Device);
		g_pSimpleProgressDlg->set_status(lngGetString(PROGRESS_INIT));

		// Begin erasing the disc.
		if (!g_Core.FixateDisc(Device,g_pSimpleProgressDlg,
			g_FixateSettings.m_bEject,
			g_FixateSettings.m_bSimulate))
		{
			g_pSimpleProgressDlg->set_status(lngGetString(PROGRESS_CANCELED));
			g_pSimpleProgressDlg->notify(ckcore::Progress::ckWARNING,lngGetString(PROGRESS_CANCELED));
			g_pSimpleProgressDlg->NotifyCompleted();

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
	g_pProgressDlg->SetAppMode(false);

	if (!g_pProgressDlg->IsWindow())
		g_pProgressDlg->Create(hWndParent);

	g_pProgressDlg->ShowWindow(true);
	g_pProgressDlg->SetWindowText(lngGetString(STITLE_BURNIMAGE));
	g_pProgressDlg->Reset();
	g_pProgressDlg->AttachProcess(&g_Core);
	g_pProgressDlg->AttachHost(hWndParent);
	ProcessMessages();

	// Set the device information.
	TCHAR szDeviceName[128];
	g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

	g_pProgressDlg->SetDevice(szDeviceName);
	g_pProgressDlg->set_status(lngGetString(PROGRESS_INIT));

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
		g_ProjectManager.VerifyCompilation(g_pProgressDlg,szDriveLetter);
	}
	else
	{
		// Add to progress dialog instead?
		MessageBox(hWndParent,_T("InfraRecorder was unable to determine the drive letter of your recorder. The disc can not be verified."),lngGetString(GENERAL_ERROR),MB_OK | MB_ICONERROR);
	}

	// We're done.
	g_pProgressDlg->set_progress(100);
	g_pProgressDlg->set_status(lngGetString(PROGRESS_DONE));
	g_pProgressDlg->NotifyCompleted();

	return 0;
}*/
