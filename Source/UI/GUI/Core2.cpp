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
#include <ckcore/filestream.hh>
#include <ckcore/nullstream.hh>
#include <ckfilesystem/filesystemwriter.hh>
#include "../../Common/Exception.h"
#include "Core2.h"
#include "Core2Format.h"
#include "Core2Blank.h"
#include "Core2Util.h"
#include "Core2Info.h"
#include "Core2Read.h"
#include "LogDlg.h"
#include "Settings.h"
#include "StringTable.h"
#include "LangUtil.h"

CCore2 g_Core2;

CCore2::CCore2()
{
}

CCore2::~CCore2()
{
}

bool CCore2::HandleEvents(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						  unsigned char &ucHandledEvents)
{
	ucHandledEvents = 0;

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Handle events.
	unsigned char ucEvents = 0xFF;
	unsigned char ucEvent[8];

	unsigned char ucStartCount = 0;

	while (ucEvents)
	{
		ucCdb[0] = SCSI_GET_EVENT_STATUS_NOTIFICATION;
		ucCdb[1] = 0x01;			// Polled.
		ucCdb[4] = ucEvents;
		ucCdb[8] = sizeof(ucEvent);
		ucCdb[9] = 0x00;

		if (!Device.transport(ucCdb,10,ucEvent,sizeof(ucEvent),ckmmc::Device::ckTM_READ))
			return false;

		ucEvents = ucEvent[3];

		if ((ucEvent[2] & 0x07) == 0 ||
			(ucEvent[0] << 8 | ucEvent[1]) == 2 ||
			(ucEvent[4] & 0x0F) == 0)	// No Changes
			return true;

		unsigned int uiDescriptor = ucEvent[4] << 24 |
			ucEvent[5] << 16 | ucEvent[6] << 8 | ucEvent[7];

		switch (ucEvent[2] & 0x07)
		{
			case 0:		// No reqeusted event classes are supported.
				return true;

			case 1:		// Operational change request/notification.
				ucHandledEvents |= 0x01;
				break;

			case 2:		// Power management.
				if ((uiDescriptor & 0xFF0000) != 0x01)
				{
					// For some reasons this code is not compatible with all recorders.
					if (ucStartCount == 10)
					{
						g_pLogDlg->print_line(_T("  Warning: Unable to start the drive, aborting after %d retries."),ucStartCount);
						return true;
					}

					g_pLogDlg->print_line(_T("  The drive is not in active state."));

					if (!StartStopUnit(Device,LOADMEDIA_START,false))
					{
						if (pProgress != NULL)
							pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_NOMEDIA));
						return false;
					}

					g_pLogDlg->print_line(_T("   -> Started the disc to make it ready for access."));
					ucStartCount++;
				}

				ucHandledEvents |= 0x02;
				break;

			case 3:		// External request.
				ucHandledEvents |= 0x04;
				break;

			case 4:		// Media.
				ucHandledEvents |= 0x08;
				break;

			case 5:		// Multiple hosts.
				ucHandledEvents |= 0x10;
				break;

			case 6:		// Device busy.
				ucHandledEvents |= 0x20;
				break;

			case 7:		// Reserved.
				ucHandledEvents |= 0x40;
				break;
		}
	}

	return true;
}

