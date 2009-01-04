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

#pragma once

class CActionManager
{
private:
	class CEraseParam
	{
	public:
		bool m_bNotifyCompleted;

		CEraseParam(bool bNotifyCompleted) : m_bNotifyCompleted(bNotifyCompleted) {}
	};

	static DWORD WINAPI BurnCompilationThread(LPVOID lpThreadParameter);
	static DWORD WINAPI CreateImageThread(LPVOID lpThreadParameter);
	static DWORD WINAPI CopyDiscOnFlyThread(LPVOID lpThreadParameter);
	static DWORD WINAPI CopyDiscThread(LPVOID lpThreadParameter);
	static DWORD WINAPI EraseThread(LPVOID lpThreadParameter);

	void QuickErase(INT_PTR iRecorder);
	bool QuickEraseQuery(INT_PTR iRecorder,HWND hWndParent);

public:
	CActionManager();
	~CActionManager();

	INT_PTR BurnCompilation(HWND hWndParent,bool bAppMode);
	INT_PTR CreateImage(HWND hWndParent,bool bAppMode);
	INT_PTR BurnImage(HWND hWndParent,bool bAppMode);
	INT_PTR BurnImageEx(HWND hWndParent,bool bAppMode,const TCHAR *szFilePath);
	INT_PTR CopyDisc(HWND hWndParent,bool bAppMode);
	INT_PTR CopyImage(HWND hWndParent,bool bAppMode);
	INT_PTR ManageTracks(bool bAppMode);
	INT_PTR Erase(HWND hWndParent,bool bAppMode);
	INT_PTR Fixate(HWND hWndParent,bool bAppMode);

	// For testing purposes only.
	//int VerifyCompilation(HWND hWndParent);
};

extern CActionManager g_ActionManager;
