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
#include <algorithm>
#include "../../Common/FileManager.h"
#include "DvdVideo.h"
#include "IfoReader.h"

namespace ckFileSystem
{
	CDvdVideo::CDvdVideo(CLog *pLog) : m_pLog(pLog)
	{
	}

	CDvdVideo::~CDvdVideo()
	{
	}

	unsigned __int64 CDvdVideo::SizeToDvdLen(unsigned __int64 uiFileSize)
	{
		return uiFileSize / DVDVIDEO_BLOCK_SIZE;
	}

	CFileTreeNode *CDvdVideo::FindVideoNode(CFileTree &FileTree,eFileSetType Type,unsigned long ulNumber)
	{
		tstring InternalPath = _T("/VIDEO_TS/");

		switch (Type)
		{
			case FST_INFO:
				if (ulNumber == 0)
				{
					InternalPath.append(_T("VIDEO_TS.IFO"));
				}
				else
				{
					TCHAR szFileName[13];
					lsprintf(szFileName,_T("VTS_%02d_0.IFO"),ulNumber);
					InternalPath.append(szFileName);
				}
				break;

			case FST_BACKUP:
				if (ulNumber == 0)
				{
					InternalPath.append(_T("VIDEO_TS.BUP"));
				}
				else
				{
					TCHAR szFileName[13];
					lsprintf(szFileName,_T("VTS_%02d_0.BUP"),ulNumber);
					InternalPath.append(szFileName);
				}
				break;

			case FST_MENU:
				if (ulNumber == 0)
				{
					InternalPath.append(_T("VIDEO_TS.VOB"));
				}
				else
				{
					TCHAR szFileName[13];
					lsprintf(szFileName,_T("VTS_%02d_0.VOB"),ulNumber);
					InternalPath.append(szFileName);
				}
				break;

			case FST_TITLE:
				{
					if (ulNumber == 0)
						return NULL;

					CFileTreeNode *pLastNode = NULL;

					// We find the last title node. There may be many of them.
					TCHAR szFileName[13];
					for (unsigned int i = 0; i < 9; i++)
					{
						szFileName[0] = '\0';
						lsprintf(szFileName,_T("VTS_%02d_%d.VOB"),ulNumber,i + 1);
						InternalPath.append(szFileName);

						CFileTreeNode *pNode = FileTree.GetNodeFromPath(InternalPath.c_str());
						if (pNode == NULL)
							break;

						pLastNode = pNode;

						// Restore the full path variable.
						InternalPath = _T("/VIDEO_TS/");
					}

					// Since we're dealing with multiple files we return immediately.
					return pLastNode;
				}

			default:
				return NULL;
		}

		return FileTree.GetNodeFromPath(InternalPath.c_str());
	}

	bool CDvdVideo::GetTotalTitlesSize(tstring &FilePath,eFileSetType Type,
		unsigned long ulNumber,unsigned __int64 &uiFileSize)
	{
		tstring FullPath = FilePath;

		if (ulNumber == 0)
			return false;

		uiFileSize = 0;

		TCHAR szFileName[13];
		for (unsigned int i = 0; i < 9; i++)
		{
			szFileName[0] = '\0';
			lsprintf(szFileName,_T("VTS_%02d_%d.VOB"),ulNumber,i + 1);
			FullPath.append(szFileName);

			if (!fs_fileexists(FullPath.c_str()))
				break;

			uiFileSize += fs_filesize(FullPath.c_str());

			// Restore the full path variable.
			FullPath = FilePath;
		}

		return true;
	}

