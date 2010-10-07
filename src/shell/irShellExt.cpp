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
#include <ckcore/types.hh>
#include <base/string_util.hh>
#include "irShellExt.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "SettingsManager.h"

CirShellExt::CirShellExt()
{
	// Load the configuration.
	g_SettingsManager.Load();

	// Load bitmaps.
	m_hBurnBitmap = LoadBitmap(g_DllInstance,MAKEINTRESOURCE(IDB_BURNBITMAP));
}

CirShellExt::~CirShellExt()
{
	// Unload bitmaps.
	if (m_hBurnBitmap != NULL)
		DeleteObject(m_hBurnBitmap);
}

HRESULT CirShellExt::FinalConstruct()
{
	return S_OK;
}
	
void CirShellExt::FinalRelease() 
{
}

/*
	CirShellExt::IsSeparatorItem
	----------------------------
	Checks if the specified menu item is a separator and returns true if it is
	and false otherwise.
*/
bool CirShellExt::IsSeparatorItem(HMENU hMenu,unsigned int uiPosition)
{
	MENUITEMINFO mii;
	memset(&mii,0,sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE;

	::GetMenuItemInfo(hMenu,uiPosition,TRUE,&mii);

	return mii.fType == MFT_SEPARATOR;
}

/*
	CirShellExt::IsProjectFile
	--------------------------
	Determines wheter the specified file is a project file or not. The function
	returns true if it is a project file, false otherwise.
*/
bool CirShellExt::IsProjectFile(const TCHAR *szFileName)
{
	ckcore::Path FilePath(szFileName);
	return !lstrcmpi(FilePath.ext_name().c_str(),ckT("irp"));
}

STDMETHODIMP CirShellExt::Initialize(LPCITEMIDLIST pidlFolder,
									 LPDATAOBJECT pDataObj,HKEY hProgID)
{
	FORMATETC fmt = { CF_HDROP,NULL,DVASPECT_CONTENT,-1,TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };

	// Check if there is any CF_HDROP data in the data object.
	if (FAILED(pDataObj->GetData(&fmt,&stg)))
		return E_INVALIDARG;

	// Get pointer to data.
	HDROP hDrop = (HDROP)GlobalLock(stg.hGlobal);
	if (hDrop == NULL)
		return E_INVALIDARG;

	// Get file name.
	unsigned int uiNumFiles = DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);
	HRESULT hResult = E_INVALIDARG;
	
	if (uiNumFiles > 0)
	{
		if (DragQueryFile(hDrop,0,m_szFileName,MAX_PATH - 1))
		{
			hResult = S_OK;

			// Check if we're dealing with a project file.
			m_bIsProject = IsProjectFile(m_szFileName);
		}
	}

	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);

	return hResult;
}

/*
	CirShellExt::FillProjectMenu
	----------------------------
	Fills the menu with project related items and retursn the next menu index to
	be used by any following menu items.
*/
unsigned int CirShellExt::FillProjectMenu(HMENU hMenu,unsigned int uiMenuIndex,
										  unsigned int uiFirstCmd)
{
	// Burn project.
	InsertMenu(hMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiFirstCmd++,lngGetString(MENU_BURNPROJECT));
	if (g_GlobalSettings.m_bShellExtIcon)
		SetMenuItemBitmaps(hMenu,uiMenuIndex,MF_BYPOSITION,m_hBurnBitmap,NULL);
	uiMenuIndex++;

	// Open project.
	InsertMenu(hMenu,uiMenuIndex++,MF_STRING | MF_BYPOSITION,uiFirstCmd++,lngGetString(MENU_OPENPROJECT));
	
	return uiMenuIndex;
}

/*
	CirShellExt::FillDiscImageMenu
	------------------------------
	Fills the menu with disc image related items and returns the next menu index
	to be used by any following menu items.
*/
unsigned int CirShellExt::FillDiscImageMenu(HMENU hMenu,unsigned int uiMenuIndex,
											unsigned int uiFirstCmd)
{
	// Burn image.
	InsertMenu(hMenu,uiMenuIndex,MF_STRING | MF_BYPOSITION,uiFirstCmd++,lngGetString(MENU_BURNIMAGE));
	if (g_GlobalSettings.m_bShellExtIcon)
		SetMenuItemBitmaps(hMenu,uiMenuIndex,MF_BYPOSITION,m_hBurnBitmap,NULL);
	uiMenuIndex++;

	return uiMenuIndex;
}

