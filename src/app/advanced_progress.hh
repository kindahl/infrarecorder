/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include <ckcore/progress.hh>

#define PROGRESS_STRINGBUFFER_SIZE		256

class CAdvancedProgress : public ckcore::Progress
{
protected:
	// Should be used by inheritors when parsing the variable argument list passed
	// to the AddLogEntry and SetStatus functions.
	TCHAR m_szStringBuffer[PROGRESS_STRINGBUFFER_SIZE];

public:
	// Called when the operation is complteted.
	virtual void NotifyCompleted() = 0;

	// Should be set to true when a real writing process is started.
	virtual void SetRealMode(bool bRealMode) = 0;

	// Not forced to be implemented by inheritor.
	virtual void SetBuffer(int iPercent);

	virtual void AllowReload() = 0;
	virtual void AllowCancel(bool bAllow) = 0;

	virtual bool RequestNextDisc() = 0;

	// Starts the smoke effect.
	virtual void StartSmoke() = 0;
};
