/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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
#include "Core2Device.h"

class CCore2TrackInfo
{
public:
	// Flags.
	enum { FLAG_NWAV = 0x01,FLAG_LRAV = 0x02,FLAG_COPY = 0x04,FLAG_DAMAGE = 0x08,
		FLAG_FP = 0x10,FLAG_PACKET = 0x20,FLAG_BLANK = 0x40,FLAG_RT = 0x80 };

	// Data modes.
	enum { DM_MODE1 = 0x1,DM_MODE2 = 0x2,DM_UNKNOWN = 0xF };

	unsigned char m_ucFlags;
	unsigned char m_ucLJRS;
	unsigned char m_ucTrackMode;
	unsigned char m_ucDataMode;
	unsigned short m_usTrackNumber;
	unsigned short m_usSessionNumber;
	unsigned long m_ulTrackAddr;
	unsigned long m_ulNextWritableAddr;
	unsigned long m_ulFreeBlocks;
	unsigned long m_ulBlockingFactor;
	unsigned long m_ulTrackSize;
	unsigned long m_ulLastRecorderAddr;
	unsigned long m_ulReadCombLBA;
	unsigned long m_ulNextJLA;
	unsigned long m_ulLastJLA;
};

class CCore2DiscInfo
{
public:
	// Last session status.	
	enum { LSS_EMTPY,LSS_INCOMPLETE,LSS_RESERVED,LSS_COMPLETE };

	// Disc status.
	enum { DS_EMTPY,DS_INCOMPLETE,DS_FINALIZED,DS_RANDOMACCESS };

	// Disc type.
	enum { DT_CDDA = 0x00,DT_CDI = 0x10,DT_XA = 0x20,DT_UNDEFINED = 0xFF };

	// Flags.
	enum { FLAG_ERASABLE = 0x02,FLAG_D = 0x04,FLAG_RESERVED = 0x08,FLAG_DACV = 0x10,
		FLAG_URU = 0x20,FLAG_DBCV = 0x40,FLAG_DIDV = 0x80 };

	unsigned char m_ucFlags;
	unsigned char m_ucLastSessStatus;
	unsigned char m_ucDiscStatus;
	unsigned char m_ucDiscType;
	unsigned char m_ucFirstTrack;
	unsigned short m_usNumSessions;
	unsigned short m_usLastSessFstTrack;
	unsigned short m_usLastSessLstTrack;
	unsigned int m_uiDiscID;
	unsigned int m_uiLastSessLeadInAddr;
	unsigned int m_uiLastLeadOutAddr;
	unsigned __int64 m_uiDiscBarCode;
	unsigned char m_ucDiscAppCode;
};

class CCore2TOCTrackDesc
{
public:
	CCore2TOCTrackDesc(unsigned char ucTrackNumber,unsigned long ulTrackAddr)
	{
		m_ucTrackNumber = ucTrackNumber;
		m_ulTrackAddr = ulTrackAddr;
	}

	unsigned char m_ucTrackNumber;
	unsigned long m_ulTrackAddr;
};

class CCore2PhysFmtInfo
{
public:
	// Disc category.
	enum { DC_DVDROM,DC_DVDRAM,DC_DVDR,DC_DVDRW,DC_HDDVDROM,DC_HDDVDRAM,DC_HDDVDR,
		DC_RESERVED1,DC_RESERVED2,DC_DVDPLUSRW,DC_DVDPLUSR,DC_RESERVED3,
		DC_RESERVED4,DC_DVDPLUSRWDL,DC_DVDPLUSRDL,DC_RESERVED5 };
	
	// Maximum rate.
	enum { MR_252 = 0x0,MR_504 = 0x1,MR_1008 = 0x2,MR_2016 = 0x3,MR_3024 = 0x4,
		MR_NOTSPECIFIED = 0xF };

	// Track path.
	enum { TP_PARALLEL,TP_OPPOSITE };

	unsigned char m_ucDiscCategory;
	unsigned char m_ucPartVersion;
	unsigned char m_ucDiscSize;
	unsigned char m_ucMaxRate;
	unsigned char m_ucNumLayers;
	unsigned char m_ucLayerType;
	unsigned char m_ucTrackPath;
	unsigned char m_ucLinearDensity;
	unsigned char m_ucTrackDensity;
	unsigned int m_uiDataStartSector;
	unsigned int m_uiDataEndSector;
	unsigned int m_uiLayer0EndSector;
};

class CCore2Info
{
public:
	enum eTrackInfoType
	{
		TIT_LBA = 0,
		TIT_TRACK = 1,
		TIT_SESSION = 2
	};

	CCore2Info();
	~CCore2Info();

	// Closely related to SCSI MMC functions.
	bool ReadCapacity(CCore2Device *pDevice,unsigned long &ulBlockAddress,
		unsigned long &ulBlockLength);
	bool ReadTrackInformation(CCore2Device *pDevice,eTrackInfoType InfoType,
		unsigned long ulTrackAddr,CCore2TrackInfo *pTrackInfo);
	bool ReadDiscInformation(CCore2Device *pDevice,CCore2DiscInfo *pDiscInfo);
	bool ReadPhysFmtInfo(CCore2Device *pDevice,CCore2PhysFmtInfo *pPhysInfo);
	bool ReadTOC(CCore2Device *pDevice,unsigned char &ucFirstTrackNumber,
		unsigned char &ucLastTrackNumber,std::vector<CCore2TOCTrackDesc> &Tracks);
	bool ReadSI(CCore2Device *pDevice,unsigned char &ucFirstSessNumber,
		unsigned char &ucLastSessNumber,unsigned long &ulLastSessFirstTrackPos);

	bool GetTotalDiscCapacity(CCore2Device *pDevice,unsigned __int64 &uiUsedBytes,
		unsigned __int64 &uiFreeBytes);
	bool GetDiscDVDRegion(CCore2Device *pDevice,unsigned char &ucRegion);
};
