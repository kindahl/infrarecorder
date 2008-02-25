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
#include <set>

namespace ckFileSystem
{
	/*
		Describes a file that should be included in the disc image.
	*/
	class CFileDescriptor
	{
	public:
		enum
		{
			FLAG_DIRECTORY = 0x01
		};

		CFileDescriptor(const TCHAR *szInternalPath,const TCHAR *szExternalPath,
			unsigned __int64 uiFileSize,unsigned char ucFlags = 0)
		{
			m_ucFlags = ucFlags;
			m_ulExtentSector = 0;
			m_uiFileSize = uiFileSize;
			m_InternalPath = szInternalPath;
			m_ExternalPath = szExternalPath;
		}

		unsigned char m_ucFlags;
		unsigned long m_ulExtentSector;	// The sector which contains the extent
										// information. This is unknown until the
										// extent information is actually written.
		unsigned __int64 m_uiFileSize;
		tstring m_InternalPath;			// Path in disc image.
		tstring m_ExternalPath;			// Path on hard drive.
	};

	/*
		Sorts the set of files according to the ECMA-119 standard.
	*/
	class CFileComparator
	{
	public:
		static int Level(const CFileDescriptor &Item)
		{
			const TCHAR *szFullPath = Item.m_InternalPath.c_str();

			int iLevel = 0;
			for (size_t i = 0; i < lstrlen(szFullPath); i++)
			{
				if (szFullPath[i] == '/' || szFullPath[i] == '\\')
					iLevel++;
			}

			if (Item.m_ucFlags & CFileDescriptor::FLAG_DIRECTORY)
				iLevel++;

			return iLevel;
		}

		bool operator() (const CFileDescriptor &Item1,const CFileDescriptor &Item2) const
		{
			int iLevelItem1 = Level(Item1);
			int iLevelItem2 = Level(Item2);

			if (iLevelItem1 < iLevelItem2)
				return true;
			else if (iLevelItem1 == iLevelItem2)
				return lstrcmp(Item1.m_InternalPath.c_str(),Item2.m_InternalPath.c_str()) < 0;
			else
				return false;
		}
	};

	typedef std::set<ckFileSystem::CFileDescriptor,ckFileSystem::CFileComparator> CFileSet;
};
