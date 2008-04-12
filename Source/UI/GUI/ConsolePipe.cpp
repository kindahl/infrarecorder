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

#include "stdafx.h"
#include "ConsolePipe.h"
#include "Settings.h"
#include "LogDlg.h"

/*
	Documentation:
	--------------
	http://support.microsoft.com/kb/q190351/
*/

HANDLE g_hCloseHandle = NULL;

CConsolePipe::CConsolePipe()
{
	m_hProcess = NULL;
	m_hStopEvent = NULL;
	m_hThread = NULL;
	m_ulThreadID = 0;

	m_hStdIn = NULL;
	m_hStdOut = NULL;

	m_bRunning = false;
}

CConsolePipe::~CConsolePipe()
{
}

bool CConsolePipe::CreateProcess(TCHAR *szCommandLine,HANDLE hStdIn,
								 HANDLE hStdOut,HANDLE hStdErr)
{
	g_LogDlg.AddLine(_T("CConsolePipe::CreateProcess"));
	g_LogDlg.AddLine(_T("  Command line = %s."),szCommandLine);

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	memset(&si,0,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput = hStdIn;
	si.hStdOutput = hStdOut;
	si.hStdError = hStdErr;
	si.wShowWindow = SW_HIDE;

	if (!::CreateProcess(NULL,szCommandLine,NULL,NULL,true,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi))
	{
		g_LogDlg.AddLine(_T("  Error: CreateProcess failed, last error = %d."),
			(int)GetLastError());
		return false;
	}

	m_hProcess = pi.hProcess;

	// Close any unnecessary handles.
	::CloseHandle(pi.hThread);

	return true;
}

/*
	CConsolePipe::Launch
	--------------------
	Launches the actual console application, bWaitForProcess specifies whether
	the function should return before the process (or rather the listener
	thread) has completed or not.
*/
bool CConsolePipe::Launch(TCHAR *szCommandLine,bool bWaitForProcess)
{
	g_LogDlg.AddLine(_T("CConsolePipe::Launch"));

	HANDLE hOutputReadTemp,hOutputWrite;
	HANDLE hInputWriteTemp,hInputRead;
	HANDLE hErrorWrite;
	SECURITY_ATTRIBUTES sa;

	// Make sure that no process is currently running.
	CleanUp();

	hOutputReadTemp = hOutputWrite = hInputWriteTemp = hInputRead = hErrorWrite = NULL;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;

	// Create the child output pipe.
	if (!::CreatePipe(&hOutputReadTemp,&hOutputWrite,&sa,0))
	{
		g_LogDlg.AddLine(_T("  Error: CreatePipe failed, last error = %d."),(int)GetLastError());
		return false;
	}

	// Duplicate the output write handle since the application might close
	// one of its handles.
	if (!::DuplicateHandle(::GetCurrentProcess(),hOutputWrite,::GetCurrentProcess(),
		&hErrorWrite,0,true,DUPLICATE_SAME_ACCESS))
	{
		::CloseHandle(hOutputReadTemp);
		::CloseHandle(hOutputWrite);

		g_LogDlg.AddLine(_T("  Error: DuplicateHandle failed, last error = %d."),(int)GetLastError());
		return false;
	}

	// Create the child input pipe.
	if (!::CreatePipe(&hInputRead,&hInputWriteTemp,&sa,0))
	{
		::CloseHandle(hOutputReadTemp);
		::CloseHandle(hOutputWrite);
		::CloseHandle(hErrorWrite);

		g_LogDlg.AddLine(_T("  Error: CreatePipe failed, last error = %d."),(int)GetLastError());
		return false;
	}

	// Create new output read handle and the input write handles. The properties
	// must be set to false, otherwise we can't close the handles since they will
	// be inherited.
	if (!::DuplicateHandle(::GetCurrentProcess(),hOutputReadTemp,::GetCurrentProcess(),
		&m_hStdOut,0,false,DUPLICATE_SAME_ACCESS))
	{
		::CloseHandle(hOutputReadTemp);
		::CloseHandle(hOutputWrite);
		::CloseHandle(hErrorWrite);
		::CloseHandle(hInputWriteTemp);
		::CloseHandle(hInputRead);

		g_LogDlg.AddLine(_T("  Error: DuplicateHandle failed, last error = %d."),(int)GetLastError());
		return false;
	}

	if (!::DuplicateHandle(::GetCurrentProcess(),hInputWriteTemp,::GetCurrentProcess(),
		&m_hStdIn,0,false,DUPLICATE_SAME_ACCESS))
	{
		::CloseHandle(hOutputReadTemp);
		::CloseHandle(hOutputWrite);
		::CloseHandle(hErrorWrite);
		::CloseHandle(hInputWriteTemp);
		::CloseHandle(hInputRead);

		::CloseHandle(m_hStdOut);

		g_LogDlg.AddLine(_T("  Error: DuplicateHandle failed, last error = %d."),(int)GetLastError());
		return false;
	}

	// Now we can close the inherited pipes.
	::CloseHandle(hOutputReadTemp);
	::CloseHandle(hInputWriteTemp);

	bool bResult = CreateProcess(szCommandLine,hInputRead,hOutputWrite,hErrorWrite);

	::CloseHandle(hInputRead);
	::CloseHandle(hOutputWrite);
	::CloseHandle(hErrorWrite);

	if (!bResult)
	{
		::CloseHandle(m_hStdIn);
		::CloseHandle(m_hStdOut);
		m_hStdIn = NULL; // RSS:080105
		m_hStdOut = NULL;
		return false;
	}

	// Create the stop event that will kill the thread.
	m_hStopEvent = ::CreateEvent(NULL,true,false,NULL);

	// Create the listener thread.
	m_hThread = ::CreateThread(NULL,0,ListenThread,this,0,&m_ulThreadID);
	if (m_hThread == NULL)
	{
		CleanUp();

		g_LogDlg.AddLine(_T("  Error: CreateThread failed, last error = %d."),(int)GetLastError());
		return false;
	}

	// Make a copy of the handle that can be used when we want to terminate the process.
	::DuplicateHandle(::GetCurrentProcess(),m_hProcess,::GetCurrentProcess(),
		&g_hCloseHandle,0,false,DUPLICATE_SAME_ACCESS);

	m_bRunning = true;

	if (bWaitForProcess)
		WaitForSingleObject(m_hThread,INFINITE);

	return true;
}

bool CConsolePipe::CleanUp()
{
	if (m_hThread != NULL)
	{
		if (::GetCurrentThreadId() != m_ulThreadID)
		{
			// If the thread does not respond within 5s it will be forced
			// to terminate.
			::SetEvent(m_hStopEvent);

			if (::WaitForSingleObject(m_hThread,5000) == WAIT_TIMEOUT)
				::TerminateThread(m_hThread,-2);
		}

		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	if (m_hStopEvent != NULL)
	{
		::CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	if (m_hProcess != NULL)
	{
		::CloseHandle(m_hProcess);
		m_hProcess = NULL;
	}

	if (m_hStdIn != NULL)
	{
		::CloseHandle(m_hStdIn);
		m_hStdIn = NULL;
	}

	if (m_hStdOut != NULL)
	{
		::CloseHandle(m_hStdOut);
		m_hStdOut = NULL;
	}

	if (g_hCloseHandle != NULL)
	{
		::CloseHandle(g_hCloseHandle);
		g_hCloseHandle = NULL;
	}

	m_ulThreadID = 0;

	m_bRunning = false;

	return true;
}

/*
	CConsolePipe::Kill
	------------------
	Terminated the running process. After calling this function the caller should
	wait untill the process has completed.
*/
bool CConsolePipe::Kill()
{
	return TerminateProcess(g_hCloseHandle,0) == TRUE;
}

int CConsolePipe::ReadOutput()
{
	char szBuffer[CONSOLEPIPE_INTERNALBUFFER_SIZE];

	while (true)
	{
		unsigned long ulBytesAvail = 0;
		if (!::PeekNamedPipe(m_hStdOut,NULL,0,NULL,&ulBytesAvail,NULL))
			break;

		if (ulBytesAvail == 0)
			return 1;

		unsigned long ulRead = 0;
		if (!::ReadFile(m_hStdOut,szBuffer,min(ulBytesAvail,CONSOLEPIPE_INTERNALBUFFER_SIZE - 1),
			&ulRead,NULL) || ulRead == 0)
			break;

		szBuffer[ulRead] = '\0';

		// Split the buffer into lines.
		for (unsigned int i = 0; i < ulRead; i++)
		{
			// HACK: 2006-07-03: Added second delimiter \r.
			if (szBuffer[i] == m_cLineDelimiter || szBuffer[i] == '\r')
			{
				// Avoid flushing an empty buffer.
				if (m_iLineBufferIndex != 0)
				{
					m_szLineBuffer[m_iLineBufferIndex] = '\0';
					FlushOutput(m_szLineBuffer);

					m_iLineBufferIndex = 0;
				}
			}
			else
			{
				m_szLineBuffer[m_iLineBufferIndex++] = szBuffer[i];
			}
		}
	}

	unsigned long ulLastErr = ::GetLastError();
	if (ulLastErr == ERROR_BROKEN_PIPE || ulLastErr == ERROR_NO_DATA)
	{
		// No need to report here.
		return 0;
	}

	return -1;
}

DWORD WINAPI CConsolePipe::ListenThread(LPVOID lpThreadParameter)
{
	CConsolePipe *pPipe = (CConsolePipe *)lpThreadParameter;
	int iResult;

	HANDLE hHandles[2];
	hHandles[0] = pPipe->m_hProcess;
	hHandles[1] = pPipe->m_hStopEvent;

	pPipe->m_iLineBufferIndex = 0;
	pPipe->m_cLineDelimiter = '\n';		// By default we split lines by the new line character.

	while (true)
	{
		iResult = pPipe->ReadOutput();
		if (iResult <= 0)
			break;

		unsigned long ulWaitResult = ::WaitForMultipleObjects(2,hHandles,false,CONSOLEPIPE_WAITTIME);
		
		if (ulWaitResult == WAIT_OBJECT_0)
		{
			iResult = pPipe->ReadOutput();
			if (iResult > 0)
				iResult = 0;
			break;
		} else if (ulWaitResult == WAIT_OBJECT_0 + 1)		// Check if the stop event has been signaled.
		{
			iResult = 1;
			break;
		}
	}

	pPipe->ProcessEnded();
	pPipe->CleanUp();
	return iResult;
}

/*
	CConsolePipe::WriteInput
	------------------------
	Write data to the client application's standard input. The function returns
	the number of characters written.
*/
unsigned long CConsolePipe::WriteInput(const char *szInput,unsigned int uiLength)
{
	// Wait if the process has not been started.
	while (!m_bRunning)
		Sleep(100);

	unsigned long ulWritten = 0;
	::WriteFile(m_hStdIn,szInput,uiLength,&ulWritten,NULL);

	return ulWritten;
}
