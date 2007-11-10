/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "../../Common/Progress.h"

#define LOGTYPE_INFORMATION				0
#define LOGTYPE_WARNING					1
#define LOGTYPE_ERROR					2
#define LOGTYPE_WINLOGO					3

#define PROGRESS_STRINGBUFFER_SIZE		256

class CAdvancedProgress : public CProgress
{
protected:
	// Should be used by inheritors when parsing the variable argument list passed
	// to the AddLogEntry and SetStatus functions.
	TCHAR m_szStringBuffer[PROGRESS_STRINGBUFFER_SIZE];

public:
	virtual void AddLogEntry(int iType,const TCHAR *szMessage,...) = 0;
	virtual void SetStatus(const TCHAR *szStatus,...) = 0;

	// Called when the operation is complteted.
	virtual void NotifyComplteted() = 0;

	// Should be set to true when a real writing process is started.
	virtual void SetRealMode(bool bRealMode) = 0;

	// Not forced to be implemented by inheritor.
	//virtual void SetProgress(int iPercent);
	virtual void SetBuffer(int iPercent);

	virtual void AllowReload() = 0;
	virtual void AllowCancel(bool bAllow) = 0;

	// Starts the smoke effect.
	virtual void StartSmoke() = 0;
};