HRESULT CirShellExt::QueryContextMenu(HMENU hmenu,UINT uMenuIndex,UINT uidFirstCmd,
									  UINT uidLastCmd,UINT uFlags)
{
	// Check if we should add any items.
	if (uFlags & CMF_DEFAULTONLY)
		return MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL,0);

	unsigned int uiFirstMenuIndex = uMenuIndex;

	// If the item before our item is not a separator we add a new separator.
	if (!IsSeparatorItem(hmenu,uMenuIndex - 1))
		InsertMenu(hmenu,uMenuIndex++,MF_SEPARATOR | MF_BYPOSITION,0,NULL);

	// Should we add the menu items in a submenu?
	if (g_GlobalSettings.m_bShellExtSubMenu)
	{
		HMENU hPopupMenu = CreatePopupMenu();

		if (m_bIsProject)
			FillProjectMenu(hPopupMenu,0,uidFirstCmd);
		else
			FillDiscImageMenu(hPopupMenu,0,uidFirstCmd);

		InsertMenu(hmenu,uMenuIndex++,MF_BYPOSITION | MF_POPUP,(UINT_PTR)hPopupMenu,_T("InfraRecorder"));
	}
	else
	{
		if (m_bIsProject)
			uMenuIndex = FillProjectMenu(hmenu,uMenuIndex,uidFirstCmd);
		else
			uMenuIndex = FillDiscImageMenu(hmenu,uMenuIndex,uidFirstCmd);
	}

	// If the item below our items is not a separator we add a new separator.
	if ((unsigned int)GetMenuItemCount(hmenu) > uMenuIndex)
	{
		if (!IsSeparatorItem(hmenu,uMenuIndex))
			InsertMenu(hmenu,uMenuIndex++,MF_SEPARATOR | MF_BYPOSITION,0,NULL);
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL,uMenuIndex - uiFirstMenuIndex);
}

HRESULT CirShellExt::GetProjectCommandString(LPSTR pszName,UINT cchMax,bool bUnicode,
											 UINT_PTR uiID)
{
USES_CONVERSION;

	switch (uiID)
	{
		case 0:
			if (bUnicode)
				lstrcpynW((LPWSTR)pszName,T2CW(lngGetString(HINT_BURNPROJECT)),cchMax);
			else
				lstrcpynA(pszName,T2CA(lngGetString(HINT_BURNPROJECT)),cchMax);

			return S_OK;

		case 1:
			if (bUnicode)
				lstrcpynW((LPWSTR)pszName,T2CW(lngGetString(HINT_OPENPROJECT)),cchMax);
			else
				lstrcpynA(pszName,T2CA(lngGetString(HINT_OPENPROJECT)),cchMax);

			return S_OK;
	}

	return E_INVALIDARG;
}

HRESULT CirShellExt::GetDiscImageCommandString(LPSTR pszName,UINT cchMax,bool bUnicode,
											   UINT_PTR uiID)
{
USES_CONVERSION;

	switch (uiID)
	{
		case 0:
			if (bUnicode)
				lstrcpynW((LPWSTR)pszName,T2CW(lngGetString(HINT_BURNIMAGE)),cchMax);
			else
				lstrcpynA(pszName,T2CA(lngGetString(HINT_BURNIMAGE)),cchMax);

			return S_OK;
	}

	return E_INVALIDARG;
}

HRESULT CirShellExt::GetCommandString(UINT_PTR idCmd,UINT uFlags,UINT *pwReserved,
									  LPSTR pszName,UINT cchMax)
{
	// Currently only help strings are supported/supplied.
	if (uFlags & GCS_HELPTEXT)
	{
		if (m_bIsProject)
			return GetProjectCommandString(pszName,cchMax,(uFlags & GCS_UNICODE) != 0,idCmd);
		else
			return GetDiscImageCommandString(pszName,cchMax,(uFlags & GCS_UNICODE) != 0,idCmd);
	}

	return E_INVALIDARG;
}

HRESULT CirShellExt::InvokeProjectCommand(HWND hWnd,unsigned int uiID)
{
	TCHAR szParam[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(g_DllInstance,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,_T("InfraRecorder.exe"));

	switch (uiID)
    {
		case 0:		// Burn project.
			lstrcpy(szParam,_T("-burnproject "));
			lstrcat(szParam,m_szFileName);

			ShellExecute(HWND_DESKTOP,_T("open"),szFileName,szParam,NULL,SW_SHOWDEFAULT);
			return S_OK;

		case 1:		// Open project.
			ShellExecute(HWND_DESKTOP,_T("open"),szFileName,m_szFileName,NULL,SW_SHOWDEFAULT);
			return S_OK;
    }

	return E_INVALIDARG;
}

HRESULT CirShellExt::InvokeDiscImageCommand(HWND hWnd,unsigned int uiID)
{
	TCHAR szParam[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(g_DllInstance,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,_T("InfraRecorder.exe"));

	switch (uiID)
    {
		case 0:		// Burn image.
			lstrcpy(szParam,_T("-burnimage "));
			lstrcat(szParam,m_szFileName);

			ShellExecute(HWND_DESKTOP,_T("open"),szFileName,szParam,NULL,SW_SHOWDEFAULT);
			return S_OK;
    }

	return E_INVALIDARG;
}

HRESULT CirShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo)
{
	// Abort if lpVerb points to a string.
	if (HIWORD(pCmdInfo->lpVerb) != 0)
		return E_INVALIDARG;

	if (m_bIsProject)
		return InvokeProjectCommand(pCmdInfo->hwnd,LOWORD(pCmdInfo->lpVerb));
	else
		return InvokeDiscImageCommand(pCmdInfo->hwnd,LOWORD(pCmdInfo->lpVerb));
}
