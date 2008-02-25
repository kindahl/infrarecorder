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

#define CONSOLEPIPE_INTERNALBUFFER_SIZE		256
#define CONSOLEPIPE_WAITTIME				100
#define CONSOLEPIPE_MAX_LINE_SIZE			1024

class CConsolePipe
{
private:
	HANDLE m_hProcess;
	HANDLE m_hStopEvent;
	HANDLE m_hThread;
	unsigned long m_ulThreadID;

	// Streams.
	HANDLE m_hStdIn;
	HANDLE m_hStdOut;

	// Line buffer.
	char m_szLineBuffer[CONSOLEPIPE_MAX_LINE_SIZE];
	int m_iLineBufferIndex;

	bool CreateProcess(TCHAR *szCommandLine,HANDLE hStdIn,HANDLE hStdOut,HANDLE hStdErr);
	bool CleanUp();
	int ReadOutput();

	static DWORD WINAPI ListenThread(LPVOID lpvThreadParam);

protected:
	virtual void FlushOutput(const char *szBuffer) = 0;
	virtual void ProcessEnded() = 0;

	char m_cLineDelimiter;

public:
	CConsolePipe();
	~CConsolePipe();

	bool Launch(TCHAR *szCommandLine,bool bWaitForProcess = false);
	bool Kill();

	unsigned long WriteInput(const char *szInput,unsigned int uiLength);

	void CloseStdIn()
	{
		::CloseHandle(m_hStdIn);
		m_hStdIn = NULL;
	}
};
