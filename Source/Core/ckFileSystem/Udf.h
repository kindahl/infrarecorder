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

#define UDF_TAGINDENT_PRIMVOLDESC				1
#define UDF_TAGINDENT_ANCHORVOLDESCPTR			2
#define UDF_TAGINDENT_VOLDESCPTR				3
#define UDF_TAGINDENT_IMPLUSEVOLDESC			4
#define UDF_TAGINDENT_PARTDESC					5
#define UDF_TAGINDENT_LOGICALVOLDESC			6
#define UDF_TAGINDENT_UNALLOCATEDSPACEDESC		7
#define UDF_TAGINDENT_TERMDESC					8
#define UDF_TAGINDENT_LOGICALVOLINTEGRITYDESC	9

#define UDF_OSCLASS_UNDEFINED					0
#define UDF_OSCLASS_DOS							1
#define UDF_OSCLASS_OS2							2
#define UDF_OSCLASS_MACOS						3
#define UDF_OSCLASS_UNIX						4

#define UDF_OSINDENTIFIER_DOS					0
#define UDF_OSINDENTIFIER_OS2					0
#define UDF_OSINDENTIFIER_MACOS					0
#define UDF_OSINDENTIFIER_UNIX_GENERIC			0
#define UDF_OSINDENTIFIER_UNIX_AIX				1
#define UDF_OSINDENTIFIER_UNIX_SOLARIS			2
#define UDF_OSINDENTIFIER_UNIX_HPUX				3
#define UDF_OSINDENTIFIER_UNIX_IRIX				4

namespace ckFileSystem
{
#pragma pack(1)	// Force byte alignment.

	/*typedef struct	// ISO 13346 4/7.1.
	{
		unsigned long ulLogicalBlockNum;
		unsigned short usPartitionRefNum;
	} tUdfAddrLB;

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
	} tUdfTimestamp;

	typedef struct	// ISO 13346 1/7.4.
	{ 
		unsigned char ucFlags;
		char ucIdentifier[23];
		char ucIdentifierSuffix[8];
	} tUdfEntityIdent;*/

	typedef struct	// ISO 13346 3/7.2.
	{
		unsigned short usTagIdentifier;
		unsigned short usDescriptorVersion;
		unsigned char ucTagChecksum;
		unsigned char ucReserved1;
		unsigned short usTagSerialNumber;
		unsigned short usDescriptorCRC;
		unsigned short usDescriptorCRCLength;
		unsigned long ulTagLocation;
	} tUdfTag;

	/*typedef struct	// ISO 13346 4/14.6.
	{
		unsigned long ulPriorRecNumDirectEntries;
		unsigned short usStrategyType;
		unsigned char ucStrategyParameter[2];
		unsigned short usNumEntries;
		unsigned char ucReserved1;
		unsigned char ucFileType;
		tUdfAddrLB ParentICBLocation;
		unsigned short usFlags;
	} tUdfTagICB;

	typedef struct	// ISO 13346 4/14.14.2.
	{
		unsigned long ulExtentLen;
		tUdfAddrLB ExtentLoc;
		unsigned char ucImplUse[6];
	} tUdfLongAllocDesc;

	typedef struct	// ISO 13346 4/14.5.
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

	/*typedef struct	// ISO 13346 4/14.14.1.
	{
		unsigned long ulExtentLen;
		unsigned long ulExtentPos;
	} tUdfShortAd;

	typedef struct	// ISO 13346 3/10.1.
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		unsigned long ulPrimVolDescNum;
		unsigned char ucVolumeIdentifier[32];		// D-characters.
		unsigned short usVolSeqNum;
		unsigned short usMaxVolSeqNum;
		unsigned short usInterchangeLevel;
		unsigned short usMaxInterchangeLevel;
		unsigned long ulCharSetList;
		unsigned long ulMaxCharSetList;
		unsigned char ucVolumeSetIdent[128];		// D-characters.
		tUdfCharSpec DescCharSet;
		tUdfCharSpec ExplanatoryCharSet;
		tUdfExtentAd VolAbstract;
		tUdfExtentAd VolCopyrightNotice;
		tUdfEntityIdent ApplicationIdent;
		tUdfTimestamp RecordDateTime;
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[64];
		unsigned long ulPredecessorVolDescSeqLoc;
		unsigned short usFlags;
		unsigned char ucReserved1[22];
	} tUdfPriVolDesc;*/

