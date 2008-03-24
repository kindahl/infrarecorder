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
#include <map>

namespace ckFileSystem
{
	class CSectorManager;

	class ISectorClient
	{
	};

	class CSectorManager
	{
	private:
		unsigned __int64 m_uiNextFreeSector;
		unsigned __int64 m_uiDataStart;
		unsigned __int64 m_uiDataLength;
		std::map<std::pair<ISectorClient *,unsigned char>,unsigned __int64> m_ClientMap;

	public:
		CSectorManager(unsigned __int64 uiStartSector);
		~CSectorManager();

		void AllocateSectors(ISectorClient *pClient,unsigned char ucIdentifier,
			unsigned __int64 uiNumSectors);
		void AllocateBytes(ISectorClient *pClient,unsigned char ucIdentifier,
			unsigned __int64 uiNumBytes);

		void AllocateDataSectors(unsigned __int64 uiNumSectors);
		void AllocateDataBytes(unsigned __int64 uiNumBytes);

		unsigned __int64 GetStart(ISectorClient *pClient,unsigned char ucIdentifier);
		unsigned __int64 GetNextFree();

		unsigned __int64 GetDataStart();
		unsigned __int64 GetDataLength();
	};
};
