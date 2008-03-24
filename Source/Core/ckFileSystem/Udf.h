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

#pragma once
#include "../../Common/Stream.h"
#include "../../Common/Crc16.h"

#define UDF_SECTOR_SIZE							2048
#define UDF_UNIQUEIDENT_MIN						16
#define UDF_CRC_POLYNOMIAL						0x11021

// Tag identifiers.
#define UDF_TAGIDENT_PRIMVOLDESC				1
#define UDF_TAGIDENT_ANCHORVOLDESCPTR			2
#define UDF_TAGIDENT_VOLDESCPTR					3
#define UDF_TAGIDENT_IMPLUSEVOLDESC				4
#define UDF_TAGIDENT_PARTDESC					5
#define UDF_TAGIDENT_LOGICALVOLDESC				6
#define UDF_TAGIDENT_UNALLOCATEDSPACEDESC		7
#define UDF_TAGIDENT_TERMDESC					8
#define UDF_TAGIDENT_LOGICALVOLINTEGRITYDESC	9
#define UDF_TAGIDENT_FILESETDESC				256
#define UDF_TAGIDENT_FILEIDENTDESC				257
#define UDF_TAGIDENT_FILEENTRYDESC				261
#define UDF_TAGIDENT_EXTENDEDATTRDESC			262

#define UDF_TAG_DESCRIPTOR_VERSION				2		// Goes hand in hand with "NSR02".

// D-string complession identifiers.
#define UDF_COMPRESSION_BYTE					8
#define UDF_COMPRESSION_UNICODE					16

// Operating system classes and identifiers.
#define UDF_OSCLASS_UNDEFINED					0
#define UDF_OSCLASS_DOS							1
#define UDF_OSCLASS_OS2							2
#define UDF_OSCLASS_MACOS						3
#define UDF_OSCLASS_UNIX						4

#define UDF_OSIDENT_UNDEFINED					0
#define UDF_OSIDENT_DOS							0
#define UDF_OSIDENT_OS2							0
#define UDF_OSIDENT_MACOS						0
#define UDF_OSIDENT_UNIX_GENERIC				0
#define UDF_OSIDENT_UNIX_AIX					1
#define UDF_OSIDENT_UNIX_SOLARIS				2
#define UDF_OSIDENT_UNIX_HPUX					3
#define UDF_OSIDENT_UNIX_IRIX					4

// Volume set interchange levels.
#define UDF_INTERCHANGE_LEVEL_MULTISET			3
#define UDF_INTERCHANGE_LEVEL_SINGLESET			2

// File set interchange levels.
#define UDF_INTERCHANGE_LEVEL_FILESET			3

// Partition flags.
#define UDF_PARTITION_FLAG_UNALLOCATED			0
#define UDF_PARTITION_FLAG_ALLOCATED			1

// Partition access types.
#define UDF_PARTITION_ACCESS_UNKNOWN			0
#define UDF_PARTITION_ACCESS_READONLY			1
#define UDF_PARTITION_ACCESS_WRITEONCE			2
#define UDF_PARTITION_ACCESS_REWRITABLE			3
#define UDF_PARTITION_ACCESS_OVERWRITABLE		4

// Partition map types.
#define UDF_PARTITION_MAP_UNKNOWN				0
#define UDF_PARTITION_MAP_TYPE1					1
#define UDF_PARTITION_MAP_TYPE2					2

// Domain flags.
#define UDF_DOMAIN_FLAG_HARD_WRITEPROTECT		1 << 0
#define UDF_DOMAIN_FLAG_SOFT_WRITEPROTECT		1 << 1

// Integrity types.
#define UDF_LOGICAL_INTEGRITY_OPEN				0
#define UDF_LOGICAL_INTEGRITY_CLOSE				1

// ICB strategies.
#define UDF_ICB_STRATEGY_UNKNOWN				0
#define UDF_ICB_STRATEGY_1						1
#define UDF_ICB_STRATEGY_2						2
#define UDF_ICB_STRATEGY_3						3
#define UDF_ICB_STRATEGY_4						4

