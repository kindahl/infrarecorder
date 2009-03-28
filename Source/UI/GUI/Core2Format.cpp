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
#include "Core2Format.h"
#include "Core2.h"
#include "Core2Util.h"
#include "LogDlg.h"
#include "StringTable.h"
#include "LangUtil.h"

CCore2Format::CCore2Format()
{
}

CCore2Format::~CCore2Format()
{
}

bool CCore2Format::WaitBkgndFormat(CCore2Device *pDevice,CAdvancedProgress *pProgress)
{
	// Initialize buffers.
	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	unsigned char ucSense[24];
	memset(ucSense,0,sizeof(ucSense));

	// Prepare command.
	ucCdb[0] = SCSI_REQUEST_SENSE;
	ucCdb[4] = sizeof(ucSense);
	ucCdb[5] = 0x00;

	unsigned long ulPrevTime = GetTickCount();

	while (true)
	{
		// See if one second have passed.
		if (GetTickCount() > (ulPrevTime + 1000))
		{
			if (pProgress->cancelled())
			{
				// It's safe to abort the format process at this stage. We just need to
				// close the current track.
				g_Core2.CloseTrackSession(pDevice,0,0,true);
				return false;
			}

			unsigned char ucResult;
			if (!pDevice->TransportWithSense(ucCdb,6,NULL,0,ucSense,ucResult))
				return false;

			// Check if we're done.
			if (ucResult == SCSISTAT_GOOD)
				return true;

			// See if we're formating a disc (in background mode).
			if (ucSense[12] == 0x04 && ucSense[13] == 0x04)
			{
				// Update the progress.
				if (pProgress != NULL)
				{
					// If the SKSV bit is set to zero we are done.
					if (ucSense[15] & 0x80)
					{
						unsigned short usProgress = ((unsigned short)ucSense[16] << 8) | ucSense[17];
						pProgress->set_progress((unsigned char)(usProgress * 100.0f / 0xFFFF));
					}
					else
					{
						pProgress->set_progress(100);
						return true;
					}
				}
			}
			else
			{
				// If we receive any other sense we're done.
				return true;
			}

			ulPrevTime = GetTickCount();
		}
	}

	return true;
}

