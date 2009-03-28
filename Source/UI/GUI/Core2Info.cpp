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
#include "Core2Info.h"
#include "SCSI.h"
#include "LogDlg.h"

CCore2Info::CCore2Info()
{
}

CCore2Info::~CCore2Info()
{
}

bool CCore2Info::ReadCapacity(CCore2Device *pDevice,unsigned long &ulBlockAddress,
						  unsigned long &ulBlockLength)
{
	if (pDevice == NULL)
		return false;

	// Initialize buffers.
	unsigned char ucBuffer[8];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_READ_CAPACITY;
	ucCdb[9] = 0;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,8))
		return false;

	ulBlockAddress = ((unsigned long)ucBuffer[0] << 24) | ((unsigned long)ucBuffer[1] << 16) |
		((unsigned long)ucBuffer[2] << 8) | ucBuffer[3];
	ulBlockLength = ((unsigned long)ucBuffer[4] << 24) | ((unsigned long)ucBuffer[5] << 16) |
		((unsigned long)ucBuffer[6] << 8) | ucBuffer[7];

	return true;
}

bool CCore2Info::ReadTrackInformation(CCore2Device *pDevice,eTrackInfoType InfoType,
									  unsigned long ulTrackAddr,CCore2TrackInfo *pTrackInfo)
{
	if (pDevice == NULL || pTrackInfo == NULL)
		return false;

	// Initialize buffers.
	unsigned char ucBuffer[48];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_READ_TRACK_INFORMATION;
	ucCdb[1] = InfoType & 0x3;
	ucCdb[2] = 0;
	ucCdb[2] = (unsigned char)(ulTrackAddr >> 24);
	ucCdb[3] = (unsigned char)(ulTrackAddr >> 16);
	ucCdb[4] = (unsigned char)(ulTrackAddr >>  8);
	ucCdb[5] = (unsigned char)(ulTrackAddr & 0xFF);
	ucCdb[7] = sizeof(ucBuffer) >> 8;		// Allocation length (MSB).
	ucCdb[8] = sizeof(ucBuffer) & 0xFF;		// Allocation length (LSB).
	ucCdb[9] = 0;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,sizeof(ucBuffer)))
		return false;

	// Check if we received to much data.
	unsigned short usDataLength = ((unsigned short)ucBuffer[0] << 8) | ucBuffer[1];
	if (usDataLength > (sizeof(ucBuffer) - 2))
		g_pLogDlg->print_line(_T("  Warning: CCore2::ReadTrackInformation received more track information than it could handle."));

	pTrackInfo->m_ucFlags = (ucBuffer[6] & 0xF0) | ((ucBuffer[5] >> 2) & 0x0C) | (ucBuffer[7] & 0x03);
	pTrackInfo->m_ucLJRS = (ucBuffer[5] >> 6) & 0x03;
	pTrackInfo->m_ucTrackMode = ucBuffer[5] & 0x0F;
	pTrackInfo->m_ucDataMode = ucBuffer[6] & 0x0F;

	pTrackInfo->m_usTrackNumber = ((unsigned short)ucBuffer[32] << 8) | ucBuffer[2];
	pTrackInfo->m_usSessionNumber = ((unsigned short)ucBuffer[33] << 8) | ucBuffer[3];

	pTrackInfo->m_ulTrackAddr = ((unsigned int)ucBuffer[8] << 24) | ((unsigned int)ucBuffer[9] << 16) |
		((unsigned int)ucBuffer[10] << 8) | ucBuffer[11];
	pTrackInfo->m_ulNextWritableAddr = ((unsigned int)ucBuffer[12] << 24) | ((unsigned int)ucBuffer[13] << 16) |
		((unsigned int)ucBuffer[14] << 8) | ucBuffer[15];
	pTrackInfo->m_ulFreeBlocks = ((unsigned int)ucBuffer[16] << 24) | ((unsigned int)ucBuffer[17] << 16) |
		((unsigned int)ucBuffer[18] << 8) | ucBuffer[19];
	pTrackInfo->m_ulBlockingFactor = ((unsigned int)ucBuffer[20] << 24) | ((unsigned int)ucBuffer[21] << 16) |
		((unsigned int)ucBuffer[22] << 8) | ucBuffer[23];
	pTrackInfo->m_ulTrackSize = ((unsigned int)ucBuffer[24] << 24) | ((unsigned int)ucBuffer[25] << 16) |
		((unsigned int)ucBuffer[26] << 8) | ucBuffer[27];
	pTrackInfo->m_ulLastRecorderAddr = ((unsigned int)ucBuffer[28] << 24) | ((unsigned int)ucBuffer[29] << 16) |
		((unsigned int)ucBuffer[30] << 8) | ucBuffer[31];
	pTrackInfo->m_ulReadCombLBA = ((unsigned int)ucBuffer[36] << 24) | ((unsigned int)ucBuffer[37] << 16) |
		((unsigned int)ucBuffer[38] << 8) | ucBuffer[39];
	pTrackInfo->m_ulNextJLA = ((unsigned int)ucBuffer[40] << 24) | ((unsigned int)ucBuffer[41] << 16) |
		((unsigned int)ucBuffer[42] << 8) | ucBuffer[43];
	pTrackInfo->m_ulLastJLA = ((unsigned int)ucBuffer[44] << 24) | ((unsigned int)ucBuffer[45] << 16) |
		((unsigned int)ucBuffer[46] << 8) | ucBuffer[47];

	return true;
}

