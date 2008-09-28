/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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

#pragma once
#include <shlobj.h>
#include <comdef.h>
#include "resource.h"
#include "irShell.h"

class ATL_NO_VTABLE CirShellExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CirShellExt,&CLSID_irShellExt>,
	public IShellExtInit,
	public IContextMenu
{
private:
	// Is set to true if the selected file is an InfraRecorder project.
	bool m_bIsProject;

	// Bitmaps.
	HBITMAP m_hBurnBitmap;

	bool IsSeparatorItem(HMENU hMenu,unsigned int uiPosition);
	bool IsProjectFile(const TCHAR *szFileName);

	unsigned int FillProjectMenu(HMENU hMenu,unsigned int uiMenuIndex,unsigned int uiFirstCmd);
	unsigned int FillDiscImageMenu(HMENU hMenu,unsigned int uiMenuIndex,unsigned int uiFirstCmd);

	HRESULT GetProjectCommandString(LPSTR pszName,UINT cchMax,bool bUnicode,UINT_PTR uiID);
	HRESULT GetDiscImageCommandString(LPSTR pszName,UINT cchMax,bool bUnicode,UINT_PTR uiID);

	HRESULT InvokeProjectCommand(HWND hWnd,unsigned int uiID);
	HRESULT InvokeDiscImageCommand(HWND hWnd,unsigned int uiID);

protected:
	TCHAR m_szFileName[MAX_PATH];

public:
	CirShellExt();
	~CirShellExt();

	DECLARE_REGISTRY_RESOURCEID(IDR_IRSHELLEXT)

	DECLARE_NOT_AGGREGATABLE(CirShellExt)

	BEGIN_COM_MAP(CirShellExt)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();	
	void FinalRelease();

	// IShellExtInit.
	STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder,LPDATAOBJECT pDataObj,HKEY hProgID);

	// IContextMenu.
	STDMETHODIMP QueryContextMenu(HMENU hmenu,UINT uMenuIndex,UINT uidFirstCmd,
		UINT uidLastCmd,UINT uFlags);
	STDMETHODIMP GetCommandString(UINT_PTR idCmd,UINT uFlags,UINT *pwReserved,
		LPSTR pszName,UINT cchMax);
	STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo);
};

OBJECT_ENTRY_AUTO(__uuidof(irShellExt),CirShellExt)
