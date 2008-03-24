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
#include "Crc16.h"

namespace ckFileSystem
{
	CCrc16::CCrc16(unsigned long ulPolynomial)
	{
		// Calculate the table entries.
		unsigned long ulCrc;

		for (int i = 0; i < 256; i++)
		{
			ulCrc = i << 8;

			for (int j = 0; j < 8; j++)
			{
				if (ulCrc & 0x8000)
					ulCrc = (ulCrc << 1) ^ ulPolynomial;
				else
					ulCrc = (ulCrc << 1);
			}

			m_usTable[i] = (unsigned short)ulCrc;
		}
	}

	unsigned short CCrc16::CalcCrc(unsigned char *pBuffer,size_t iBufferLen)
	{
		unsigned long ulCrc = 0;
		for (unsigned int i = 0; i < iBufferLen; i++)
			ulCrc = (ulCrc << 8) ^ m_usTable[((ulCrc >> 8) ^ pBuffer[i]) & 0xFF];

		return (unsigned short)(ulCrc & 0xFFFF);
	}
};
