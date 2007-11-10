/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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

HANDLE fs_open(const TCHAR *szFileName,TCHAR *pMode);
bool fs_close(HANDLE hFile);
__int64 fs_seek(HANDLE hFile,__int64 iDistance,int iMode);
__int64 fs_tell(HANDLE hFile);
__int64 fs_filesize(HANDLE hFile);
unsigned long fs_read(void *pBuffer,unsigned long ulCount,HANDLE hFile);
unsigned long fs_write(void *pBuffer,unsigned long ulCount,HANDLE hFile);
//long fs_bseek(HANDLE hFile,long lDistance,int iMode);

bool fs_fileexists(const TCHAR *szFileName);
bool fs_deletefile(const TCHAR *szFileName);
bool fs_deletedir(const TCHAR *szFolderName/*,bool bForce*/);
bool fs_renamefile(const TCHAR *szOldName,const TCHAR *szNewName);
bool fs_getmodtime(HANDLE hFile,unsigned short &usFileDate,unsigned short &usFileTime);
bool fs_getmodtime(const TCHAR *szFileName,unsigned short &usFileDate,unsigned short &usFileTime);
bool fs_getdirmodtime(const TCHAR *szFileName,unsigned short &usFileDate,unsigned short &usFileTime);
bool fs_setmodtime(const TCHAR *szFileName,unsigned short usFileDate,unsigned short usFileTime);
bool fs_filereadonly(const TCHAR *szFileName);
__int64 fs_filesize(const TCHAR *szFileName);
unsigned long fs_getfileattributes(const TCHAR *szFileName);
bool fs_setfileattributes(const TCHAR *szFileName,unsigned long ulAttr);

bool fs_createdir(const TCHAR *szPath);
bool fs_directoryexists(const TCHAR *szDirName);
bool fs_createpath(const TCHAR *szPath);
bool fs_validpath(const TCHAR *szPath);

__int64 fs_freediskspace(const TCHAR *szDirName);

bool fs_createtempfilepath(const TCHAR *szBaseFolder,const TCHAR *szPrefix,TCHAR *szTempName);
bool fs_createtemppath(const TCHAR *szBaseFolder,const TCHAR *szPrefix,TCHAR *szTempName);