// ICB file types.
#define UDF_ICB_FILETYPE_UNKNOWN				0
#define UDF_ICB_FILETYPE_UNALLOCATED_SPACE		1
#define UDF_ICB_FILETYPE_PART_INTEG_ENTRY		2
#define UDF_ICB_FILETYPE_INDIRECT_ENTRY			3
#define UDF_ICB_FILETYPE_DIRECTORY				4
#define UDF_ICB_FILETYPE_RANDOM_BYTES			5
#define UDF_ICB_FILETYPE_BLOCK_DEVICE			6
#define UDF_ICB_FILETYPE_CHARACTER_DEVICE		7
#define UDF_ICB_FILETYPE_EXTENDED_ATTR			8
#define UDF_ICB_FILETYPE_FIFO_FILE				9
#define UDF_ICB_FILETYPE_C_ISSOCK				10
#define UDF_ICB_FILETYPE_TERMINAL_ENTRY			11
#define UDF_ICB_FILETYPE_SYMBOLIC_LINK			12
#define UDF_ICB_FILETYPE_STREAM_DIRECTORY		13

// ICB file flags.
#define UDF_ICB_FILEFLAG_SHORT_ALLOC_DESC		0
#define UDF_ICB_FILEFLAG_LONG_ALLOC_DESC		1
#define UDF_ICB_FILEFLAG_EXTENDED_ALLOC_DESC	2
#define UDF_ICB_FILEFLAG_ONE_ALLOC_DESC			3
#define UDF_ICB_FILEFLAG_SORTED					1 << 3
#define UDF_ICB_FILEFLAG_NOT_RELOCATABLE		1 << 4
#define UDF_ICB_FILEFLAG_ARCHIVE				1 << 5
#define UDF_ICB_FILEFLAG_SETUID					1 << 6
#define UDF_ICB_FILEFLAG_SETGID					1 << 7
#define UDF_ICB_FILEFLAG_STICKY					1 << 8
#define UDF_ICB_FILEFLAG_CONTIGUOUS				1 << 9
#define UDF_ICB_FILEFLAG_SYSTEM					1 << 10
#define UDF_ICB_FILEFLAG_TRANSFORMED			1 << 11
#define UDF_ICB_FILEFLAG_MULTIVERSIONS			1 << 12
#define UDF_ICB_FILEFLAG_STREAM					1 << 13

// ICB file permissions.
#define UDF_ICB_FILEPERM_OTHER_EXECUTE			1 << 0
#define UDF_ICB_FILEPERM_OTHER_WRITE			1 << 1
#define UDF_ICB_FILEPERM_OTHER_READ				1 << 2
#define UDF_ICB_FILEPERM_OTHER_CHANGEATTRIB		1 << 3
#define UDF_ICB_FILEPERM_OTHER_DELETE			1 << 4
#define UDF_ICB_FILEPERM_GROUP_EXECUTE			1 << 5
#define UDF_ICB_FILEPERM_GROUP_WRITE			1 << 6
#define UDF_ICB_FILEPERM_GROUP_READ				1 << 7
#define UDF_ICB_FILEPERM_GROUP_CHANGEATTRIB		1 << 8
#define UDF_ICB_FILEPERM_GROUP_DELETE			1 << 9
#define UDF_ICB_FILEPERM_OWNER_EXECUTE			1 << 10
#define UDF_ICB_FILEPERM_OWNER_WRITE			1 << 11
#define UDF_ICB_FILEPERM_OWNER_READ				1 << 12
#define UDF_ICB_FILEPERM_OWNER_CHANGEATTRIB		1 << 13
#define UDF_ICB_FILEPERM_OWNER_DELETE			1 << 14

// File characterisic flags.
#define UDF_FILECHARFLAG_EXIST					1 << 0
#define UDF_FILECHARFLAG_DIRECTORY				1 << 1
#define UDF_FILECHARFLAG_DELETED				1 << 2
#define UDF_FILECHARFLAG_PARENT					1 << 3
#define UDF_FILECHARFLAG_METADATA				1 << 4