	typedef struct	// ISO 13346 3/10.2.
	{
		tUdfTag DescTag;
		tUdfExtentAd MainVolDescSeqExtent;
		tUdfExtentAd ReserveVolDescSeqExtent;
		unsigned char ucReserved1[480];
	} tUdfAnchorVolDescPtr;		// Must be 512 bytes.

	/*typedef struct	// ISO 13346 3/10.6.
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		tUdfCharSpec DescriptorCharacterSet;
		unsigned char ucLogicalVolIdent[128];	// D-characters.
		unsigned long ulLogicalBlockSize;
		tUdfEntityIdent DomainIdent;
		unsigned char ucLogicalVolContentsUse[16];
		unsigned long ulMapTableLength;
		unsigned long ulNumPartitionMaps;
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[128];
		tUdfExtentAd IntegritySeqExtent;
		unsigned char ucPartitionMaps[1];		// Actually ulNumPartitionMaps.
	} tUdfLogicalVolDesc;	// No maximum size.

	typedef struct	// ISO 13346 3/10.8.
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		unsigned long ulNumAllocDesc;
		//tUdfExtentAd AllocDesc[ulNumAllocDesc];
	} tUdfUnallocSpaceDesc;	// No maximum size.

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
		//unsigned char ucImpl[];
	} tUdfLogicalVolIntegrityDescImplUse;

	typedef struct	// ISO 13346 3/10.10.
	{
		tUdfTag DescTag;
		tUdfTimestamp RecDateTime;
		unsigned long ulIntegrityType;
		tUdfExtentAd NextIntegrityExtent;
		tUdfLogicalVolHeaderDesc LogicalVolContentsUse;
		unsigned long ulNumPartitions;
		unsigned long ulLenImplUse;
		unsigned long ulFreeSpaceTable;
		unsigned long ulSizeTable;
		tUdfLogicalVolIntegrityDescImplUse ucImplUse;
	} tUdfLogicalVolIntegrityDesc;

	typedef struct
	{
		tUdfTag DescTag;
		unsigned long ulVolDescSeqNum;
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[460];
	} tUdfImplUseVolumeDesc;

	typedef struct
	{
		tUdfCharSpec LVICharset;
		unsigned char LogicalVolumeIdent[128];	// D-characters.
		unsigned char LVInfo1[36];	// D-characters.
		unsigned char LVInfo2[36];	// D-characters.
		unsigned char LVInfo3[36];	// D-characters.
		tUdfEntityIdent ImplIdent;
		unsigned char ucImplUse[128];
	} tUdfLVInfo;

	typedef struct	// ISO 13346 4/14.1.
	{
		tUdfTag DescTag;
		tUdfTimestamp RecDateTime;
		unsigned short usInterchangeLevel;
		unsigned short usMaxInterchangeLevel;
		unsigned long ulCharSetList;
		unsigned long ulMaxCharSetList;
		unsigned long ulFileSetNum;
		unsigned long ulFileSetDescNum;
		tUdfCharSpec LogicalVolumeIdentifierCharacterSet;
		unsigned char ucLogicalVolIdent[128];	// D-characters.
		tUdfCharSpec FileSetCharacterSet;
		unsigned char ucFileSetIdent[32];		// D-characters.
		unsigned char ucCopyFileIdent[32];		// D-characters.
		unsigned char ucAbstFileIdent[32];		// D-characters.
		tUdfLongAllocDesc RootDirectoryICB;
		tUdfEntityIdent DomainIdent;
		tUdfLongAllocDesc NextExtent;
		unsigned char ucReserved1[48];
	} tUdfFileSetDesc;	// Must be 512 bytes.

	typedef struct	// ISO 13346 4/14.3.
	{
		tUdfShortAd UnallocSpaceTable;
		tUdfShortAd UnallocSpaceBitmap;
		tUdfShortAd PartitionIntegrityTable;
		tUdfShortAd FreedSpaceTable;
		tUdfShortAd FreedSpaceBitmap;
		unsigned char ucReserved1[88];
	} tUdfPartitionHeaderDesc;

	typedef struct	// ISO 13346 4/14.4.
	{
		tUdfTag DescTag;
		unsigned short usFileVerNum;
		unsigned char ucFileCharacteristics;
		unsigned char ucFileIdentLen;
		tUdfLongAllocDesc ICB;
		unsigned short usImplUseLen;
		//unsigned char ucImplUse[usImplUseLen];
		//char szFileIdent[ucFileIdentLen];
		//unsigned char ucPadding[...];
	} tUdfFileIdentDesc;	// Maximum of a logical block size.

	typedef struct	// ISO 13346 4/14.9.
	{
		tUdfTag DescTag;
		tUdfTagICB ICBTag;
		unsigned long ulUid;
		unsigned long ulGid;
		unsigned long ulPermissions;
		unsigned short usFileLinkCount;
		unsigned char ucRecFormat;
		unsigned char ucRecDispAttr;
		unsigned long ulRecLen;
		unsigned __int64 uiInfoLe;
		unsigned __int64 uiLogicalBlocksRecorded;
		tUdfTimestamp AccessTime;
		tUdfTimestamp ModificationTime;
		tUdfTimestamp AttributeTime;
		unsigned long ulCheckpoint;
		tUdfLongAllocDesc ExtendedAttrICB;
		tUdfEntityIdent ImplIdent;
		unsigned __int64 uiUniqueIdent;
		unsigned long ulExtendedAttrLen;
		unsigned long ulAllocationDescLen;
		//unsigned char ucExtendedAttr[ulExtendedAttrLen];
		//unsigned char ucAllocationDesc[ulAllocationDescLen];
	} tUdfFileEntry;	// Maximum of a logical block size.

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
		tUdfTimestamp RecTime;
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
	} tUdfPathComponent;

	typedef struct	// ISO 13346 4/14.10.1.
	{
		tUdfTag DescTag;
		unsigned long ulImplAttrLoc;
		unsigned long ulAppAttrLoc;
	} tUdfExtendedAttrHeaderDesc;

	typedef struct	// ISO 13346 4/14.10.4.
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

	typedef struct	// ISO 13346 4/14.10.8.
	{
		unsigned long ulAttrType;
		unsigned char ucAttrSubtype;
		unsigned char ucReserved1[3];
		unsigned long ulAttrLength;
		unsigned long ulImplUseLen;
		tUdfEntityIdent ImplIdent;
		//unsigned char ucImplementationUse[ulImplUseLen];
	} tUdfImplUseExtendedAttr;

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

	typedef struct
	{
		unsigned char ucType;			// Must be 0.
		unsigned char ucIdent[5];		// "BEA01", "NSR03", "TEA01".
		unsigned char ucStructVer;		// Must be 1.
		unsigned char ucStructData[2041];
	} tUdfVolStructDesc;

#pragma pack()	// Switch back to normal alignment.

	class CUdf
	{
	public:
		CUdf();
		~CUdf();

		bool WriteVolDesc(COutStream *pOutStream);
		bool WriteAnchorVolDescPtr(COutStream *pOutStream,unsigned long ulSecLocation,
			tUdfExtentAd MainVolDescSeqExtent,tUdfExtentAd ReserveVolDescSeqExtent);

		unsigned long GetVolDescSize();
	};
};
