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

/*
	WM_SETDEVICEINDEX
	-----------------
	Sets the device index variable to the value of lParam.
	Supported windows: CBurnImageDlg and CCopyDiscDlg.
	If wParam = 1  when sending to a CCopyDiscDlg window the source device
	index is set instead of the (default) target device index.
*/
#define WM_SETDEVICEINDEX				WM_APP +  0

/*
	WM_GETDEVICEINDEX
	-----------------
	Returns the value of the device index variable.
	Supported windows: CBurnImageDlg and CCopyDiscDlg.
	If wParam = 1 when sending to a CCopyDiscDlg window the source device index
	is returned instead of the target device index (which is default).
*/
#define WM_GETDEVICEINDEX				WM_APP +  1

/*
	WM_SETCLONEMODE
	---------------
	Sent to the host of a CCopyDiscGeneralPage class when the clone check box
	checked. wParam is true if the check box is checked and false if it's
	unchecked.
*/
#define WM_SETCLONEMODE					WM_APP +  2

/*
	Messages associated with the CShellListViewCtrl control.
*/
#define WM_SLVC_BROWSEOBJECT			WM_APP +  8
#define WM_SLVC_DONEBROWSEOBJECT		WM_APP +  9
#define WM_SLVC_CHANGEFOLDER			WM_APP + 10
#define WM_SLVC_CHANGEFOLDERLINK		WM_APP + 11
#define WM_SLVC_FILECOMMAND				WM_APP + 12

/*
	WM_CHC_SETSORTCOLUMN
	--------------------
	Set to the CCustomHeaderCtrl object to select which column that should be
	drawn with a sort arrow. wParams specifies the index of the column and
	lParam the sort direction (0 for up and 1 for down).
*/
#define WM_CHC_SETSORTCOLUMN			WM_APP + 14

/*
	WM_SHELLCHANGE
	--------------
	Sent by CDirectoryMonitor when there has been a change in a directory.
*/
#define WM_SHELLCHANGE					WM_APP + 15

/*
	WM_CONTROLCUSTOMDRAW
	--------------------
	Custom custom draw message sent to a control implementation that needs to
	be custom drawn. Lparam is a pointer to a NMLVCUSTOMDRAW structure.
*/
#define WM_CONTROLCUSTOMDRAW			WM_APP + 16

/*
	WM_CHECKMEDIA_BROADCAST
	-----------------------
	Sent to a window parent to make it broad cast a WM_CHECKMEDIA message to all
	it's children (pages only). lParam is the index of the currently selected
	device.
*/
#define WM_CHECKMEDIA_BROADCAST			WM_APP + 17

/*
	WM_CHECKMEDIA
	-------------
	Sent to a page whenever the media information should be updated. lParam is the
	index of the currently selected device.
*/
#define WM_CHECKMEDIA					WM_APP + 18

/*
	WM_LABELCONTAINER_CLOSE
	-----------------------
	Sent to the host of a label container control when the close button is
	pressed.
*/
#define WM_LABELCONTAINER_CLOSE			WM_APP + 19
