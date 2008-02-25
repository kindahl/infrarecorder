/*
 * Copyright (C) 2006-2008 Christian Kindahl, christian dot kindahl at gmail dot com
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

#pragma once

#ifdef VERSION_COMPATIBILITY_CHECK

/*
	Major
	-----
	 4 = Windows NT 4.0, Windows Me, Windows 98, or Windows 95.
	 5 = Windows Server 2003, Windows XP, or Windows 2000

	Minor
	-----
	 0 = Windows 2000, Windows NT 4.0, or Windows 95
	 1 = Windows XP
	 2 = Windows Server 2003
	10 = Windows 98
	90 = Windows Me
*/

#define MAJOR_WINNT			4
#define MAJOR_WIN95			4
#define MAJOR_WIN98			4
#define MAJOR_WINME			4
#define MAJOR_WIN2000		5
#define MAJOR_WINXP			5
#define MAJOR_WIN2003		5
#define MAJOR_WINVISTA		6

#define MINOR_WINNT4		0
#define MINOR_WIN95			0
#define MINOR_WIN2000		0
#define MINOR_WINVISTA		0
#define MINOR_WINXP			1
#define MINOR_WIN2003		2
#define MINOR_WIN98			10
#define MINOR_WINME			90

class CWinVer
{
public:
	// Windows.
	unsigned long m_ulMajorVersion;
	unsigned long m_ulMinorVersion;

	// Internet Explorer.
	unsigned long m_ulMajorIEVersion;
	unsigned long m_ulMinorIEVersion;

	// Common Controls.
	unsigned long m_ulMajorCCVersion;
	unsigned long m_ulMinorCCVersion;

	CWinVer();
	~CWinVer();
};

extern CWinVer g_WinVer;

#endif
