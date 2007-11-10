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

#include "stdafx.h"
#include <wmsdk.h>
#include "LibraryHelper.h"

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