bool CCore2::WaitForUnit(ckmmc::Device &Device,CAdvancedProgress *pProgress)
{
	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	unsigned char ucSense[24];
	memset(ucSense,0,sizeof(ucSense));

	// Prepare command.
	ucCdb[0] = SCSI_TEST_UNIT_READY;

	while (true)
	{
		Sleep(1000);

		unsigned char ucResult;
		if (!Device.transport_with_sense(ucCdb,6,NULL,0,ckmmc::Device::ckTM_READ,
										 ucSense,ucResult))
		{
			return false;
		}

		// Check if we're done.
		if (ucResult == SCSISTAT_GOOD)
			return true;

		// Check for errors.
		/*switch (ucSense[2] & 0x0F)
		{
			case SENSEKEY_MEDIUM_ERROR:
			case SENSEKEY_HARDWARE_ERROR:
				return false;

			case SENSEKEY_NOT_READY:
				break;
		}*/

		// See if we're formating a disc.
		unsigned char ucCode = CheckSense(ucSense);
		if (ucCode == SENSE_FORMATINPROGRESS ||
			ucCode == SENSE_LONGWRITEINPROGRESS)
		{
			// Check if the operation has been cancelled.
			if (pProgress != NULL && pProgress->cancelled())
			{
				// Stop the unit and unlock the media.
				CloseTrackSession(Device,0,0,true);
				LockMedia(Device,false);
				return false;
			}

			// Update the progress.
			if (pProgress != NULL)
			{
				// If the SKSV bit is set to zero we are done.
				if (ucSense[15] & 0x80)
				{
					unsigned short usProgress = ((unsigned short)ucSense[16] << 8) | ucSense[17];
					pProgress->set_progress((unsigned char)(usProgress * 100.0f / 0xFFFF));
				}
				else
				{
					pProgress->set_progress(100);
					return true;
				}
			}
		}
	}

	return true;
}

CCore2::eMediaChange CCore2::CheckMediaChange(ckmmc::Device &Device)
{
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Handle events.
	unsigned char ucEvent[8];

	ucCdb[0] = SCSI_GET_EVENT_STATUS_NOTIFICATION;
	ucCdb[1] = 0x01;			// Polled.
	ucCdb[4] = 0x10;
	ucCdb[8] = sizeof(ucEvent);
	ucCdb[9] = 0x00;

	if (!Device.transport(ucCdb,10,ucEvent,sizeof(ucEvent),
						  ckmmc::Device::ckTM_READ))
	{
		return MEDIACHANGE_NOCHANGE;
	}

	unsigned char ucEventCode = ucEvent[4] & 0x0F;
	switch (ucEventCode)
	{
		case 1:
			Device.refresh();
			return MEDIACHANGE_EJECTREQUEST;
		case 2:
			Device.refresh();
			return MEDIACHANGE_NEWMEDIA;
		case 3:
			Device.refresh();
			return MEDIACHANGE_MEDIAREMOVAL;
		case 4:
			Device.refresh();
			return MEDIACHANGE_MEDIACHANGED;
		case 5:
			Device.refresh();
			return MEDIACHANGE_BGFORMAT_COMPLETED;
		case 6:
			Device.refresh();
			return MEDIACHANGE_BGFORMAT_RESTARTED;
	}

	return MEDIACHANGE_NOCHANGE;
}

bool CCore2::LockMedia(ckmmc::Device &Device,bool bLock)
{
	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_PREVENTALLOW_MEDIUM_REMOVAL;
	ucCdb[4] = (unsigned char)bLock;
	ucCdb[5] = 0x00;

	if (!Device.transport(ucCdb,6,NULL,0,ckmmc::Device::ckTM_READ))
		return false;

	return true;
}

