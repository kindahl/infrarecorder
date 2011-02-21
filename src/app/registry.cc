/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include "registry.hh"

CRegistry::CRegistry()
{
	m_hRootKey = HKEY_CURRENT_USER;
}

CRegistry::~CRegistry()
{
}

void CRegistry::SetRoot(HKEY hKey)
{
	m_hRootKey = hKey;
}

bool CRegistry::OpenKey(const TCHAR *szKeyName,bool bCreate)
{
	if (RegOpenKeyEx(m_hRootKey,szKeyName,0,KEY_ALL_ACCESS,&m_hKey) == ERROR_SUCCESS)
		return true;

	if (bCreate)
	{
		DWORD dwResult;

		if (RegCreateKeyEx(m_hRootKey,szKeyName,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&m_hKey,&dwResult) == ERROR_SUCCESS)
			return true;
	}

	return false;
}

bool CRegistry::CloseKey()
{
	if (RegCloseKey(m_hKey) == ERROR_SUCCESS)
		return true;

	return false;
}

bool CRegistry::DeleteKey(const TCHAR *szKeyName)
{
	if (RegDeleteKey(m_hKey,szKeyName) == ERROR_SUCCESS)
		return true;

	return false;
}

bool CRegistry::ReadBool(const TCHAR *szValueName,bool &bResult)
{
	DWORD dwValue,dwSize = sizeof(DWORD),dwType = REG_DWORD;

	if (RegQueryValueEx(m_hKey,szValueName,NULL,&dwType,(LPBYTE)&dwValue,&dwSize) == ERROR_SUCCESS)
	{
		bResult = dwValue >= 1 ? true : false;
		return true;
	}

	return false;
}

bool CRegistry::ReadInteger(const TCHAR *szValueName,int &iResult)
{
	DWORD dwValue,dwSize = sizeof(DWORD),dwType = REG_DWORD;

	if (RegQueryValueEx(m_hKey,szValueName,NULL,&dwType,(LPBYTE)&dwValue,&dwSize) == ERROR_SUCCESS)
	{
		iResult = (int)dwValue;
		return true;
	}

	return false;
}

bool CRegistry::ReadInteger64(const TCHAR *szValueName,__int64 &iResult)
{
	DWORD dwSize = sizeof(__int64),dwType = REG_QWORD;
	LARGE_INTEGER liValue;

	if (RegQueryValueEx(m_hKey,szValueName,NULL,&dwType,(LPBYTE)&liValue,&dwSize) == ERROR_SUCCESS)
	{
		iResult = (__int64)liValue.QuadPart;
		return true;
	}

	return false;
}

bool CRegistry::ReadString(const TCHAR *szValueName,TCHAR *szResult,
	unsigned long ulBufferSize)
{
	DWORD dwBufferSize = ulBufferSize;

	if (RegQueryValueEx(m_hKey,szValueName,NULL,NULL,(LPBYTE)szResult,
		&dwBufferSize) == ERROR_SUCCESS)
	{
		return true;
	}

	return false;
}

bool CRegistry::ReadStringEx(const TCHAR *szValueName,TCHAR *&szResult)
{
	unsigned long ulBufferSize = STRING_CHUNK_SIZE;
	szResult = new TCHAR[STRING_CHUNK_SIZE];
	long lResult = 0;

	while ((lResult = RegQueryValueEx(m_hKey,szValueName,NULL,NULL,(LPBYTE)szResult,
		&ulBufferSize)) == ERROR_MORE_DATA)
    {
        // Increase the buffer size.
        ulBufferSize += STRING_CHUNK_SIZE;

        // Reallocate memory for the buffer. Why doesn't the realloc code work?
        //realloc(szResult,ulBufferSize);
		delete [] szResult;
		szResult = new TCHAR[ulBufferSize];
    }

	if (lResult != ERROR_SUCCESS)
	{
		delete [] szResult;
		return false;
	}

	return true;
}

bool CRegistry::WriteBool(const TCHAR *szValueName,bool bValue)
{
	DWORD dwValue = (DWORD)bValue;

	if (RegSetValueEx(m_hKey,szValueName,NULL,(DWORD)REG_DWORD,(LPBYTE)&dwValue,
		(DWORD)sizeof(DWORD)) == ERROR_SUCCESS)
	{
		return true;
	}

	return false;
}

bool CRegistry::WriteInteger(const TCHAR *szValueName,int iValue)
{
	DWORD dwValue = (DWORD)iValue;

	if (RegSetValueEx(m_hKey,szValueName,NULL,(DWORD)REG_DWORD,(LPBYTE)&dwValue,
		(DWORD)sizeof(DWORD)) == ERROR_SUCCESS)
	{
		return true;
	}

	return false;
}

bool CRegistry::WriteInteger64(const TCHAR *szValueName,__int64 iValue)
{
	LARGE_INTEGER liValue;
	liValue.QuadPart = iValue;

	if (RegSetValueEx(m_hKey,szValueName,NULL,(DWORD)REG_QWORD,(LPBYTE)&liValue,
		(DWORD)sizeof(__int64)) == ERROR_SUCCESS)
	{
		return true;
	}

	return false;
}

bool CRegistry::WriteString(const TCHAR *szValueName,TCHAR *szValue,unsigned long ulBufferSize)
{
	if (RegSetValueEx(m_hKey,szValueName,NULL,REG_SZ,(LPBYTE)szValue,ulBufferSize) == ERROR_SUCCESS)
		return true;

	return false;
}

bool CRegistry::WriteStringEx(const TCHAR *szValueName,TCHAR *szValue)
{
	unsigned long ulBufferSize = lstrlen(szValue);

	if (RegSetValueEx(m_hKey,szValueName,NULL,REG_SZ,(LPBYTE)szValue,ulBufferSize) == ERROR_SUCCESS)
		return true;

	return false;
}
