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
#include <ckcore/directory.hh>
#include "../../Common/XmlProcessor.h"
#include "Settings.h"
#include "cdrtoolsParseStrings.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "Core.h"
#include "DriveLetterDlg.h"
#include "Scsi.h"
#include "DeviceManager.h"

CDeviceManager g_DeviceManager;

CDeviceManager::CDeviceManager()
{
	Reset();
}

CDeviceManager::~CDeviceManager()
{
}

void CDeviceManager::Reset()
{
	m_iMode = -1;

	m_uiDeviceInfoCount = 0;
	m_uiDeviceInfoExCount = 0;
	m_uiDeviceCapCount = 0;

	// For verification.
	m_uiVeriCounter = 0;
	m_bVeriResult = true;
}

int CDeviceManager::GetDeviceType(const TCHAR *szTypeString)
{
	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_CDROM),CDRTOOLS_TYPE_CDROM_LENGTH))
		return DEVICEMANAGER_TYPE_CDROM;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_COMMUNICATION),CDRTOOLS_TYPE_COMMUNICATION_LENGTH))
		return DEVICEMANAGER_TYPE_COMMUNICATION;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_DISC),CDRTOOLS_TYPE_DISC_LENGTH))
		return DEVICEMANAGER_TYPE_DISC;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_JUKEBOX),CDRTOOLS_TYPE_JUKEBOX_LENGTH))
		return DEVICEMANAGER_TYPE_JUKEBOX;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_OPTICALSTORAGE),CDRTOOLS_TYPE_OPTICALSTORAGE_LENGTH))
		return DEVICEMANAGER_TYPE_OPTICALSTORAGE;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_PRINTER),CDRTOOLS_TYPE_PRINTER_LENGTH))
		return DEVICEMANAGER_TYPE_PRINTER;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_PROCESSOR),CDRTOOLS_TYPE_PROCESSOR_LENGTH))
		return DEVICEMANAGER_TYPE_PROCESSOR;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_SCANNER),CDRTOOLS_TYPE_SCANNER_LENGTH))
		return DEVICEMANAGER_TYPE_SCANNER;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_TAPE),CDRTOOLS_TYPE_TAPE_LENGTH))
		return DEVICEMANAGER_TYPE_TAPE;

	if (!lstrncmp(szTypeString,_T(CDRTOOLS_TYPE_WORM),CDRTOOLS_TYPE_WORM_LENGTH))
		return DEVICEMANAGER_TYPE_WORM;

	return DEVICEMANAGER_TYPE_UNKNOWN;
}

void CDeviceManager::ScanBusOutput(const char *szBuffer)
{
	// Make sure that we have capacity for adding more devices.
	if (m_uiDeviceInfoCount >= DEVICEMANAGER_MAXDEVICES)
		return;

	if (szBuffer[0] == '\t')
	{
		int iBus,iTarget,iLun;
		iBus = iTarget = iLun = 0;

		char szMultiInfo[DEVICEMANAGER_MAXLINESIZE];
		sscanf(szBuffer,"\t%d,%d,%d\t%*d) %[^\0]",
			&iBus,&iTarget,&iLun,szMultiInfo);

		// Skip asterisks.
		if (szMultiInfo[0] == '*')
			return;

		// Skip unaccessable devices.
		if (szMultiInfo[0] == '?')
			return;

		// Skip host adaptor information.
		if (szMultiInfo[0] == 'H')
			return;

		// Skip nameless devices (usually flagged as NON CCS disks).
		if (!strncmp(szMultiInfo,"'' '' ''",8))
			return;

		// Add a new entry to the device list.
		tDeviceInfo *pDeviceInfo = &m_DeviceInfo[m_uiDeviceInfoCount++];
		memset(pDeviceInfo,0,sizeof(tDeviceInfo));

		pDeviceInfo->Address.m_iBus = iBus;
		pDeviceInfo->Address.m_iTarget = iTarget;
		pDeviceInfo->Address.m_iLun = iLun;

		// Convert the information to UTF-16 if necessary.
		TCHAR szInfo[DEVICEMANAGER_MAXLINESIZE];
#ifdef UNICODE
		AnsiToUnicode(szInfo,szMultiInfo,sizeof(szInfo) / sizeof(wchar_t));
#else
		strcpy(szInfo,szMultiInfo);
#endif

		// Split the info line (sizes taken from scsireg.h).
		TCHAR szType[68];	// Largest string: "Removable vendor specific 4294967295 unknown device type 0xFFFFFFFF"
		lsscanf(szInfo,_T("'%[^']' '%[^']' '%[^']' %[^\0]"),
			pDeviceInfo->szVendor,
			pDeviceInfo->szIdentification,
			pDeviceInfo->szRevision,
			szType);

		TCHAR *pType = szType;

		// See if the device is removable and in that case temove the "Removable " prefix.
		if (!lstrncmp(szType,_T(CDRTOOLS_REMOVABLE),CDRTOOLS_REMOVABLE_LENGTH))
		{
			pType += CDRTOOLS_REMOVABLE_LENGTH;
			pDeviceInfo->bRemovable = true;
		}

		// Parse the type string.
		pDeviceInfo->iType = GetDeviceType(pType);

		// Remove unnecesary white-spaces.
		TrimRight(pDeviceInfo->szVendor);
		TrimRight(pDeviceInfo->szIdentification);
		TrimRight(pDeviceInfo->szRevision);

		// Get the device drive letter.
		pDeviceInfo->Address.m_cDriveLetter = '\0';
		if (pDeviceInfo->iType == DEVICEMANAGER_TYPE_CDROM)
		{
			bool bFoundDriveLetter = true;
			if (!CCore2DriverSpti::GetDriveLetter(pDeviceInfo->Address.m_iBus,pDeviceInfo->Address.m_iTarget,
				pDeviceInfo->Address.m_iLun,pDeviceInfo->Address.m_cDriveLetter))
			{
				// This is not water proof. External USB and FireWire devices will fail the above
				// function call because USB and FireWire devices can't return an address on the
				// requested form since the USB and FireWire bus can contain multiple devices.
				// In a second attempt to locate the drive a search is performed by the device name.
				// If two identical devices are connected (same revision) to the system there will
				// be a conflict. Maybe I can solve this by using the ASPI driver?
				if (!CCore2DriverSpti::GetDriveLetter(pDeviceInfo->szVendor,pDeviceInfo->szIdentification,
					pDeviceInfo->szRevision,pDeviceInfo->Address.m_cDriveLetter))
				{
					// Ask for the user to specify a drive letter.
					TCHAR szTitle[128];
					lsnprintf_s(szTitle,128,lngGetString(DRIVELETTER_TITLE),pDeviceInfo->szVendor,
						pDeviceInfo->szIdentification,pDeviceInfo->szRevision);

					CDriveLetterDlg DriveLetterDlg;
					if (DriveLetterDlg.DoModal(::GetActiveWindow(),(LPARAM)szTitle) == IDOK)
					{
						pDeviceInfo->Address.m_cDriveLetter = DriveLetterDlg.GetDriveLetter();
						bFoundDriveLetter = true;
					}
					else
					{
						bFoundDriveLetter = false;
					}
				}
			}

			// If we can't find a drive letter, remove the device.
			if (!bFoundDriveLetter)
				m_uiDeviceInfoCount--;
		}
	}
}

