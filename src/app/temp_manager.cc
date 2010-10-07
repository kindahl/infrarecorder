/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.hh"
#include <ckcore/directory.hh>
#include "temp_manager.hh"
#include "settings.hh"

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
	ckcore::Directory::remove(m_szEmptyDir);
	m_szEmptyDir[0] = '\0';

	// Remove any files.
	for (unsigned int i = 0; i < m_szFileNames.size(); i++)
    {
		ckcore::File File(m_szFileNames[i].c_str());
		ckcore::Directory Directory(m_szFileNames[i].c_str());

		if (File.exist())
			File.remove();
		else if (Directory.exist())
			Directory.remove();
	}

	m_szFileNames.clear();
}

const TCHAR *CTempManager::GetEmtpyDirectory()
{
	if (m_szEmptyDir[0] == '\0')
	{
		ckcore::Directory TempDir = ckcore::Directory::temp();
		TempDir.create();

		lstrcpy(m_szEmptyDir,TempDir.name().c_str());
	}

	return m_szEmptyDir;
}