// Parition entity flags.
#define UDF_ENTITYFLAG_DVDVIDEO					1 << 1

namespace ckFileSystem
{
#pragma pack(1)	// Force byte alignment.

	/*
		Volume Structures.
	*/
	typedef struct
	{
		unsigned char ucType;				// Must be 0.
		unsigned char ucIdent[5];			// "BEA01", "NSR03", "TEA01".
		unsigned char ucStructVer;			// Must be 1.
		unsigned char ucStructData[2041];
	} tUdfVolStructDesc;

	typedef struct
	{
		unsigned char ucCharSetType;		// Must be 0.
		unsigned char ucCharSetInfo[63];	// "OSTA Compressed Unicode".
	} tUdfCharSpec;

	typedef struct	// ISO 13346 1/7.3.
	{
		unsigned short usTypeAndTimezone;
		unsigned short usYear;
		unsigned char ucMonth;
		unsigned char ucDay;
		unsigned char ucHour;
		unsigned char ucMinute;
		unsigned char ucSecond;
		unsigned char ucCentisec;
		unsigned char ucHundredsOfMicrosec;
		unsigned char ucMicrosec;
	} tUdfTimeStamp;

	typedef struct	// ISO 13346 1/7.4.
	{ 
		unsigned char ucFlags;
		unsigned char ucIdentifier[23];
		unsigned char ucIdentifierSuffix[8];
	} tUdfEntityIdent;

	typedef struct	// ISO 13346 3/7.2.
	{
		unsigned short usTagIdentifier;
		unsigned short usDescriptorVersion;
		unsigned char ucTagChecksum;
		unsigned char ucReserved1;
		unsigned short usTagSerialNumber;
		unsigned short usDescriptorCrc;
		unsigned short usDescriptorCrcLen;
		unsigned long ulTagLocation;
	} tUdfTag;

	/*typedef struct	// ISO 13346 4/14.5.
	{
		tUdfTag DescTag;
		unsigned long ulPrevAllocExtentLoc;
		unsigned long ulAllocDescLen;
	} tUdfAllocExtentDesc;*/

	typedef struct	// ISO 13346 3/7.1
	{
		unsigned long ulExtentLen;
		unsigned long ulExtentLoc;
	} tUdfExtentAd;

