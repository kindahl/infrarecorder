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

typedef SNDFILE *(*tirc_sf_open)(const char *path,int mode,SF_INFO *sfinfo);
typedef int (*tirc_sf_close)(SNDFILE *sndfile);
typedef int (*tirc_sf_format_check)(const SF_INFO *info);
typedef sf_count_t (*tirc_sf_read_raw)(SNDFILE *sndfile,void *ptr,sf_count_t bytes);
typedef sf_count_t (*tirc_sf_write_raw)(SNDFILE *sndfile,const void *ptr,sf_count_t bytes);

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

	tirc_sf_open irc_sf_open;
	tirc_sf_close irc_sf_close;
	tirc_sf_format_check irc_sf_format_check;
	tirc_sf_read_raw irc_sf_read_raw;
	tirc_sf_write_raw irc_sf_write_raw;
};