void CDeviceManager::DevicesOutput(const char *szBuffer)
{
	// Make sure that we have capacity for adding more devices.
	if (m_uiVeriCounter >= DEVICEMANAGER_MAXDEVICES)
		return;

	if (szBuffer[0] == ' ')
	{
		szBuffer += 4;

		// Add a new entry to the device list.
		tDeviceInfo *pDeviceInfo = &m_DeviceInfo[m_uiDeviceInfoCount++];
		memset(pDeviceInfo,0,sizeof(tDeviceInfo));

		char cDriveLetter = NULL;
		char szInfo[DEVICEMANAGER_MAXLINESIZE];

		sscanf(szBuffer,"dev='%c:'\t%[^\0]",&cDriveLetter,szInfo);
		char *pInfo = szInfo + 9;

		pDeviceInfo->Address.m_cDriveLetter = cDriveLetter;

		// Convert the information to UTF-16 if necessary.
		TCHAR szAutoInfo[DEVICEMANAGER_MAXLINESIZE];
#ifdef UNICODE
		AnsiToUnicode(szAutoInfo,pInfo,sizeof(szAutoInfo) / sizeof(TCHAR));
#else
		strcpy(szAutoInfo,pInfo);
#endif

		TCHAR szSkip[68];
		lsscanf(szAutoInfo,_T("'%[^']' '%[^']' '%[^']' %[^\0]"),
			pDeviceInfo->szVendor,
			pDeviceInfo->szIdentification,
			pDeviceInfo->szRevision,
			szSkip);

		// Remove unnecesary white-spaces.
		TrimRight(pDeviceInfo->szVendor);
		TrimRight(pDeviceInfo->szIdentification);
		TrimRight(pDeviceInfo->szRevision);

		pDeviceInfo->bRemovable = true;
		pDeviceInfo->iType = DEVICEMANAGER_TYPE_CDROM;

		// Get the device address.
		if (!CCore2DriverSpti::GetDriveAddress(pDeviceInfo->Address.m_cDriveLetter,
			pDeviceInfo->Address.m_iBus,
			pDeviceInfo->Address.m_iTarget,
			pDeviceInfo->Address.m_iLun))
		{
			pDeviceInfo->Address.m_iBus = -1;
			pDeviceInfo->Address.m_iTarget = -1;
			pDeviceInfo->Address.m_iLun = -1;

			// If we can't find a drive letter, remove the device.
			//m_uiDeviceInfoCount--;
		}
	}
}

