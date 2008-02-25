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
#include "Win32FileManager.h"
#include "../StringUtil.h"

HANDLE fs_open(const TCHAR *szFileName,TCHAR *pMode)
{    
    switch (pMode[0])
    {
        case 'r':
            return CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);

        case 'w':
            return CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,NULL);

		case 'a':
            return CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
    }

    return INVALID_HANDLE_VALUE;
}

bool fs_close(HANDLE hFile)
{
    return CloseHandle(hFile) == TRUE;
}

__int64 fs_seek(HANDLE hFile,__int64 iDistance,int iMode)
{
    LARGE_INTEGER li;

    li.QuadPart = iDistance;
    li.LowPart = SetFilePointer(hFile,li.LowPart,&li.HighPart,iMode);

    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
		return -1;

	return (__int64)li.QuadPart;
}

__int64 fs_tell(HANDLE hFile)
{
    LARGE_INTEGER li;
	
	li.QuadPart = 0;
	li.LowPart = SetFilePointer(hFile,0,&li.HighPart,FILE_CURRENT);
    
    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
		return -1;

    return (__int64)li.QuadPart;
}

__int64 fs_filesize(HANDLE hFile)
{
	LARGE_INTEGER li;
	
	li.QuadPart = 0;
	li.LowPart = GetFileSize(hFile,(LPDWORD)&li.HighPart);

	if (li.LowPart == 0xFFFFFFFF)
    {
		if (GetLastError() != ERROR_SUCCESS)
			return -1;
    }
    
    return (__int64)li.QuadPart;
}

unsigned long fs_getfileattributes(const TCHAR *szFileName)
{
	return GetFileAttributes(szFileName);
}

bool fs_setfileattributes(const TCHAR *szFileName,unsigned long ulAttr)
{
	return SetFileAttributes(szFileName,ulAttr) == TRUE;
}

unsigned long fs_read(void *pBuffer,unsigned long ulCount,HANDLE hFile)
{
    unsigned long ulRead;

	if (ReadFile(hFile,pBuffer,ulCount,&ulRead,NULL) == FALSE)
		return 0;
	else
		return ulRead;
}

unsigned long fs_write(void *pBuffer,unsigned long ulCount,HANDLE hFile)
{
    unsigned long ulWritten;

	if (WriteFile(hFile,pBuffer,ulCount,&ulWritten,NULL) == FALSE)
		return 0;
	else
		return ulWritten;
}

/*long fs_bseek(HANDLE hFile,long lDistance,int iMode)
{
    long lResult = SetFilePointer(hFile,lDistance,NULL,iMode);

    if (lResult == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
		return -1;

	return lDistance;
}*/