	bool CDvdVideo::ReadFileSetInfoRoot(CFileTree &FileTree,CIfoVmgData &VmgData,
		std::vector<unsigned long> &TitleSetSectors)
	{
		unsigned __int64 uiMenuSize = 0,uiInfoSize = 0;

		CFileTreeNode *pInfoNode = FindVideoNode(FileTree,FST_INFO,0);
		if (pInfoNode != NULL)
			uiInfoSize = pInfoNode->m_uiFileSize;

		CFileTreeNode *pMenuNode = FindVideoNode(FileTree,FST_MENU,0);
		if (pMenuNode != NULL)
			uiMenuSize = pMenuNode->m_uiFileSize;

		CFileTreeNode *pBackupNode = FindVideoNode(FileTree,FST_BACKUP,0);

		// Verify the information.
		if ((VmgData.ulLastVmgSector + 1) < (SizeToDvdLen(uiInfoSize) << 1))
		{
			m_pLog->AddLine(_T("  Error: Invalid VIDEO_TS.IFO file size."));
			return false;
		}

		// Find the actuall size of .IFO.
		unsigned __int64 uiInfoLength = 0;
		if (pMenuNode == NULL)
		{
			if ((VmgData.ulLastVmgSector + 1) > (SizeToDvdLen(uiInfoSize) << 1))
				uiInfoLength = VmgData.ulLastVmgSector - SizeToDvdLen(uiInfoSize) + 1;
			else
				uiInfoLength = VmgData.ulLastVmgIfoSector + 1;
		}
		else
		{
			if ((VmgData.ulLastVmgIfoSector + 1) < VmgData.ulVmgMenuVobSector)
				uiInfoLength = VmgData.ulVmgMenuVobSector;
			else
				uiInfoLength = VmgData.ulLastVmgIfoSector + 1;
		}

		if (uiInfoLength > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: VIDEO_TS.IFO is larger than 4 million blocks (%I64d blocks)."),uiInfoLength);
			return false;
		}

		if (pInfoNode != NULL)
			pInfoNode->m_ulDataPadLen = (unsigned long)uiInfoLength - (unsigned long)SizeToDvdLen(uiInfoSize);

		// Find the actuall size of .VOB.
		unsigned __int64 uiMenuLength = 0;
		if (pMenuNode != NULL)
		{
			uiMenuLength = VmgData.ulLastVmgSector - uiInfoLength - SizeToDvdLen(uiInfoSize) + 1;

			if (uiMenuLength > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Error: VIDEO_TS.VOB is larger than 4 million blocks (%I64d blocks)."),uiMenuLength);
				return false;
			}

			pMenuNode->m_ulDataPadLen = (unsigned long)uiMenuLength - (unsigned long)SizeToDvdLen(uiMenuSize);
		}

		// Find the actuall size of .BUP.
		unsigned __int64 uiBupLength = 0;
		if (TitleSetSectors.size() > 0)
			uiBupLength = *TitleSetSectors.begin() - uiMenuLength - uiInfoLength;
		else			
			uiBupLength = VmgData.ulLastVmgSector + 1 - uiMenuLength - uiInfoLength;	// If no title sets are used.

		if (uiBupLength > 0xFFFFFFFF)
		{
			m_pLog->AddLine(_T("  Error: VIDEO_TS.BUP is larger than 4 million blocks (%I64d blocks)."),uiBupLength);
			return false;
		}

		if (pBackupNode != NULL)
			pBackupNode->m_ulDataPadLen = (unsigned long)uiBupLength - (unsigned long)SizeToDvdLen(uiInfoSize);

		return true;
	}

