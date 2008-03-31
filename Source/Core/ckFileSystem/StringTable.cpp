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
#include "StringTable.h"

namespace ckFileSystem
{
	CDynStringTable g_StringTable;

	CDynStringTable::CDynStringTable()
	{
		m_Strings[WARNING_FSDIRLEVEL] = _T("The directory structure is deeper than %d levels. Deep files and folders will be ignored.");
		m_Strings[WARNING_SKIPFILE] = _T("Skipping \"%s\".");
		m_Strings[WARNING_SKIP4GFILE] = _T("Skipping \"%s\", the file is larger than 4 GiB.");
		m_Strings[ERROR_PATHTABLESIZE] = _T("The disc image path table is to large. The project contains too many files.");
		m_Strings[ERROR_OPENWRITE] = _T("Unable to open file for writing: %s.");
		m_Strings[ERROR_OPENREAD] = _T("Unable to open file for reading: %s.");
	}

	const TCHAR *CDynStringTable::GetString(eStrings StringID)
	{
		return m_Strings[StringID];
	}

	/*
		For translation purposes.
	*/
	void CDynStringTable::SetString(eStrings StringID,const TCHAR *szString)
	{
		m_Strings[StringID] = szString;
	}
};