bool CCore2Format::FormatUnit(CCore2Device *pDevice,CAdvancedProgress *pProgress,bool bFull)
{
	g_pLogDlg->print_line(_T("CCore2Format::FormatUnit"));

	// Initialize buffers.
	unsigned char ucBuffer[192];
	memset(ucBuffer,0,sizeof(ucBuffer));

	unsigned char ucCdb[16];
	memset(ucCdb,0,sizeof(ucCdb));

	// Perform a device inquiry.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_INQUIRY;
	ucCdb[4] = 192;
	ucCdb[5] = 0;

	if (!pDevice->Transport(ucCdb,6,ucBuffer,192))
		return false;

	// Make sure the device type is a CDROM.
	if ((ucBuffer[0] & 0x1F) != 0x05)
		return false;

	// Get the device configuration, to see what media that's mounted.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_GET_CONFIGURATION;
	ucCdb[8] = 0x08;

	if (!pDevice->Transport(ucCdb,9,ucBuffer,192))
		return false;

	unsigned short usProfile = ucBuffer[6] << 8 | ucBuffer[7];
	g_pLogDlg->print_line(_T("  Current profile: 0x%.4X."),usProfile);

	if (usProfile != PROFILE_DVDPLUSRW && usProfile != PROFILE_DVDPLUSRW_DL &&
		usProfile != PROFILE_DVDRAM && usProfile != PROFILE_DVDMINUSRW_RESTOV &&
		usProfile != PROFILE_DVDMINUSRW_SEQ)
	{
		g_pLogDlg->print_line(_T("  Error: Unsupported media."));
		return false;
	}

	// Read the format capacities length.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_READ_FORMAT_CAPACITIES;
	ucCdb[8] = 0x04;
	ucCdb[9] = 0x00;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,192))
		return false;

	unsigned char ucCapListLen = ucBuffer[3];
	g_pLogDlg->print_line(_T("  Capacity list length: %d bytes."),ucCapListLen);

	if (ucCapListLen % 8 != 0 || ucCapListLen == 0)
	{
		g_pLogDlg->print_line(_T("  Error: Invalid capacity list length."));
		return false;
	}

	// Read the actual format capacities data.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_READ_FORMAT_CAPACITIES;
	ucCdb[7] = (ucCapListLen + 0x04) >> 8;
	ucCdb[8] = (ucCapListLen + 0x04) & 0xFF;
	ucCdb[9] = 0x00;

	if (!pDevice->Transport(ucCdb,10,ucBuffer,ucCapListLen + 0x04))
		return false;

	// Locate the appropriate formattable capacity descriptor.
	unsigned int uiFmtDescOffset = 0;
	switch (usProfile)
	{
		case PROFILE_DVDPLUSRW:
		case PROFILE_DVDPLUSRW_DL:
			for (uiFmtDescOffset = 8; uiFmtDescOffset < ucCapListLen; uiFmtDescOffset += 8)
			{
				if (ucBuffer[uiFmtDescOffset + 8] >> 2 == 0x26)
					break;
			}
			break;

		case PROFILE_DVDRAM:
			for (uiFmtDescOffset = 8; uiFmtDescOffset < ucCapListLen; uiFmtDescOffset += 8)
			{
				if (ucBuffer[uiFmtDescOffset + 8] >> 2 == 0x01)
					break;
			}
			break;

		case PROFILE_DVDMINUSRW_RESTOV:
		case PROFILE_DVDMINUSRW_SEQ:
			for (uiFmtDescOffset = 8; uiFmtDescOffset < ucCapListLen; uiFmtDescOffset += 8)
			{
				if (ucBuffer[uiFmtDescOffset + 8] >> 2 == (bFull ? 0x10 : 0x15))
					break;
			}
			break;
	}

	if ((ucBuffer[8] & 0x03) == 0x03)		// No media present or unknown capacity.
	{
		g_pLogDlg->print_line(_T("  Error: Unable to determine media capacity."));
		return false;
	}
	else
	{
		unsigned int uiCapacity = ucBuffer[4] << 24 | ucBuffer[5] << 16 |
			ucBuffer[6] << 8 | ucBuffer[7];

		g_pLogDlg->print_line(_T("  Disc capacity: %.2f GiB (%I64d bytes)."),
			((double)uiCapacity * 2048)/1073741824,(__int64)uiCapacity * 2048);

		if ((ucBuffer[8] & 0x03) == 0x01)	// Unformatted or blank media.
			g_pLogDlg->print_line(_T("  The disc media is unformatted or blank."));
		else if ((ucBuffer[8] & 0x03) == 0x02)	// Formatted media.
			g_pLogDlg->print_line(_T("  The disc media is formatted."));
	}

	// Handle any unhandled events.
	unsigned char ucEvents;
	if (g_Core2.HandleEvents(pDevice,pProgress,ucEvents))
	{
		g_pLogDlg->print_line(_T("  Handled events: 0x%.2X"),ucEvents);
	}
	else
	{
		return false;
	}

	// Start the smoke.
	pProgress->StartSmoke();

	// Format the disc.
	memset(ucCdb,0,16);
	ucCdb[0] = SCSI_FORMAT_UNIT;
	ucCdb[1] = 0x11;
	ucCdb[5] = 0x00;

	// Format list header.
	ucBuffer[uiFmtDescOffset + 0] = 0x00;
	ucBuffer[uiFmtDescOffset + 1] = 0x02;	// Immed.
	ucBuffer[uiFmtDescOffset + 2] = 0x00;
	ucBuffer[uiFmtDescOffset + 3] = 0x08;

	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(PROGRESS_BEGINFORMAT),
		lngGetString(WRITEMODE_REAL));
	pProgress->set_status(lngGetString(STATUS_FORMAT));

	if (!pDevice->Transport(ucCdb,6,ucBuffer + uiFmtDescOffset,12,CCore2Device::DATAMODE_WRITE))
	{
		pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_FORMAT));
		return false;
	}

	if (!g_Core2.WaitForUnit(pDevice,pProgress))
		return false;

	// Stop the background format process (DVD+RW only).
	if (usProfile == PROFILE_DVDPLUSRW || usProfile == PROFILE_DVDPLUSRW_DL)
	{
		// Let the background format continue on DVD+RW discs if a full format was requested.
		if (bFull)
		{
			pProgress->AllowCancel(true);
			pProgress->set_status(lngGetString(STATUS_FORMATBKGND));

			if (!WaitBkgndFormat(pDevice,pProgress))
				return false;
		}

		// Stop the background format.
		pProgress->set_status(lngGetString(STATUS_CLOSETRACK));

		if (!g_Core2.CloseTrackSession(pDevice,0x00,0x00,true))
			pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_STOPBKGNDFORMAT));

		if (!g_Core2.WaitForUnit(pDevice,pProgress))
			return false;
	}

	pProgress->notify(ckcore::Progress::ckINFORMATION,lngGetString(SUCCESS_FORMAT));
	return true;
}