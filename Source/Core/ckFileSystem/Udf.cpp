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
#include "Udf.h"
#include "Iso9660.h"
#include "../../Common/StringUtil.h"

namespace ckFileSystem
{
	/*
		Identifiers.
	*/
	const unsigned char g_pIdentUdfCharSet[] = {
		0x4F,0x53,0x54,0x41,0x20,0x43,0x6F,0x6D,0x70,0x72,0x65,0x73,
		0x73,0x65,0x64,0x20,0x55,0x6E,0x69,0x63,0x6F,0x64,0x65 };
	const unsigned char g_pIdentUdfEntityCompliant[] = {
		0x2A,0x4F,0x53,0x54,0x41,0x20,0x55,0x44,0x46,0x20,0x43,0x6F,
		0x6D,0x70,0x6C,0x69,0x61,0x6E,0x74 };

	// Used.
	const unsigned char g_pIdentUdfEntityLVInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4C,0x56,0x20,0x49,0x6E,0x66,0x6F };
	const unsigned char g_pIdentUdfEntityDomain[] = {
		0x2A,0x4F,0x53,0x54,0x41,0x20,0x55,0x44,0x46,0x20,0x43,0x6F,
		0x6D,0x70,0x6C,0x69,0x61,0x6E,0x74 };
	const unsigned char g_pIdentUdfFreeEaSpace[] = {
		0x2A,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x45,0x41,0x53,
		0x70,0x61,0x63,0x65 };
	const unsigned char g_pUdentUdfCgms[] = {
		0x2A,0x55,0x44,0x46,0x20,0x44,0x56,0x44,0x20,0x43,0x47,0x4D,
		0x53,0x20,0x49,0x6E,0x66,0x6F
	};

	const unsigned char g_pIdentUdfEntityFreeEASpace[] = {
		0x2A,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x45,0x41,0x53,
		0x70,0x61,0x63,0x65 };
	const unsigned char g_pIdentUdfEntityFreeAppEASpace[] = {
		0x2A,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x41,0x70,0x70,
		0x45,0x41,0x53,0x70,0x61,0x63,0x65 };
	const unsigned char g_pIdentUdfEntityDVDCGMSInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x44,0x56,0x44,0x20,0x43,0x47,0x4D,
		0x53,0x20,0x49,0x6E,0x66,0x6F };
	const unsigned char g_pIdentUdfEntityOS2EA[] = {
		0x2A,0x55,0x44,0x46,0x41,0x20,0x45,0x41};
	const unsigned char g_pIdentUdfEntityOS2EALen[] = {
		0x2A,0x55,0x44,0x46,0x20,0x45,0x41,0x4C,0x65,0x6E,0x67,0x74,
		0x68 };
	const unsigned char g_pIdentUdfEntityMacVolInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x56,0x6F,0x6C,
		0x75,0x6D,0x65,0x49,0x6E,0x66,0x6F };
	const unsigned char g_pIdentUdfEntityMacFinderInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x49,0x69,0x6E,
		0x64,0x65,0x72,0x49,0x6E,0x66,0x6F };
	const unsigned char g_pIdentUdfEntityMacUniqueTable[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x55,0x6E,0x69,
		0x71,0x75,0x65,0x49,0x44,0x54,0x61,0x62,0x6C,0x65 };
	const unsigned char g_pIdentUdfEntityMacResFork[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x52,0x65,0x73,
		0x6F,0x75,0x72,0x63,0x65,0x46,0x6F,0x72,0x6B };

	// Initial volume descriptor identifiers.
	const char *g_pIdentBEA = "BEA01";
	const char *g_pIdentNSR = "NSR02";
	const char *g_pIdentTEA = "TEA01";

	const char *g_IdentPartContentFdc = "+FDC01";	// As if it were a volume recorded according to ECMA-107.
	const char *g_IdentPartContentCd = "+CD001";	// As if it were a volume recorded according to ECMA-119.
	const char *g_IdentPartContentCdw = "+CDW02";	// As if it were a volume recorded according to ECMA-168.
	const char *g_IdentPartContentNsr = "+NSR02";	// According to Part 4 of this ECMA Standard.

	CUdf::CUdf(bool bDvdVideo) : m_Crc16(UDF_CRC_POLYNOMIAL),m_bDvdVideo(bDvdVideo)
	{
		InitVolDescPrimary();
		InitVolDescPartition();
		InitVolDescLogical();

		// Intialize the byte buffer.
		m_pByteBuffer = NULL;
		m_ulByteBufferSize = 0;

		// Default parition type is read only.
		m_PartAccessType = AT_READONLY;
	}

	CUdf::~CUdf()
	{
		if (m_pByteBuffer != NULL)
		{
			delete [] m_pByteBuffer;
			m_pByteBuffer = NULL;

			m_ulByteBufferSize = 0;
		}
	}

	void CUdf::AllocateByteBuffer(unsigned long ulMinSize)
	{
		if (m_ulByteBufferSize < ulMinSize)
		{
			if (m_pByteBuffer != NULL)
				delete [] m_pByteBuffer;

			m_ulByteBufferSize = ulMinSize;
			m_pByteBuffer = new unsigned char[m_ulByteBufferSize];
		}
	}

	/*
		Takes a string of unicode wide characters and returns an OSTA CS0
		compressed unicode string. The unicode MUST be in the byte order of
		the compiler in order to obtain correct results. Returns an error
		if the compression ID is invalid.

		Note: This routine assumes the implementation already knows, by
		the local environment, how many bits are appropriate and therefore does
		no checking to test if the input characters fit into that number of
		bits or not.

		The function returns the total number of bytes in the compressed OSTA
		CS0 string, including the compression ID. -1 is returned if the
		compression ID is invalid.
	*/
	size_t CUdf::CompressUnicodeStr(size_t iNumChars,unsigned char ucCompID,
		const unsigned short *pInString,unsigned char *pOutString)
	{
		if (ucCompID != 8 && ucCompID != 16)
			return -1;

		// Place compression code in first byte.
		pOutString[0] = ucCompID;

		size_t iByteIndex = 1,iUnicodeIndex = 0;
		while (iUnicodeIndex < iNumChars)
		{
			if (ucCompID == 16)
			{
				// First, place the high bits of the char into the byte stream.
				pOutString[iByteIndex++] = (pInString[iUnicodeIndex] & 0xFF00) >> 8;
			}
			// Then place the low bits into the stream.
			pOutString[iByteIndex++] = pInString[iUnicodeIndex] & 0x00FF;
			iUnicodeIndex++;
		}

		return iByteIndex;
	}

	/*
		Helper function for filling a tUdfCharSpec structure.
	*/
	void CUdf::MakeCharSpec(tUdfCharSpec &CharSpec)
	{
		memset(&CharSpec,0,sizeof(tUdfCharSpec));
		CharSpec.ucCharSetType = 0;
		memcpy(CharSpec.ucCharSetInfo,g_pIdentUdfCharSet,sizeof(g_pIdentUdfCharSet));
	}

	/*
		Helper function for filling a tUdfEntityIdent structure.
	*/
	void CUdf::MakeIdent(tUdfEntityIdent &ImplIdent,eIdentType IdentType)
	{
		ImplIdent.ucFlags = 0;
		memset(ImplIdent.ucIdentifier,0,sizeof(ImplIdent.ucIdentifier));
		memset(ImplIdent.ucIdentifierSuffix,0,sizeof(ImplIdent.ucIdentifierSuffix));

		unsigned char ucOsClass,ucOsIdent;
		MakeOsIdentifiers(ucOsClass,ucOsIdent);

		switch (IdentType)
		{
			case IT_DEVELOPER:
				{
					char szAppIdentifier[] = { 0x2A,0x49,0x6E,0x66,0x72,0x61,0x52,0x65,0x63,0x6F,0x72,0x64,0x65,0x72 };
					memcpy(ImplIdent.ucIdentifier,szAppIdentifier,sizeof(szAppIdentifier));

					ImplIdent.ucIdentifierSuffix[0] = ucOsClass;
					ImplIdent.ucIdentifierSuffix[1] = ucOsIdent;
				}
				break;

			case IT_LVINFO:
				memcpy(ImplIdent.ucIdentifier,g_pIdentUdfEntityLVInfo,sizeof(g_pIdentUdfEntityLVInfo));

				Write721(ImplIdent.ucIdentifierSuffix,0x0102);	// Currently only UDF 1.02 is supported.

				ImplIdent.ucIdentifierSuffix[2] = ucOsClass;
				ImplIdent.ucIdentifierSuffix[3] = ucOsIdent;
				break;

			case IT_DOMAIN:
				memcpy(ImplIdent.ucIdentifier,g_pIdentUdfEntityDomain,sizeof(g_pIdentUdfEntityDomain));

				Write721(ImplIdent.ucIdentifierSuffix,0x0102);	// Currently only UDF 1.02 is supported.

				ImplIdent.ucIdentifierSuffix[2] = UDF_DOMAIN_FLAG_HARD_WRITEPROTECT | UDF_DOMAIN_FLAG_SOFT_WRITEPROTECT;
				ImplIdent.ucIdentifierSuffix[3] = ucOsIdent;
				break;

			case IT_FREEEASPACE:
				memcpy(ImplIdent.ucIdentifier,g_pIdentUdfFreeEaSpace,sizeof(g_pIdentUdfFreeEaSpace));

				Write721(ImplIdent.ucIdentifierSuffix,0x0102);	// Currently only UDF 1.02 is supported.
				break;

			case IT_CGMS:
				memcpy(ImplIdent.ucIdentifier,g_pUdentUdfCgms,sizeof(g_pUdentUdfCgms));

				Write721(ImplIdent.ucIdentifierSuffix,0x0102);	// Currently only UDF 1.02 is supported.
				break;
		}
	}

	/*
		Helper function for creating a tag.
	*/
	void CUdf::MakeTag(tUdfTag &Tag,unsigned short usIdentifier)
	{
		memset(&Tag,0,sizeof(tUdfTag));
		Tag.usTagIdentifier = usIdentifier;
		Tag.usDescriptorVersion = UDF_TAG_DESCRIPTOR_VERSION;
		Tag.usTagSerialNumber = 1;
		Tag.usDescriptorCrc = 0;
		Tag.usDescriptorCrcLen = 0;
	}

	/*
		Helper function for calculating descriptor CRC and tag checksum.
	*/
	void CUdf::MakeTagChecksums(tUdfTag &Tag,unsigned char *pBuffer)
	{
		Tag.usDescriptorCrc = m_Crc16.CalcCrc(pBuffer,Tag.usDescriptorCrcLen);

		// Sum of bytes 0-3 and 5-15 modulo 256.
		unsigned char ucChecksum = 0;
		for (int i = 0; i < sizeof(tUdfTag); i++)
			ucChecksum += ((unsigned char *)&Tag)[i];

		Tag.ucTagChecksum = ucChecksum;
	}

	unsigned short CUdf::MakeExtAddrChecksum(unsigned char *pBuffer)
	{
		unsigned short usChecksum = 0;
		for (unsigned int i = 0; i < 48; i++)
			usChecksum += *pBuffer++;

		return usChecksum;
	}

	/*
		Generates a unique volume set indentifer to identify a particular
		volume. pVolSysIdent is assumed to hold 128 bytes.
	*/
	void CUdf::MakeVolSetIdent(unsigned char *pVolSetIdent,size_t iVolSetIdentSize)
	{
		if (iVolSetIdentSize < 18)
			return;

		TCHAR szCharSet[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
		wchar_t szGeneratedIdent[16];
		for (unsigned int i = 0; i < sizeof(szGeneratedIdent)/sizeof(wchar_t); i++)
			szGeneratedIdent[i] = szCharSet[rand() % 16];

		// Make a compatible D-string.
		memset(pVolSetIdent,0,iVolSetIdentSize);

		unsigned char ucByteLen = (unsigned char)CompressUnicodeStr(16,
			UDF_COMPRESSION_BYTE,(const unsigned short *)szGeneratedIdent,pVolSetIdent);

		pVolSetIdent[iVolSetIdentSize - 1] = ucByteLen;
	}

	void CUdf::MakeDateTime(SYSTEMTIME &st,tUdfTimeStamp &DateTime)
	{
		DateTime.usTypeAndTimezone = 1 << 12;		// Type: local time.

		TIME_ZONE_INFORMATION tzi;
		GetTimeZoneInformation(&tzi);
		DateTime.usTypeAndTimezone |= -tzi.Bias;

		DateTime.usYear = st.wYear;
		DateTime.ucMonth = (unsigned char)st.wMonth;
		DateTime.ucDay = (unsigned char)st.wDay;
		DateTime.ucHour = (unsigned char)st.wHour;
		DateTime.ucMinute = (unsigned char)st.wMinute;
		DateTime.ucSecond = (unsigned char)st.wSecond;
		DateTime.ucCentisec = st.wMilliseconds/10;
		DateTime.ucHundredsOfMicrosec = 0;
		DateTime.ucMicrosec = 0;
	}

	void CUdf::MakeOsIdentifiers(unsigned char &ucOsClass,unsigned char &ucOsIdent)
	{
		// UDF 1.02 does not support any Windows identification.
		ucOsClass = UDF_OSCLASS_UNDEFINED;
		ucOsIdent = UDF_OSIDENT_UNDEFINED;
	}

	unsigned char CUdf::MakeFileIdent(unsigned char *pOutBuffer,const TCHAR *szFileName)
	{
		size_t iNameLen = lstrlen(szFileName);
		size_t iCopyLen = iNameLen < (254 >> 1) ? iNameLen : (254 >> 1);		// One byte is reserved for compression descriptor.

		// DVD-Video should use 8 bits to represent one character.
		unsigned char ucStrComp = m_bDvdVideo ? UDF_COMPRESSION_BYTE : UDF_COMPRESSION_UNICODE;

	#ifdef UNICODE
		unsigned char ucByteLen = (unsigned char)CompressUnicodeStr(iCopyLen,ucStrComp,
			(const unsigned short *)szFileName,pOutBuffer);
	#else
		wchar_t szWideFileName[125];
		AnsiToUnicode(szWideFileName,szFileName,sizeof(szWideFileName) / sizeof(wchar_t));

		unsigned char ucByteLen = (unsigned char)CompressUnicodeStr(iCopyLen,ucStrComp,
			(const unsigned short *)szWideFileName,pOutBuffer);
	#endif
		return ucByteLen;
	}

	void CUdf::InitVolDescPrimary()
	{
		memset(&m_PrimVolDesc,0,sizeof(tUdfPrimVolDesc));

		// Other members.
		MakeIdent(m_PrimVolDesc.ImplIdent,IT_DEVELOPER);
		MakeIdent(m_PrimVolDesc.ApplicationIdent,IT_DEVELOPER);

		MakeCharSpec(m_PrimVolDesc.DescCharSet);
		MakeCharSpec(m_PrimVolDesc.ExplanatoryCharSet);

		MakeVolSetIdent(m_PrimVolDesc.ucVolSetIdent,sizeof(m_PrimVolDesc.ucVolSetIdent));
	}

	void CUdf::InitVolDescPartition()
	{
		memset(&m_PartVolDesc,0,sizeof(tUdfPartVolDesc));

		// Other members.
		m_PartVolDesc.usPartFlags = UDF_PARTITION_FLAG_ALLOCATED;
		m_PartVolDesc.usPartNum = 0;		// We always create the first parition using these routines.

		m_PartVolDesc.PartConentIdent.ucFlags = m_bDvdVideo ? UDF_ENTITYFLAG_DVDVIDEO : 0;
		memcpy(m_PartVolDesc.PartConentIdent.ucIdentifier,g_IdentPartContentNsr,strlen(g_IdentPartContentNsr));

		switch (m_PartAccessType)
		{
			case AT_UNKNOWN:
				m_PartVolDesc.ulAccessType = UDF_PARTITION_ACCESS_UNKNOWN;
				break;
			case AT_WRITEONCE:
				m_PartVolDesc.ulAccessType = UDF_PARTITION_ACCESS_WRITEONCE;
				break;
			case AT_REWRITABLE:
				m_PartVolDesc.ulAccessType = UDF_PARTITION_ACCESS_REWRITABLE;
				break;
			case AT_OVERWRITABLE:
				m_PartVolDesc.ulAccessType = UDF_PARTITION_ACCESS_OVERWRITABLE;
				break;
			//case AT_READONLY:
			default:
				m_PartVolDesc.ulAccessType = UDF_PARTITION_ACCESS_READONLY;
				break;
		}

		MakeIdent(m_PartVolDesc.ImplIdent,IT_DEVELOPER);
	}

	void CUdf::InitVolDescLogical()
	{
		memset(&m_LogicalVolDesc,0,sizeof(tUdfLogicalVolDesc));

		// Other members.
		MakeCharSpec(m_LogicalVolDesc.DescriptorCharSet);

		m_LogicalVolDesc.ulLogicalBlockSize = UDF_SECTOR_SIZE;

		MakeIdent(m_LogicalVolDesc.DomainIdent,IT_DOMAIN);
		MakeIdent(m_LogicalVolDesc.ImplIdent,IT_DEVELOPER);
	
		Write731(m_LogicalVolDesc.ucLogicalVolContentsUse,UDF_SECTOR_SIZE);	// ?
	}

	void CUdf::SetVolumeLabel(const TCHAR *szLabel)
	{
		size_t iLabelLen = lstrlen(szLabel);
		size_t iPrimaryCopyLen = iLabelLen < 15 ? iLabelLen : 15;	// Two bytes are reserved for string format.
		size_t iLogicalCopyLen = iLabelLen < 63 ? iLabelLen : 63;	// Two bytes are reserved for string format.

		// We need to update both the logical and primary descriptor identifiers.
		memset(m_PrimVolDesc.ucVolIdentifier,0,sizeof(m_PrimVolDesc.ucVolIdentifier));
		memset(m_LogicalVolDesc.ucLogicalVolIdent,0,sizeof(m_LogicalVolDesc.ucLogicalVolIdent));

		// DVD-Video should use 8 bits to represent one character.
		unsigned char ucStrComp = m_bDvdVideo ? UDF_COMPRESSION_BYTE : UDF_COMPRESSION_UNICODE;

	#ifdef UNICODE
		unsigned char ucByteLen = (unsigned char)CompressUnicodeStr(iPrimaryCopyLen,ucStrComp,
			(const unsigned short *)szLabel,m_PrimVolDesc.ucVolIdentifier);
		m_PrimVolDesc.ucVolIdentifier[31] = ucByteLen;

		ucByteLen = (unsigned char)CompressUnicodeStr(iLogicalCopyLen,ucStrComp,
			(const unsigned short *)szLabel,m_LogicalVolDesc.ucLogicalVolIdent);
		m_LogicalVolDesc.ucLogicalVolIdent[127] = ucByteLen;
	#else
		wchar_t szWidePrimaryLabel[17];
		AnsiToUnicode(szWidePrimaryLabel,szLabel,sizeof(szWidePrimaryLabel) / sizeof(wchar_t));

		unsigned char ucByteLen = (unsigned char)CompressUnicodeStr(iPrimaryCopyLen,ucStrComp,
			(const unsigned short *)szWidePrimaryLabel,m_PrimVolDesc.ucVolIdentifier);
		m_PrimVolDesc.ucVolIdentifier[31] = ucByteLen;

		wchar_t szWideLogicalLabel[17];
		AnsiToUnicode(szWideLogicalLabel,szLabel,sizeof(szWideLogicalLabel) / sizeof(wchar_t));
		ucByteLen = (unsigned char)CompressUnicodeStr(iLogicalCopyLen,ucStrComp,
			(const unsigned short *)szWideLogicalLabel,m_LogicalVolDesc.ucLogicalVolIdent);
		m_LogicalVolDesc.ucLogicalVolIdent[127] = ucByteLen;
	#endif
	}

	void CUdf::SetPartAccessType(ePartAccessType AccessType)
	{
		m_PartAccessType = AccessType;
	}

	/*
		Write the initial volume descriptors. They're of the same format as the
		ISO9660 volume descriptors.
	*/
	bool CUdf::WriteVolDescInitial(COutStream *pOutStream)
	{
		tUdfVolStructDesc VolStructDesc;
		memset(&VolStructDesc,0,sizeof(tUdfVolStructDesc));
		VolStructDesc.ucType = 0;
		VolStructDesc.ucStructVer = 1;

		unsigned long ulProcessedSize;
		memcpy(VolStructDesc.ucIdent,g_pIdentBEA,sizeof(VolStructDesc.ucIdent));
		if (pOutStream->Write(&VolStructDesc,sizeof(tUdfVolStructDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfVolStructDesc))
			return false;

		memcpy(VolStructDesc.ucIdent,g_pIdentNSR,sizeof(VolStructDesc.ucIdent));
		if (pOutStream->Write(&VolStructDesc,sizeof(tUdfVolStructDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfVolStructDesc))
			return false;

		memcpy(VolStructDesc.ucIdent,g_pIdentTEA,sizeof(VolStructDesc.ucIdent));
		if (pOutStream->Write(&VolStructDesc,sizeof(tUdfVolStructDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfVolStructDesc))
			return false;

		return true;
	}

	bool CUdf::WriteVolDescPrimary(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
		unsigned long ulSecLocation,SYSTEMTIME &stImageCreate)
	{
		// Make the tag.
		MakeTag(m_PrimVolDesc.DescTag,UDF_TAGIDENT_PRIMVOLDESC);
		m_PrimVolDesc.DescTag.ulTagLocation = ulSecLocation;
		m_PrimVolDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfPrimVolDesc) - sizeof(tUdfTag);

		// Update the primary volume descriptor data.
		m_PrimVolDesc.ulVolDescSeqNum = ulVolDescSeqNum;
		m_PrimVolDesc.ulPrimVolDescNum = ulVolDescSeqNum;

		m_PrimVolDesc.usVolSeqNum = 1;		// This is the first disc in the volume set.
		m_PrimVolDesc.usMaxVolSeqNum = 1;

		m_PrimVolDesc.usInterchangeLevel = UDF_INTERCHANGE_LEVEL_SINGLESET;
		m_PrimVolDesc.usMaxInterchangeLevel = UDF_INTERCHANGE_LEVEL_SINGLESET;
		m_PrimVolDesc.ulCharSetList = 1;
		m_PrimVolDesc.ulMaxCharSetList = 1;

		//m_PrimVolDesc.VolAbstract;
		//m_PrimVolDesc.VolCopyrightNotice;
		//m_PrimVolDesc.ApplicationIdent;
		MakeDateTime(stImageCreate,m_PrimVolDesc.RecordDateTime);

		//m_PrimVolDesc.ucImplUse[64];
		//m_PrimVolDesc.ulPredecessorVolDescSeqLoc;
		//m_PrimVolDesc.usFlags;
		//m_PrimVolDesc.ucReserved1[22];

		// Calculate checksums.
		MakeTagChecksums(m_PrimVolDesc.DescTag,(unsigned char *)(&m_PrimVolDesc) + sizeof(tUdfTag));

		unsigned long ulProcessedSize;
		if (pOutStream->Write(&m_PrimVolDesc,sizeof(tUdfPrimVolDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfPrimVolDesc))
			return false;

		// Pad the sector from 512 to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfPrimVolDesc)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	bool CUdf::WriteVolDescImplUse(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
		unsigned long ulSecLocation)
	{	
		tUdfImplUseVolDesc ImplUseVolDesc;
		memset(&ImplUseVolDesc,0,sizeof(tUdfImplUseVolDesc));

		// Create tag.
		MakeTag(ImplUseVolDesc.DescTag,UDF_TAGIDENT_IMPLUSEVOLDESC);
		ImplUseVolDesc.DescTag.ulTagLocation = ulSecLocation;
		ImplUseVolDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfImplUseVolDesc) - sizeof(tUdfTag);

		MakeIdent(ImplUseVolDesc.ImplIdent,IT_LVINFO);

		ImplUseVolDesc.ulVolDescSeqNum = ulVolDescSeqNum;

		MakeCharSpec(ImplUseVolDesc.LvInfo.LvInfoCharset);
		memcpy(ImplUseVolDesc.LvInfo.LogicalVolIdent,m_LogicalVolDesc.ucLogicalVolIdent,
			sizeof(ImplUseVolDesc.LvInfo.LogicalVolIdent));		// Steal the value from the logical descriptor.
		MakeIdent(ImplUseVolDesc.LvInfo.ImplIdent,IT_DEVELOPER);

		// Calculate tag checksums.
		MakeTagChecksums(ImplUseVolDesc.DescTag,(unsigned char *)(&ImplUseVolDesc) + sizeof(tUdfTag));

		unsigned long ulProcessedSize;
		if (pOutStream->Write(&ImplUseVolDesc,sizeof(tUdfImplUseVolDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfImplUseVolDesc))
			return false;

		// Pad the sector from 512 to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfImplUseVolDesc)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	/**
		@param ulPartLen is the partition size in sectors.
	*/
	bool CUdf::WriteVolDescPartition(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
		unsigned long ulSecLocation,unsigned long ulPartStartLoc,unsigned long ulPartLen)
	{
		// Make the tag.
		MakeTag(m_PartVolDesc.DescTag,UDF_TAGIDENT_PARTDESC);
		m_PartVolDesc.DescTag.ulTagLocation = ulSecLocation;
		m_PartVolDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfPartVolDesc) - sizeof(tUdfTag);

		m_PartVolDesc.ulVolDescSeqNum = ulVolDescSeqNum;
		m_PartVolDesc.ulPartStartLoc = ulPartStartLoc;	// nero_udfiso: 267
		m_PartVolDesc.ulPartLen = ulPartLen;			// nero_udfiso: 67

		// Calculate tag checksums.
		MakeTagChecksums(m_PartVolDesc.DescTag,(unsigned char *)(&m_PartVolDesc) + sizeof(tUdfTag));

		unsigned long ulProcessedSize;
		if (pOutStream->Write(&m_PartVolDesc,sizeof(tUdfPartVolDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfPartVolDesc))
			return false;

		// Pad the sector from 512 to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfPartVolDesc)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	bool CUdf::WriteVolDescLogical(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
		unsigned long ulSecLocation,tUdfExtentAd &IntegritySeqExtent)
	{
		// Make the tag.
		MakeTag(m_LogicalVolDesc.DescTag,UDF_TAGIDENT_LOGICALVOLDESC);
		m_LogicalVolDesc.DescTag.ulTagLocation = ulSecLocation;
		m_LogicalVolDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfLogicalVolDesc) +
			sizeof(tUdfLogicalPartMapType1) - sizeof(tUdfTag);

		m_LogicalVolDesc.ulVolDescSeqNum = ulVolDescSeqNum;
		m_LogicalVolDesc.ulMapTableLength = 6;
		m_LogicalVolDesc.ulNumPartitionMaps = 1;

		m_LogicalVolDesc.IntegritySeqExtent = IntegritySeqExtent;
		//m_LogicalVolDesc.IntegritySeqExtent.ulExtentLen = 4096
		//m_LogicalVolDesc.IntegritySeqExtent.ulExtentLoc = 64

		// Write parition map.
		tUdfLogicalPartMapType1 PartMap;
		memset(&PartMap,0,sizeof(tUdfLogicalPartMapType1));

		PartMap.ucPartMapType = UDF_PARTITION_MAP_TYPE1;
		PartMap.ucPartMapLen = 6;
		PartMap.usVolSeqNum = 1;
		PartMap.usPartNum = 0;

		// Calculate tag checksums.
		unsigned char ucCompleteBuffer[sizeof(tUdfLogicalVolDesc) + sizeof(tUdfLogicalPartMapType1)];
		memcpy(ucCompleteBuffer,&m_LogicalVolDesc,sizeof(tUdfLogicalVolDesc));
		memcpy(ucCompleteBuffer + sizeof(tUdfLogicalVolDesc),&PartMap,sizeof(tUdfLogicalPartMapType1));
		MakeTagChecksums(m_LogicalVolDesc.DescTag,ucCompleteBuffer + sizeof(tUdfTag));

		// Write logical volume descriptor.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&m_LogicalVolDesc,sizeof(tUdfLogicalVolDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfLogicalVolDesc))
			return false;

		// Write partition map.
		if (pOutStream->Write(&PartMap,sizeof(tUdfLogicalPartMapType1),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfLogicalPartMapType1))
			return false;

		// Pad the sector.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfLogicalVolDesc) -
			sizeof(tUdfLogicalPartMapType1)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	bool CUdf::WriteVolDescUnalloc(COutStream *pOutStream,unsigned long ulVolDescSeqNum,
		unsigned long ulSecLocation)
	{
		tUdfUnallocSpaceDesc UnallocSpaceDesc;
		memset(&UnallocSpaceDesc,0,sizeof(tUdfUnallocSpaceDesc));

		// Make the tag.
		MakeTag(UnallocSpaceDesc.DescTag,UDF_TAGIDENT_UNALLOCATEDSPACEDESC);
		UnallocSpaceDesc.DescTag.ulTagLocation = ulSecLocation;
		UnallocSpaceDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfUnallocSpaceDesc) - sizeof(tUdfTag);

		UnallocSpaceDesc.ulVolDescSeqNum = ulVolDescSeqNum;
		UnallocSpaceDesc.ulNumAllocDesc = 0;

		// Calculate checksums.
		MakeTagChecksums(UnallocSpaceDesc.DescTag,(unsigned char *)(&UnallocSpaceDesc) + sizeof(tUdfTag));

		// Write to the output stream.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&UnallocSpaceDesc,sizeof(tUdfUnallocSpaceDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfUnallocSpaceDesc))
			return false;

		// Pad the sector from 512 to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfUnallocSpaceDesc)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	bool CUdf::WriteVolDescTerm(COutStream *pOutStream,unsigned long ulSecLocation)
	{
		tUdfTermVolDesc TermDesc;
		memset(&TermDesc,0,sizeof(tUdfTermVolDesc));

		// Make the tag.
		MakeTag(TermDesc.DescTag,UDF_TAGIDENT_TERMDESC);
		TermDesc.DescTag.ulTagLocation = ulSecLocation;
		TermDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfTermVolDesc) - sizeof(tUdfTag);
		MakeTagChecksums(TermDesc.DescTag,(unsigned char *)(&TermDesc) + sizeof(tUdfTag));

		// Write to the output stream.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&TermDesc,sizeof(tUdfTermVolDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfTermVolDesc))
			return false;

		// Pad the sector from 512 to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfTermVolDesc)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	/**
		@param ulFileCount the number of files in the file system not including
		extended attribute records.
		@param ulDirCount the number of directories in the file system not
		including the root directory.
		@param uiUniqueIdent must be larger than the unique udentifiers of any
		file entry.
	*/
	bool CUdf::WriteVolDescLogIntegrity(COutStream *pOutStream,unsigned long ulSecLocation,
		unsigned long ulFileCount,unsigned long ulDirCount,unsigned long ulPartLen,
		unsigned __int64 uiUniqueIdent,SYSTEMTIME &stImageCreate)
	{
		tUdfLogicalVolIntegrityDesc LogIntegrityDesc;
		memset(&LogIntegrityDesc,0,sizeof(tUdfLogicalVolIntegrityDesc));

		// Make the tag.
		MakeTag(LogIntegrityDesc.DescTag,UDF_TAGIDENT_LOGICALVOLINTEGRITYDESC);
		LogIntegrityDesc.DescTag.ulTagLocation = ulSecLocation;
		LogIntegrityDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfLogicalVolIntegrityDesc) - sizeof(tUdfTag);

		MakeDateTime(stImageCreate,LogIntegrityDesc.RecordDateTime);

		LogIntegrityDesc.ulIntegrityType = UDF_LOGICAL_INTEGRITY_CLOSE;
		LogIntegrityDesc.ulNumPartitions = 1;
		LogIntegrityDesc.ulLenImplUse = sizeof(tUdfLogicalVolIntegrityDescImplUse);
		LogIntegrityDesc.ulFreeSpaceTable = 0;		// No free space available on the partition.
		LogIntegrityDesc.ulSizeTable = ulPartLen;

		// Must be larger than the unique udentifiers of any file entry.
		LogIntegrityDesc.LogicalVolContentsUse.uiUniqueIdent = uiUniqueIdent;

		MakeIdent(LogIntegrityDesc.ucImplUse.ImplIdent,IT_DEVELOPER);
		LogIntegrityDesc.ucImplUse.ulNumFiles = ulFileCount;
		LogIntegrityDesc.ucImplUse.ulNumDirectories = ulDirCount;
		LogIntegrityDesc.ucImplUse.usMinUdfRevRead = 0x0102;	// Currently only UDF 1.02 is supported.
		LogIntegrityDesc.ucImplUse.usMinUdfRevWrite = 0x0102;	// Currently only UDF 1.02 is supported.
		LogIntegrityDesc.ucImplUse.usMaxUdfRevWrite = 0x0102;	// Currently only UDF 1.02 is supported.

		// Calculate tag checksums.
		MakeTagChecksums(LogIntegrityDesc.DescTag,(unsigned char *)(&LogIntegrityDesc) + sizeof(tUdfTag));

		// Write to the output stream.
		unsigned long ulProcessedSize;
		if (pOutStream->Write(&LogIntegrityDesc,sizeof(tUdfLogicalVolIntegrityDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfLogicalVolIntegrityDesc))
			return false;

		// Pad the sector to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfLogicalVolIntegrityDesc)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	bool CUdf::WriteAnchorVolDescPtr(COutStream *pOutStream,unsigned long ulSecLocation,
		tUdfExtentAd &MainVolDescSeqExtent,tUdfExtentAd &ReserveVolDescSeqExtent)
	{
		tUdfAnchorVolDescPtr AnchorVolDescPtr;
		memset(&AnchorVolDescPtr,0,sizeof(tUdfAnchorVolDescPtr));

		// Setup tag.
		MakeTag(AnchorVolDescPtr.DescTag,UDF_TAGIDENT_ANCHORVOLDESCPTR);
		AnchorVolDescPtr.DescTag.ulTagLocation = ulSecLocation;
		AnchorVolDescPtr.DescTag.usDescriptorCrcLen = sizeof(tUdfAnchorVolDescPtr) - sizeof(tUdfTag);

		// Other members.
		memcpy(&AnchorVolDescPtr.MainVolDescSeqExtent,&MainVolDescSeqExtent,sizeof(tUdfExtentAd));
		memcpy(&AnchorVolDescPtr.ReserveVolDescSeqExtent,&ReserveVolDescSeqExtent,sizeof(tUdfExtentAd));

		// Calculate tag checksums.
		MakeTagChecksums(AnchorVolDescPtr.DescTag,(unsigned char *)(&AnchorVolDescPtr) + sizeof(tUdfTag));

		unsigned long ulProcessedSize = 0;
		if (pOutStream->Write(&AnchorVolDescPtr,sizeof(tUdfAnchorVolDescPtr),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfAnchorVolDescPtr))
			return false;

		// Pad the sector to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfAnchorVolDescPtr)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	/**
		Writes a file set decsriptor structure to the output stream.
		@param ulSecLocation sector position relative to the first logical block of the partition.
	*/
	bool CUdf::WriteFileSetDesc(COutStream *pOutStream,unsigned long ulSecLocation,
		unsigned long ulRootSecLocation,SYSTEMTIME &stImageCreate)
	{
		tUdfFileSetDesc FileSetDesc;
		memset(&FileSetDesc,0,sizeof(tUdfFileSetDesc));

		// Setup tag.
		MakeTag(FileSetDesc.DescTag,UDF_TAGIDENT_FILESETDESC);
		FileSetDesc.DescTag.ulTagLocation = ulSecLocation;
		FileSetDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfFileSetDesc) - sizeof(tUdfTag);

		MakeDateTime(stImageCreate,FileSetDesc.RecordDateTime);

		FileSetDesc.usInterchangeLevel = UDF_INTERCHANGE_LEVEL_FILESET;
		FileSetDesc.usMaxInterchangeLevel = UDF_INTERCHANGE_LEVEL_FILESET;
		FileSetDesc.ulCharSetList = 1;
		FileSetDesc.ulMaxCharSetList = 1;
		FileSetDesc.ulFileSetNum = 0;
		FileSetDesc.ulFileSetDescNum = 0;

		MakeCharSpec(FileSetDesc.LogicalVolIdentCharSet);
		MakeCharSpec(FileSetDesc.FileSetCharSet);
		memcpy(FileSetDesc.ucLogicalVolIdent,m_LogicalVolDesc.ucLogicalVolIdent,
			sizeof(FileSetDesc.ucLogicalVolIdent));	// Steal the value from the logical descriptor.
		memcpy(FileSetDesc.ucFileSetIdent,m_PrimVolDesc.ucVolIdentifier,
			sizeof(FileSetDesc.ucFileSetIdent));	// Steal the value from the primary descriptor.

		FileSetDesc.RootDirectoryIcb.ulExtentLen = UDF_SECTOR_SIZE;
		FileSetDesc.RootDirectoryIcb.ExtentLoc.ulLogicalBlockNum = ulRootSecLocation;
		FileSetDesc.RootDirectoryIcb.ExtentLoc.usPartitionRefNum = 0;	// Wee only support one partition.

		MakeIdent(FileSetDesc.DomainIdent,IT_DOMAIN);

		// Calculate tag checksums.
		MakeTagChecksums(FileSetDesc.DescTag,(unsigned char *)(&FileSetDesc) + sizeof(tUdfTag));

		unsigned long ulProcessedSize = 0;
		if (pOutStream->Write(&FileSetDesc,sizeof(tUdfFileSetDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfFileSetDesc))
			return false;

		// Pad the sector to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (UDF_SECTOR_SIZE - sizeof(tUdfAnchorVolDescPtr)); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	/*
		Note: This function does not pad to closest sector.
	*/
	bool CUdf::WriteFileIdentParent(COutStream *pOutStream,unsigned long ulSecLocation,
		unsigned long ulFileEntrySecLoc)
	{
		tUdfFileIdentDesc FileIdentDesc;
		memset(&FileIdentDesc,0,sizeof(tUdfFileIdentDesc));

		// Setup tag.
		MakeTag(FileIdentDesc.DescTag,UDF_TAGIDENT_FILEIDENTDESC);
		FileIdentDesc.DescTag.ulTagLocation = ulSecLocation;
		FileIdentDesc.DescTag.usDescriptorCrcLen = sizeof(tUdfFileIdentDesc) + 2 - sizeof(tUdfTag);	// Always pad two bytes.

		// Setup other members.
		FileIdentDesc.usFileVerNum = 1;
		FileIdentDesc.ucFileCharacteristics = UDF_FILECHARFLAG_DIRECTORY | UDF_FILECHARFLAG_PARENT;
		FileIdentDesc.ucFileIdentLen = 0;

		FileIdentDesc.Icb.ulExtentLen = UDF_SECTOR_SIZE;
		FileIdentDesc.Icb.ExtentLoc.ulLogicalBlockNum = ulFileEntrySecLoc;
		FileIdentDesc.Icb.ExtentLoc.usPartitionRefNum = 0;	// Always first partition.

		// Calculate tag checksums.
		unsigned char ucCompleteBuffer[sizeof(tUdfFileIdentDesc) + 2];
		memcpy(ucCompleteBuffer,&FileIdentDesc,sizeof(tUdfFileIdentDesc));

		// Padded bytes.
		ucCompleteBuffer[sizeof(tUdfFileIdentDesc)    ] = 0;
		ucCompleteBuffer[sizeof(tUdfFileIdentDesc) + 1] = 0;

		MakeTagChecksums(FileIdentDesc.DescTag,ucCompleteBuffer + sizeof(tUdfTag));

		// Re-copy the tag since the CRC and checksum has been updated.
		memcpy(ucCompleteBuffer,&FileIdentDesc.DescTag,sizeof(tUdfTag));

		// Write to the output stream.
		unsigned long ulProcessedSize = 0;
		if (pOutStream->Write(ucCompleteBuffer,sizeof(ucCompleteBuffer),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(ucCompleteBuffer))
			return false;

		return true;
	}

	/*
		Note: This function does not pad to closest sector.
	*/
	bool CUdf::WriteFileIdent(COutStream *pOutStream,unsigned long ulSecLocation,
		unsigned long ulFileEntrySecLoc,bool bIsDirectory,const TCHAR *szFileName)
	{
		tUdfFileIdentDesc FileIdentDesc;
		memset(&FileIdentDesc,0,sizeof(tUdfFileIdentDesc));

		// Setup tag.
		MakeTag(FileIdentDesc.DescTag,UDF_TAGIDENT_FILEIDENTDESC);
		FileIdentDesc.DescTag.ulTagLocation = ulSecLocation;

		// Setup other members.
		FileIdentDesc.usFileVerNum = 1;
		FileIdentDesc.ucFileCharacteristics = bIsDirectory ? UDF_FILECHARFLAG_DIRECTORY : 0;
		
		// Create file identifier.
		unsigned char ucFileIdent[255];
		FileIdentDesc.ucFileIdentLen = MakeFileIdent(ucFileIdent,szFileName);

		FileIdentDesc.Icb.ulExtentLen = UDF_SECTOR_SIZE;	// The file entry will always fit within one sector.
		FileIdentDesc.Icb.ExtentLoc.ulLogicalBlockNum = ulFileEntrySecLoc;
		FileIdentDesc.Icb.ExtentLoc.usPartitionRefNum = 0;	// Always first partition.

		// Pad the file identifier.
		unsigned short usPadSize = 4 * (unsigned short)((FileIdentDesc.ucFileIdentLen +
			FileIdentDesc.usImplUseLen + 38 + 3)/4) -
			(FileIdentDesc.ucFileIdentLen + FileIdentDesc.usImplUseLen + 38);

		// Update tag with checksums.
		unsigned short usDescLen = sizeof(tUdfFileIdentDesc) + FileIdentDesc.ucFileIdentLen + usPadSize;
		FileIdentDesc.DescTag.usDescriptorCrcLen = usDescLen - sizeof(tUdfTag);

		AllocateByteBuffer(usDescLen);
		memset(m_pByteBuffer,0,usDescLen);
		memcpy(m_pByteBuffer,&FileIdentDesc,sizeof(tUdfFileIdentDesc));
		memcpy(m_pByteBuffer + sizeof(tUdfFileIdentDesc),ucFileIdent,FileIdentDesc.ucFileIdentLen);

		MakeTagChecksums(FileIdentDesc.DescTag,m_pByteBuffer + sizeof(tUdfTag));

		// Re-copy the tag since the CRC and checksum has been updated.
		memcpy(m_pByteBuffer,&FileIdentDesc.DescTag,sizeof(tUdfTag));

		// Write to the output stream.
		unsigned long ulProcessedSize = 0;
		if (pOutStream->Write(m_pByteBuffer,usDescLen,&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != usDescLen)
			return false;

		return true;
	}

	/**
		@param usFileLinkCount the number of identifiers in the extent.
		@param uiUniqueIdent a unique identifier for this file.
		@param uiInfoLength the length of all file identifiers in bytes for
		directories and the size of the file on files.
	*/
	bool CUdf::WriteFileEntry(COutStream *pOutStream,unsigned long ulSecLocation,
		bool bIsDirectory,unsigned short usFileLinkCount,unsigned __int64 uiUniqueIdent,
		unsigned long ulInfoLocation,unsigned __int64 uiInfoLength,
		SYSTEMTIME &stAccessTime,SYSTEMTIME &stModTime,SYSTEMTIME &stAttrTime)
	{
		tUdfFileEntry FileEntry;
		memset(&FileEntry,0,sizeof(tUdfFileEntry));

		// According to ECMA 14.14.2.2 the location must be 0 if the length is 0.
		if (uiInfoLength == 0)
			ulInfoLocation = 0;

		// Setup tag.
		MakeTag(FileEntry.DescTag,UDF_TAGIDENT_FILEENTRYDESC);
		FileEntry.DescTag.ulTagLocation = ulSecLocation;
		FileEntry.DescTag.usDescriptorCrcLen = sizeof(tUdfFileEntry) - sizeof(tUdfTag);

		// Set up ICB.
		FileEntry.IcbTag.ulPriorRecNumDirectEntries = 0;	// Seems to be optional.
		FileEntry.IcbTag.usStrategyType = UDF_ICB_STRATEGY_4;
		FileEntry.IcbTag.usNumEntries = 1;					// Seems to be fixed to 1.
		FileEntry.IcbTag.ucFileType = bIsDirectory ? UDF_ICB_FILETYPE_DIRECTORY :
			UDF_ICB_FILETYPE_RANDOM_BYTES;

		FileEntry.IcbTag.ParentIcbLocation.ulLogicalBlockNum = 0;	// Is optional.
		FileEntry.IcbTag.ParentIcbLocation.usPartitionRefNum = 0;	// Is optional.

		FileEntry.IcbTag.usFlags = UDF_ICB_FILEFLAG_ARCHIVE/* | UDF_ICB_FILEFLAG_LONG_ALLOC_DESC*/;
		FileEntry.IcbTag.usFlags |= m_bDvdVideo ? UDF_ICB_FILEFLAG_SHORT_ALLOC_DESC :
			UDF_ICB_FILEFLAG_LONG_ALLOC_DESC;

		if (m_bDvdVideo)
		{
			FileEntry.IcbTag.usFlags |= UDF_ICB_FILEFLAG_NOT_RELOCATABLE | UDF_ICB_FILEFLAG_CONTIGUOUS;

			// DVD-Video does not allow files larger than 1 GiB.
			if (uiInfoLength > 0x40000000)
				return false;
		}

		FileEntry.ulUid = 0xFFFFFFFF;
		FileEntry.ulGid = 0xFFFFFFFF;
		FileEntry.ulPermissions =
			UDF_ICB_FILEPERM_OTHER_EXECUTE | UDF_ICB_FILEPERM_OTHER_READ |
			UDF_ICB_FILEPERM_GROUP_EXECUTE | UDF_ICB_FILEPERM_GROUP_READ |
			UDF_ICB_FILEPERM_OWNER_EXECUTE | UDF_ICB_FILEPERM_OWNER_READ;

		FileEntry.usFileLinkCount = usFileLinkCount;

		FileEntry.uiInfoLen = uiInfoLength;			// flow.txt = 40, root = 264
		FileEntry.uiLogicalBlocksRecorded = BytesToSector64(uiInfoLength);

		// File time stamps.
		MakeDateTime(stAccessTime,FileEntry.AccessTime);
		MakeDateTime(stModTime,FileEntry.ModificationTime);
		MakeDateTime(stAttrTime,FileEntry.AttributeTime);

		FileEntry.ulCheckpoint = 1;
		MakeIdent(FileEntry.ImplIdent,IT_DEVELOPER);
		FileEntry.uiUniqueIdent = uiUniqueIdent;

		// Allocation descriptor.
		tUdfLongAllocDesc AllocDesc;
		FileEntry.ulAllocDescLen = 0;

		// Calculate the total number of bytes needed to store all descriptors.
		unsigned long ulTotAllocDescSize = ((unsigned long)(uiInfoLength / 0x3FFFF800) + 1) *
			sizeof(tUdfLongAllocDesc);

		// Add the extended attributes length information, and replace the
		// allocation descriptor length since DVD-Video only supports short
		// allocation descriptors.
		if (m_bDvdVideo)
		{
			FileEntry.ulExtendedAttrLen = sizeof(tUdfExtendedAttrHeaderDesc) +
				sizeof(tUdfExendedAttrFreeEaSpace) + sizeof(tUdfExtendedAttrCgms);

			ulTotAllocDescSize = sizeof(tUdfShortAllocDesc);
		}

		// FIXME: Move everything to m_pByteBuffer.
		unsigned char *pCompleteBuffer = new unsigned char[sizeof(tUdfFileEntry) +
			FileEntry.ulExtendedAttrLen + ulTotAllocDescSize];

		// Extended attributes that seems to be necessary for DVD-Video support.
		if (m_bDvdVideo)
		{
			// Exended attributes header.
			tUdfExtendedAttrHeaderDesc ExtHeader;
			memset(&ExtHeader,0,sizeof(tUdfExtendedAttrHeaderDesc));

			// Setup the tag.
			MakeTag(ExtHeader.DescTag,UDF_TAGIDENT_EXTENDEDATTRDESC);
			ExtHeader.DescTag.ulTagLocation = ulSecLocation;
			ExtHeader.DescTag.usDescriptorCrcLen = sizeof(tUdfExtendedAttrHeaderDesc) - sizeof(tUdfTag);

			ExtHeader.ulImplAttrLoc = sizeof(tUdfExtendedAttrHeaderDesc);
			ExtHeader.ulAppAttrLoc = sizeof(tUdfExtendedAttrHeaderDesc) +
				sizeof(tUdfExendedAttrFreeEaSpace) + sizeof(tUdfExtendedAttrCgms);

			// Compute tag checksums.
			MakeTagChecksums(ExtHeader.DescTag,(unsigned char *)&ExtHeader + sizeof(tUdfTag));

			// Free EA space descriptor.
			tUdfExendedAttrFreeEaSpace EaSpaceDesc;
			memset(&EaSpaceDesc,0,sizeof(tUdfExendedAttrFreeEaSpace));

			EaSpaceDesc.ulAttrType = UDF_SECTOR_SIZE;
			EaSpaceDesc.ucAttrSubtype = 1;
			EaSpaceDesc.ulAttrLength = sizeof(tUdfExendedAttrFreeEaSpace);
			EaSpaceDesc.ulImplUseLen = 4;
			MakeIdent(EaSpaceDesc.ImplIdent,IT_FREEEASPACE);
			EaSpaceDesc.usHeaderChecksum = MakeExtAddrChecksum((unsigned char *)&EaSpaceDesc);
			EaSpaceDesc.usFreeSpace = 0;

			// CGMS descriptor.
			tUdfExtendedAttrCgms CgmsDesc;
			memset(&CgmsDesc,0,sizeof(tUdfExtendedAttrCgms));

			CgmsDesc.ulAttrType = UDF_SECTOR_SIZE;
			CgmsDesc.ucAttrSubtype = 1;
			CgmsDesc.ulAttrLength = sizeof(tUdfExtendedAttrCgms);
			CgmsDesc.ulImplUseLen = 8;
			MakeIdent(CgmsDesc.ImplIdent,IT_CGMS);
			CgmsDesc.usHeaderChecksum = MakeExtAddrChecksum((unsigned char *)&CgmsDesc);
			CgmsDesc.ucCgmsInfo = 0;
			CgmsDesc.ucDataStructType = 0;
			CgmsDesc.ulProtSysInfo = 0;

			unsigned long ulBufferPos = sizeof(tUdfFileEntry);
			memcpy(pCompleteBuffer + ulBufferPos,&ExtHeader,sizeof(tUdfExtendedAttrHeaderDesc));
			ulBufferPos += sizeof(tUdfExtendedAttrHeaderDesc);

			memcpy(pCompleteBuffer + ulBufferPos,&EaSpaceDesc,sizeof(tUdfExendedAttrFreeEaSpace));
			ulBufferPos += sizeof(tUdfExendedAttrFreeEaSpace);

			memcpy(pCompleteBuffer + ulBufferPos,&CgmsDesc,sizeof(tUdfExtendedAttrCgms));
			ulBufferPos += sizeof(tUdfExtendedAttrCgms);

			// Allocation descriptor.
			tUdfShortAllocDesc AllocDesc;
			memset(&AllocDesc,0,sizeof(tUdfShortAllocDesc));

			AllocDesc.ulExtentLen = (unsigned long)uiInfoLength;
			AllocDesc.ulExtentPos = ulInfoLocation;

			FileEntry.ulAllocDescLen = sizeof(tUdfShortAllocDesc);
			memcpy(pCompleteBuffer + ulBufferPos,&AllocDesc,sizeof(tUdfShortAllocDesc));
		}
		else
		{
			unsigned long ulBufferPos = sizeof(tUdfFileEntry) + FileEntry.ulExtendedAttrLen;
			while (uiInfoLength > 0x3FFFF800)
			{
				memset(&AllocDesc,0,sizeof(tUdfLongAllocDesc));
				AllocDesc.ulExtentLen = 0x3FFFF800;		// FIXME: Maybe I should not state that the memory has been recorded (ECMA 14.14.1.1).
				AllocDesc.ExtentLoc.ulLogicalBlockNum = ulInfoLocation;
				AllocDesc.ExtentLoc.usPartitionRefNum = 0;

				FileEntry.ulAllocDescLen += sizeof(tUdfLongAllocDesc);

				// Copy the entry to the complete buffer.
				memcpy(pCompleteBuffer + ulBufferPos,&AllocDesc,sizeof(tUdfLongAllocDesc));
				ulBufferPos += sizeof(tUdfLongAllocDesc);

				uiInfoLength -= 0x3FFFF800;
				ulInfoLocation += 0x3FFFF800 / UDF_SECTOR_SIZE;
			}

			// Add a descriptor containing the remaining bytes.
			memset(&AllocDesc,0,sizeof(tUdfLongAllocDesc));
			AllocDesc.ulExtentLen = (unsigned long)uiInfoLength;	// FIXME: Same as above.
			AllocDesc.ExtentLoc.ulLogicalBlockNum = ulInfoLocation;
			AllocDesc.ExtentLoc.usPartitionRefNum = 0;

			FileEntry.ulAllocDescLen += sizeof(tUdfLongAllocDesc);

			// Copy the entry to the complete buffer.
			memcpy(pCompleteBuffer + ulBufferPos,&AllocDesc,sizeof(tUdfLongAllocDesc));
		}

		// Calculate checksums.
		unsigned short usDescLen = sizeof(tUdfFileEntry) +
			(unsigned short)FileEntry.ulExtendedAttrLen +
			(unsigned short)FileEntry.ulAllocDescLen;
		FileEntry.DescTag.usDescriptorCrcLen = usDescLen - sizeof(tUdfTag);

		// The file entry part is done, copy it.
		memcpy(pCompleteBuffer,&FileEntry,sizeof(tUdfFileEntry));

		MakeTagChecksums(FileEntry.DescTag,pCompleteBuffer + sizeof(tUdfTag));

		// Re-copy the tag since the CRC and checksum has been updated.
		memcpy(pCompleteBuffer,&FileEntry.DescTag,sizeof(tUdfTag));

		// Write to the output stream.
		unsigned long ulProcessedSize = 0;
		int iWriteResult = pOutStream->Write(pCompleteBuffer,usDescLen,&ulProcessedSize);
		delete [] pCompleteBuffer;

		if (iWriteResult != STREAM_OK)
			return false;
		if (ulProcessedSize != usDescLen)
			return false;

		// Pad the sector to 2048 bytes.
		char szTemp[1] = { 0 };
		for (unsigned long i = 0; i < (unsigned long)(UDF_SECTOR_SIZE - usDescLen); i++)
			pOutStream->Write(szTemp,1,&ulProcessedSize);

		return true;
	}

	unsigned long CUdf::CalcFileIdentParentSize()
	{
		return sizeof(tUdfFileIdentDesc) + 2;
	}

	unsigned long CUdf::CalcFileIdentSize(const TCHAR *szFileName)
	{
		unsigned long ulFileNameLen = lstrlen(szFileName);
		if (ulFileNameLen > 254)
			ulFileNameLen = 254;

		unsigned long ulFileIdentLen = m_bDvdVideo ? ulFileNameLen + 1 : (ulFileNameLen << 1) + 1;
		unsigned long ulFileImplUseLen = 0;

		unsigned long ulPadSize = 4 * (unsigned short)((ulFileIdentLen + ulFileImplUseLen + 38 + 3)/4) -
			(ulFileIdentLen + ulFileImplUseLen + 38);

		return sizeof(tUdfFileIdentDesc) + ulFileIdentLen + ulFileImplUseLen + ulPadSize;
	}

	unsigned long CUdf::CalcFileEntrySize()
	{
		return UDF_SECTOR_SIZE;
	}

	/**
		Returns the number of bytes needed for the initial volume recognition
		sequence.
	*/
	unsigned long CUdf::GetVolDescInitialSize()
	{
		return sizeof(tUdfVolStructDesc) * 3;
	}
};
