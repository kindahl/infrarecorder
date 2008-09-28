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

// Commands.
#define SC_HA_INQUIRY				0x00
#define SC_GET_DEV_TYPE				0x01
#define SC_EXEC_SCSI_CMD			0x02
#define SC_ABORT_SRB				0x03
#define SC_RESET_DEV				0x04
#define SC_SET_HA_PARMS				0x05
#define SC_GET_DISK_INFO			0x06

// Status codes.
#define SS_PENDING					0x00
#define SS_COMP						0x01
#define SS_ABORTED					0x02
#define SS_ABORT_FAIL				0x03
#define SS_ERR						0x04
#define SS_INVALID_CMD				0x80
#define SS_INVALID_HA				0x81
#define SS_NO_DEVICE				0x82
#define SS_INVALID_SRB				0xE0
#define SS_OLD_MANAGER				0xE1
#define SS_BUFFER_ALIGN				0xE1
#define SS_ILLEGAL_MODE				0xE2
#define SS_NO_ASPI					0xE3
#define SS_FAILED_INIT				0xE4
#define SS_ASPI_IS_BUSY				0xE5
#define SS_BUFFER_TO_BIG			0xE6
#define SS_MISMATCHED_COMPONENTS	0xE7
#define SS_NO_ADAPTERS				0xE8
#define SS_INSUFFICIENT_RESOURCES	0xE9
#define SS_ASPI_IS_SHUTDOWN			0xEA
#define SS_BAD_INSTALL				0xEB

// Miscellaneous.
#define SRB_EVENT_NOTIFY			0x40
#define SRB_DIR_IN					0x08
#define SRB_DIR_OUT					0x10

typedef struct
{
	BYTE SRB_Cmd;					// ASPI command code = SC_HA_INQUIRY.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// ASPI request flags.
	DWORD SRB_Hdr_Rsvd;				// Reserved, MUST = 0.
	BYTE HA_Count;					// Number of host adapters present.
	BYTE HA_SCSI_ID;				// SCSI ID of host adapter.
	BYTE HA_ManagerId[16];			// String describing the manager.
	BYTE HA_Identifier[16];			// String describing the host adapter.
	BYTE HA_Unique[16];				// Host Adapter Unique parameters.
	WORD HA_Rsvd1;
} SRB_HaInquiry;

typedef struct
{
	BYTE SRB_Cmd;					// ASPI command code = SC_GET_DEV_TYPE.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// Reserved.
	DWORD SRB_Hdr_Rsvd;				// Reserved.
	BYTE SRB_Target;				// Target's SCSI ID.
	BYTE SRB_Lun;					// Target's LUN number.
	BYTE SRB_DeviceType;			// Target's peripheral device type.
	BYTE SRB_Rsvd1;
} SRB_GDEVBlock;

typedef struct
{
	BYTE SRB_Cmd;					// ASPI command code = SC_EXEC_SCSI_CMD.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// ASPI request flags.
	DWORD SRB_Hdr_Rsvd;				// Reserved.
	BYTE SRB_Target;				// Target's SCSI ID.
	BYTE SRB_Lun;					// Target's LUN number.
	WORD SRB_Rsvd1;					// Reserved for Alignment.
	DWORD SRB_BufLen;				// Data Allocation Length.
	BYTE *SRB_BufPointer;			// Data Buffer Point.
	BYTE SRB_SenseLen;				// Sense Allocation Length.
	BYTE SRB_CDBLen;				// CDB Length.
	BYTE SRB_HaStat;				// Host Adapter Status.
	BYTE SRB_TargStat;				// Target Status.
	void (*SRB_PostProc)();			// Post routine.
	void *SRB_Rsvd2;				// Reserved.
	BYTE SRB_Rsvd3[16];				// Reserved for expansion.
	BYTE CDBByte[16];				// SCSI CDB.
	BYTE SenseArea[24 + 2];			// Request sense buffer - var length.
} SRB_ExecSCSICmd;

typedef struct
{
	BYTE SRB_Cmd;					// ASPI command code = SC_ABORT_SRB.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// Reserved.
	DWORD SRB_Hdr_Rsvd;				// Reserved, MUST = 0.
	VOID *SRB_ToAbort;				// Pointer to SRB to abort.
} SRB_Abort;

typedef struct
{
	BYTE SRB_Cmd;					// ASPI cmd code = SC_RESET_DEV.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// Reserved.
	DWORD SRB_Hdr_Rsvd;				// Reserved.
	BYTE SRB_Target;				// Target's SCSI ID.
	BYTE SRB_Lun;					// Target's LUN number.
	BYTE SRB_Rsvd1[12];				// Reserved for Alignment.
	BYTE SRB_HaStat;				// Host Adapter Status.
	BYTE SRB_TargStat;				// Target Status.
	void (*SRB_PostProc)();			// Post routine.
	void *SRB_Rsvd2;				// Reserved.
	BYTE SRB_Rsvd3[32];				// Reserved.
} SRB_BusDeviceReset;

typedef struct
{
	BYTE SRB_Cmd;					// ASPI cmd code = SC_RESET_DEV.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// Reserved.
	DWORD SRB_Hdr_Rsvd;				// Reserved.
	BYTE SRB_Target;				// Target's SCSI ID.
	BYTE SRB_Lun;					// Target's LUN number.
	BYTE SRB_DriveFlags;			// Driver flags.
	BYTE SRB_Int13HDriveInfo;		// Host Adapter Status.
	BYTE SRB_Heads;					// Preferred number of heads trans.
	BYTE SRB_Sectors;				// Preferred number of sectors trans.
	BYTE SRB_Rsvd1[10];				// Reserved.
} SRB_GetDiskInfo;

// SRB header
typedef struct
{
	BYTE SRB_Cmd;					// ASPI cmd code = SC_RESET_DEV.
	BYTE SRB_Status;				// ASPI command status byte.
	BYTE SRB_HaId;					// ASPI host adapter number.
	BYTE SRB_Flags;					// Reserved.
	DWORD SRB_Hdr_Rsvd;				// Reserved.
} SRB_Header;

typedef union
{
	SRB_Header common;
	SRB_HaInquiry inquiry;
	SRB_ExecSCSICmd cmd;
	SRB_Abort abort;
	SRB_BusDeviceReset reset;
	SRB_GDEVBlock devtype;
	SRB_GetDiskInfo diskinfo;
} SRB, *PSRB, *LPSRB;

typedef unsigned long (*tGetASPI32SupportInfo)();
typedef unsigned long (*tSendASPI32Command)(LPSRB psrb);

class CCore2DriverASPI
{
private:
	HINSTANCE m_hDllInstance;
	bool m_bDriverLoaded;

	// Address to the opened device.
	int m_iBus,m_iTarget,m_iLun;

	tGetASPI32SupportInfo GetASPI32SupportInfo;
	tSendASPI32Command SendASPI32Command;

	bool LoadDriver();
	bool UnloadDriver();

public:
	CCore2DriverASPI();
	~CCore2DriverASPI();

	enum
	{
		DATAMODE_READ,
		DATAMODE_WRITE,
		DATAMODE_UNSPECIFIED
	};

	bool Open(int iBus,int iTarget,int iLun);
	bool Close();

	bool Transport(unsigned char *pCdb,unsigned char ucCdbLength,
		unsigned char *pData,unsigned long ulDataLength,int iDataMode = DATAMODE_READ);
	bool TransportWithSense(unsigned char *pCdb,unsigned char ucCdbLength,
		unsigned char *pData,unsigned long ulDataLength,
		unsigned char *pSense,unsigned char &ucResult,int iDataMode = DATAMODE_READ);
};
