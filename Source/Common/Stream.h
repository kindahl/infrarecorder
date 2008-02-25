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
#define STREAM_FAIL				0x00
#define STREAM_OK				0x01

class CInStream
{
public:
	virtual int Read(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize) = 0;
	virtual bool EOS() = 0;
};

class COutStream
{
public:
	virtual int Write(void *pBuffer,unsigned long ulSize,unsigned long *pProcessedSize) = 0;
};
