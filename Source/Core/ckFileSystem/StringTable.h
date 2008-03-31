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
#include <map>

namespace ckFileSystem
{
	enum eStrings
	{
		WARNING_FSDIRLEVEL,
		WARNING_SKIPFILE,
		WARNING_SKIP4GFILE,
		WARNING_SKIP4GFILEISO,
		ERROR_PATHTABLESIZE,
		ERROR_OPENWRITE,
		ERROR_OPENREAD
	};

	class CDynStringTable
	{
	private:
		std::map<eStrings,const TCHAR *> m_Strings;

	public:
		CDynStringTable();

		const TCHAR *GetString(eStrings StringID);
		void SetString(eStrings StringID,const TCHAR *szString);
	};

	extern CDynStringTable g_StringTable;
};