bool CCore2::StartStopUnit(ckmmc::Device &Device,eLoadMedia Action,bool bImmed)
{
	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	unsigned char ucSense[24];
	memset(ucSense,0,sizeof(ucSense));

	ucCdb[0] = SCSI_START_STOP_UNIT;
	ucCdb[1] = (unsigned char)bImmed;
	ucCdb[4] = (unsigned char)Action;
	ucCdb[5] = 0x00;

	unsigned char ucResult = 0;
	if (!Device.transport_with_sense(ucCdb,6,NULL,0,ckmmc::Device::ckTM_READ,
									 ucSense,ucResult))
	{
		return false;
	}

	if (ucResult != SCSISTAT_GOOD)
	{
		if ((ucSense[2] & 0x0F) == SENSEKEY_NOT_READY)
		{
			if (ucSense[12] == 0x3A)
			{
				switch (ucSense[13])
				{
					case 0x00:	// MEDIUM NOT PRESENT
					case 0x01:	// MEDIUM NOT PRESENT – TRAY CLOSED.
						break;

					case 0x02:	// MEDIUM NOT PRESENT – TRAY OPEN
						if (Action != LOADMEDIA_LOAD)
						{
							g_pLogDlg->print_line(_T("  Warning: Unable to start media, trying to manually load it."));
							return StartStopUnit(Device,LOADMEDIA_LOAD,bImmed);
						}
						break;
				}
			}
		}

		g_pLogDlg->print_line(_T("  Error: Could not start or stop the unit."));

		// Dump CDB.
		g_pLogDlg->print(_T("  CDB:\t"));
		for (unsigned int i = 0; i < 6; i++)
		{
			if (i == 0)
				g_pLogDlg->print(_T("0x%.2X"),ucCdb[i]);
			else
				g_pLogDlg->print(_T(",0x%.2X"),ucCdb[i]);
		}

		g_pLogDlg->print_line(_T(""));

		// Dump sense information.
		g_pLogDlg->print_line(_T("  Sense:\t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
			ucSense[0],ucSense[1],ucSense[2],ucSense[3],
			ucSense[4],ucSense[5],ucSense[6],ucSense[7]);

		g_pLogDlg->print_line(_T("  \t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
			ucSense[ 8],ucSense[ 9],ucSense[10],ucSense[11],
			ucSense[12],ucSense[13],ucSense[14],ucSense[15]);

		g_pLogDlg->print_line(_T("  \t0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X,0x%.2X"),
			ucSense[16],ucSense[17],ucSense[18],ucSense[19],
			ucSense[20],ucSense[21],ucSense[22],ucSense[23]);

		return false;
	}

	return true;
}

bool CCore2::CloseTrackSession(ckmmc::Device &Device,unsigned char ucCloseFunction,
							   unsigned short usTrackNumber,bool bImmed)
{
	if (ucCloseFunction > 0x07)
		return false;

	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_CLOSE_TRACK_SESSION;
	ucCdb[1] = bImmed ? 0x01 : 0x00;
	ucCdb[2] = ucCloseFunction & 0x07;
	ucCdb[4] = static_cast<unsigned char>(usTrackNumber >> 8);
	ucCdb[5] = static_cast<unsigned char>(usTrackNumber & 0xFF);
	ucCdb[9] = 0x00;

	if (!Device.transport(ucCdb,10,NULL,0,ckmmc::Device::ckTM_READ))
		return false;

	return true;
}

bool CCore2::SetDiscSpeeds(ckmmc::Device &Device,unsigned short usReadSpeed,
						   unsigned short usWriteSpeed)
{
	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[ 0] = SCSI_SET_CD_SPEED;
	ucCdb[ 2] = static_cast<unsigned char>(usReadSpeed >> 8);
	ucCdb[ 3] = static_cast<unsigned char>(usReadSpeed & 0xFF);
	ucCdb[ 4] = static_cast<unsigned char>(usWriteSpeed >> 8);
	ucCdb[ 5] = static_cast<unsigned char>(usWriteSpeed & 0xFF);
	ucCdb[11] = 0x08;

	if (!Device.transport(ucCdb,12,NULL,0,ckmmc::Device::ckTM_READ))
		return false;

	return true;
}

/**
	Updates mode page 5 of the specified device.
	@param pDevice the device to update.
	@param bTestWrite specifies wether the drive should test write or not.
	@param WriteMode specifies the write mode to use for recording.
	@param bSilent if true, the function will not output any error to the log if it failed.
	@return true if the mode page was successfully updated, false otherwise.
*/
bool CCore2::UpdateModePage5(ckmmc::Device &Device,bool bTestWrite,
							 bool bSilent)
{
	// Initialize buffers.
	unsigned char ucBuffer[128];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Read the current code page information.
	ucCdb[0] = SCSI_MODE_SENSE10;
	ucCdb[2] = 0x05;					// Default values and code page.
	ucCdb[7] = sizeof(ucBuffer) >> 8;	// Allocation length (MSB).
	ucCdb[8] = sizeof(ucBuffer) & 0xFF;	// Allocation length (LSB).
	ucCdb[9] = 0x00;

	if (!Device.transport(ucCdb,10,ucBuffer,sizeof(ucBuffer),
						  ckmmc::Device::ckTM_READ))
	{
		return false;
	}

	if ((ucBuffer[8] & 0x3F) != 0x05)
		return false;

	// Change the code page.
	//ucBuffer[8 + 2] |= bTestWrite ? 0x10 : 0x00;
	if (bTestWrite)
		ucBuffer[8 + 2] |= 0x10;
	else
		ucBuffer[8 + 2] &= ~0x10;

	// Send the updated code page.
	unsigned short usFileListSize = (ucBuffer[0] << 8 | ucBuffer[1]) + 2;

	memset(ucCdb,0,sizeof(ucCdb));
	ucCdb[0] = SCSI_MODE_SELECT10;
	ucCdb[1] = 0x10;	// PF (not using vendor specified mode page).
	ucCdb[7] = static_cast<unsigned char>(usFileListSize >> 8);
	ucCdb[8] = static_cast<unsigned char>(usFileListSize & 0xFF);
	ucCdb[9] = 0x00;

	if (!bSilent)
	{
		if (!Device.transport(ucCdb,10,ucBuffer,usFileListSize,
							  ckmmc::Device::ckTM_WRITE))
		{
			return false;
		}
	}
	else
	{
		unsigned char ucSense[24];
		memset(ucSense,0,sizeof(ucSense));

		unsigned char ucResult = 0;
		if (!Device.transport_with_sense(ucCdb,10,ucBuffer,usFileListSize,
										 ckmmc::Device::ckTM_WRITE,ucSense,ucResult))
		{
			return false;
		}

		if (ucResult != SCSISTAT_GOOD)
			return false;
	}

	return true;
}

bool CCore2::EraseDisc(ckmmc::Device &Device,CAdvancedProgress *pProgress,
					   int iMethod,bool bForce,bool bEject,bool bSimulate,
					   unsigned int uiSpeed)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore2::EraseDisc"));

		TCHAR szParameters[128];
		lsprintf(szParameters,_T("  Method = %d, Force = %d, Eject = %d, Simulate = %d."),
			iMethod,(int)bForce,(int)bEject,(int)bSimulate);
		g_pLogDlg->print_line(szParameters);
	}

	// Setup the progress dialog.
	pProgress->AllowCancel(false);
	pProgress->SetRealMode(!bSimulate);

	// Turn test writing on if selected.
	if (!UpdateModePage5(Device,bSimulate))
		g_pLogDlg->print_line(_T("  Warning: Unable to update code page 0x05."));

// Disabled until these routines are concidered stable enough.
#if 0
	// Lock the media.
	if (!LockMedia(pDevice,true))
		g_pLogDlg->print_line(_T("  Warning: Unable to lock device media."));
#endif

	// Set write speed.
	if (!SetDiscSpeeds(Device,0xFFFF,(unsigned short)uiSpeed))
		g_pLogDlg->print_line(_T("  Warning: Unable to set disc erase speed."));

	bool bResult = false;
	switch (iMethod)
	{
		case ERASE_FORMAT_QUICK:
		case ERASE_FORMAT_FULL:
			{
				CCore2Format Core2Format;
				bResult = Core2Format.FormatUnit(Device,pProgress,iMethod == ERASE_FORMAT_FULL);
			}
			break;

		case ERASE_BLANK_FULL:
		case ERASE_BLANK_MINIMAL:
		case ERASE_BLANK_UNCLOSE:
		case ERASE_BLANK_SESSION:
			{
				CCore2Blank Core2Blank;
				bResult = Core2Blank.Blank(Device,pProgress,iMethod,bForce,bSimulate);
			}
			break;
	}

	// Unlock the media.
	if (!LockMedia(Device,false))
		g_pLogDlg->print_line(_T("  Warning: Unable to unlock device media."));

	// Eject the media (if requested).
	if (bResult && bEject)
	{
		if (!StartStopUnit(Device,LOADMEDIA_EJECT,true))
			g_pLogDlg->print_line(_T("  Warning: Unable to eject media."));
	}

	return bResult;
}

