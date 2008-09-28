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

enum eStringTable
{
	GENERAL_WARNING = 0,

	ERROR_LOADSETTINGS,

	TAB_DATA,
	TAB_AUDIO,
	TAB_COPY,
	TAB_OTHER,

	TITLE_DATACD,
	TITLE_MIXED,
	TITLE_AUDIO,
	TITLE_BURNIMAGE,
	TITLE_CREATEIMAGE,
	TITLE_ERASE,
	TITLE_FIXATE,
	TITLE_MANAGETRACKS,
	TITLE_COPY,

	DESC_DATACD,
	DESC_MIXED,
	DESC_AUDIO,
	DESC_BURNIMAGE,
	DESC_CREATEIMAGE,
	DESC_ERASE,
	DESC_FIXATE,
	DESC_MANAGETRACKS,
	DESC_COPY,

	// Added version 0.42.
	TAB_VIDEO,
	TITLE_DVDVIDEO,
	DESC_DVDVIDEO,
	TITLE_DATADVD,
	DESC_DATADVD
};

extern TCHAR *g_szStringTable[];
