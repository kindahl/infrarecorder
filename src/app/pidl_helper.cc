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

#include "stdafx.h"
#include "pidl_helper.hh"

CPidlHelper::CPidlHelper()
{
	if (FAILED(SHGetMalloc(&m_spMalloc)))
		m_spMalloc = NULL;
}

CPidlHelper::~CPidlHelper()
{
}

/*
	CPidlHelper::FreePidl
	---------------------
	Frees the pidl using ILFree. It assumes that the pidl has been allocated using
	ILClone or its equivalent.
*/
void CPidlHelper::FreePidl(LPITEMIDLIST &pidl)
{
	// FIXME: Use CoTaskMemFree(pidl); on Windows 2000 and newer.
	ILFree(pidl);
	pidl = NULL;
}

/*
	CPidlHelper::Split
	------------------
	Splits the pidl into a parent pidl and a relative child pidl. The caller is
	responsible for freeing the pidls created. The function returns true if the
	pidls where successfully created, false otherwise.
*/
bool CPidlHelper::Split(LPCITEMIDLIST pidl,LPITEMIDLIST *pParent,LPITEMIDLIST *pChild)
{
	if (pParent)
	{
		*pParent = NULL;
		ILRemoveLastID(*pParent = ILClone(pidl));

		if (!*pParent)
			return false;
	}

	if (pChild)
	{
		*pChild = NULL;
		*pChild = ILClone(ILFindLastID(pidl));

		if (!*pChild)
		{
			ILFree(*pParent);
			return false;
		}
	}

	return true;
}

/*
	CPidlHelper::GetPathName
	------------------------
	Puts iMaxLength number of bits of the pidls name into szPathName. It assumes
	that pidl is a single SHITEMID that belongs to pParent. It return true if no
	errors occured, false otherwise.
*/
bool CPidlHelper::GetPathName(IShellFolder *pParent,LPCITEMIDLIST pidl,
	TCHAR *szPathName,int iMaxLength)
{
	if (!pidl || !pParent)
		return false;

	STRRET strret;
	if (FAILED(pParent->GetDisplayNameOf(pidl,SHGDN_FORPARSING/* | SHGDN_FORADDRESSBAR*/,&strret)))
		return false;

	// Get pidl from name.
	return STRRET2TCHAR(pidl,&strret,szPathName,iMaxLength);
}

/*
	CPidlHelper::GetDisplayPathName
	-------------------------------
	Puts iMaxLength number of bits of the pidls display name into szPathName.
	It assumes that pidl is a single SHITEMID that belongs to pParent. It return
	true if no errors occured, false otherwise.
*/
bool CPidlHelper::GetDisplayPathName(IShellFolder *pParent,LPCITEMIDLIST pidl,
	TCHAR *szPathName,int iMaxLength)
{
	if (!pidl || !pParent)
		return false;

	STRRET strret;
	if (FAILED(pParent->GetDisplayNameOf(pidl,SHGDN_FORPARSING | SHGDN_FORADDRESSBAR,&strret)))
		return false;

	return STRRET2TCHAR(pidl,&strret,szPathName,iMaxLength);
}

/*
	CPidlHelper::GetPathName
	------------------------
	Puts the pidls name into szPathName. It return true if no errors occured,
	false otherwise.
*/
bool CPidlHelper::GetPathName(LPCITEMIDLIST pidl,TCHAR *szPathName)
{
	if (!pidl)
		return false;

	SHGetPathFromIDList(pidl,szPathName);

	if (!lstrlen(szPathName))
		return false;

	return true;
}

