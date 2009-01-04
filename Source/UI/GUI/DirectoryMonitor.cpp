/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

#include "stdafx.h"
#include "DirectoryMonitor.h"

CDirectoryMonitor::CDirectoryMonitor()
{
}

CDirectoryMonitor::~CDirectoryMonitor()
{
}

/*
	CDirectoryMonitor::Register
	---------------------------
	Register the change notify entry using SHChangeNotifyEntry. The hWndNotify
	handle should be the window to receive the messages.  uiMsg the notification
	number. It returns true of no errors occured, false otherwise.
*/
bool CDirectoryMonitor::Register(HWND hWndNotify,unsigned int uiMsg,int iFilter,
	LPITEMIDLIST pidl,bool bRecursive)
{
	SHChangeNotifyEntry shNotifyEntry = { pidl,bRecursive };

	/*
	Where are these defined?
	SHCNRF_InterruptLevel = 0x0001
	SHCNRF_ShellLevel = 0x0002
	SHCNRF_RecursiveInterrupt = 0x1000
	SHCNRF_NewDelivery = 0x8000
	*/

	int iFlags = 0x0003; // SHCNRF_InterruptLevel & SHCNRF_ShellLevel

	if (bRecursive)
		iFlags |= 0x1000;

	m_ulNotifyID = SHChangeNotifyRegister(hWndNotify,iFlags,iFilter,uiMsg,1,&shNotifyEntry);

	if (!m_ulNotifyID)
		return false;

	return true;
}

bool CDirectoryMonitor::Deregister()
{
	if (!m_ulNotifyID)
		return false;

	if (SUCCEEDED(SHChangeNotifyDeregister(m_ulNotifyID)))
		return true;

	return false;
}
