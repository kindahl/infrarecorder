/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "resource.h"
#include "shell.hh"

class CShellModule : public CAtlDllModuleT<CShellModule>
{
public :
    DECLARE_LIBID(LIBID_ShellLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SHELL,"{8E8DAC3C-E7C5-4495-9903-430C1F38CF86}")
};

CShellModule _AtlModule;
HINSTANCE g_DllInstance = NULL;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID lpReserved)
{
    g_DllInstance = hInstance;
    return _AtlModule.DllMain(dwReason,lpReserved); 
}

/*
    DllCanUnloadNow
    ---------------
    Used to determine whether the DLL can be unloaded by OLE.
*/
STDAPI DllCanUnloadNow()
{
    return _AtlModule.DllCanUnloadNow();
}

/*
    DllGetClassObject
    -----------------
    Returns a class factory to create an object of the requested type.
*/
STDAPI DllGetClassObject(REFCLSID rclsid,REFIID riid,LPVOID *ppv)
{
    return _AtlModule.DllGetClassObject(rclsid,riid,ppv);
}

/*
    DllRegisterServer
    -----------------
    Adds entries to the system registry.
*/
STDAPI DllRegisterServer()
{
    HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
    return hr;
}

/*
    DllUnregisterServer
    -------------------
    Removes entries from the system registry.
*/
STDAPI DllUnregisterServer()
{
    HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
    return hr;
}
