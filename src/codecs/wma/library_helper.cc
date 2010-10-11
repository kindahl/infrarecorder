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

#include "stdafx.hh"
#include <wmsdk.h>
#include "library_helper.hh"

CLibraryHelper::CLibraryHelper()
{
	m_hDllInstance = NULL;
	irc_WMCreateProfileManager = NULL;
	irc_WMCreateWriter = NULL;
	irc_WMCreateSyncReader = NULL;
}

CLibraryHelper::~CLibraryHelper()
{
}

bool CLibraryHelper::Load(const TCHAR *szFileName)
{
	m_hDllInstance = LoadLibrary(szFileName);
	if (m_hDllInstance == NULL)
		return false;

	irc_WMCreateProfileManager = (tirc_WMCreateProfileManager)GetProcAddress(m_hDllInstance,"WMCreateProfileManager");
	if (irc_WMCreateProfileManager == NULL)
		return false;

	irc_WMCreateWriter = (tirc_WMCreateWriter)GetProcAddress(m_hDllInstance,"WMCreateWriter");
	if (irc_WMCreateWriter == NULL)
		return false;

	irc_WMCreateSyncReader = (tirc_WMCreateSyncReader)GetProcAddress(m_hDllInstance,"WMCreateSyncReader");
	if (irc_WMCreateSyncReader == NULL)
		return false;

	return true;
}

bool CLibraryHelper::Unload()
{
	if (m_hDllInstance != NULL)
		return false;

	FreeLibrary(m_hDllInstance);
	m_hDllInstance = NULL;

	irc_WMCreateProfileManager = NULL;
	irc_WMCreateWriter = NULL;
	irc_WMCreateSyncReader = NULL;

	return true;
}

bool CLibraryHelper::IsLoaded()
{
	return m_hDllInstance != NULL;
}
