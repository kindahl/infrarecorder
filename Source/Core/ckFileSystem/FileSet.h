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
			FLAG_DIRECTORY = 0x01,
			FLAG_IMPORTED = 0x02
		};

		CFileDescriptor(const TCHAR *szInternalPath,const TCHAR *szExternalPath,
			unsigned __int64 uiFileSize,unsigned char ucFlags = 0,void *pData = NULL)
		{
			m_ucFlags = ucFlags;
			m_uiFileSize = uiFileSize;
			m_InternalPath = szInternalPath;
			m_ExternalPath = szExternalPath;
			m_pData = pData;
		}

		unsigned char m_ucFlags;
		unsigned __int64 m_uiFileSize;
		tstring m_InternalPath;			// Path in disc image.
		tstring m_ExternalPath;			// Path on hard drive.

		void *m_pData;					// Pointer to a user-defined structure, designed for CIso9660TreeNode
	};

	/*
		Sorts the set of files according to the ECMA-119 standard.
	*/
	class CFileComparator
	{
	private:
		bool m_bDvdVideo;

		/*
			Returns a weight of the specified file name, a ligher file should
			be placed heigher in the directory hierarchy.
		*/
		unsigned long GetFileWeight(const TCHAR *szFullPath) const
		{
			unsigned long ulWeight = 0xFFFFFFFF;

			// Quick test for optimization.
			if (szFullPath[1] == 'V')
			{
				if (!lstrcmp(szFullPath,_T("/VIDEO_TS")))	// The VIDEO_TS folder should be first.
					ulWeight = 0;
				else if (!lstrncmp(szFullPath,_T("/VIDEO_TS/"),10))
				{
					const TCHAR *szFileName = szFullPath + 10;

					if (!lstrncmp(szFileName,_T("VIDEO_TS"),8))
					{
						ulWeight -= 0x80000000;

						const TCHAR *szFileExt = szFileName + 9;
						if (!lstrcmp(szFileExt,_T("IFO")))
							ulWeight -= 3;
						else if (!lstrcmp(szFileExt,_T("VOB")))
							ulWeight -= 2;
						else if (!lstrcmp(szFileExt,_T("BUP")))
							ulWeight -= 1;
					}
					else if (!lstrncmp(szFileName,_T("VTS_"),4))
					{
						ulWeight -= 0x40000000;

						// Just a safety measure.
						if (lstrlen(szFileName) < 64)
						{
							TCHAR szFileExt[64];
							unsigned long ulNum = 0,ulSubNum = 0;

							if (lsscanf(szFileName,_T("VTS_%u_%u.%[^\0]"),&ulNum,&ulSubNum,szFileExt) == 3)
							{
								// The first number is worth the most, the lower the lighter.
								ulWeight -= 0xFFFFFF - (ulNum << 8);

								if (!lstrcmp(szFileExt,_T("IFO")))
								{
									ulWeight -= 0xFF;
								}
								else if (!lstrcmp(szFileExt,_T("VOB")))
								{
									ulWeight -= 0x0F - ulSubNum;
								}
								else if (!lstrcmp(szFileExt,_T("BUP")))
								{
									ulWeight -= 1;
								}
							}
						}
					}
				}
			}

			return ulWeight;
		}

	public:
		/*
			@param bDvdVideo set to true to use DVD-Video compatible sorting.
		*/
		CFileComparator(bool bDvdVideo) : m_bDvdVideo(bDvdVideo)
		{
		}

		static int Level(const CFileDescriptor &Item)
		{
			const TCHAR *szFullPath = Item.m_InternalPath.c_str();

			int iLevel = 0;
			for (size_t i = 0; i < (size_t)lstrlen(szFullPath); i++)
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
			if (m_bDvdVideo)
			{
				unsigned long ulWeight1 = GetFileWeight(Item1.m_InternalPath.c_str());
				unsigned long ulWeight2 = GetFileWeight(Item2.m_InternalPath.c_str());

				if (ulWeight1 != ulWeight2)
				{
					if (ulWeight1 < ulWeight2)
						return true;
					else
						return false;
				}
			}

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