bool CCore2::ReadDataTrack(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						   unsigned char ucTrackNumber,bool bIgnoreErr,const TCHAR *szFilePath)
{
	// Initialize log.
	if (g_GlobalSettings.m_bLog)
	{
		g_pLogDlg->print_line(_T("CCore2::ReadTrack"));

		TCHAR szParameters[128];
		lsprintf(szParameters,_T("  Track = %d."),ucTrackNumber);
		g_pLogDlg->print_line(szParameters);
	}

	CCore2Info Info;
	CCore2Read Read;

	// Print the maximum read speed (for diagnostics purposes).
	ckcore::tuint32 uiMaxReadSpeed = Device.property(ckmmc::Device::ckPROP_MAX_READ_SPD);
	g_pLogDlg->print_line(_T("  Maximum read speed: %d KiB/s."),uiMaxReadSpeed);

	if (!SetDiscSpeeds(Device,0xFFFF,0xFFFF))
		g_pLogDlg->print_line(_T("  Warning: Unable to set the device read speed."));

	unsigned char ucFirstTrackNumber = 0,ucLastTrackNumber = 0;
	std::vector<CCore2TOCTrackDesc> Tracks;

	if (Info.ReadTOC(Device,ucFirstTrackNumber,ucLastTrackNumber,Tracks))
	{
		g_pLogDlg->print_line(_T("  First and last disc track number: %d, %d."),
			ucFirstTrackNumber,ucLastTrackNumber);

		// Validate the requested track number.
		if (ucFirstTrackNumber > ucTrackNumber || ucLastTrackNumber < ucTrackNumber)
		{
			g_pLogDlg->print_line(_T("  Error: The requested track number is invalid."));
			return false;
		}
	}
	else
	{
		g_pLogDlg->print_line(_T("  Error: Unable to read TOC information to validate selected track."));
		return false;
	}

	// Obtain track start address.
	unsigned long ulTrackAddr = Tracks[ucTrackNumber - 1].m_ulTrackAddr;

	CCore2TrackInfo TrackInfo;
	if (!Info.ReadTrackInformation(Device,CCore2Info::TIT_LBA,ulTrackAddr,&TrackInfo))
	{
		g_pLogDlg->print_line(_T("  Error: Unable to read track information."));
		return false;
	}

	unsigned long ulTrackSize = TrackInfo.m_ulTrackSize;

	// Always assume a post-gap of 150 sectors. This may be a bad idea.
	ulTrackSize -= 150;
	g_pLogDlg->print_line(_T("  Track span: %d-%d."),ulTrackAddr,ulTrackAddr + ulTrackSize);

	unsigned long ulMin = (ulTrackSize + 150)/(60*75);
	unsigned long ulSec = (ulTrackSize + 150 - ulMin * 60 * 75)/75;
	unsigned long ulFrame = ulTrackSize + 150 - ulMin * 60 * 75 - ulSec * 75;
	g_pLogDlg->print_line(_T("  Track length: %02d:%02d:%02d"),ulMin,ulSec,ulFrame);

	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINREADTRACK),ucTrackNumber);
	pProgress->set_status(lngGetString(STATUS_READTRACK));

