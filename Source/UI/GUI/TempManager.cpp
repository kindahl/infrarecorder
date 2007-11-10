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

#include "stdafx.h"
#include "TempManager.h"
#include "../../Common/FileManager.h"
#include "Settings.h"

CTempManager g_TempManager;

CTempManager::CTempManager()
{
	m_szEmptyDir[0] = '\0';
}

CTempManager::~CTempManager()
{
}

/*
	CTempManager::AddObject
	-----------------------
	Adds a new file or folder to the list of files and folder to be removed when
	the application closes. Please note that directories has to be empty to be able
	to remove them.
*/
void CTempManager::AddObject(const TCHAR *szFileName)
{
	m_szFileNames.push_back(szFileName);
}

void CTempManager::CleanUp()
{
	// Remove the empty directory (if created).
	if (fs_directoryexists(m_szEmptyDir))
		fs_deletedir(m_szEmptyDir);
	m_szEmptyDir[0] = '\0';

	// Remove any files.
	for (unsigned int i = 0; i < m_szFileNames.size(); i++)
    {
		if (fs_fileexists(m_szFileNames[i].c_str()))
			fs_deletefile(m_szFileNames[i].c_str());
        else if (fs_directoryexists(m_szFileNames[i].c_str()))
			fs_deletedir(m_szFileNames[i].c_str());
	}

	m_szFileNames.clear();
}

const TCHAR *CTempManager::GetEmtpyDirectory()
{
	if (m_szEmptyDir[0] == '\0')
		fs_createtemppath(g_GlobalSettings.m_szTempPath,_T("irEmpty"),m_szEmptyDir);

	return m_szEmptyDir;
}