/*
	CDeviceManager::LoadCapParseDoes
	--------------------------------
	Parses the specified buffer to find out which feature that the
	"does-statement" refers to.
*/
void CDeviceManager::LoadCapParseDoes(const char *szBuffer,
									   tDeviceCap *pDeviceCap)
{
	char *pBuffer = (char *)szBuffer;

	if (!strncmp(pBuffer,"read",4))
	{
		pBuffer += 5;

		// Read and write.
		if (!strncmp(pBuffer,CDRTOOLS_CAP_CDR,CDRTOOLS_CAP_CDR_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_READCDR;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_CDRW,CDRTOOLS_CAP_CDRW_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_READCDRW;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_DVDROM,CDRTOOLS_CAP_DVDROM_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_READDVDROM;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_DVDR,CDRTOOLS_CAP_DVDR_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_READDVDR;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_DVDRAM,CDRTOOLS_CAP_DVDRAM_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_READDVDRAM;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_MODE2FORM1,CDRTOOLS_CAP_MODE2FORM1_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READMODE2FORM1;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_MODE2FORM2,CDRTOOLS_CAP_MODE2FORM2_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READMODE2FORM2;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_DIGITALAUDIO,CDRTOOLS_CAP_DIGITALAUDIO_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READDIGITALAUDIO;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_MULTISESSION,CDRTOOLS_CAP_MULTISESSION_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READMULTISESSION;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_FIXEDPACKET,CDRTOOLS_CAP_FIXEDPACKET_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READFIXEDPACKET;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_CDBARCODE,CDRTOOLS_CAP_CDBARCODE_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READCDBARCODE;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_RWSUBCODE,CDRTOOLS_CAP_RWSUBCODE_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READRWSUBCODE;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_RAWPWSUBCODE,CDRTOOLS_CAP_RAWPWSUBCODE_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_READRAWPWSUBCODE;
	}
	else if (!strncmp(pBuffer,"write",5))
	{
		pBuffer += 6;

		if (!strncmp(pBuffer,CDRTOOLS_CAP_CDR,CDRTOOLS_CAP_CDR_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_WRITECDR;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_CDRW,CDRTOOLS_CAP_CDRW_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_WRITECDRW;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_DVDR,CDRTOOLS_CAP_DVDR_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_WRITEDVDR;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_DVDRAM,CDRTOOLS_CAP_DVDRAM_LENGTH))
			pDeviceCap->uiMedia |= DEVICEMANAGER_CAP_WRITEDVDRAM;
	}
	else if (!strncmp(pBuffer,"support",7))
	{
		pBuffer += 8;

		if (!strncmp(pBuffer,CDRTOOLS_CAP_TESTWRITING,CDRTOOLS_CAP_TESTWRITING_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_TESTWRITING;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_BUFRECORDING,CDRTOOLS_CAP_BUFRECORDING_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_BUFRECORDING;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_C2EP,CDRTOOLS_CAP_C2EP_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_C2EP;
		else if ((pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_PLAYAUDIOCD) &&
				 !strncmp(pBuffer,CDRTOOLS_CAP_INDIVIDUALVC,CDRTOOLS_CAP_INDIVIDUALVC_LENGTH))
			pDeviceCap->uiAudio |= DEVICEMANAGER_CAP_INDIVIDUALVC;
		else if ((pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_PLAYAUDIOCD) &&
				 !strncmp(pBuffer,CDRTOOLS_CAP_INDEPENDENTMUTE,CDRTOOLS_CAP_INDEPENDENTMUTE_LENGTH))
			pDeviceCap->uiAudio |= DEVICEMANAGER_CAP_INDEPENDENTMUTE;
		else if ((pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_PLAYAUDIOCD) &&
				 !strncmp(pBuffer,CDRTOOLS_CAP_DOPORT1,CDRTOOLS_CAP_DOPORT1_LENGTH))
			pDeviceCap->uiAudio |= DEVICEMANAGER_CAP_DOPORT1;
		else if ((pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_PLAYAUDIOCD) &&
				 !strncmp(pBuffer,CDRTOOLS_CAP_DOPORT2,CDRTOOLS_CAP_DOPORT2_LENGTH))
			pDeviceCap->uiAudio |= DEVICEMANAGER_CAP_DOPORT2;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_EJECTCDSS,CDRTOOLS_CAP_EJECTCDSS_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_EJECTCDSS;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_CHANGEDISCSIDE,CDRTOOLS_CAP_CHANGEDISCSIDE_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_CHANGEDISCSIDE;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_INDIVIDUALDP,CDRTOOLS_CAP_INDIVIDUALDP_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_INDIVIDUALDP;
	}
	else if (!strncmp(pBuffer,"return",6))
	{
		pBuffer += 7;

		if ((pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READRWSUBCODE) &&
			!strncmp(pBuffer,CDRTOOLS_CAP_RWSUBCODE,CDRTOOLS_CAP_RWSUBCODE_LENGTH))
			pDeviceCap->uiRW |= DEVICEMANAGER_CAP_RETURNRWSUBCODE;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_CDCATALOGNUMBER,CDRTOOLS_CAP_CDCATALOGNUMBER_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_RETURNCDCN;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_CDISRCINFO,CDRTOOLS_CAP_CDISRCINFO_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_RETURNCDISRC;
	}
	else if (!strncmp(pBuffer,"have ",5))
	{
		pBuffer += 6;

		if (!strncmp(pBuffer,CDRTOOLS_CAP_VALIDDATA,CDRTOOLS_CAP_VALIDDATA_LENGTH))
			pDeviceCap->uiDigitalOutput |= DEVICEMANAGER_CAP_HASVALIDDATA;
		else if (!strncmp(pBuffer,CDRTOOLS_CAP_LESIC,CDRTOOLS_CAP_LESIC_LENGTH))
			pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_HASLESIC;
	}
	// Not prefixed.
	else if ((pDeviceCap->uiGeneral & DEVICEMANAGER_CAP_READDIGITALAUDIO) &&
			 !strncmp(pBuffer,CDRTOOLS_CAP_NSDARA,CDRTOOLS_CAP_NSDARA_LENGTH))
		pDeviceCap->uiDigitalAudio |= DEVICEMANAGER_CAP_RESTARTNSDARA;
	else if (!strncmp(pBuffer,CDRTOOLS_CAP_COMPOSITEAVDATA,CDRTOOLS_CAP_COMPOSITEAVDATA_LENGTH))
		pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_DELIVERCOMPOSITE;
	else if (!strncmp(pBuffer,CDRTOOLS_CAP_AUDIOCD,CDRTOOLS_CAP_AUDIOCD_LENGTH))
		pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_PLAYAUDIOCD;
	else if (((pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT1) ||
			 (pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT2)) && 
			 !strncmp(pBuffer,CDRTOOLS_CAP_SENDDIGDAT,CDRTOOLS_CAP_SENDDIGDAT_LENGTH))
		pDeviceCap->uiDigitalOutput |= DEVICEMANAGER_CAP_DOSENDDIGDAT;
	else if (((pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT1) ||
			 (pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT2)) &&
			 !strncmp(pBuffer,CDRTOOLS_CAP_SETLRCK,CDRTOOLS_CAP_SETLRCK_LENGTH))
		pDeviceCap->uiDigitalOutput |= DEVICEMANAGER_CAP_DOSETLRCK;
	else if (!strncmp(pBuffer,CDRTOOLS_CAP_LMOPU,CDRTOOLS_CAP_LMOPU_LENGTH))
		pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_LMOPU;
	else if (!strncmp(pBuffer,CDRTOOLS_CAP_ALLOWML,CDRTOOLS_CAP_ALLOWML_LENGTH))
		pDeviceCap->uiGeneral |= DEVICEMANAGER_CAP_ALLOWML;
}

void CDeviceManager::LoadCapOutput(const char *szBuffer)
{
	// Make sure that we have capacity for adding more information.
	if (m_uiDeviceCapCount >= DEVICEMANAGER_MAXDEVICES)
		return;

	// We only parse information strings starting with two white-spaces.
	if (szBuffer[0] == ' ' && szBuffer[1] == ' ')
	{
		char *pBuffer = (char *)szBuffer + 2;

		// Add a new entry to the device capabilities list.
		tDeviceCap *pDeviceCap = &m_DeviceCap[m_uiDeviceCapCount];

		if (!strncmp(pBuffer,"Does",4))
		{
			pBuffer += 5;
			bool bDoes = true;

			if (!strncmp(pBuffer,"not",3))
			{
				pBuffer += 4;
				bDoes = false;
			}

			if (bDoes)
				LoadCapParseDoes(pBuffer,pDeviceCap);
		}
		else if (!strncmp(pBuffer,"Number of",9))
		{
			pBuffer += 10;

			if (!strncmp(pBuffer,CDRTOOLS_CAP_NUMVCL,CDRTOOLS_CAP_NUMVCL_LENGTH))
				pDeviceCap->uiNumVolLevels = atoi(pBuffer + 23);
		}
		else if (!strncmp(pBuffer,"Buffer",6))		// "Buffer size in KB: "
		{
			pBuffer += 7;
			pDeviceCap->uiBufferSize = atoi(pBuffer + 12);
		}
		else if (!strncmp(pBuffer,"Copy",4))		// "Copy management revision supported: "
		{
			pBuffer += 5;
			pDeviceCap->uiCopyRevision = atoi(pBuffer + 32);
		}
		else if (!strncmp(pBuffer,"Load",4))		// "Loading mechanism type: "
		{
			pBuffer += 20;

			if (!strcmp(pBuffer,"tray"))
				pDeviceCap->ucLoadType = DEVICEMANAGER_LMT_TRAY;
			else
				pDeviceCap->ucLoadType = DEVICEMANAGER_LMT_CADDY;
		}
		else if (!strncmp(pBuffer,"Rot",3))			// "Rotational control selected: "
		{
			pBuffer += 26;
			
			// Can be found in: scsi_cdr.c
			if (!strcmp(pBuffer,"CLV/PCAV"))
				pDeviceCap->ucRotCtrl = DEVICEMANAGER_ROTCTRL_CLVPCAV;
			else if (!strcmp(pBuffer,"CAV"))
				pDeviceCap->ucRotCtrl =  DEVICEMANAGER_ROTCTRL_CAV;
			else if (!strcmp(pBuffer,"reserved(2)"))
				pDeviceCap->ucRotCtrl =  DEVICEMANAGER_ROTCTRL_RESERVED2;
			else
				pDeviceCap->ucRotCtrl =  DEVICEMANAGER_ROTCTRL_RESERVED3;
		}
		else if (((pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT1) ||
				 (pDeviceCap->uiAudio & DEVICEMANAGER_CAP_DOPORT2)) &&
				 !strncmp(pBuffer,"Length",6))		// "Length of data in BCLKs: "
		{	
			pBuffer += 19;

			// Can be found in: scsi_cdr.c
			if (!strcmp(pBuffer,"32"))
				pDeviceCap->ucDigitalAudioBL = DEVICEMANAGER_DABL_32;
			else if (!strcmp(pBuffer,"16"))
				pDeviceCap->ucDigitalAudioBL = DEVICEMANAGER_DABL_16;
			else if (!strcmp(pBuffer,"24"))
				pDeviceCap->ucDigitalAudioBL = DEVICEMANAGER_DABL_24;
			else
				pDeviceCap->ucDigitalAudioBL = DEVICEMANAGER_DABL_24I2S;
		}
		else if (!strncmp(pBuffer,"Maximum",7))		// "Maximum read/write  speed: "
		{
			pBuffer += 8;
			unsigned char ucOffset = 0;

			if (!strncmp(pBuffer,"read",4))
				ucOffset = 3;

			pBuffer += 13;

			unsigned int uiMaxKB,uiMaxCD,uiMaxDVD;
			uiMaxKB = uiMaxCD = uiMaxDVD = 0;
			sscanf(pBuffer,"%5d kB/s (CD %3dx, DVD %2dx)",&uiMaxKB,&uiMaxCD,&uiMaxDVD);

			pDeviceCap->uiMaxSpeeds[ucOffset    ] = uiMaxKB;
			pDeviceCap->uiMaxSpeeds[ucOffset + 1] = uiMaxCD;
			pDeviceCap->uiMaxSpeeds[ucOffset + 2] = uiMaxDVD;
		}
		else if (!strncmp(pBuffer,"Current ",8))	// "Current read/write  speed: "
		{
			pBuffer += 8;
			unsigned char ucOffset = 0;

			if (!strncmp(pBuffer,"read",4))
				ucOffset = 3;

			pBuffer += 13;

			unsigned int uiCurKB,uiCurCD,uiCurDVD;
			uiCurKB = uiCurCD = uiCurDVD = 0;
			sscanf(pBuffer,"%5d kB/s (CD %3dx, DVD %2dx)",&uiCurKB,&uiCurCD,&uiCurDVD);

			pDeviceCap->uiCurSpeeds[ucOffset    ] = uiCurKB;
			pDeviceCap->uiCurSpeeds[ucOffset + 1] = uiCurCD;
			pDeviceCap->uiCurSpeeds[ucOffset + 2] = uiCurDVD;
		}
#ifdef DEVICEMANAGER_COLLECTWRITESPEEDS
		else if (!strncmp(pBuffer,"Write",5))		// "Write speed # "
		{
			pBuffer += 14;

			unsigned int uiKB,uiCD,uiDVD;
			uiKB = uiCD = uiDVD = 0;
			sscanf(pBuffer,"%*d: %5d kB/s %*s (CD %3ux, DVD %2ux)",&uiKB,&uiCD,&uiDVD);

			pDeviceCap->uiWriteSpeeds[pDeviceCap->uiWriteSpeedsCount][0] = uiKB;
			pDeviceCap->uiWriteSpeeds[pDeviceCap->uiWriteSpeedsCount][1] = uiCD;
			pDeviceCap->uiWriteSpeeds[pDeviceCap->uiWriteSpeedsCount][2] = uiDVD;
			pDeviceCap->uiWriteSpeedsCount++;
		}
#endif
	}
}

void CDeviceManager::LoadExInfoOutput(const char *szBuffer)
{
	// Make sure that we have capacity for adding more information.
	if (m_uiDeviceInfoExCount >= DEVICEMANAGER_MAXDEVICES)
		return;

	if (!strncmp(szBuffer,CDRTOOLS_WRITEFLAGS,CDRTOOLS_WRITEFLAGS_LENGTH))
	{
		tDeviceInfoEx *pDeviceInfoEx = &m_DeviceInfoEx[m_uiDeviceInfoExCount];

		char *pBuffer = (char *)szBuffer + 17;
		strcpy(pDeviceInfoEx->szWriteFlags,pBuffer);
	}
	else if (!strncmp(szBuffer,CDRTOOLS_WRITEMODES,CDRTOOLS_WRITEMODES_LENGTH))
	{
		tDeviceInfoEx *pDeviceInfoEx = &m_DeviceInfoEx[m_uiDeviceInfoExCount];

		char *pBuffer = (char *)szBuffer + 17;
		strcpy(pDeviceInfoEx->szWriteModes,pBuffer);
		strcat(pDeviceInfoEx->szWriteModes," ");		// For easier parsing.
	}
}

void CDeviceManager::VerifyBusOutput(const char *szBuffer)
{
	// Make sure that we have capacity for adding more devices.
	if (m_uiVeriCounter >= DEVICEMANAGER_MAXDEVICES)
		return;

	if (szBuffer[0] == '\t')
	{
		int iBus,iTarget,iLun;
		iBus = iTarget = iLun = 0;

		char szMultiInfo[DEVICEMANAGER_MAXLINESIZE];
		sscanf(szBuffer,"\t%d,%d,%d\t%*d) %[^\0]",
			&iBus,&iTarget,&iLun,szMultiInfo);

		// Skip asterisks.
		if (szMultiInfo[0] == '*')
			return;

		// Skip unaccessable devices.
		if (szMultiInfo[0] == '?')
			return;

		// Skip host adaptor information.
		if (szMultiInfo[0] == 'H')
			return;

		// Verify the device bus information.
		if ((m_DeviceInfo[m_uiVeriCounter].Address.m_iBus != iBus) ||
			(m_DeviceInfo[m_uiVeriCounter].Address.m_iTarget != iTarget) ||
			(m_DeviceInfo[m_uiVeriCounter].Address.m_iLun != iLun))
		{
			m_bVeriResult = false;
			m_uiVeriCounter++;
			return;
		}

		// Convert the information to UTF-16 if necessary.
		TCHAR szInfo[DEVICEMANAGER_MAXLINESIZE];
#ifdef UNICODE
		AnsiToUnicode(szInfo,szMultiInfo,sizeof(szInfo) / sizeof(wchar_t));
#else
		strcpy(szInfo,szMultiInfo);
#endif

		TCHAR szVendor[9];
		TCHAR szIdentification[17];
		TCHAR szRevision[5];

		// Split the info line (sizes taken from scsireg.h).
		lsscanf(szInfo,_T("'%[^']' '%[^']' '%[^']' %*[^\0]"),
			szVendor,
			szIdentification,
			szRevision);

		// Remove unnecesary white-spaces.
		TrimRight(szVendor);
		TrimRight(szIdentification);
		TrimRight(szRevision);

		// Verify the device identification, in case a new device has been
		// attached to the same address as a previous.
		if (lstrcmp(szVendor,m_DeviceInfo[m_uiVeriCounter].szVendor) ||
			lstrcmp(szIdentification,m_DeviceInfo[m_uiVeriCounter].szIdentification) ||
			lstrcmp(szRevision,m_DeviceInfo[m_uiVeriCounter].szRevision))
		{
			m_bVeriResult = false;
		}

		m_uiVeriCounter++;
	}
}

void CDeviceManager::VerifyDevicesOutput(const char *szBuffer)
{
	// Make sure that we have capacity for adding more devices.
	if (m_uiVeriCounter >= DEVICEMANAGER_MAXDEVICES)
		return;

	if (szBuffer[0] == ' ')
	{
		szBuffer += 4;

		char cDriveLetter = NULL;
		char szInfo[DEVICEMANAGER_MAXLINESIZE];

		sscanf(szBuffer,"dev='%c:'\t%[^\0]",&cDriveLetter,szInfo);
		char *pInfo = szInfo + 9;

		// Convert the information to UTF-16 if necessary.
		TCHAR szAutoInfo[DEVICEMANAGER_MAXLINESIZE];
#ifdef UNICODE
		AnsiToUnicode(szAutoInfo,pInfo,sizeof(szAutoInfo) / sizeof(TCHAR));
#else
		strcpy(szAutoInfo,pInfo);
#endif

		TCHAR szSkip[68],szVendor[9],szIdentification[17],szRevision[5];
		szSkip[0] = szVendor[0] = szIdentification[0] = szRevision[0] = '\0';

		lsscanf(szAutoInfo,_T("'%[^']' '%[^']' '%[^']' %[^\0]"),
			szVendor,szIdentification,szRevision,szSkip);

		// Remove unnecesary white-spaces.
		TrimRight(szVendor);
		TrimRight(szIdentification);
		TrimRight(szRevision);

		int iBus = -1,iTarget = -1,iLun = -1;

		// Get the device address.
		CCore2DriverSpti::GetDriveAddress(cDriveLetter,iBus,iTarget,iLun);

		// Verify the device bus information.
		if ((m_DeviceInfo[m_uiVeriCounter].Address.m_iBus != iBus) ||
			(m_DeviceInfo[m_uiVeriCounter].Address.m_iTarget != iTarget) ||
			(m_DeviceInfo[m_uiVeriCounter].Address.m_iLun != iLun))
		{
			m_bVeriResult = false;
			m_uiVeriCounter++;
			return;
		}

		// Verify the device identification, in case a new device has been
		// attached to the same address as a previous.
		if (lstrcmp(szVendor,m_DeviceInfo[m_uiVeriCounter].szVendor) ||
			lstrcmp(szIdentification,m_DeviceInfo[m_uiVeriCounter].szIdentification) ||
			lstrcmp(szRevision,m_DeviceInfo[m_uiVeriCounter].szRevision))
		{
			m_bVeriResult = false;
		}

		m_uiVeriCounter++;
	}
}

void CDeviceManager::event_output(const std::string &block)
{
	// Always skip the copyright line.
	if (!strncmp(block.c_str(),CDRTOOLS_COPYRIGHT,CDRTOOLS_COPYRIGHT_LENGTH))
		return;

	switch (m_iMode)
	{
		case MODE_SCANBUS:
			ScanBusOutput(block.c_str());
			break;

		case MODE_DEVICES:
			DevicesOutput(block.c_str());
			break;

		case MODE_LOADCAP:
			LoadCapOutput(block.c_str());
			break;

		case MODE_LOADEXINFO:
			LoadExInfoOutput(block.c_str());
			break;

		case MODE_VERIFYBUS:
			VerifyBusOutput(block.c_str());
			break;

		case MODE_VERIFYDEVICES:
			VerifyDevicesOutput(block.c_str());
			break;
	}
}

void CDeviceManager::event_finished()
{
	// We don't care if the process has ended.
}

bool CDeviceManager::ScanBus()
{
	// Reset the device information counter.
	m_uiDeviceInfoCount = 0;

	// Prepare command line.
	TCHAR szCommandLine[MAX_PATH];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T(CORE_WRITEAPP));

#ifdef CDRKIT_DEVICESCAN
	m_iMode = MODE_DEVICES;

	lstrcat(szCommandLine,_T("\" -devices"));
#else
	m_iMode = MODE_SCANBUS;

	lstrcat(szCommandLine,_T("\" -scanbus"));
#endif

	if (!create(szCommandLine))
		return false;

	wait();
	return true;
}

bool CDeviceManager::LoadCapabilities()
{
	m_iMode = MODE_LOADCAP;

	// Reset the device capabilities counter.
	m_uiDeviceCapCount = 0;

	TCHAR szCommandLine[MAX_PATH];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T(CORE_WRITEAPP));
	lstrcat(szCommandLine,_T("\" -prcap dev="));
	int iDevEnd = lstrlen(szCommandLine);

	for (unsigned int i = 0; i < GetDeviceCount(); i++)
	{
		tDeviceInfo *pDeviceInfo = GetDeviceInfo(i);

		// Clear the current capabilites data.
		memset(&m_DeviceCap[m_uiDeviceCapCount],0,sizeof(tDeviceCap));

		if (pDeviceInfo->iType == DEVICEMANAGER_TYPE_CDROM)
		{
			szCommandLine[iDevEnd] = '\0';

			TCHAR szDeviceAdr[32];
			GetDeviceAddr(pDeviceInfo,szDeviceAdr);
			lstrcat(szCommandLine,szDeviceAdr);

			// Launch the command line.
			if (!create(szCommandLine))
				return false;

			wait();
		}

		// Increase the counter.
		m_uiDeviceCapCount++;
	}

	return true;
}

bool CDeviceManager::LoadExInfo()
{
	m_iMode = MODE_LOADEXINFO;

	// Reset the device extended information counter.
	m_uiDeviceInfoExCount = 0;

	TCHAR szCommandLine[MAX_PATH];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T(CORE_WRITEAPP));
	lstrcat(szCommandLine,_T("\" -checkdrive dev="));

	int iDevEnd = lstrlen(szCommandLine);

	for (unsigned int i = 0; i < GetDeviceCount(); i++)
	{
		tDeviceInfo *pDeviceInfo = GetDeviceInfo(i);

		// Clear the current extended information data.
		memset(&m_DeviceInfoEx[m_uiDeviceInfoExCount],0,sizeof(tDeviceInfoEx));

		if (IsDeviceRecorder(i))
		{
			szCommandLine[iDevEnd] = '\0';

			TCHAR szDeviceAdr[32];
			GetDeviceAddr(pDeviceInfo,szDeviceAdr);
			lstrcat(szCommandLine,szDeviceAdr);

			// Launch the command line.
			if (!create(szCommandLine))
				return false;

			wait();
		}

		// Increase the counter.
		m_uiDeviceInfoExCount++;
	}

	return true;
}

unsigned int CDeviceManager::GetDeviceCount()
{
	return m_uiDeviceInfoCount;
}

tDeviceInfo *CDeviceManager::GetDeviceInfo(UINT_PTR uiIndex)
{
	return &m_DeviceInfo[uiIndex];
}

tDeviceInfoEx *CDeviceManager::GetDeviceInfoEx(UINT_PTR uiIndex)
{
	return &m_DeviceInfoEx[uiIndex];
}

tDeviceCap *CDeviceManager::GetDeviceCap(UINT_PTR uiIndex)
{
	return &m_DeviceCap[uiIndex];
}

bool CDeviceManager::IsDeviceReader(UINT_PTR uiIndex)
{
	return m_DeviceInfo[uiIndex].iType == DEVICEMANAGER_TYPE_CDROM;
}

/*
	CDeviceManager::IsDeviceRecorder
	--------------------------------
	Returns true if the specified device has recording capabilities.
*/
bool CDeviceManager::IsDeviceRecorder(UINT_PTR uiIndex)
{
	if (!IsDeviceReader(uiIndex))
		return false;

	tDeviceCap *pDeviceCap = &m_DeviceCap[uiIndex];

	return (pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITECDR) ||
		   (pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITECDRW) ||
		   (pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDR) ||
		   (pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDRAM);
}

/*
	CDeviceManager::IsDeviceDVDRecorder
	-----------------------------------
	Returns true if the specified device has DVD recording capabilities.
*/
bool CDeviceManager::IsDeviceDVDRecorder(UINT_PTR uiIndex)
{
	if (!IsDeviceReader(uiIndex))
		return false;

	tDeviceCap *pDeviceCap = &m_DeviceCap[uiIndex];

	return (pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDR) ||
		   (pDeviceCap->uiMedia & DEVICEMANAGER_CAP_WRITEDVDRAM);
}

void CDeviceManager::GetDeviceName(tDeviceInfo *pDeviceInfo,TCHAR *szDeviceName)
{
	lsprintf(szDeviceName,_T("%C: %s %s %s"),
		pDeviceInfo->Address.m_cDriveLetter,
		pDeviceInfo->szVendor,
		pDeviceInfo->szIdentification,
		pDeviceInfo->szRevision);
}

void CDeviceManager::GetDeviceAddr(tDeviceInfo *pDeviceInfo,TCHAR *szDeviceAddr)
{
#ifdef CDRKIT_DEVICESCAN
	lsprintf(szDeviceAddr,_T("%c:"),(char)pDeviceInfo->Address.m_cDriveLetter);
#else
	lsprintf(szDeviceAddr,_T("%d,%d,%d"),pDeviceInfo->Address.m_iBus,
		pDeviceInfo->Address.m_iTarget,pDeviceInfo->Address.m_iLun);
#endif
}

void CDeviceManager::GetDeviceAddrA(tDeviceInfo *pDeviceInfo,char *szDeviceAddr)
{
#ifdef CDRKIT_DEVICESCAN
	sprintf(szDeviceAddr,"%c:",(char)pDeviceInfo->Address.m_cDriveLetter);
#else
	sprintf(szDeviceAddr,"%d,%d,%d",pDeviceInfo->Address.m_iBus,
		pDeviceInfo->Address.m_iTarget,pDeviceInfo->Address.m_iLun);
#endif
}

/*
	CDeviceManager::GetDeviceSpeeds
	-------------------------------
	Fills the specified combo box with different read or write speeds up to the
	maximum speed.

	NOTE: The function is obsolete and should not be used. Safe to remove?
*/
/*void CDeviceManager::GetDeviceSpeeds(UINT_PTR uiIndex,CComboBox *pComboBox,
									 bool bWriteSpeeds,bool bDVDSpeeds)
{
	tDeviceCap *pDeviceCap = g_DeviceManager.GetDeviceCap(uiIndex);

	pComboBox->AddString(lngGetString(MISC_MAXIMUM));
	pComboBox->SetItemData(0,-1);		// -1 = Maximum.

	if (bDVDSpeeds)
	{
		// Maximum speed.
		unsigned int uiBaseIndex = bWriteSpeeds ? 0 : 3;

		TCHAR szBuffer[128];
		lsprintf(szBuffer,_T("%dx (DVD)"),
			pDeviceCap->uiMaxSpeeds[uiBaseIndex + 2]);
		pComboBox->AddString(szBuffer);
		pComboBox->SetItemData(1,-1);		// -1 = Maximum.

		unsigned int iCurSpeed = 9;
		while (iCurSpeed < pDeviceCap->uiMaxSpeeds[uiBaseIndex + 1])
		{
			lsprintf(szBuffer,_T("%dx (DVD)"),iCurSpeed/9);
			pComboBox->InsertString(2,szBuffer);
			pComboBox->SetItemData(2,iCurSpeed/9);

			iCurSpeed += 9;
		}
	}
	else
	{
		// Maximum speed.
		unsigned int uiBaseIndex = bWriteSpeeds ? 0 : 3;

		TCHAR szBuffer[128];
		lsprintf(szBuffer,_T("%dx (CD)"),
			pDeviceCap->uiMaxSpeeds[uiBaseIndex + 1]);
		pComboBox->AddString(szBuffer);
		pComboBox->SetItemData(1,-1);		// -1 = Maximum.

		unsigned int iCurSpeed = 1;
		while (iCurSpeed < pDeviceCap->uiMaxSpeeds[uiBaseIndex + 1])
		{
			lsprintf(szBuffer,_T("%dx (CD)"),iCurSpeed);
			pComboBox->InsertString(2,szBuffer);
			pComboBox->SetItemData(2,iCurSpeed);

			if (iCurSpeed == 1)
				iCurSpeed = 2;
			else if (iCurSpeed < 12)
				iCurSpeed += 2;
			else if (iCurSpeed < 24)
				iCurSpeed += 4;
			else
				iCurSpeed += 8;
		}
	}
}*/

bool CDeviceManager::GetConfigPath(TCHAR *szConfigPath)
{
#ifdef PORTABLE
    GetModuleFileName(NULL,szConfigPath,MAX_PATH - 1);
	ExtractFilePath(szConfigPath);
#else
#ifdef UNICODE
	if (!SUCCEEDED(SHGetFolderPath(HWND_DESKTOP,CSIDL_APPDATA | CSIDL_FLAG_CREATE,NULL,
		SHGFP_TYPE_CURRENT,szConfigPath)))
		return false;
#else	// Win 9x.
	if (!SUCCEEDED(SHGetSpecialFolderPath(HWND_DESKTOP,szConfigPath,CSIDL_APPDATA,true)))
		return false;
#endif
	IncludeTrailingBackslash(szConfigPath);
	lstrcat(szConfigPath,_T("InfraRecorder\\"));

	// Create the file path if it doesn't exist.
	ckcore::Directory::create(szConfigPath);
#endif

	lstrcat(szConfigPath,_T("Devices.xml"));
	return true;
}

bool CDeviceManager::SaveConfiguration()
{
	// If the device manager doesn't contain any devices there is not need to
	// save anything.
	if (GetDeviceCount() == 0)
		return false;

	CXmlProcessor Xml;
	Xml.AddElement(_T("InfraRecorder"),_T(""),true);
		Xml.AddElement(_T("Devices"),_T(""),true);
		Xml.AddElementAttr(_T("count"),(long)GetDeviceCount());
			
		// Add all devices.
		TCHAR szElemName[32];

		for (unsigned int i = 0; i < GetDeviceCount(); i++)
		{
			tDeviceInfo *pDeviceInfo = GetDeviceInfo(i);
			tDeviceInfoEx *pDeviceInfoEx = GetDeviceInfoEx(i);
			tDeviceCap *pDeviceCap = GetDeviceCap(i);

			bool bIsRecorder = IsDeviceRecorder(i);

			lsprintf(szElemName,_T("Device%d"),i);
			Xml.AddElement(szElemName,_T(""),true);
				Xml.AddElementAttr(_T("type"),pDeviceInfo->iType);
				Xml.AddElementAttr(_T("removable"),pDeviceInfo->bRemovable);
				Xml.AddElementAttr(_T("letter"),(int)pDeviceInfo->Address.m_cDriveLetter);
	
				// General information.
				Xml.AddElement(_T("ID"),_T(""),true);
					Xml.AddElementAttr(_T("bus"),pDeviceInfo->Address.m_iBus);
					Xml.AddElementAttr(_T("target"),pDeviceInfo->Address.m_iTarget);
					Xml.AddElementAttr(_T("lun"),pDeviceInfo->Address.m_iLun);
					Xml.AddElement(_T("Vendor"),pDeviceInfo->szVendor);
					Xml.AddElement(_T("Identification"),pDeviceInfo->szIdentification);
					Xml.AddElement(_T("Revision"),pDeviceInfo->szRevision);
				Xml.LeaveElement();

			if (IsDeviceReader(i))
			{
				// Capabilities.
				Xml.AddElement(_T("Capabilities"),_T(""),true);
					Xml.AddElement(_T("Media"),(long)pDeviceCap->uiMedia);
					Xml.AddElement(_T("General"),(long)pDeviceCap->uiGeneral);
					Xml.AddElement(_T("DigitalAudio"),(long)pDeviceCap->uiDigitalAudio);
					Xml.AddElement(_T("RW"),(long)pDeviceCap->uiRW);

					Xml.AddElement(_T("Audio"),(long)pDeviceCap->uiAudio);
					Xml.AddElement(_T("DigitalOutput"),(long)pDeviceCap->uiDigitalOutput);
					Xml.AddElement(_T("NumVolLevels"),(long)pDeviceCap->uiNumVolLevels);

					Xml.AddElement(_T("BufferSize"),(long)pDeviceCap->uiBufferSize);
					Xml.AddElement(_T("LoadType"),(long)pDeviceCap->ucLoadType);
					Xml.AddElement(_T("RotationControl"),(long)pDeviceCap->ucRotCtrl);
					Xml.AddElement(_T("DigitalAudioBL"),(long)pDeviceCap->ucDigitalAudioBL);
					Xml.AddElement(_T("MaxSpeeds"),_T(""),true);
						if (bIsRecorder)
						{
							Xml.AddElement(_T("Write"),(long)pDeviceCap->uiMaxSpeeds[0],true);
								Xml.AddElementAttr(_T("cd"),(long)pDeviceCap->uiMaxSpeeds[1]);
								Xml.AddElementAttr(_T("dvd"),(long)pDeviceCap->uiMaxSpeeds[2]);
							Xml.LeaveElement();
						}
						Xml.AddElement(_T("Read"),(long)pDeviceCap->uiMaxSpeeds[3],true);
							Xml.AddElementAttr(_T("cd"),(long)pDeviceCap->uiMaxSpeeds[4]);
							Xml.AddElementAttr(_T("dvd"),(long)pDeviceCap->uiMaxSpeeds[5]);
						Xml.LeaveElement();
					Xml.LeaveElement();
					Xml.AddElement(_T("CurrentSpeeds"),_T(""),true);
						if (bIsRecorder)
						{
							Xml.AddElement(_T("Write"),(long)pDeviceCap->uiCurSpeeds[0],true);
								Xml.AddElementAttr(_T("cd"),(long)pDeviceCap->uiCurSpeeds[1]);
								Xml.AddElementAttr(_T("dvd"),(long)pDeviceCap->uiCurSpeeds[2]);
							Xml.LeaveElement();
						}
						Xml.AddElement(_T("Read"),(long)pDeviceCap->uiCurSpeeds[3],true);
							Xml.AddElementAttr(_T("cd"),(long)pDeviceCap->uiCurSpeeds[4]);
							Xml.AddElementAttr(_T("dvd"),(long)pDeviceCap->uiCurSpeeds[5]);
						Xml.LeaveElement();
					Xml.LeaveElement();

#ifdef DEVICEMANAGER_COLLECTWRITESPEEDS
					if (bIsRecorder)
					{
						Xml.AddElement(_T("WriteSpeeds"),_T(""),true);
						Xml.AddElementAttr(_T("count"),(long)pDeviceCap->uiWriteSpeedsCount);

						for (unsigned int j = 0; j < pDeviceCap->uiWriteSpeedsCount; j++)
						{
							lsprintf(szElemName,_T("Item%d"),j);
							Xml.AddElement(szElemName,(long)pDeviceCap->uiWriteSpeeds[j][0],true);
								Xml.AddElementAttr(_T("cd"),(long)pDeviceCap->uiWriteSpeeds[j][1]);
								Xml.AddElementAttr(_T("dvd"),(long)pDeviceCap->uiWriteSpeeds[j][2]);
							Xml.LeaveElement();
						}
						Xml.LeaveElement();
					}
#endif
				Xml.LeaveElement();

				// Extended information.
				if (bIsRecorder)
				{
					Xml.AddElement(_T("Extended"),_T(""),true);
#ifdef UNICODE
						TCHAR szBuffer[DEVICEMANAGER_MAXLINESIZE];

						AnsiToUnicode(szBuffer,pDeviceInfoEx->szWriteFlags,sizeof(szBuffer) / sizeof(wchar_t));
						Xml.AddElement(_T("Flags"),szBuffer);

						AnsiToUnicode(szBuffer,pDeviceInfoEx->szWriteModes,sizeof(szBuffer) / sizeof(wchar_t));
						Xml.AddElement(_T("Modes"),szBuffer);
#else
						Xml.AddElement(_T("Flags"),pDeviceInfoEx->szWriteFlags);
						Xml.AddElement(_T("Flags"),pDeviceInfoEx->szWriteModes);
#endif
					Xml.LeaveElement();
				}
			}

			Xml.LeaveElement();
		}

		Xml.LeaveElement();
	Xml.LeaveElement();

	// Get the correct file-path.
	TCHAR szConfigPath[MAX_PATH];
	if (!GetConfigPath(szConfigPath))
		return false;

	return Xml.Save(szConfigPath) == XMLRES_OK;
}

bool CDeviceManager::LoadConfiguration()
{
	CXmlProcessor Xml;
	bool bResult = true;

	// Get the correct file-path.
	TCHAR szConfigPath[MAX_PATH];
    if (!GetConfigPath(szConfigPath))
		return false;

	// Load the file.
	int iResult = Xml.Load(szConfigPath);
	if (iResult != XMLRES_OK && iResult != XMLRES_FILEERROR)
		return false;

	if (!Xml.EnterElement(_T("InfraRecorder")))
		return false;

	if (!Xml.EnterElement(_T("Devices")))
		return false;

	TCHAR szElemName[32];
	unsigned int iDeviceCount = 0;
	Xml.GetSafeElementAttrValue(_T("count"),(int *)&iDeviceCount);

	// Initialize counters.
	m_uiDeviceInfoCount = iDeviceCount;
	m_uiDeviceInfoExCount = iDeviceCount;
	m_uiDeviceCapCount = iDeviceCount;

	for (unsigned int i = 0; i < iDeviceCount; i++)
	{
		// Zero memory.
		memset(&m_DeviceInfo[i],0,sizeof(tDeviceInfo));
		memset(&m_DeviceInfoEx[i],0,sizeof(tDeviceInfoEx));
		memset(&m_DeviceCap[i],0,sizeof(tDeviceCap));

		tDeviceInfo *pDeviceInfo = &m_DeviceInfo[i];
		tDeviceInfoEx *pDeviceInfoEx = &m_DeviceInfoEx[i];
		tDeviceCap *pDeviceCap = &m_DeviceCap[i];

		lsprintf(szElemName,_T("Device%d"),i);
		if (!Xml.EnterElement(szElemName))
		{
			bResult = false;
			break;
		}

		Xml.GetSafeElementAttrValue(_T("type"),(int *)&pDeviceInfo->iType);
		Xml.GetSafeElementAttrValue(_T("removable"),(bool *)&pDeviceInfo->bRemovable);
		Xml.GetSafeElementAttrValue(_T("letter"),(int *)&pDeviceInfo->Address.m_cDriveLetter);

		// General information.
		if (!Xml.EnterElement(_T("ID")))
		{
			bResult = false;
			break;
		}

		Xml.GetSafeElementAttrValue(_T("bus"),(int *)&pDeviceInfo->Address.m_iBus);
		Xml.GetSafeElementAttrValue(_T("target"),(int *)&pDeviceInfo->Address.m_iTarget);
		Xml.GetSafeElementAttrValue(_T("lun"),(int *)&pDeviceInfo->Address.m_iLun);
		Xml.GetSafeElementData(_T("Vendor"),pDeviceInfo->szVendor,9);
		Xml.GetSafeElementData(_T("Identification"),pDeviceInfo->szIdentification,17);
		Xml.GetSafeElementData(_T("Revision"),pDeviceInfo->szRevision,5);
		Xml.LeaveElement();

		if (!IsDeviceReader(i))
		{
			Xml.LeaveElement();		// Device
			continue;
		}

		// Capabilities.
		if (!Xml.EnterElement(_T("Capabilities")))
		{
			bResult = false;
			break;
		}

		Xml.GetSafeElementData(_T("Media"),(int *)&pDeviceCap->uiMedia);
		Xml.GetSafeElementData(_T("General"),(int *)&pDeviceCap->uiGeneral);
		Xml.GetSafeElementData(_T("DigitalAudio"),(int *)&pDeviceCap->uiDigitalAudio);
		Xml.GetSafeElementData(_T("RW"),(int *)&pDeviceCap->uiRW);

		Xml.GetSafeElementData(_T("Audio"),(int *)&pDeviceCap->uiAudio);
		Xml.GetSafeElementData(_T("DigitalOutput"),(int *)&pDeviceCap->uiDigitalOutput);
		Xml.GetSafeElementData(_T("NumVolLevels"),(int *)&pDeviceCap->uiNumVolLevels);

		Xml.GetSafeElementData(_T("BufferSize"),(int *)&pDeviceCap->uiBufferSize);
		Xml.GetSafeElementData(_T("LoadType"),(int *)&pDeviceCap->ucLoadType);
		Xml.GetSafeElementData(_T("RotationControl"),(int *)&pDeviceCap->ucRotCtrl);
		Xml.GetSafeElementData(_T("DigitalAudioBL"),(int *)&pDeviceCap->ucDigitalAudioBL);

		// Max speeds.
		if (!Xml.EnterElement(_T("MaxSpeeds")))
		{
			bResult = false;
			break;
		}

		bool bIsRecorder = IsDeviceRecorder(i);

		if (bIsRecorder)
		{
			if (!Xml.EnterElement(_T("Write")))
			{
				bResult = false;
				break;
			}

			Xml.GetSafeElementAttrValue(_T("cd"),(int *)&pDeviceCap->uiMaxSpeeds[1]);
			Xml.GetSafeElementAttrValue(_T("dvd"),(int *)&pDeviceCap->uiMaxSpeeds[2]);
			Xml.GetSafeElementData((int *)&pDeviceCap->uiMaxSpeeds[0]);
			Xml.LeaveElement();
		}

		if (!Xml.EnterElement(_T("Read")))
		{
			bResult = false;
			break;
		}

		Xml.GetSafeElementAttrValue(_T("cd"),(int *)&pDeviceCap->uiMaxSpeeds[4]);
		Xml.GetSafeElementAttrValue(_T("dvd"),(int *)&pDeviceCap->uiMaxSpeeds[5]);
		Xml.GetSafeElementData((int *)&pDeviceCap->uiMaxSpeeds[3]);
		Xml.LeaveElement();
		Xml.LeaveElement();

		// Current speeds.
		if (!Xml.EnterElement(_T("CurrentSpeeds")))
		{
			bResult = false;
			break;
		}

		if (bIsRecorder)
		{
			if (!Xml.EnterElement(_T("Write")))
			{
				bResult = false;
				break;
			}

			Xml.GetSafeElementAttrValue(_T("cd"),(int *)&pDeviceCap->uiCurSpeeds[1]);
			Xml.GetSafeElementAttrValue(_T("dvd"),(int *)&pDeviceCap->uiCurSpeeds[2]);
			Xml.GetSafeElementData((int *)&pDeviceCap->uiCurSpeeds[0]);
			Xml.LeaveElement();
		}

		if (!Xml.EnterElement(_T("Read")))
		{
			bResult = false;
			break;
		}

		Xml.GetSafeElementAttrValue(_T("cd"),(int *)&pDeviceCap->uiCurSpeeds[4]);
		Xml.GetSafeElementAttrValue(_T("dvd"),(int *)&pDeviceCap->uiCurSpeeds[5]);
		Xml.GetSafeElementData((int *)&pDeviceCap->uiCurSpeeds[3]);
		Xml.LeaveElement();
		Xml.LeaveElement();

		// Write speeds.
#ifdef DEVICEMANAGER_COLLECTWRITESPEEDS
		if (bIsRecorder)
		{
			if (!Xml.EnterElement(_T("WriteSpeeds")))
			{
				bResult = false;
				break;
			}

			Xml.GetSafeElementAttrValue(_T("count"),(int *)&pDeviceCap->uiWriteSpeedsCount);

			for (unsigned int j = 0; j < pDeviceCap->uiWriteSpeedsCount; j++)
			{
				lsprintf(szElemName,_T("Item%d"),j);

				if (!Xml.EnterElement(szElemName))
				{
					bResult = false;
					break;
				}

				Xml.GetSafeElementAttrValue(_T("cd"),(int *)&pDeviceCap->uiWriteSpeeds[j][1]);
				Xml.GetSafeElementAttrValue(_T("dvd"),(int *)&pDeviceCap->uiWriteSpeeds[j][2]);
				Xml.GetSafeElementData((int *)&pDeviceCap->uiWriteSpeeds[j][0]);
				Xml.LeaveElement();
			}

			if (!bResult)
				break;

			Xml.LeaveElement();
		}
#endif

		Xml.LeaveElement();

		// Extended information.
		if (bIsRecorder)
		{
			if (!Xml.EnterElement(_T("Extended")))
			{
				bResult = false;
				break;
			}

#ifdef UNICODE
			TCHAR szBuffer[DEVICEMANAGER_MAXLINESIZE];
			
			Xml.GetSafeElementData(_T("Flags"),szBuffer,DEVICEMANAGER_MAXLINESIZE);
			UnicodeToAnsi(pDeviceInfoEx->szWriteFlags,szBuffer,sizeof(pDeviceInfoEx->szWriteFlags));

			Xml.GetSafeElementData(_T("Modes"),szBuffer,DEVICEMANAGER_MAXLINESIZE);
			UnicodeToAnsi(pDeviceInfoEx->szWriteModes,szBuffer,sizeof(pDeviceInfoEx->szWriteModes));
#else
			Xml.GetSafeElementData(_T("Flags"),pDeviceInfoEx->szWriteFlags,CONSOLEPIPE_MAX_LINE_SIZE);
			Xml.GetSafeElementData(_T("Modes"),pDeviceInfoEx->szWriteModes,CONSOLEPIPE_MAX_LINE_SIZE);
#endif
			Xml.LeaveElement();
		}


		Xml.LeaveElement();		// Device
	}

	Xml.LeaveElement();
	Xml.LeaveElement();

	return bResult;
}

bool CDeviceManager::VerifyConfiguration()
{
	// Reset the verification counter.
	m_uiVeriCounter = 0;
	m_bVeriResult = true;

	TCHAR szCommandLine[MAX_PATH];
	lstrcpy(szCommandLine,_T("\""));
	lstrcat(szCommandLine,g_GlobalSettings.m_szCDRToolsPath);
	lstrcat(szCommandLine,_T(CORE_WRITEAPP));

#ifdef CDRKIT_DEVICESCAN
	m_iMode = MODE_VERIFYDEVICES;

	lstrcat(szCommandLine,_T("\" -devices"));
#else
	m_iMode = MODE_VERIFYBUS;

	lstrcat(szCommandLine,_T("\" -scanbus"));
#endif

	if (!create(szCommandLine))
		return false;

	wait();
	return m_bVeriResult;
}
