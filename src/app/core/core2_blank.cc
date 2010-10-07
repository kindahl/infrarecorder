/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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

#include "stdafx.hh"
#include "core2_blank.hh"
#include "core2.hh"
#include "log_dlg.hh"
#include "string_table.hh"
#include "lang_util.hh"

CCore2Blank::CCore2Blank()
{
}

CCore2Blank::~CCore2Blank()
{
}

bool CCore2Blank::Blank(ckmmc::Device &Device,CAdvancedProgress *pProgress,
						int iMethod,bool bForce,bool bSimulate)
{
	g_pLogDlg->print_line(_T("CCore2Blank::Blank"));

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
			g_pLogDlg->print_line(_T("  Warning: Unknown erase method, using full erase."));
			ucBlankType = 0x00;
			break;
	}

	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Handle any unhandled events.
	unsigned char ucEvents;
	if (g_Core2.HandleEvents(Device,pProgress,ucEvents))
	{
		g_pLogDlg->print_line(_T("  Handled events: 0x%.2X"),ucEvents);
	}
	else
	{
		g_pLogDlg->print_line(_T("  Error: Failed to handle events, handled events: 0x%.2X"),ucEvents);
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
	Device.timeout(60 * 120);

	if (!Device.transport(ucCdb,12,NULL,0,ckmmc::Device::ckTM_READ))
	{
		Device.timeout(60);

		pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_ERASE));
		return false;
	}

	Device.timeout(60);

	if (!g_Core2.WaitForUnit(Device,pProgress))
		return false;

	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_ERASE));
	return true;
}