#ifdef DEBUG
	g_pLogDlg->print_line(_T("  Tracks (start, length):"));
	std::vector<CCore2TOCTrackDesc>::const_iterator it;
	for (it = Tracks.begin(); it != Tracks.end(); it++)
	{
		CCore2TrackInfo TrackInfo;
		if (Info.ReadTrackInformation(Device,CCore2Info::TIT_LBA,(*it).m_ulTrackAddr,&TrackInfo))
		{
			unsigned long ulMin = (TrackInfo.m_ulTrackSize + 150)/(60*75);
			unsigned long ulSec = (TrackInfo.m_ulTrackSize + 150 - ulMin * 60 * 75)/75;
			unsigned long ulFrame = TrackInfo.m_ulTrackSize + 150 - ulMin * 60 * 75 - ulSec * 75;
			g_pLogDlg->print_line(_T("    %d, %02d:%02d:%02d (%d)"),(*it).m_ulTrackAddr,ulMin,ulSec,ulFrame,TrackInfo.m_ulTrackSize);
		}
	}
#endif

	ckcore::FileOutStream OutStream(szFilePath);
	if (!OutStream.open())
	{
		g_pLogDlg->print_line(_T("  Error: Unable to open the output file: \"%s\"."),szFilePath);
		return false;
	}

	Core2ReadFunction::CReadUserData ReadFunc(Device,&OutStream);

	// Start reading the selected sectors from the disc.
	bool bResult = Read.ReadData(Device,pProgress,&ReadFunc,ulTrackAddr,ulTrackSize,bIgnoreErr);
	OutStream.close();

	if (bResult)
		pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_READTRACK),ucTrackNumber);

	return bResult;
}

