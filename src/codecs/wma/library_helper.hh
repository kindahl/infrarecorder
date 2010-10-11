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

#pragma once

typedef HRESULT (__stdcall *tirc_WMCreateProfileManager)(IWMProfileManager **ppProfileManager);
typedef HRESULT (__stdcall *tirc_WMCreateWriter)(IUnknown *pUnkCert,IWMWriter **ppWriter);
typedef HRESULT (__stdcall *tirc_WMCreateSyncReader)(IUnknown *pUnkCert,DWORD dwRights,IWMSyncReader **ppSyncReader);

class CLibraryHelper
{
private:
	HINSTANCE m_hDllInstance;

public:
	CLibraryHelper();
	~CLibraryHelper();

	bool Load(const TCHAR *szFileName);
	bool Unload();
	bool IsLoaded();

	tirc_WMCreateProfileManager irc_WMCreateProfileManager;
	tirc_WMCreateWriter irc_WMCreateWriter;
	tirc_WMCreateSyncReader irc_WMCreateSyncReader;
};
