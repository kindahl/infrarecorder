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

#include "stdafx.hh"
#include "scsi.hh"
#include "core2_util.hh"

/**
	Parses the specified sense buffer and returns a value representing the
	specific error.
	@param pSenseBuf a pointer to the sense buffer, it should be atleast 24
	bytes long
	@return the correcsponding internal error value.
*/
unsigned char CheckSense(unsigned char *pSenseBuf)
{
	// Sense key.
	switch (pSenseBuf[2] & 0x0F)
	{
		case 0x02:
			{
				// Additional sense code.
				switch (pSenseBuf[12])
				{
					case 0x04:
						{
							// Additional sense code qualifier.
							switch (pSenseBuf[13])
							{
								case 0x04:
									return SENSE_FORMATINPROGRESS;

								case 0x08:
									return SENSE_LONGWRITEINPROGRESS;
							}
						}
						break;
				}
			}
			break;

		case 0x05:
			{
				// Additional sense code.
				switch (pSenseBuf[12])
				{
					case 0x64:
						{
							switch (pSenseBuf[13])
							{
								case 0x00:
									return SENSE_ILLEGALMODEFORTHISTRACK;

								case 0x01:
									return SENSE_INVALIDPACKETSIZE;
							}
						}
						break;
				}
			}
			break;
	}

	return 0;
}
