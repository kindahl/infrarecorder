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

#include "stdafx.h"
#include "Core2Blank.h"
#include "Core2.h"
#include "LogDlg.h"
#include "StringTable.h"
#include "LangUtil.h"

CCore2Blank::CCore2Blank()
{
}

CCore2Blank::~CCore2Blank()
{
}

bool CCore2Blank::Blank(CCore2Device *pDevice,CAdvancedProgress *pProgress,
						int iMethod,bool bForce,bool bSimulate)
{
	g_LogDlg.print_line(_T("CCore2Blank::Blank"));

	unsigned char ucBlankType;
	switch (iMethod)
	{
		case CCore2::ERASE_BLANK_FULL:
			ucBlankType = 0x00;
			break;

		case CCore2::ERASE_BLANK_MINIMAL:
			ucBlankType = 0x01;
			break;

		case CCore2::ERASE_BLANK_UNCLOSE:
			ucBlankType = 0x05;
			break;

		case CCore2::ERASE_BLANK_SESSION:
			ucBlankType = 0x06;
			break;

		default:
			g_LogDlg.print_line(_T("  Warning: Unknown erase method, using full erase."));
			ucBlankType = 0x00;
			break;
	}

	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Handle any unhandled events.
	unsigned char ucEvents;
	if (g_Core2.HandleEvents(pDevice,pProgress,ucEvents))
	{
		g_LogDlg.print_line(_T("  Handled events: 0x%.2X"),ucEvents);
	}
	else
	{
		g_LogDlg.print_line(_T("  Error: Failed to handle events, handled events: 0x%.2X"),ucEvents);
		return false;
	}

	// Start the smoke.
	if (!bSimulate)
		pProgress->StartSmoke();

	// Execute the blank command.
	memset(ucCdb,0,16);
	ucCdb[ 0] = SCSI_BLANK;
	ucCdb[ 1] = 0x10 | ucBlankType;		// Immed and blank type.
	ucCdb[11] = 0x00;

	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINERASE),bSimulate ? 
		lngGetString(WRITEMODE_SIMULATION) : lngGetString(WRITEMODE_REAL));
	pProgress->set_status(lngGetString(STATUS_ERASE));

	// Worst case scenario if the immed flag has no effect (DVD-RW DL at 1x).
	pDevice->SetNextTimeOut(60 * 120);

	if (!pDevice->Transport(ucCdb,12,NULL,0))
	{
		pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_ERASE));
		return false;
	}

	if (!g_Core2.WaitForUnit(pDevice,pProgress))
		return false;

	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_ERASE));
	return true;
}
