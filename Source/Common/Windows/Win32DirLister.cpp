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
#include "Win32DirLister.h"
#include "../StringUtil.h"

CDirLister::CDirLister()
{
}

CDirLister::~CDirLister()
{
	// Clear the file list vector.
	m_FileList.clear();
}

bool CDirLister::ListDirectory(const TCHAR *szPath,TCHAR *szFilter)
{
	WIN32_FIND_DATA FileData;
	HANDLE hFile;

	// We want to search for all files and folders.
	TCHAR szFullPath[MAX_PATH];
	lstrcpy(szFullPath,szPath);

	// Make sure that the path has a trailing backslash.
	IncludeTrailingBackslash(szFullPath);

	lstrcat(szFullPath,szFilter);

	// Get the handle of the first file.
	hFile = FindFirstFile(szFullPath,&FileData);

	// Clear the file list vector.
	m_FileList.clear();

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// We want to ingore the "." and ".." records.
			if (!lstrcmp(FileData.cFileName,TEXT(".")) || !lstrcmp(FileData.cFileName,TEXT("..")))
				continue;

			tFileInfo FileInfo;
			FileInfo.FileData = FileData;
			lstrcpy(FileInfo.szFilePath,szPath);

			m_FileList.push_back(FileInfo);
		}
		while (FindNextFile(hFile,&FileData));

		FindClose(hFile);
	}

	return (m_FileList.size() > 0);
}

__int64 CDirLister::GetFolderSize(const TCHAR *szPath)
{
	__int64 iResult = 0;
	WIN32_FIND_DATA FileData;
	HANDLE hFile;

	// We want to search for all files and folders.
	TCHAR szFullPath[MAX_PATH];
	lstrcpy(szFullPath,szPath);

	// Make sure that the path has a trailing backslash.
	IncludeTrailingBackslash(szFullPath);

	lstrcat(szFullPath,TEXT("*.*"));

	// Get the handle of the first file.
	hFile = FindFirstFile(szFullPath,&FileData);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			// We want to ingore the "." and ".." records.
			if (!lstrcmp(FileData.cFileName,TEXT(".")) || !lstrcmp(FileData.cFileName,TEXT("..")))
				continue;

			// Check if we're dealing with a file or a folder.
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR szNextPath[MAX_PATH];
				lstrcpy(szNextPath,szPath);
				lstrcat(szNextPath,FileData.cFileName);

				// Since it was a folder we recursively search that folder too.
				iResult += GetFolderSize(szNextPath);
			}
			else
			{
				ULARGE_INTEGER ul;
				ul.LowPart = FileData.nFileSizeLow;
                ul.HighPart = FileData.nFileSizeHigh;

				// The item was a file so we add its size to the counter.
				iResult += ul.QuadPart;
			}
		}
		while (FindNextFile(hFile,&FileData));

		FindClose(hFile);
	}

	return iResult;
}

__int64 CDirLister::GetItemSize(unsigned int uiIndex)
{
	__int64 iResult = 0;

	// Check whether the item is a folder or a file. 
	if (m_FileList[uiIndex].FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		TCHAR szFullName[MAX_PATH];
		lstrcpy(szFullName,m_FileList[uiIndex].szFilePath);
		lstrcat(szFullName,m_FileList[uiIndex].FileData.cFileName);

		iResult = GetFolderSize(szFullName);
	}
	else
	{
		ULARGE_INTEGER ul;
		ul.LowPart = m_FileList[uiIndex].FileData.nFileSizeLow;
		ul.HighPart = m_FileList[uiIndex].FileData.nFileSizeHigh;

		iResult = ul.QuadPart;
	}

	return iResult;
}
