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

inline ATL::CWindow GetParentWindow(const ATL::CWindow *pWnd)
{
    // Version 3.0 of the ATL library does not have the CWindow::GetParent()
    // overload that returns a CWindow object. Unfortunately, ATL 3.0 is the
    // latest freely available version of the ATL library, it comes with the
    // Microsoft Windows Server 2003 R2 Platform SDK. Supporting ATL 3.0 means
    // you can compile InfraRecorder with Visual Studio 2005 Express, which is
    // also free. WTL 8.0 also supports ATL 3.0 / Visual Studio 2005 Express,
    // see the bundled readme file.
    //
    // The missing GetParent() overload is actually very handy, and this routine
    // is a good workaround that works on both ATL 3.0 and the newer versions.
    //
    // An alternative to this routine would be to manually add the missing
    // overload to the ATL 3.0 headers. This is however cumbersome for the casual
    // developer who just wants to compile the project once. Modifying the
    // standard headers may also cause compilation trouble in other projects or
    // with other versions of Visual Studio.

#if _ATL_VER <= 0x0300
    return ATL::CWindow(pWnd->GetParent());
#else
    // Use the one that comes with ATL.
    return pWnd->GetParent();
#endif
}

#if _ATL_VER <= 0x0300
    #pragma comment(lib,"shell32")
    #pragma comment(lib,"gdi32")
    #pragma comment(lib,"comdlg32")
#endif