/*
	CCore2::ReadFullTOC
	-------------------
	Reads the full TOC and saves the data into a cdrtools compatible file.
*/
bool CCore2::ReadFullTOC(ckmmc::Device &Device,const TCHAR *szFileName)
{
	// Initialize buffers.
	unsigned char ucBuffer[4 + 2048];		// It feels stupid to allocate this much memory when
											// only 2 bytes of data is needed. The problem is that
											// some drives (tested with TSSTCorp CD/DVDW SH-S183A SB00)
											// returns more data then requested which may cause buffer
											// overruns.
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_READ_TOC_PMA_ATIP;
	ucCdb[2] = 0x02;						// Full Q sub-code data.
	ucCdb[7] = sizeof(ucBuffer) >> 8;		// Allocation length (MSB).
	ucCdb[8] = sizeof(ucBuffer) & 0xFF;		// Allocation length (LSB).
	ucCdb[9] = 0;

	if (!Device.transport(ucCdb,10,ucBuffer,sizeof(ucBuffer),
						  ckmmc::Device::ckTM_READ))
	{
		return false;
	}

	// Save the data to the specified file name.
	ckcore::File File(szFileName);
	if (!File.open(ckcore::File::ckOPEN_WRITE))
	{
		g_pLogDlg->print_line(_T("  Error: Unable to open file \"%s\" for writing."),szFileName);
		return false;
	}

	unsigned short usDataLen = ((unsigned short)ucBuffer[0] << 8) | ucBuffer[1];
	if (File.write(ucBuffer,usDataLen + 2) == -1)
	{
		g_pLogDlg->print_line(_T("  Error: Unable to write to file \"%s\"."),szFileName);
		return false;
	}

	return true;
}

