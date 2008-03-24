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
#include <vector>
#include "../../Common/FileStream.h"

#define IFO_INDETIFIER_VMG				"DVDVIDEO-VMG"
#define IFO_INDETIFIER_VTS				"DVDVIDEO-VTS"
#define IFO_IDENTIFIER_LEN				12

namespace ckFileSystem
{
	class CIfoVmgData
	{
	public:
		unsigned long ulLastVmgSector;
		unsigned long ulLastVmgIfoSector;
		unsigned short usNumVmgTitleSets;
		unsigned long ulVmgMenuVobSector;
		unsigned long ulSrptSector;

		std::vector<unsigned long> Titles;
	};

	class CIfoVtsData
	{
	public:
		unsigned long ulLastVtsSector;
		unsigned long ulLastVtsIfoSector;
		unsigned long ulVtsMenuVobSector;
		unsigned long ulVtsVobSector;
	};

	class CIfoReader
	{
	public:
		enum eIfoType
		{
			IT_UNKNOWN,
			IT_VMG,
			IT_VTS
		};

	private:
		eIfoType m_IfoType;

		CInFileStream InStream;

	public:

		CIfoReader();
		~CIfoReader();

		bool Open(const TCHAR *szFullPath);
		bool Close();

		bool ReadVmg(CIfoVmgData &VmgData);
		bool ReadVts(CIfoVtsData &VtsData);

		eIfoType GetType();
	};
};
