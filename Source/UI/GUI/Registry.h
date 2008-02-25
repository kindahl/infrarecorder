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

#define STRING_CHUNK_SIZE				128

class CRegistry
{
private:
	HKEY m_hRootKey;
	HKEY m_hKey;

public:
	CRegistry();
	~CRegistry();

	void SetRoot(HKEY hKey);
	bool OpenKey(const TCHAR *szKeyName,bool bCreate = true);
	bool CloseKey();
	bool DeleteKey(const TCHAR *szKeyName);

	bool ReadBool(const TCHAR *szValueName,bool &bResult);
	bool ReadInteger(const TCHAR *szValueName,int &iResult);
	bool ReadInteger64(const TCHAR *szValueName,__int64 &iResult);
	bool ReadString(const TCHAR *szValueName,TCHAR *szResult,unsigned long ulBufferSize);
	bool ReadStringEx(const TCHAR *szValueName,TCHAR *&szResult);

	bool WriteBool(const TCHAR *szValueName,bool bValue);
	bool WriteInteger(const TCHAR *szValueName,int iValue);
	bool WriteInteger64(const TCHAR *szValueName,__int64 iValue);
	bool WriteString(const TCHAR *szValueName,TCHAR *szValue,unsigned long ulBufferSize);
	bool WriteStringEx(const TCHAR *szValueName,TCHAR *szValue);
};
