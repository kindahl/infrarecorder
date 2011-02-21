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
#include "shell_ext_util.hh"
#include <base/string_util.hh>

bool ExecShellExtFunction(const char *szFunctionName)
{
	// Calculate file path.
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL,szFileName,MAX_PATH - 1);
	ExtractFilePath(szFileName);
	lstrcat(szFileName,_T("shell.dll"));

	// Load library.
	HINSTANCE hInstance = LoadLibrary(szFileName);
	if (!hInstance)
		return false;

	// Call the function.
	HRESULT (STDAPICALLTYPE* pfn)();

	(FARPROC&) pfn = GetProcAddress(hInstance,szFunctionName);
	if (pfn == NULL)
	{
		FreeLibrary(hInstance);
		return false;
	}

	if (FAILED(pfn()))
	{
		FreeLibrary(hInstance);
		return false;
	}

	FreeLibrary(hInstance);
	return true;
}

/*
	RegisterShellExtension
	----------------------
	Registers the shell extension.
*/
bool RegisterShellExtension()
{
	return ExecShellExtFunction("DllRegisterServer");
}

/*
	RegisterShellExtension
	----------------------
	Unregisters the shell extension.
*/
bool UnregisterShellExtension()
{
	return ExecShellExtFunction("DllUnregisterServer");
}

/*
	IsShellExtensionRegistered
	--------------------------
	Returns true if the shell extension is registered with the system,
	it returns false otherwise.
*/
bool IsShellExtensionRegistered()
{
	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	if (Reg.OpenKey(_T("AppID\\shell.DLL"),false))
	{
		TCHAR szAppID[39];

		if (Reg.ReadString(_T("AppID"),szAppID,39 * sizeof(TCHAR)))
		{
			Reg.CloseKey();

			if (!lstrcmp(szAppID,_T(IRSHELL_APPID)))
				return true;
		}

		Reg.CloseKey();
		return false;
	}

	return false;
}

/*
	InstallExtension
	----------------
	Associates a file extension with the shell extension.
*/
bool InstallExtension(const TCHAR *szFileExt,CRegistry *pReg)
{
	if (pReg->OpenKey(szFileExt))
	{
		TCHAR szKeyName[64];

		// If no key is specified we create our own.
		if (!pReg->ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)) || !lstrcmp(szKeyName,_T("")))
		{
			// Extension key name.
			lstrcpy(szKeyName,_T("iriso"));

			if (!pReg->WriteString(_T(""),szKeyName,64 * sizeof(TCHAR)))
			{
				pReg->CloseKey();
				return false;
			}

			pReg->CloseKey();

			// Key name description.
			if (!pReg->OpenKey(_T("iriso"),true))
				return false;

			if (!pReg->WriteString(_T(""),_T("InfraRecorder disc image"),25 * sizeof(TCHAR)))
			{
				pReg->CloseKey();
				return false;
			}
		}

		// Open key name and install shell extension.
		pReg->CloseKey();

		TCHAR szFullName[256];
		lstrcpy(szFullName,szKeyName),
		lstrcat(szFullName,_T("\\ShellEx\\ContextMenuHandlers\\Shell"));

		if (!pReg->OpenKey(szFullName))
			return false;

		if (!pReg->WriteString(_T(""),_T(IRSHELL_GUID),38 * sizeof(TCHAR)))
		{
			pReg->CloseKey();
			return false;
		}

		pReg->CloseKey();
		return true;
	}

	return false;
}

bool InstallProjectExtension(CRegistry *pReg)
{
	if (pReg->OpenKey(_T(".irp")))
	{
		TCHAR szKeyName[64];

		// If no key is specified we create our own.
		if (!pReg->ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)) || !lstrcmp(szKeyName,_T("")))
		{
			// Extension key name.
			lstrcpy(szKeyName,_T("irproject"));

			if (!pReg->WriteString(_T(""),szKeyName,64 * sizeof(TCHAR)))
			{
				pReg->CloseKey();
				return false;
			}

			pReg->CloseKey();

			// Key name description.
			if (!pReg->OpenKey(_T("irproject"),true))
				return false;

			if (!pReg->WriteString(_T(""),_T("InfraRecorder project"),25 * sizeof(TCHAR)))
			{
				pReg->CloseKey();
				return false;
			}
		}

		// Open key name and install shell extension.
		pReg->CloseKey();

		TCHAR szFullName[256];
		lstrcpy(szFullName,szKeyName),
		lstrcat(szFullName,_T("\\ShellEx\\ContextMenuHandlers\\Shell"));

		if (!pReg->OpenKey(szFullName))
			return false;

		if (!pReg->WriteString(_T(""),_T(IRSHELL_GUID),38 * sizeof(TCHAR)))
		{
			pReg->CloseKey();
			return false;
		}

		pReg->CloseKey();
		return true;
	}

	return false;
}

/*
	UninstallExtension
	------------------
	Removes the association of a file extension with the shell extension.
*/
bool UninstallExtension(const TCHAR *szFileExt,CRegistry *pReg)
{
	if (!pReg->OpenKey(szFileExt,false))
		return true;

	TCHAR szKeyName[64];
	if (pReg->ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)))
	{
		pReg->CloseKey();

		if (lstrcmp(szKeyName,_T("")))
		{
			TCHAR szFullName[256];
			lstrcpy(szFullName,szKeyName),
			lstrcat(szFullName,_T("\\ShellEx\\ContextMenuHandlers"));

			if (!pReg->OpenKey(szFullName,false))
				return false;

			bool bResult = pReg->DeleteKey(_T("Shell"));
				pReg->CloseKey();
			return bResult;
		}
	}

	return true;
}

/*
	IsExtensionInstalled
	--------------------
	Returns true if the shell extension is associated with the speicifed
	file extension, otherwise it returns false.
*/
bool IsExtensionInstalled(const TCHAR *szFileExt,CRegistry *pReg)
{
	if (!pReg->OpenKey(szFileExt,false))
		return false;

	TCHAR szKeyName[64];
	if (pReg->ReadString(_T(""),szKeyName,64 * sizeof(TCHAR)))
	{
		pReg->CloseKey();

		if (lstrcmp(szKeyName,_T("")))
		{
			TCHAR szFullName[256];
			lstrcpy(szFullName,szKeyName),
			lstrcat(szFullName,_T("\\ShellEx\\ContextMenuHandlers\\Shell"));

			if (!pReg->OpenKey(szFullName,false))
				return false;

			pReg->CloseKey();
			return true;
		}
	}

	return false;
}
