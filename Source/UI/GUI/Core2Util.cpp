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
#include <map>
#include "SCSI.h"
#include "Core2Util.h"

/**
	Translates the specified speed in kB/s to speeds on the form of: 1x, 2x, and
	so on using the specified profile.
	@param usProfile the profile to use for generating the display speed.
	@param ulSpeed the speed that should be translated (given in kB/s).
	@return the translated speed.
*/
float GetDispSpeed(unsigned short usProfile,unsigned long ulSpeed)
{
	switch (usProfile)
	{
		case PROFILE_NONE:
			return 0.0f;

		case PROFILE_NONREMOVABLE:
		case PROFILE_REMOVABLE:
		case PROFILE_MOPTIC_E:
		case PROFILE_OPTIC_WO:
		case PROFILE_AS_MO:
		case PROFILE_CDROM:
		case PROFILE_CDR:
		case PROFILE_CDRW:
//			return (float)ulSpeed/176.0f;
			return (float)((int)(ulSpeed/176.0f));

		case PROFILE_DVDROM:
		case PROFILE_DVDMINUSR_SEQ:
		case PROFILE_DVDRAM:
		case PROFILE_DVDMINUSRW_RESTOV:
		case PROFILE_DVDMINUSRW_SEQ:
		case PROFILE_DVDMINUSR_DL_SEQ:
		case PROFILE_DVDMINUSR_DL_JUMP:
		case PROFILE_DVDPLUSRW:
		case PROFILE_DVDPLUSR:
		case PROFILE_DVDPLUSRW_DL:
		case PROFILE_DVDPLUSR_DL:
			return (float)ulSpeed/1385.0f;

		case PROFILE_BDROM:
		case PROFILE_BDR_SRM:
		case PROFILE_BDR_RRM:
		case PROFILE_BDRE:
		case PROFILE_HDDVDROM:
		case PROFILE_HDDVDR:
		case PROFILE_HDDVDRAM:
			return (float)ulSpeed/36000.0f;		// Needs to be verified.
	}

	return true;
}

/**
	Converts the specifed speed in blocks (sectors) per second to speeds on the
	form of: 1x, 2x, and so on using the specified profile.
	@param usProfile the profile to use for generating the display speed.
	@param ulSpeed the speed that should be translated (given in blocks/s).
	@return the translated speed.
*/
float GetDispSpeedSEC(unsigned short usProfile,unsigned long ulSpeed)
{
	switch (usProfile)
	{
		case PROFILE_NONE:
			return 0.0f;

		case PROFILE_NONREMOVABLE:
		case PROFILE_REMOVABLE:
		case PROFILE_MOPTIC_E:
		case PROFILE_OPTIC_WO:
		case PROFILE_AS_MO:
		case PROFILE_CDROM:
		case PROFILE_CDR:
		case PROFILE_CDRW:
			return (float)ulSpeed/75.0f;

		case PROFILE_DVDROM:
		case PROFILE_DVDMINUSR_SEQ:
		case PROFILE_DVDRAM:
		case PROFILE_DVDMINUSRW_RESTOV:
		case PROFILE_DVDMINUSRW_SEQ:
		case PROFILE_DVDMINUSR_DL_SEQ:
		case PROFILE_DVDMINUSR_DL_JUMP:
		case PROFILE_DVDPLUSRW:
		case PROFILE_DVDPLUSR:
		case PROFILE_DVDPLUSRW_DL:
		case PROFILE_DVDPLUSR_DL:
			return (float)ulSpeed/675.0f;

		case PROFILE_BDROM:
		case PROFILE_BDR_SRM:
		case PROFILE_BDR_RRM:
		case PROFILE_BDRE:
		case PROFILE_HDDVDROM:
		case PROFILE_HDDVDR:
		case PROFILE_HDDVDRAM:
			return (float)ulSpeed/18432.0f;		// Needs to be verified.
	}

	return true;
}

/**
	Generates a vector of selected display speeds including and below the
	specified maximum speed.
	@param ulMaxDispSpeed the maximum display speed (1x, 2x, ...).
	@param Speeds the result vector that be updated with the display speeds.
	@return true if successfull and false otherwise.
*/
bool GetDispSpeeds(unsigned long ulMaxDispSpeed,std::vector<unsigned int> &Speeds)
{
	int iCurSpeed = ulMaxDispSpeed;
	while (iCurSpeed >= 1)
	{
		Speeds.push_back(iCurSpeed);

		if (iCurSpeed == 2)
			iCurSpeed = 1;
		else if (iCurSpeed <= 12)
			iCurSpeed -= 2;
		else if (iCurSpeed <= 24)
			iCurSpeed -= 4;
		else
			iCurSpeed -= 8;
	}

	return true;
}

/**
	Generates a vector of speeds in the same fasion as GetDispSpeeds except for
	that the speeds are in default kB/s format.
	@param usProfile the profile to use for generating the speeds.
	@param ulMaxSpeed the maximum speed in kB/s.
	@param Speeds the result vector that be updated with the speeds.
	@return true if successfull and false otherwise.
*/
bool GetSpeeds(unsigned short usProfile,unsigned long ulMaxSpeed,
			   std::vector<unsigned int> &Speeds)
{
	float fDispSpeed = GetDispSpeed(usProfile,ulMaxSpeed);
	float fFactor = (float)ulMaxSpeed/fDispSpeed;

	GetDispSpeeds((unsigned long)fDispSpeed,Speeds);

	std::vector<unsigned int>::iterator itSpeed;
	for (itSpeed = Speeds.begin(); itSpeed != Speeds.end(); itSpeed++)
		*itSpeed = (unsigned int)((*itSpeed) * fFactor);

	return true;
}

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