int CCore2::CreateImage(ckcore::OutStream &OutStream,ckfilesystem::FileSet &Files,
						ckcore::Progress &Progress,bool bFailOnError,
						std::map<tstring,tstring> *pFilePathMap)
{
	ckfilesystem::FileSystem::Type FileSysType;
	switch (g_ProjectSettings.m_iFileSystem)
	{
		case FILESYSTEM_ISO9660:
			FileSysType = g_ProjectSettings.m_bJoliet ?
				ckfilesystem::FileSystem::TYPE_ISO9660_JOLIET :
				ckfilesystem::FileSystem::TYPE_ISO9660;
			break;
				
		case FILESYSTEM_ISO9660_UDF:
			FileSysType = g_ProjectSettings.m_bJoliet ?
				ckfilesystem::FileSystem::TYPE_ISO9660_UDF_JOLIET : 
				ckfilesystem::FileSystem::TYPE_ISO9660_UDF;
				break;

		case FILESYSTEM_DVDVIDEO:
			FileSysType = ckfilesystem::FileSystem::TYPE_DVDVIDEO;
			break;

		case FILESYSTEM_UDF:
			FileSysType = ckfilesystem::FileSystem::TYPE_UDF;			
			break;

		default:
			ATLASSERT(false);
			throw CreateIrInternalError(_T(__FILE__),__LINE__);
	}

	ckfilesystem::Iso9660::InterLevel InterchangeLevel;
	switch (g_ProjectSettings.m_iIsoLevel)
	{
		case 0:
			InterchangeLevel = ckfilesystem::Iso9660::LEVEL_1;			
			break;

		case 1:
			InterchangeLevel = ckfilesystem::Iso9660::LEVEL_2;
			break;

		case 2:
			InterchangeLevel = ckfilesystem::Iso9660::LEVEL_3;
			break;

		case 3:
			InterchangeLevel = ckfilesystem::Iso9660::ISO9660_1999;
			break;

		default:
			ATLASSERT(false);
	}

	ckfilesystem::FileSystem FileSys(FileSysType,Files);

	FileSys.set_long_joliet_names(g_ProjectSettings.m_bJolietLongNames);
	FileSys.set_interchange_level(InterchangeLevel);
	FileSys.set_include_file_ver_info(!g_ProjectSettings.m_bOmitVerNum);
	FileSys.set_relax_max_dir_level(g_ProjectSettings.m_bDeepDirs);

	FileSys.set_volume_label(g_ProjectSettings.m_szLabel);
	FileSys.set_text_fields(g_ProjectSettings.m_szSystem,
		g_ProjectSettings.m_szVolumeSet,g_ProjectSettings.m_szPublisher,
		g_ProjectSettings.m_szPreparer);
	FileSys.set_file_fields(g_ProjectSettings.m_szCopyright,
		g_ProjectSettings.m_szAbstract,g_ProjectSettings.m_szBibliographic);

	std::list<CProjectBootImage *>::const_iterator itBootImage;
	for (itBootImage = g_ProjectSettings.m_BootImages.begin(); itBootImage !=
		g_ProjectSettings.m_BootImages.end(); itBootImage++)
	{
		switch ((*itBootImage)->m_iEmulation)
		{
			case PROJECTBI_BOOTEMU_NONE:
				FileSys.add_boot_image_no_emu((*itBootImage)->m_FullPath.c_str(),
					!(*itBootImage)->m_bNoBoot,(*itBootImage)->m_uiLoadSegment,(*itBootImage)->m_uiLoadSize);
				break;

			case PROJECTBI_BOOTEMU_FLOPPY:
				FileSys.add_boot_image_floppy((*itBootImage)->m_FullPath.c_str(),
					!(*itBootImage)->m_bNoBoot);
				break;

			case PROJECTBI_BOOTEMU_HARDDISK:
				FileSys.add_boot_image_hard_disk((*itBootImage)->m_FullPath.c_str(),
					!(*itBootImage)->m_bNoBoot);
				break;
		}
	}

	unsigned long ulSectorOffset = 0;
	if (g_ProjectSettings.m_bMultiSession)
		ulSectorOffset = (unsigned long)g_ProjectSettings.m_uiNextWritableAddr;

	ckfilesystem::FileSystemWriter FileSysWriter(*g_pLogDlg,FileSys,bFailOnError);
	int iResult = FileSysWriter.write(OutStream,Progress,ulSectorOffset);

	if (pFilePathMap != NULL)
		FileSysWriter.file_path_map(*pFilePathMap);

	return iResult;
}

/*
	A wrapper method for the function above.
*/
int CCore2::CreateImage(const TCHAR *szFullPath,ckfilesystem::FileSet &Files,
						ckcore::Progress &Progress,bool bFailOnError,
						std::map<tstring,tstring> *pFilePathMap)
{
	ckcore::FileOutStream FileStream(szFullPath);
	if (!FileStream.open())
	{
		g_pLogDlg->print_line(_T("  Error: Unable to obtain file handle to \"%s\"."),szFullPath);
		return RESULT_FAIL;
	}

	return CreateImage(FileStream,Files,Progress,bFailOnError,pFilePathMap);
}

int CCore2::EstimateImageSize(ckfilesystem::FileSet &Files,ckcore::Progress &Progress,
							  unsigned __int64 &uiImageSize)
{
	ckcore::NullStream OutStream;
	int iResult = CreateImage(OutStream,Files,Progress,true);

	uiImageSize = OutStream.written();
	return iResult;
}