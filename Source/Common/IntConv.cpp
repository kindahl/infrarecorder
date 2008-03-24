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
#include "stdafx.h"
#include "IntConv.h"

unsigned long BeToLe32(unsigned long ulBigEndian)
{
	unsigned char *pTemp = (unsigned char *)&ulBigEndian;

	return ((unsigned long)pTemp[0] << 24) | ((unsigned long)pTemp[1] << 16) |
			((unsigned long)pTemp[2] << 8) | pTemp[3];
}

unsigned short BeToLe16(unsigned short usBigEndian)
{
	unsigned char *pTemp = (unsigned char *)&usBigEndian;

	return ((unsigned short)pTemp[0] << 8) | pTemp[1];
}
