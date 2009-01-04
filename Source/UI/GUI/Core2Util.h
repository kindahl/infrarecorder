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

#pragma once
#include <vector>

#define SENSE_FORMATINPROGRESS			0x01
#define SENSE_LONGWRITEINPROGRESS		0x02
#define SENSE_ILLEGALMODEFORTHISTRACK	0x03
#define SENSE_INVALIDPACKETSIZE			0x04

float GetDispSpeed(unsigned short usProfile,unsigned long ulSpeed);
float GetDispSpeedSEC(unsigned short usProfile,unsigned long ulSpeed);
bool GetDispSpeeds(unsigned long ulMaxDispSpeed,std::vector<unsigned int> &Speeds);
bool GetSpeeds(unsigned short usProfile,unsigned long ulMaxSpeed,
			   std::vector<unsigned int> &Speeds);

unsigned char CheckSense(unsigned char *pSenseBuf);
