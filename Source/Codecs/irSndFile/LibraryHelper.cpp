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
#include <sndfile.h>
#include "LibraryHelper.h"

CLibraryHelper::CLibraryHelper()
{
	m_hDllInstance = NULL;
	irc_sf_open = NULL;
	irc_sf_close = NULL;
	irc_sf_format_check = NULL;
	irc_sf_read_raw = NULL;
	irc_sf_write_raw = NULL;
}

CLibraryHelper::~CLibraryHelper()
{
}

bool CLibraryHelper::Load(const TCHAR *szFileName)
{
	m_hDllInstance = LoadLibrary(szFileName);
	if (m_hDllInstance == NULL)
		return false;

	irc_sf_open = (tirc_sf_open)GetProcAddress(m_hDllInstance,"sf_open");
	if (irc_sf_open == NULL)
		return false;

	irc_sf_close = (tirc_sf_close)GetProcAddress(m_hDllInstance,"sf_close");
	if (irc_sf_close == NULL)
		return false;

	irc_sf_format_check = (tirc_sf_format_check)GetProcAddress(m_hDllInstance,"sf_format_check");
	if (irc_sf_format_check == NULL)
		return false;

	irc_sf_read_raw = (tirc_sf_read_raw)GetProcAddress(m_hDllInstance,"sf_read_raw");
	if (irc_sf_read_raw == NULL)
		return false;

	irc_sf_write_raw = (tirc_sf_write_raw)GetProcAddress(m_hDllInstance,"sf_write_raw");
	if (irc_sf_write_raw == NULL)
		return false;

	return true;
}

bool CLibraryHelper::Unload()
{
	if (m_hDllInstance != NULL)
		return false;

	FreeLibrary(m_hDllInstance);
	m_hDllInstance = NULL;

	irc_sf_open = NULL;
	irc_sf_close = NULL;
	irc_sf_format_check = NULL;
	irc_sf_read_raw = NULL;
	irc_sf_write_raw = NULL;

	return true;
}

bool CLibraryHelper::IsLoaded()
{
	return m_hDllInstance != NULL;
}
