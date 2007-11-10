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

class CPidlHelper 
{
protected:
	CComPtr<IMalloc> m_spMalloc;

private:
	LPITEMIDLIST CreatePidl(unsigned int uiSize);
	ITEMIDLIST *GetNextPidlItem(LPCITEMIDLIST pidl);
	unsigned int GetPidlSize(LPCITEMIDLIST pidl);

public:
	CPidlHelper();
	virtual ~CPidlHelper();

	bool GetPathName(IShellFolder *pParent,LPCITEMIDLIST pidl,TCHAR *szPathName,int iMaxLength = MAX_PATH);
	bool GetDisplayPathName(IShellFolder *pParent,LPCITEMIDLIST pidl,TCHAR *szPathName,int iMaxLength = MAX_PATH);
	bool GetPathName(LPCITEMIDLIST pidl,TCHAR *szPathName);
	bool GetPidl(TCHAR *szPathName,LPITEMIDLIST *ppidl);
	bool Split(LPCITEMIDLIST pidl,LPITEMIDLIST *pParent,LPITEMIDLIST *pChild);
	bool STRRET2TCHAR(LPCITEMIDLIST pidl,STRRET *psr,TCHAR *szTarget,int iMaxChars = MAX_PATH,bool bFreeOleStr = true);
	void FreePidl(LPITEMIDLIST &pidl);

	LPITEMIDLIST ConcatenatePidl(LPCITEMIDLIST pidl1,LPCITEMIDLIST pidl2);
};