	bool CDvdVideo::ReadFileSetInfo(CFileTree &FileTree,std::vector<unsigned long> &TitleSetSectors)
	{
		unsigned long ulCounter = 1;

		std::vector<unsigned long>::const_iterator itTitleSet;
		for (itTitleSet = TitleSetSectors.begin(); itTitleSet != TitleSetSectors.end(); itTitleSet++)
		{
			CFileTreeNode *pInfoNode = FindVideoNode(FileTree,FST_INFO,ulCounter);
			if (pInfoNode == NULL)
			{
				m_pLog->AddLine(_T("  Error: Unable to find IFO file in file tree."));
				return false;
			}

			CIfoReader IfoReader;
			if (!IfoReader.Open(pInfoNode->m_FileFullPath.c_str()))
			{
				m_pLog->AddLine(_T("  Error: Unable to open and identify %s."),pInfoNode->m_FileName.c_str());
				return false;
			}

			if (IfoReader.GetType() != CIfoReader::IT_VTS)
			{
				m_pLog->AddLine(_T("  Error: %s is not of VTS format."),pInfoNode->m_FileName.c_str());
				return false;
			}

			CIfoVtsData VtsData;
			if (!IfoReader.ReadVts(VtsData))
			{
				m_pLog->AddLine(_T("  Error: Unable to read VTS data from %s."),pInfoNode->m_FileName.c_str());
				return false;
			}

			// Test if VTS_XX_0.VOB is present.
			unsigned __int64 uiMenuSize = 0;
			CFileTreeNode *pMenuNode = FindVideoNode(FileTree,FST_MENU,ulCounter);
			if (pMenuNode != NULL)
				uiMenuSize = pMenuNode->m_uiFileSize;

			// Test if VTS_XX_X.VOB are present.
			unsigned __int64 uiTitleSize = 0;

			tstring FilePath = pInfoNode->m_FileFullPath;
			FilePath.resize(FilePath.find_last_of('/') + 1);

			bool bTitle = GetTotalTitlesSize(FilePath,FST_TITLE,ulCounter,uiTitleSize);

			// Test if VTS_XX_0.IFO are present.
			unsigned __int64 uiInfoSize = pInfoNode->m_uiFileSize;

			// Check that the title will fit in the space given by the IFO file.
			if ((VtsData.ulLastVtsSector + 1) < (SizeToDvdLen(uiInfoSize) << 1))
			{
				m_pLog->AddLine(_T("  Error: Invalid size of %s."),pInfoNode->m_FileName.c_str());
				return false;
			}
			else if (bTitle && pMenuNode != NULL && (VtsData.ulLastVtsSector + 1 < (SizeToDvdLen(uiInfoSize) << 1) +
				SizeToDvdLen(uiTitleSize) +  SizeToDvdLen(uiMenuSize)))
			{
				m_pLog->AddLine(_T("  Error: Either IFO or menu VOB related to %s is of incorrect size. (1)"),
					pInfoNode->m_FileName.c_str());
				return false;
			}
			else if (bTitle && pMenuNode == NULL && (VtsData.ulLastVtsSector + 1 < (SizeToDvdLen(uiInfoSize) << 1) +
				SizeToDvdLen(uiTitleSize)))
			{
				m_pLog->AddLine(_T("  Error: Either IFO or menu VOB related to %s is of incorrect size. (2)"),
					pInfoNode->m_FileName.c_str());
				return false;
			}
			else if (!bTitle && pMenuNode != NULL && (VtsData.ulLastVtsSector + 1 < (SizeToDvdLen(uiInfoSize) << 1) +
				    SizeToDvdLen(uiMenuSize)))
			{
				m_pLog->AddLine(_T("  Error: Either IFO or menu VOB related to %s is of incorrect size. (3)"),
					pInfoNode->m_FileName.c_str());
				return false;
			}

			// Find the actuall size of VTS_XX_0.IFO.
			unsigned __int64 uiInfoLength = 0;
			if (!bTitle && pMenuNode == NULL)
			{
				uiInfoLength = VtsData.ulLastVtsSector - SizeToDvdLen(uiInfoSize) + 1;
			}
			else if (!bTitle)
			{
				uiInfoLength = VtsData.ulVtsVobSector;
			}
			else
			{
				if (VtsData.ulLastVtsIfoSector + 1 < VtsData.ulVtsMenuVobSector)
					uiInfoLength = VtsData.ulVtsMenuVobSector;
				else
					uiInfoLength = VtsData.ulLastVtsIfoSector + 1;
			}

			if (uiInfoLength > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Error: IFO file larger than 4 million blocks."));
				return false;
			}

			pInfoNode->m_ulDataPadLen = (unsigned long)uiInfoLength - (unsigned long)SizeToDvdLen(uiInfoSize);

			// Find the actuall size of VTS_XX_0.VOB.
			unsigned __int64 uiMenuLength = 0;
			if (pMenuNode != NULL)
			{
				if (bTitle && (VtsData.ulVtsVobSector - VtsData.ulVtsMenuVobSector > SizeToDvdLen(uiMenuSize)))
				{
					uiMenuLength = VtsData.ulVtsVobSector - VtsData.ulVtsMenuVobSector;
				}
				else if (!bTitle &&	(VtsData.ulVtsVobSector + SizeToDvdLen(uiMenuSize) +
					SizeToDvdLen(uiInfoSize) - 1 < VtsData.ulLastVtsSector))
				{
					uiMenuLength = VtsData.ulLastVtsSector - SizeToDvdLen(uiInfoSize) - VtsData.ulVtsMenuVobSector + 1;
				}
				else
				{
					uiMenuLength = VtsData.ulVtsVobSector - VtsData.ulVtsMenuVobSector;
				}

				if (uiMenuLength > 0xFFFFFFFF)
				{
					m_pLog->AddLine(_T("  Error: Menu VOB file larger than 4 million blocks."));
					return false;
				}

				pMenuNode->m_ulDataPadLen = (unsigned long)uiMenuLength - (unsigned long)SizeToDvdLen(uiMenuSize);
			}

			// Find the actuall size of VTS_XX_[1 to 9].VOB.
			unsigned __int64 uiTitleLength = 0;
			if (bTitle)
			{
				uiTitleLength = VtsData.ulLastVtsSector + 1 - uiInfoLength -
					uiMenuLength - SizeToDvdLen(uiInfoSize);

				if (uiTitleLength > 0xFFFFFFFF)
				{
					m_pLog->AddLine(_T("  Error: Title files larger than 4 million blocks."));
					return false;
				}

				// We only pad the last title node (not sure if that is correct).
				CFileTreeNode *pLastTitleNode = FindVideoNode(FileTree,FST_TITLE,ulCounter);
				if (pLastTitleNode != NULL)
					pLastTitleNode->m_ulDataPadLen = (unsigned long)uiTitleLength - (unsigned long)SizeToDvdLen(uiTitleSize);
			}

			// Find the actuall size of VTS_XX_0.BUP.
			unsigned __int64 uiBupLength;
			if (TitleSetSectors.size() > ulCounter) {
				uiBupLength = TitleSetSectors[ulCounter] - TitleSetSectors[ulCounter - 1] -
					uiTitleLength - uiMenuLength - uiInfoLength;
			}
			else
			{
				uiBupLength = VtsData.ulLastVtsSector + 1 - uiTitleLength - uiMenuLength - uiInfoLength;
			}

			if (uiBupLength > 0xFFFFFFFF)
			{
				m_pLog->AddLine(_T("  Error: BUP file larger than 4 million blocks."));
				return false;
			}

			CFileTreeNode *pBackupNode = FindVideoNode(FileTree,FST_BACKUP,ulCounter);
			if (pBackupNode != NULL)
				pBackupNode->m_ulDataPadLen = (unsigned long)uiBupLength - (unsigned long)SizeToDvdLen(uiInfoSize);

			// We're done.
			IfoReader.Close();

			// Increase the counter.
			ulCounter++;
		}

		return true;
	}

