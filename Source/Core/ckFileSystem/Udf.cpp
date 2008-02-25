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

namespace ckFileSystem
{
	/*
		Identifiers.
	*/
	unsigned char g_pIndentUdfCharSet[] = {
		0x4F,0x53,0x54,0x41,0x20,0x43,0x6F,0x6D,0x70,0x72,0x65,0x73,
		0x73,0x65,0x64,0x20,0x55,0x6E,0x69,0x63,0x6F,0x64,0x65 };
	unsigned char g_pIndentUdfEntityCompliant[] = {
		0x2A,0x4F,0x53,0x54,0x41,0x20,0x55,0x44,0x46,0x20,0x43,0x6F,
		0x6D,0x70,0x6C,0x69,0x61,0x6E,0x74 };
	unsigned char g_pIndentUdfEntityLVInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4C,0x56,0x20,0x49,0x6E,0x66,0x6F };
	unsigned char g_pIndentUdfEntityFreeEASpace[] = {
		0x2A,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x45,0x41,0x53,
		0x70,0x61,0x63,0x65 };
	unsigned char g_pIndentUdfEntityFreeAppEASpace[] = {
		0x2A,0x55,0x44,0x46,0x20,0x46,0x72,0x65,0x65,0x41,0x70,0x70,
		0x45,0x41,0x53,0x70,0x61,0x63,0x65 };
	unsigned char g_pIndentUdfEntityDVDCGMSInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x44,0x56,0x44,0x20,0x43,0x47,0x4D,
		0x53,0x20,0x49,0x6E,0x66,0x6F };
	unsigned char g_pIndentUdfEntityOS2EA[] = {
		0x2A,0x55,0x44,0x46,0x41,0x20,0x45,0x41};
	unsigned char g_pIndentUdfEntityOS2EALen[] = {
		0x2A,0x55,0x44,0x46,0x20,0x45,0x41,0x4C,0x65,0x6E,0x67,0x74,
		0x68 };
	unsigned char g_pIndentUdfEntityMacVolInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x56,0x6F,0x6C,
		0x75,0x6D,0x65,0x49,0x6E,0x66,0x6F };
	unsigned char g_pIndentUdfEntityMacFinderInfo[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x49,0x69,0x6E,
		0x64,0x65,0x72,0x49,0x6E,0x66,0x6F };
	unsigned char g_pIndentUdfEntityMacUniqueTable[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x55,0x6E,0x69,
		0x71,0x75,0x65,0x49,0x44,0x54,0x61,0x62,0x6C,0x65 };
	unsigned char g_pIndentUdfEntityMacResFork[] = {
		0x2A,0x55,0x44,0x46,0x20,0x4D,0x61,0x63,0x20,0x52,0x65,0x73,
		0x6F,0x75,0x72,0x63,0x65,0x46,0x6F,0x72,0x6B };

	const char *g_IdentBEA = "BEA01";
	const char *g_IdentNSR = "NSR02";
	const char *g_IdentTEA = "TEA01";

	CUdf::CUdf()
	{
	}

	CUdf::~CUdf()
	{
	}

	bool CUdf::WriteVolDesc(COutStream *pOutStream)
	{
		tUdfVolStructDesc VolStructDesc;
		memset(&VolStructDesc,0,sizeof(tUdfVolStructDesc));
		VolStructDesc.ucType = 0;
		VolStructDesc.ucStructVer = 1;

		unsigned long ulProcessedSize;
		memcpy(VolStructDesc.ucIdent,g_IdentBEA,sizeof(VolStructDesc.ucIdent));
		if (pOutStream->Write(&VolStructDesc,sizeof(tUdfVolStructDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfVolStructDesc))
			return false;

		memcpy(VolStructDesc.ucIdent,g_IdentNSR,sizeof(VolStructDesc.ucIdent));
		if (pOutStream->Write(&VolStructDesc,sizeof(tUdfVolStructDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfVolStructDesc))
			return false;

		memcpy(VolStructDesc.ucIdent,g_IdentTEA,sizeof(VolStructDesc.ucIdent));
		if (pOutStream->Write(&VolStructDesc,sizeof(tUdfVolStructDesc),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfVolStructDesc))
			return false;

		return true;
	}

	// ECMA-167: 10.2
	//   Main Volume Descriptor Sequence Extent
	//   Reserve Volume Descriptor Sequence Extent
	bool CUdf::WriteAnchorVolDescPtr(COutStream *pOutStream,unsigned long ulSecLocation,
		tUdfExtentAd MainVolDescSeqExtent,tUdfExtentAd ReserveVolDescSeqExtent)
	{
		tUdfAnchorVolDescPtr AnchorVolDescPtr;
		memset(&AnchorVolDescPtr,0,sizeof(tUdfAnchorVolDescPtr));

		AnchorVolDescPtr.DescTag.usTagIdentifier = UDF_TAGINDENT_ANCHORVOLDESCPTR;
		AnchorVolDescPtr.DescTag.usDescriptorVersion = 2;	// Goes hand in hand with "NSR02".
		AnchorVolDescPtr.DescTag.usTagSerialNumber = 0;
		AnchorVolDescPtr.DescTag.usDescriptorCRC = 0;		// Skip CRC calculation.
		AnchorVolDescPtr.DescTag.usDescriptorCRCLength = 0;	// Skip CRC calculation.
		AnchorVolDescPtr.DescTag.ulTagLocation = ulSecLocation;

		// nero_udf.iso.
		// 7.1.5 32-bit unsigned numerical values (bytes/sector).
		// For example, the decimal number 305 419 896 has #12345678 as its hexadecimal representation and shall be
		// recorded as #78 #56 #34 #12.
		// AnchorVolDescPtr.MainVolDescSeqExtent.ulExtentLen = (#00800000) 0x00008000 = 32768
		// AnchorVolDescPtr.MainVolDescSeqExtent.ulExtentLoc = (#20000000) 0x00000020 = 32
		// ReserveVolDescSeqExtent.MainVolDescSeqExtent.ulExtentLen = (#00800000) 0x00008000 = 32768
		// ReserveVolDescSeqExtent.MainVolDescSeqExtent.ulExtentLoc = (#30000000) 0x00000030 = 48

		/*
		8.3 Volume descriptors
		Characteristics of the volume shall be specified by volume descriptors recorded in Volume Descriptor Sequences as
		described in 3/8.4.2.
		A volume descriptor shall be one of the following types:
		-  Primary Volume Descriptor (see 3/10.1)
		-  Implementation Use Volume Descriptor (see 3/10.4)
		- 3/7 -
		-  Partition Descriptor (see 3/10.5)
		-  Logical Volume Descriptor (see 3/10.6)
		-  Unallocated Space Descriptor (see 3/10.8)
		*/

		// Sum of bytes 0-3 and 5-15 modulo 256.
		unsigned char ucChecksum = 0;
		for (int i = 0; i < 16; i++)
			ucChecksum += ((unsigned char *)&AnchorVolDescPtr)[i];

		AnchorVolDescPtr.DescTag.ucTagChecksum = ucChecksum;

		memcpy(&AnchorVolDescPtr.MainVolDescSeqExtent,&MainVolDescSeqExtent,sizeof(tUdfExtentAd));
		memcpy(&AnchorVolDescPtr.ReserveVolDescSeqExtent,&ReserveVolDescSeqExtent,sizeof(tUdfExtentAd));

		unsigned long ulProcessedSize = 0;
		if (pOutStream->Write(&AnchorVolDescPtr,sizeof(tUdfAnchorVolDescPtr),&ulProcessedSize) != STREAM_OK)
			return false;
		if (ulProcessedSize != sizeof(tUdfAnchorVolDescPtr))
			return false;

		return true;
	}

	unsigned long CUdf::GetVolDescSize()
	{
		return sizeof(tUdfVolStructDesc) * 3;
	}
};
