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
#include "../../Common/IntConv.h"
#include "DvdVideo.h"
#include "IfoReader.h"

namespace ckFileSystem
{
	CIfoReader::CIfoReader()
	{
		m_IfoType = IT_UNKNOWN;
	}

	CIfoReader::~CIfoReader()
	{
		Close();
	}

	/**
		Open the IFO file and determine it's type.
		@param szFullPath the full file path to the file to open.
		@return true if the file was successfully opened and identified, false
		otherwise.
	*/
	bool CIfoReader::Open(const TCHAR *szFullPath)
	{
		if (!InStream.Open(szFullPath))
			return false;

		char szIdentifier[IFO_IDENTIFIER_LEN + 1];

		unsigned long ulProcessedSize;
		if (InStream.Read(szIdentifier,IFO_IDENTIFIER_LEN,&ulProcessedSize) != STREAM_OK)
		{
			Close();
			return false;
		}

		szIdentifier[IFO_IDENTIFIER_LEN] = '\0';
		if (!strcmp(szIdentifier,IFO_INDETIFIER_VMG))
			m_IfoType = IT_VMG;
		else if (!strcmp(szIdentifier,IFO_INDETIFIER_VTS))
			m_IfoType = IT_VTS;
		else
		{
			Close();
			return false;
		}

		return true;
	}

	bool CIfoReader::Close()
	{
		m_IfoType = IT_UNKNOWN;
		return InStream.Close();
	}

	bool CIfoReader::ReadVmg(CIfoVmgData &VmgData)
	{
		// Read last sector of VMG.
		unsigned long ulSector,ulProcessedSize;

		InStream.Seek(12,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VmgData.ulLastVmgSector = BeToLe32(ulSector);

		// Read last sector of IFO.
		InStream.Seek(28,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VmgData.ulLastVmgIfoSector = BeToLe32(ulSector);

		// Read number of VTS  title sets.
		unsigned short usNumTitles;

		InStream.Seek(62,FILE_BEGIN);
		if (InStream.Read(&usNumTitles,sizeof(usNumTitles),&ulProcessedSize) != STREAM_OK)
			return false;

		VmgData.usNumVmgTitleSets = BeToLe16(usNumTitles);

		// Read start sector of VMG Menu VOB.
		InStream.Seek(192,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VmgData.ulVmgMenuVobSector = BeToLe32(ulSector);

		// Read sector offset to TT_SRPT.
		InStream.Seek(196,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VmgData.ulSrptSector = BeToLe32(ulSector);

		// Read the TT_SRPT titles.
		InStream.Seek(DVDVIDEO_BLOCK_SIZE * VmgData.ulSrptSector,FILE_BEGIN);
		if (InStream.Read(&usNumTitles,sizeof(usNumTitles),&ulProcessedSize) != STREAM_OK)
			return false;

		usNumTitles = BeToLe16(usNumTitles);
		for (unsigned short i = 0; i < usNumTitles; i++)
		{
			InStream.Seek((DVDVIDEO_BLOCK_SIZE * VmgData.ulSrptSector) + 8 + (i * 12) + 8,FILE_BEGIN);
			if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
				return false;

			VmgData.Titles.push_back(BeToLe32(ulSector));
		}

		return true;
	}

	bool CIfoReader::ReadVts(CIfoVtsData &VtsData)
	{
		// Read last sector of VTS.
		unsigned long ulSector,ulProcessedSize;

		InStream.Seek(12,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VtsData.ulLastVtsSector = BeToLe32(ulSector);

		// Read last sector of IFO.
		InStream.Seek(28,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VtsData.ulLastVtsIfoSector = BeToLe32(ulSector);

		// Read start sector of VTS Menu VOB.
		InStream.Seek(192,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VtsData.ulVtsMenuVobSector = BeToLe32(ulSector);

		// Read start sector of VTS Title VOB.
		InStream.Seek(196,FILE_BEGIN);
		if (InStream.Read(&ulSector,sizeof(ulSector),&ulProcessedSize) != STREAM_OK)
			return false;

		VtsData.ulVtsVobSector = BeToLe32(ulSector);
		return true;
	}

	CIfoReader::eIfoType CIfoReader::GetType()
	{
		return m_IfoType;
	}
};