/*
	CPidlHelper::GetPidl
	--------------------
	Generates a pidl from a path name. The function returns true if no errors
	occured, otherwise it returns false.
*/
bool CPidlHelper::GetPidl(TCHAR *szPathName,LPITEMIDLIST *ppidl)
{
	CComBSTR cbsPath(szPathName);
	USES_CONVERSION;
	LPWSTR wszPath = const_cast<LPWSTR>(T2CW(szPathName));
	//HRVERIFY(wszPath);

	CComPtr<IShellFolder>spShellFolder;
	SHGetDesktopFolder(&spShellFolder);

	ULONG dwEaten;
	spShellFolder->ParseDisplayName(0,0,wszPath,&dwEaten,ppidl,0);

	return (ppidl != NULL);
}

/*
	CPidlHelper::strret2TCHAR
	-------------------------
	Converts a STREET structure to a TCHAR string of iMaxChars length. It assumes
	that szTarget is a buffer large enough to contain the string. True is returned
	upong success, false otherwise.
*/
bool CPidlHelper::STRRET2TCHAR(LPCITEMIDLIST pidl,STRRET *psr,TCHAR *szTarget,
							  int iMaxChars,bool bFreeOleStr)
{
	USES_CONVERSION;

	if (!m_spMalloc)
		return false;

	TCHAR *pszTmp = NULL;

	switch (psr->uType)
	{
		case STRRET_WSTR:
			pszTmp = W2T(psr->pOleStr);

			/*if (bFreeOleStr)
				m_spMalloc->Free(psr->pOleStr);*/
			break;

		case STRRET_OFFSET:
			pszTmp = A2T((char *)pidl + psr->uOffset);
			break;

		case STRRET_CSTR:
			pszTmp = A2T(psr->cStr);
			break;
	}

	if (pszTmp != NULL)
	{
		lstrcpyn(szTarget,pszTmp,iMaxChars);

		// The ole-string can't be freed before we have copied the result string.
		if (psr->uType == STRRET_WSTR && bFreeOleStr)
			m_spMalloc->Free(psr->pOleStr);
	}

	return NULL != pszTmp;
}

LPITEMIDLIST CPidlHelper::CreatePidl(unsigned int uiSize)
{
	LPITEMIDLIST pidl = NULL;

	IMalloc* pMalloc;
	if (FAILED(SHGetMalloc(&pMalloc)))
		return false;

	pidl = (LPITEMIDLIST)pMalloc->Alloc(uiSize);
	if (pidl)
	   ZeroMemory(pidl,uiSize);

	pMalloc->Release();
	return pidl;
}

ITEMIDLIST *CPidlHelper::GetNextPidlItem(LPCITEMIDLIST pidl)
{
	if (pidl)
	   return (ITEMIDLIST *)(BYTE *)(((BYTE *)pidl) + pidl->mkid.cb);
	else
	   return NULL;
}

unsigned int CPidlHelper::GetPidlSize(LPCITEMIDLIST pidl)
{
	unsigned int uiTotal = 0;
	ITEMIDLIST *pidlTemp = (ITEMIDLIST *)pidl;

	if (pidlTemp)
	{
		while (pidlTemp->mkid.cb)
		{
			uiTotal += pidlTemp->mkid.cb;
			pidlTemp = GetNextPidlItem(pidlTemp);
		}  

		// Requires a 16 bit zero value for the NULL terminator.
		uiTotal += 2 * sizeof(BYTE);
	}

	return uiTotal;
}

LPITEMIDLIST CPidlHelper::ConcatenatePidl(LPCITEMIDLIST pidl1,LPCITEMIDLIST pidl2)
{
	LPITEMIDLIST pidlNew;
	unsigned int cb1 = 0,cb2 = 0;

	if (pidl1)
		cb1 = GetPidlSize(pidl1) - (2 * sizeof(BYTE));

	cb2 = GetPidlSize(pidl2);
	pidlNew = CreatePidl(cb1 + cb2);

	if (pidlNew)
	{
		if (pidl1)   
			CopyMemory(pidlNew, pidl1, cb1);

	   CopyMemory(((LPBYTE)pidlNew) + cb1, pidl2, cb2);
	}

	return pidlNew;
}