bool CCore2Info::ReadDiscInformation(CCore2Device *pDevice,CCore2DiscInfo *pDiscInfo)
{
	if (pDevice == NULL || pDiscInfo == NULL)
		return false;

	// Initialize buffers.
	unsigned char ucBuffer[2048];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[0] = SCSI_READ_DISC_INFORMATION;
	ucCdb[1] = 0x00;	// Standard disc information.
	ucCdb[7] = 0x08;
	ucCdb[8] = 0x00;
	ucCdb[9] = 0x00;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,2048))
		return false;

	pDiscInfo->m_ucLastSessStatus = (ucBuffer[2] >> 2) & 0x03;
	pDiscInfo->m_ucDiscStatus = ucBuffer[2] & 0x03;
	pDiscInfo->m_ucFirstTrack = ucBuffer[3];
	pDiscInfo->m_usNumSessions = ucBuffer[4] | (ucBuffer[9] << 8);
	pDiscInfo->m_usLastSessFstTrack = ucBuffer[5] | (ucBuffer[10] << 8);
	pDiscInfo->m_usLastSessLstTrack = ucBuffer[6] | (ucBuffer[11] << 8);
	pDiscInfo->m_ucFlags = (ucBuffer[7] & 0xFC) | (((ucBuffer[2] & 0x10) > 0) << 1);
	pDiscInfo->m_ucDiscType = ucBuffer[8];
	pDiscInfo->m_uiDiscID = (ucBuffer[12] << 24) | (ucBuffer[13] << 16) |
		(ucBuffer[14] << 8) | ucBuffer[15];
	pDiscInfo->m_uiLastSessLeadInAddr = (ucBuffer[16] << 24) | (ucBuffer[17] << 16) |
		(ucBuffer[18] << 8) | ucBuffer[19];
	pDiscInfo->m_uiLastLeadOutAddr = (ucBuffer[20] << 24) | (ucBuffer[21] << 16) |
		(ucBuffer[22] << 8) | ucBuffer[23];
	pDiscInfo->m_uiDiscBarCode = ((unsigned __int64)ucBuffer[24] << 56) |
		((unsigned __int64)ucBuffer[25] << 48) | ((unsigned __int64)ucBuffer[26] << 40) |
		((unsigned __int64)ucBuffer[27] << 32) | (ucBuffer[28] << 24) |
		(ucBuffer[29] << 16) | (ucBuffer[30] << 8) | ucBuffer[31];
	pDiscInfo->m_ucDiscAppCode = ucBuffer[32];

	return true;
}