	bool CDvdVideo::PrintFilePadding(CFileTree &FileTree)
	{
		m_pLog->AddLine(_T("CDvdVideo::PrintFilePadding"));

		CFileTreeNode *pVideoTsNode = FileTree.GetNodeFromPath(_T("/VIDEO_TS"));
		if (pVideoTsNode == NULL)
		{
			m_pLog->AddLine(_T("  Error: Unable to locate VIDEO_TS folder in file tree."));
			return false;
		}

		std::vector<CFileTreeNode *>::const_iterator itVideoFile;;
		for (itVideoFile = pVideoTsNode->m_Children.begin(); itVideoFile !=
			pVideoTsNode->m_Children.end(); itVideoFile++)
		{
			m_pLog->AddLine(_T("  %s: pad %u sector(s)."),
				(*itVideoFile)->m_FileName.c_str(),(*itVideoFile)->m_ulDataPadLen);
		}

		return true;
	}

	bool CDvdVideo::CalcFilePadding(CFileTree &FileTree)
	{
		// First locate VIDEO_TS.IFO.
		CFileTreeNode *pVideoTsNode = FileTree.GetNodeFromPath(_T("/VIDEO_TS/VIDEO_TS.IFO"));
		if (pVideoTsNode == NULL)
		{
			m_pLog->AddLine(_T("  Error: Unable to locate VIDEO_TS.IFO in file tree."));
			return false;
		}

		// Read and validate VIDEO_TS.INFO.
		CIfoReader IfoReader;
		if (!IfoReader.Open(pVideoTsNode->m_FileFullPath.c_str()))
		{
			m_pLog->AddLine(_T("  Error: Unable to open and identify VIDEO_TS.IFO."));
			return false;
		}

		if (IfoReader.GetType() != CIfoReader::IT_VMG)
		{
			m_pLog->AddLine(_T("  Error: VIDEO_TS.IFO is not of VMG format."));
			return false;
		}

		CIfoVmgData VmgData;
		if (!IfoReader.ReadVmg(VmgData))
		{
			m_pLog->AddLine(_T("  Error: Unable to read VIDEO_TS.IFO VMG data."));
			return false;
		}

		// Make a vector of all title set vectors (instead of titles).
		std::vector<unsigned long> TitleSetSectors(VmgData.Titles.size());
		std::vector<unsigned long>::const_iterator itLast =
			std::unique_copy(VmgData.Titles.begin(),VmgData.Titles.end(),TitleSetSectors.begin());
		TitleSetSectors.resize(itLast - TitleSetSectors.begin());

		// Sort the titles according to the start of the vectors.
		std::sort(TitleSetSectors.begin(),TitleSetSectors.end());

		if (!ReadFileSetInfoRoot(FileTree,VmgData,TitleSetSectors))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain necessary information from VIDEO_TS.* files."));
			return false;
		}

		if (!ReadFileSetInfo(FileTree,TitleSetSectors))
		{
			m_pLog->AddLine(_T("  Error: Unable to obtain necessary information from DVD-Video files."));
			return false;
		}

		return true;
	}
};