bool fs_fileexists(const TCHAR *szFileName)
{
	DWORD dwAttrib = GetFileAttributes(szFileName);

	return (dwAttrib != -1) && ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

bool fs_deletefile(const TCHAR *szFileName)
{
	return DeleteFile(szFileName) != 0;
}

/*
	fs_deletedir
	------------
	Removes a folder from the harddrive. If bForce is set to true all sub-
	folders and files will be deleted as well.
*/
bool fs_deletedir(const TCHAR *szFolderName/*,bool bForce*/)
{
	/*if (bForce)
	{
		SHFILEOPSTRUCT shFileOp;
		memset(&shFileOp,0,sizeof(SHFILEOPSTRUCT));

		shFileOp.hwnd = HWND_DESKTOP;	// FIXME: Must be a real valid handle.
		shFileOp.wFunc = FO_DELETE;
		shFileOp.pFrom = szFolderName;
		shFileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
		
		return SHFileOperation(&shFileOp) == 0;
	}*/

	return RemoveDirectory(szFolderName) != 0;
}

bool fs_renamefile(const TCHAR *szOldName,const TCHAR *szNewName)
{
	return MoveFile(szOldName,szNewName) != 0;
}

bool fs_getmodtime(HANDLE hFile,unsigned short &usFileDate,unsigned short &usFileTime)
{
	FILETIME FileTime,LocalFileTime;

	if (GetFileTime(hFile,NULL,NULL,&FileTime) != TRUE)
		return false;

	if (FileTimeToLocalFileTime(&FileTime,&LocalFileTime) == TRUE)
		return FileTimeToDosDateTime(&LocalFileTime,&usFileDate,&usFileTime) == TRUE;
	
	return false;
}

bool fs_getmodtime(const TCHAR *szFileName,unsigned short &usFileDate,unsigned short &usFileTime)
{
	HANDLE hFile = CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	FILETIME FileTime,LocalFileTime;

	bool bResult = GetFileTime(hFile,NULL,NULL,&FileTime) == TRUE;
	CloseHandle(hFile);

	if (!bResult)
		return false;

	if (FileTimeToLocalFileTime(&FileTime,&LocalFileTime) == TRUE)
		return FileTimeToDosDateTime(&LocalFileTime,&usFileDate,&usFileTime) == TRUE;
	
	return false;
}

/*
	fs_getdirmodtime
	----------------
	Retrieves the modification time of the specified folder. Please note that
	GetFileAttributesEx is not Windows 95 compatible.
*/
bool fs_getdirmodtime(const TCHAR *szFileName,unsigned short &usFileDate,unsigned short &usFileTime)
{
	WIN32_FILE_ATTRIBUTE_DATA FileInfo;
	if (GetFileAttributesEx(szFileName,GetFileExInfoStandard,&FileInfo) == TRUE)
	{
		FILETIME LocalFileTime;

		if (FileTimeToLocalFileTime(&FileInfo.ftLastWriteTime,&LocalFileTime))
			return FileTimeToDosDateTime(&LocalFileTime,&usFileDate,&usFileTime) == TRUE;
	}

	return false;
}

bool fs_setmodtime(const TCHAR *szFileName,unsigned short usFileDate,unsigned short usFileTime)
{
	HANDLE hFile = CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	FILETIME LocalFileTime;
	if (DosDateTimeToFileTime(usFileDate,usFileTime,&LocalFileTime) != TRUE)
		return false;

	FILETIME FileTime;
	if (LocalFileTimeToFileTime(&LocalFileTime,&FileTime) != TRUE)
		return false;

	bool bResult = SetFileTime(hFile,NULL,NULL,&FileTime) == TRUE;
	CloseHandle(hFile);

	return bResult;
}

bool fs_filereadonly(const TCHAR *szFileName)
{
	DWORD dwAttrib = GetFileAttributes(szFileName);

	return (dwAttrib != -1) && (dwAttrib & FILE_ATTRIBUTE_READONLY);
}

__int64 fs_filesize(const TCHAR *szFileName)
{
	HANDLE hFile = CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	__int64 iResult = fs_filesize(hFile);

	CloseHandle(hFile);
    
    return iResult;
}

bool fs_createdir(const TCHAR *szPath)
{
	if (SUCCEEDED(CreateDirectory(szPath,NULL)))
		return true;

	return false;
}

bool fs_directoryexists(const TCHAR *szDirName)
{
	DWORD dwAttrib = GetFileAttributes(szDirName);

	return (dwAttrib != -1) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

/*
	fs_createpath
	-------------
	Create the full specified path. If a file path is specified all directories
	needed for the file will be created. If a folder path is specified, be sure to
	include a trailing backslash or the last directory will not be created.
*/
bool fs_createpath(const TCHAR *szPath)
{
	for (int i = 0; i < lstrlen(szPath); i++)
	{
		if (szPath[i] == '\\' || szPath[i] == '/')
		{
			TCHAR *szTempPath = SubString(szPath,0,i);

			if (!fs_directoryexists(szTempPath))
			{
				if (!fs_createdir(szTempPath))
				{
					delete [] szTempPath;
					return false;
				}
			}

			delete [] szTempPath;
		}
	}

	return true;
}

bool fs_validpath(const TCHAR *szPath)
{
	// 1. First we check if the path is empty then if it's of the correct length.
	// 2. The smallest usable path is (drive):/ hence the minimum length of 3.
	// 3. If the above criterias are met then we make sure that a drive is specified. 
	return !(szPath == NULL || lstrlen(szPath) < 3 || szPath[1] != ':');
}

__int64 fs_freediskspace(const TCHAR *szDirName)
{
	// The dummies are only used for Windows 9x support.
	__int64 iFreeBytes = 0;
	__int64 iDummy1 = 0;
	__int64 iDummy2 = 0;

	if (GetDiskFreeSpaceEx(szDirName,(PULARGE_INTEGER)&iDummy1,
		(PULARGE_INTEGER)&iDummy2,(PULARGE_INTEGER)&iFreeBytes))
	{
		return iFreeBytes;
	}

	return -1;
}

/*
	fs_createtempfilepath
	---------------------
	Tries to create an unique temporary file name in the specified base folder.
	The files is created if it does not exist and the handle is released.
*/
bool fs_createtempfilepath(const TCHAR *szBaseFolder,const TCHAR *szPrefix,TCHAR *szTempName)
{
	return GetTempFileName(szBaseFolder,szPrefix,0,szTempName) != 0;
}

/*
	fs_createtemppath
	-----------------
	Tries to create an unique temporary folder in the specified base folder.
	The folder is created if it does not exist.
	IMORTANT: The base folder must exist.
*/
bool fs_createtemppath(const TCHAR *szBaseFolder,const TCHAR *szPrefix,TCHAR *szTempName)
{
	if (!fs_createtempfilepath(szBaseFolder,szPrefix,szTempName))
		return false;

	if (fs_fileexists(szTempName))
		fs_deletefile(szTempName);

	return fs_createdir(szTempName);
}