	typedef struct	// ISO 13346 3/10.1.
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		unsigned long ulPrimVolDescNum;
		unsigned char ucVolIdentifier[32];		// D-characters.
		unsigned short usVolSeqNum;
		unsigned short usMaxVolSeqNum;
		unsigned short usInterchangeLevel;
		unsigned short usMaxInterchangeLevel;
		unsigned long ulCharSetList;
		unsigned long ulMaxCharSetList;
		unsigned char ucVolSetIdent[128];	// D-characters.
		tUdfCharSpec DescCharSet;
		tUdfCharSpec ExplanatoryCharSet;
		tUdfExtentAd VolAbstract;
		tUdfExtentAd VolCopyrightNotice;
		tUdfEntityIdent ApplicationIdent;
		tUdfTimeStamp RecordDateTime;
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[64];
		unsigned long ulPredecessorVolDescSeqLoc;
		unsigned short usFlags;
		unsigned char ucReserved1[22];
	} tUdfPrimVolDesc;

	typedef struct
	{
		tUdfCharSpec LvInfoCharset;
		unsigned char LogicalVolIdent[128];		// D-characters.
		unsigned char LvInfo1[36];				// D-characters.
		unsigned char LvInfo2[36];				// D-characters.
		unsigned char LvInfo3[36];				// D-characters.
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[128];
	} tUdfLvInfo;

	typedef struct
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		tUdfEntityIdent ImplIdent;
		tUdfLvInfo LvInfo;
	} tUdfImplUseVolDesc;

	typedef struct
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		unsigned short usPartFlags;
		unsigned short usPartNum;
		tUdfEntityIdent PartConentIdent;
		unsigned char ucPartContentUse[128];
		unsigned long ulAccessType;
		unsigned long ulPartStartLoc;
		unsigned long ulPartLen;
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[128];
		unsigned char ucReserved[156];
	} tUdfPartVolDesc;

	typedef struct	// ISO 13346 3/10.6.
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		tUdfCharSpec DescriptorCharSet;
		unsigned char ucLogicalVolIdent[128];	// D-characters.
		unsigned long ulLogicalBlockSize;
		tUdfEntityIdent DomainIdent;
		unsigned char ucLogicalVolContentsUse[16];
		unsigned long ulMapTableLength;
		unsigned long ulNumPartitionMaps;
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[128];
		tUdfExtentAd IntegritySeqExtent;
		//unsigned char ucPartitionMaps[1];		// Actually ulNumPartitionMaps.
	} tUdfLogicalVolDesc;	// No maximum size.

	typedef struct	// ISO 13346 3/17
	{
		unsigned char ucPartMapType;			// Always UDF_PARTITION_MAP_TYPE1.
		unsigned char ucPartMapLen;				// Always 6.
		unsigned short usVolSeqNum;
		unsigned short usPartNum;
	} tUdfLogicalPartMapType1;

	typedef struct	// ISO 13346 3/18
	{
		unsigned char ucPartMapType;			// Always UDF_PARTITION_MAP_TYPE2.
		unsigned char ucPartMapLen;				// Always 64.
		unsigned char ucPartIdent[62];
	} tUdfLogicalPartMapType2;

	typedef struct	// ISO 13346 3/10.8.
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		unsigned long ulNumAllocDesc;
		//tUdfExtentAd AllocDesc[1];			// Actually ulNumAllocDesc.
	} tUdfUnallocSpaceDesc;	// No maximum size.

	typedef struct
	{
		tUdfTag DescTag;
		unsigned char ucReserved[496];
	} tUdfTermVolDesc;

	typedef struct	// ISO 13346 4/14.15.
	{
		unsigned __int64 uiUniqueIdent;
		unsigned char ucReserved1[24];
	} tUdfLogicalVolHeaderDesc;

	typedef struct
	{
		tUdfEntityIdent ImplIdent;
		unsigned long ulNumFiles;
		unsigned long ulNumDirectories;
		unsigned short usMinUdfRevRead;
		unsigned short usMinUdfRevWrite;
		unsigned short usMaxUdfRevWrite;
	} tUdfLogicalVolIntegrityDescImplUse;

	typedef struct	// ISO 13346 3/10.10.
	{
		tUdfTag DescTag;
		tUdfTimeStamp RecordDateTime;
		unsigned long ulIntegrityType;
		tUdfExtentAd NextIntegrityExtent;
		tUdfLogicalVolHeaderDesc LogicalVolContentsUse;
		unsigned long ulNumPartitions;
		unsigned long ulLenImplUse;
		unsigned long ulFreeSpaceTable;
		unsigned long ulSizeTable;
		tUdfLogicalVolIntegrityDescImplUse ucImplUse;
	} tUdfLogicalVolIntegrityDesc;

	typedef struct	// ISO 13346 3/10.2.
	{
		tUdfTag DescTag;
		tUdfExtentAd MainVolDescSeqExtent;
		tUdfExtentAd ReserveVolDescSeqExtent;
		unsigned char ucReserved1[480];
	} tUdfAnchorVolDescPtr;		// Must be 512 bytes.

	/*
		Partition Structures.
	*/

	typedef struct	// ISO 13346 4/7.1.
	{
		unsigned long ulLogicalBlockNum;
		unsigned short usPartitionRefNum;
	} tUdfAddrLb;

	typedef struct	// ISO 13346 - 4/14.14.1. (Short Allocation Descriptor)
	{
		unsigned long ulExtentLen;
		unsigned long ulExtentPos;
	} tUdfShortAllocDesc;

	typedef struct	// ISO 13346 - 4/14.14.2. (Long Allocation Descriptor)
	{
		unsigned long ulExtentLen;
		tUdfAddrLb ExtentLoc;
		unsigned char ucImplUse[6];
	} tUdfLongAllocDesc;

	typedef struct	// ISO 13346 4/14.1.
	{
		tUdfTag DescTag;
		tUdfTimeStamp RecordDateTime;
		unsigned short usInterchangeLevel;
		unsigned short usMaxInterchangeLevel;
		unsigned long ulCharSetList;
		unsigned long ulMaxCharSetList;
		unsigned long ulFileSetNum;
		unsigned long ulFileSetDescNum;
		tUdfCharSpec LogicalVolIdentCharSet;
		unsigned char ucLogicalVolIdent[128];	// D-characters.
		tUdfCharSpec FileSetCharSet;
		unsigned char ucFileSetIdent[32];		// D-characters.
		unsigned char ucCopyFileIdent[32];		// D-characters.
		unsigned char ucAbstFileIdent[32];		// D-characters.
		tUdfLongAllocDesc RootDirectoryIcb;
		tUdfEntityIdent DomainIdent;
		tUdfLongAllocDesc NextExtent;
		unsigned char ucReserved1[48];
	} tUdfFileSetDesc;	// Must be 512 bytes.

	typedef struct	// ISO 13346 4/14.6.
	{
		unsigned long ulPriorRecNumDirectEntries;
		unsigned short usStrategyType;
		unsigned char ucStrategyParam[2];
		unsigned short usNumEntries;
		unsigned char ucReserved1;
		unsigned char ucFileType;
		tUdfAddrLb ParentIcbLocation;
		unsigned short usFlags;
	} tUdfTagIcb;

	typedef struct	// ISO 13346 4/14.9.
	{
		tUdfTag DescTag;
		tUdfTagIcb IcbTag;
		unsigned long ulUid;
		unsigned long ulGid;
		unsigned long ulPermissions;
		unsigned short usFileLinkCount;
		unsigned char ucRecFormat;
		unsigned char ucRecDispAttr;
		unsigned long ulRecLen;
		unsigned __int64 uiInfoLen;
		unsigned __int64 uiLogicalBlocksRecorded;
		tUdfTimeStamp AccessTime;
		tUdfTimeStamp ModificationTime;
		tUdfTimeStamp AttributeTime;
		unsigned long ulCheckpoint;
		tUdfLongAllocDesc ExtendedAttrIcb;
		tUdfEntityIdent ImplIdent;
		unsigned __int64 uiUniqueIdent;
		unsigned long ulExtendedAttrLen;
		unsigned long ulAllocDescLen;

		//unsigned char ucExtendedAttr[ulExtendedAttrLen];
		//unsigned char ucAllocationDesc[ulAllocationDescLen];
	} tUdfFileEntry;	// Maximum of a logical block size.

	typedef struct	// ISO 13346 4/14.4.
	{
		tUdfTag DescTag;
		unsigned short usFileVerNum;
		unsigned char ucFileCharacteristics;
		unsigned char ucFileIdentLen;
		tUdfLongAllocDesc Icb;
		unsigned short usImplUseLen;
		//unsigned char ucImplUse[1];			// Actually usImplUseLen.
		//char szFileIdent[1];					// Actually ucFileIdentLen.
		//unsigned char ucPadding[...];
	} tUdfFileIdentDesc;	// Maximum of a logical block size.

	/*
		Extended attributes.
	*/
	typedef struct	// ISO 13346 4/14.10.1.
	{
		tUdfTag DescTag;
		unsigned long ulImplAttrLoc;
		unsigned long ulAppAttrLoc;
	} tUdfExtendedAttrHeaderDesc;

	/*typedef struct	// ISO 13346 4/14.10.8.
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLength;
		unsigned long ulImplUseLen;
		tUdfEntityIdent ImplIdent;
		//unsigned char ucImplementationUse[ulImplUseLen];
	} tUdfImplUseExtendedAttr;*/

	typedef struct	// UDF 1.02 - 3.3.4.5.1.1
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLength;
		unsigned long ulImplUseLen;
		tUdfEntityIdent ImplIdent;
		unsigned short usHeaderChecksum;
		unsigned short usFreeSpace;
	} tUdfExendedAttrFreeEaSpace;

	typedef struct	// UDF 1.02 - 3.3.4.5.1.2
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLength;
		unsigned long ulImplUseLen;
		tUdfEntityIdent ImplIdent;
		unsigned short usHeaderChecksum;
		unsigned char ucCgmsInfo;
		unsigned char ucDataStructType;
		unsigned long ulProtSysInfo;
	} tUdfExtendedAttrCgms;

	/*typedef struct	// ISO 13346 4/14.3.
	{
		tUdfShortAllocDesc UnallocSpaceTable;
		tUdfShortAllocDesc UnallocSpaceBitmap;
		tUdfShortAllocDesc PartitionIntegrityTable;
		tUdfShortAllocDesc FreedSpaceTable;
		tUdfShortAllocDesc FreedSpaceBitmap;
		unsigned char ucReserved1[88];
	} tUdfPartitionHeaderDesc;

	typedef struct	// ISO 13346 4/14.11.
	{
		tUdfTag DescTag;
		tUdfTagICB ICBTag;
		unsigned long ulAllocationDescLen;
		//unsigned char ucAllocationDesc[ulAllocationDescLen];
	} tUdfUnallocdSpaceEntry;	// Maximum of a logical block size.

	typedef struct	// ISO 13346 4/14.11.
	{
		tUdfTag DescTag;
		unsigned long ulNumBits;
		unsigned long ulNumBytes;
		//unsigned char ucBitmap[ulNumBytes];
	} tUdfSpaceBitmap;	// No maximum size.

	typedef struct	// ISO 13346 4/14.13.
	{
		tUdfTag DescTag;
		tUdfTagICB ICBTag;
		tUdfTimeStamp RecTime;
		unsigned char ucIntegrityType;
		unsigned char ucReserved1[175];
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[256];
	} tUdfPartitionIntegrityEntry;

	typedef struct	// ISO 13346 4/14.16.1.
	{
		unsigned char ucComponentType;
		unsigned char ucComponentIdentLen;
		unsigned short usComponentFileVerNum;
		//char ComponentIdentifier[ucComponentIdentLen];
	} tUdfPathComponent;*/

	/*typedef struct	// ISO 13346 4/14.10.4.
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLen;
		unsigned short usOwnerIdent;
		unsigned short usGroupIdent;
		unsigned short usPermission;
	} tUdfAltPermissionsExtendedAttr;

	typedef struct	// ISO 13346 4/14.10.5.
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLen;
		unsigned long ulDataLen;
		unsigned long ulFileTimeExistence;
		unsigned char ucFileTimes;
	} tUdfFileTimesExtendedAttr;

	typedef struct	// ISO 13346 4/14.10.7.
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLength;
		unsigned long ulImplUseLen;
		unsigned long ulMajorDevIdent;
		unsigned long ulMinorDevIdent;
		//unsigned char ucImplementationUse[ulImplUseLen];
	} tUdfDevSpecExtendedAttr;

	typedef struct	// ISO 13346 4/14.10.9.
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLength;
		unsigned long ulAppUseLen;
		tUdfEntityIdent AppIdent;
		//unsigned char ucAppUse[ulAppUseLen];
	} tUdfAppUseExtendedAttr;*/

