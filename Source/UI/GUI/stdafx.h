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

// Change these values to use different versions.
#define WINVER			0x0400
//#define _WIN32_WINNT	0x0400
//#define _WIN32_WINNT	0x0500

#ifndef UNICODE		// Target non-unicode releases to Win9x/ME.
#define _WIN32_WINNT	0x0400
#else
#define _WIN32_WINNT	0x0500
#endif

#define _WIN32_IE		0x0500
#define _RICHEDIT_VER	0x0100

#define VERSION_COMPATIBILITY_CHECK

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlsplit.h>		// CSplitterWindowImpl
#include <htmlhelp.h>

#include <ckcore/types.hh>
using namespace ckcore;

// Manifest.
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Button image list support.
#if (_WIN32_WINNT < 0x501)
#define BUTTON_IMAGELIST_ALIGN_CENTER   4

typedef struct
{
    HIMAGELIST  himl;   // Index: Normal, hot pushed, disabled. If count is less than 4, we use index 1
    RECT        margin; // Margin around icon.
    UINT        uAlign;
} BUTTON_IMAGELIST, *PBUTTON_IMAGELIST;

#define BCM_FIRST               0x1600
#define BCM_SETIMAGELIST        (BCM_FIRST + 0x0002)
#endif