bool CCore2Info::ReadPhysFmtInfo(CCore2Device *pDevice,CCore2PhysFmtInfo *pPhysInfo)
{
	if (pDevice == NULL || pPhysInfo == NULL)
		return false;

	// Initialize buffers.
	unsigned char ucBuffer[192];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[ 0] = SCSI_READ_DISC_STRUCTURE;
	ucCdb[ 7] = 0x00;
	ucCdb[ 8] = 0x08 >> 8;
	ucCdb[ 9] = 0x08;
	ucCdb[11] = 0x00;

	if (!pDevice->Transport(ucCdb,12,ucBuffer,192))
		return false;

	//uiBookType = ucBuffer[4] & 0xFF;

	pPhysInfo->m_ucDiscCategory = ucBuffer[4] >> 4;
	pPhysInfo->m_ucPartVersion = ucBuffer[4] & 0xF;
	pPhysInfo->m_ucDiscSize = ucBuffer[5] >> 4;
	pPhysInfo->m_ucMaxRate = ucBuffer[5] & 0xF;
	pPhysInfo->m_ucNumLayers = ((ucBuffer[6] & 0x60) >> 5) + 1;
	pPhysInfo->m_ucTrackPath = (ucBuffer[6] & 0x10) > 0;
	pPhysInfo->m_ucLayerType = ucBuffer[6] & 0xF;
	pPhysInfo->m_ucLinearDensity = ucBuffer[7] >> 4;
	pPhysInfo->m_ucTrackDensity = ucBuffer[7] & 0xF;
	pPhysInfo->m_uiDataStartSector = (ucBuffer[9] << 16) | (ucBuffer[10] << 8) | ucBuffer[11];
	pPhysInfo->m_uiDataEndSector = (ucBuffer[13] << 16) | (ucBuffer[14] << 8) | ucBuffer[15];
	pPhysInfo->m_uiLayer0EndSector = (ucBuffer[17] << 16) | (ucBuffer[18] << 8) | ucBuffer[19];

	return true;
}

bool CCore2Info::ReadTOC(CCore2Device *pDevice,unsigned char &ucFirstTrackNumber,
						 unsigned char &ucLastTrackNumber,std::vector<CCore2TOCTrackDesc> &Tracks)
{
	if (pDevice == NULL)
		return false;

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
	ucCdb[2] = 0x00;						// TOC.
	ucCdb[7] = sizeof(ucBuffer) >> 8;		// Allocation length (MSB).
	ucCdb[8] = sizeof(ucBuffer) & 0xFF;		// Allocation length (LSB).
	ucCdb[9] = 0;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,sizeof(ucBuffer)))
		return false;

	unsigned short usDataLen = ((unsigned short)ucBuffer[0] << 8) | ucBuffer[1];
	if (usDataLen < 2)
		return false;
		
	ucFirstTrackNumber = ucBuffer[2];
	ucLastTrackNumber = ucBuffer[3];

	// Fill the Tracks vector.
	for (unsigned char uc = ucFirstTrackNumber; uc <= ucLastTrackNumber; uc++)
	{
		unsigned char ucCurTrackNumber = ucBuffer[6 + 8 * (uc - 1)];
		unsigned long ulTrackAddr = (ucBuffer[8 + 8 * (uc - 1)] << 24) |
			(ucBuffer[9 + 8 * (uc - 1)] << 16) | (ucBuffer[10 + 8 * (uc - 1)] << 8) |
			ucBuffer[11 + 8 * (uc - 1)];

		Tracks.push_back(CCore2TOCTrackDesc(ucCurTrackNumber,ulTrackAddr));
	}

	return true;
}

/*
	Read session information.
*/
bool CCore2Info::ReadSI(CCore2Device *pDevice,unsigned char &ucFirstSessNumber,
						unsigned char &ucLastSessNumber,
						unsigned long &ulLastSessFirstTrackPos)
{
	if (pDevice == NULL)
		return false;

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
	ucCdb[2] = 0x01;						// Session information.
	ucCdb[7] = sizeof(ucBuffer) >> 8;		// Allocation length (MSB).
	ucCdb[8] = sizeof(ucBuffer) & 0xFF;		// Allocation length (LSB).
	ucCdb[9] = 0;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,sizeof(ucBuffer)))
		return false;

	unsigned short usDataLen = ((unsigned short)ucBuffer[0] << 8) | ucBuffer[1];
	if (usDataLen < 2)
		return false;
		
	ucFirstSessNumber = ucBuffer[2];
	ucLastSessNumber = ucBuffer[3];

	ulLastSessFirstTrackPos = (ucBuffer[8] << 24) | (ucBuffer[9] << 16) |
		(ucBuffer[10] << 8) |ucBuffer[11];

	return true;
}