#pragma pack()	// Switch back to normal alignment.

	class CUdf
	{
	public:
		enum ePartAccessType
		{
			AT_UNKNOWN,
			AT_READONLY,
			AT_WRITEONCE,
			AT_REWRITABLE,
			AT_OVERWRITABLE
		};

	private:
		// Enumartion of different descriptor types.
		enum eIdentType
		{
			IT_DEVELOPER,
			IT_LVINFO,
			IT_DOMAIN,
			IT_FREEEASPACE,
			IT_CGMS
		};

		tUdfPrimVolDesc m_PrimVolDesc;
		tUdfPartVolDesc m_PartVolDesc;
		tUdfLogicalVolDesc m_LogicalVolDesc;

		CCrc16 m_Crc16;

		// Determines what access will be given to the parition.
		ePartAccessType m_PartAccessType;

		// Set to true of writing a DVD-Video compatible file system.
		bool m_bDvdVideo;

		// Buffer used for various data storage. This is used for performance reasons.
		unsigned char *m_pByteBuffer;
		unsigned long m_ulByteBufferSize;

		void AllocateByteBuffer(unsigned long ulMinSize);

		size_t CompressUnicodeStr(size_t iNumChars,unsigned char ucCompID,
			const unsigned short *pInString,unsigned char *pOutString);

		void InitVolDescPrimary();
		void InitVolDescPartition();
		void InitVolDescLogical();

		void MakeCharSpec(tUdfCharSpec &CharSpec);
		void MakeIdent(tUdfEntityIdent &ImplIdent,eIdentType IdentType);
		void MakeTag(tUdfTag &Tag,unsigned short usIdentifier);
		void MakeTagChecksums(tUdfTag &Tag,unsigned char *pBuffer);
		void MakeVolSetIdent(unsigned char *pVolSetIdent,size_t iVolSetIdentSize);
		void MakeDateTime(SYSTEMTIME &st,tUdfTimeStamp &DateTime);
		void MakeOsIdentifiers(unsigned char &ucOsClass,unsigned char &ucOsIdent);

		unsigned char MakeFileIdent(unsigned char *pOutBuffer,const TCHAR *szFileName);

		unsigned short MakeExtAddrChecksum(unsigned char *pBuffer);

	public:
		CUdf(bool bDvdVideo);
		~CUdf();

		// Change of internal state functions.
		void SetVolumeLabel(const TCHAR *szLabel);
		void SetPartAccessType(ePartAccessType AccessType);

		// Write functions.
		bool WriteVolDescInitial(COutStream *pOutStream);
		bool WriteVolDescPrimary(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
			unsigned long ulSecLocation,SYSTEMTIME &stImageCreate);
		bool WriteVolDescImplUse(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
			unsigned long ulSecLocation);
		bool WriteVolDescPartition(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
			unsigned long ulSecLocation,unsigned long ulPartStartLoc,unsigned long ulPartLen);
		bool WriteVolDescLogical(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
			unsigned long ulSecLocation,tUdfExtentAd &IntegritySeqExtent);
		bool WriteVolDescUnalloc(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
			unsigned long ulSecLocation);
		bool WriteVolDescTerm(COutStream *pOutStream,unsigned long ulSecLocation);
		bool WriteVolDescLogIntegrity(COutStream *pOutStream,unsigned long ulSecLocation,
			unsigned long ulFileCount,unsigned long ulDirCount,unsigned long ulPartLen,
			unsigned __int64 uiUniqueIdent,SYSTEMTIME &stImageCreate);
		bool WriteAnchorVolDescPtr(COutStream *pOutStream,unsigned long ulSecLocation,
			tUdfExtentAd &MainVolDescSeqExtent,tUdfExtentAd &ReserveVolDescSeqExtent);

		bool WriteFileSetDesc(COutStream *pOutStream,unsigned long ulSecLocation,
			unsigned long ulRootSecLocation,SYSTEMTIME &stImageCreate);
		bool WriteFileIdentParent(COutStream *pOutStream,unsigned long ulSecLocation,
			unsigned long ulFileEntrySecLoc);
		bool WriteFileIdent(COutStream *pOutStream,unsigned long ulSecLocation,
			unsigned long ulFileEntrySecLoc,bool bIsDirectory,const TCHAR *szFileName);
		bool WriteFileEntry(COutStream *pOutStream,unsigned long ulSecLocation,
			bool bIsDirectory,unsigned short usFileLinkCount,unsigned __int64 uiUniqueIdent,
			unsigned long ulInfoLocation,unsigned __int64 uiInfoLength,
			SYSTEMTIME &stAccessTime,SYSTEMTIME &stModTime,SYSTEMTIME &stAttrTime);

		// Helper functions.
		unsigned long CalcFileIdentParentSize();
		unsigned long CalcFileIdentSize(const TCHAR *szFileName);
		unsigned long CalcFileEntrySize();

		unsigned long GetVolDescInitialSize();
	};
};
