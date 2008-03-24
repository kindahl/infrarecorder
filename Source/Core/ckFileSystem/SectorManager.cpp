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
#include "SectorManager.h"
#include "Iso9660.h"		// FIXME:

namespace ckFileSystem
{
	CSectorManager::CSectorManager(unsigned __int64 uiStartSector)
	{
		m_uiNextFreeSector = uiStartSector;
		m_uiDataStart = 0;
		m_uiDataLength = 0;
	}

	CSectorManager::~CSectorManager()
	{
	}

	/**
		Allocates a number of sectors for the client. The identifier is used
		for identifying the allocated sector range. This is necessary when the
		client requests which sector range it was given.
		@param pClient the client that requests the memory.
		@param ucIdentifier the unique identifier choosen by the client for the
		requested sector range.
		@param uiNumSectors the number of sectors to allocate.
	*/
	void CSectorManager::AllocateSectors(ISectorClient *pClient,unsigned char ucIdentifier,
		unsigned __int64 uiNumSectors)
	{
		m_ClientMap[std::make_pair(pClient,ucIdentifier)] = m_uiNextFreeSector;

		m_uiNextFreeSector += uiNumSectors;
	}

	void CSectorManager::AllocateBytes(ISectorClient *pClient,unsigned char ucIdentifier,
		unsigned __int64 uiNumBytes)
	{
		m_ClientMap[std::make_pair(pClient,ucIdentifier)] = m_uiNextFreeSector;

		m_uiNextFreeSector += BytesToSector64(uiNumBytes);
	}

	/**
		Allocation of data sectors is separated from the other allocation. The reason
		is that there can only be one data allocation and it should be accessible by
		any client.
	*/
	void CSectorManager::AllocateDataSectors(unsigned __int64 uiNumSectors)
	{
		m_uiDataStart = m_uiNextFreeSector;
		m_uiDataLength = uiNumSectors;

		m_uiNextFreeSector += m_uiDataLength;
	}

	void CSectorManager::AllocateDataBytes(unsigned __int64 uiNumBytes)
	{
		m_uiDataStart = m_uiNextFreeSector;
		m_uiDataLength = BytesToSector64(uiNumBytes);

		m_uiNextFreeSector += m_uiDataLength;
	}

	/**
		Returns the starting sector of the allocated sector range allocated by
		the client that matches the identifier.
		@param pClient te client that owns the sector range.
		@param ucIdentifier the unique identifier selected by the client when
		allocating.
		@return the start sector.
	*/
	unsigned __int64 CSectorManager::GetStart(ISectorClient *pClient,unsigned char ucIdentifier)
	{
		return m_ClientMap[std::make_pair(pClient,ucIdentifier)];
	}

	/**
		Returns the next free unallocated sector. This should be used with care.
	*/
	unsigned __int64 CSectorManager::GetNextFree()
	{
		return m_uiNextFreeSector;
	}

	unsigned __int64 CSectorManager::GetDataStart()
	{
		return m_uiDataStart;
	}

	unsigned __int64 CSectorManager::GetDataLength()
	{
		return m_uiDataLength;
	}
};
