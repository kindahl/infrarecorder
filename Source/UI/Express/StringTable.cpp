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

TCHAR *g_szStringTable[] = {
	_T("Warning"),

	_T("Unable to load the configuration file, it may be corrupt. The XML processor returned: %d."),

	_T("data"),
	_T("audio"),
	_T("copy"),
	_T("other"),

	_T("Create Data CD"),
	_T("Create Mixed-Mode Disc"),
	_T("Create Audio Disc"),
	_T("Burn Image to Disc"),
	_T("Create Image from Disc"),
	_T("Erase Disc"),
	_T("Fixate Disc"),
	_T("Manage Tracks"),
	_T("Copy Disc"),

	_T("Create a regular data CD that contains files and folders of your choice."),
	_T("Create a disc that contains files and folders as well as audio tracks."),
	_T("Create a regular audio disc that plays in all CD players."),
	_T("Burn an existing disc image to a real physical disc."),
	_T("Create a complete disc image file from a physical disc."),
	_T("Erase the contents of a rewritable disc."),
	_T("Fixate a disc that has not already been fixated."),
	_T("View disc track layout, save tracks and verify tracks."),
	_T("Make a backup copy of a disc."),

	// Added version 0.42.
	_T("video"),
	_T("Create DVD-Video Disc"),
	_T("Create a DVD-Video compatible disc from a directory on your computer."),
	_T("Create Data DVD"),
	_T("Create a regular data DVD that contains files and folders of your choice.")
};
