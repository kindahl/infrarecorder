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

class CProgress
{
public:
	// Not forced to be implemented by inheritor.
	virtual void SetProgress(int iPercent);

	// Returns true of the operation has been cancelled.
	virtual bool IsCanceled() = 0;
};

class CFilesProgress
{
private:
	unsigned __int64 m_uiTotalBytes;
public:
	unsigned __int64 m_uiProcessedBytes;

public:
	CFilesProgress(unsigned __int64 uiTotalBytes);

	int UpdateProcessed(unsigned long ulProcessedBytes);
};
