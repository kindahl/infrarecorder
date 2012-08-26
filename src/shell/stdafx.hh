/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER					// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400			// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef UNICODE		// Target non-unicode releases to Win9x/ME.
#define _WIN32_WINNT	0x0400
#else
#define _WIN32_WINNT	0x0500
#endif				

#ifndef _WIN32_WINDOWS			// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410	// Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE				// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0500		// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// Some CString constructors will be explicit.

// Turns off ATL's hiding of some common and often safely ignored warning messages.
#define _ATL_ALL_WARNINGS

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <shlobj.h>

// Global DLL insance supplied by the DLL entrypoint.
extern HINSTANCE g_DllInstance;

using namespace ATL;