/*
	CCore2Info::GetTotalDiscCapacity
	--------------------------------
	Counts the number of sectors marked as formatted (used) and
	unformatted/blank (free) on the disc mounted on the specified device. The function
	returns true of successfull, false otherwise.
*/
bool CCore2Info::GetTotalDiscCapacity(CCore2Device *pDevice,unsigned __int64 &uiUsedBytes,
									  unsigned __int64 &uiFreeBytes)
{
	g_pLogDlg->print_line(_T("CCore2Info::GetTotalDiscCapacity"));

	uiUsedBytes = 0;
	uiFreeBytes = 0;

	// Initialize buffers.
	unsigned char ucBuffer[192];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Get the device configuration, to see what media that's mounted.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_GET_CONFIGURATION;
	ucCdb[8] = 0x08;

	if (!pDevice->Transport(ucCdb,9,ucBuffer,192))
		return false;

	unsigned short usProfile = ucBuffer[6] << 8 | ucBuffer[7];
	g_pLogDlg->print_line(_T("  Current profile: 0x%.4X."),usProfile);

	bool bReadOnly = true;
	switch (usProfile)
	{
		case PROFILE_CDR:
		case PROFILE_CDRW:
		case PROFILE_DVDMINUSR_SEQ:
		case PROFILE_DVDRAM:
		case PROFILE_DVDMINUSRW_RESTOV:
		case PROFILE_DVDMINUSRW_SEQ:
		case PROFILE_DVDMINUSR_DL_SEQ:
		case PROFILE_DVDMINUSR_DL_JUMP:
		case PROFILE_DVDPLUSRW:
		case PROFILE_DVDPLUSR:
		case PROFILE_DVDPLUSRW_DL:
		case PROFILE_DVDPLUSR_DL:
		case PROFILE_BDR_SRM:
		case PROFILE_BDR_RRM:
		case PROFILE_BDRE:
		case PROFILE_HDDVDR:
		case PROFILE_HDDVDRAM:
			bReadOnly = false;
			break;
	}

	// Read the format capacities length.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_READ_FORMAT_CAPACITIES;
	ucCdb[8] = 0x04;
	ucCdb[9] = 0x00;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,192))
		return false;

	unsigned char ucCapListLen = ucBuffer[3];
	g_pLogDlg->print_line(_T("  Capacity list length: %d bytes."),ucCapListLen);

	if (ucCapListLen % 8 != 0 || ucCapListLen == 0)
	{
		g_pLogDlg->print_line(_T("  Error: Invalid capacity list length."));
		return false;
	}

	// Read the actual format capacities data.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_READ_FORMAT_CAPACITIES;
	ucCdb[7] = (ucCapListLen + 0x04) >> 8;
	ucCdb[8] = (ucCapListLen + 0x04) & 0xFF;
	ucCdb[9] = 0x00;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,ucCapListLen + 0x04))
		return false;

	if ((ucBuffer[8] & 0x03) == 0x03)	// No media present or unknown capacity.
	{
		g_pLogDlg->print_line(_T("  No media present."));
		return false;
	}

	// Format descriptors.
	for (unsigned int i = 0; i < ucCapListLen; i += 8)
	{
		unsigned int uiCapacity = ucBuffer[i + 4] << 24 | ucBuffer[i + 5] << 16 |
			ucBuffer[i + 6] << 8 | ucBuffer[i + 7];
		unsigned int uiBlockSpare = ucBuffer[i + 9] << 16 |ucBuffer[i + 10] << 8 |
			ucBuffer[i + 11];

		if (uiBlockSpare == 2048)
		{
			if (ucBuffer[i + 8] == 0x02)		// Formatted media
				uiUsedBytes += (unsigned __int64)uiCapacity * 2048;
			else if (ucBuffer[i + 8] == 0x01)	// Unformatted or blank media.
				uiFreeBytes += (unsigned __int64)uiCapacity * 2048;
		}
	}

	return true;
}

bool CCore2Info::GetDiscDVDRegion(CCore2Device *pDevice,unsigned char &ucRegion)
{
	// Initialize buffers.
	unsigned char ucBuffer[8];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	ucCdb[ 0] = SCSI_READ_DISC_STRUCTURE;
	ucCdb[ 7] = 0x01;
	ucCdb[ 8] = 0x08 >> 8;
	ucCdb[ 9] = 0x08;
	ucCdb[11] = 0x00;

	if (!pDevice->Transport(ucCdb,12,ucBuffer,8))
		return false;

	unsigned char ucRegMask = ucBuffer[5];
	ucRegion = 0;

	if (ucRegMask != 0xFF)
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			if (!((ucRegMask >> i) & 0x01))
				ucRegion = i + 1;
		}
	}

	return true;
}