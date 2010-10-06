/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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

#define CDRTOOLS_COPYRIGHT					"Cdrecord-ProDVD-ProBD-Clone"
#define CDRTOOLS_COPYRIGHT_LENGTH			27
#define CDRTOOLS_SCSIBUS					"scsibus"
#define CDRTOOLS_SCSIBUS_LENGTH				7
#ifdef CDRKIT
#define CDRTOOLS_ERROR						"wodim: "
#define CDRTOOLS_ERROR_LENGTH				7
#define CDRTOOLS_ERROR3						"readom: "
#define CDRTOOLS_ERROR3_LENGTH				8
#define CDRTOOLS_ERROR4						"icedax: "
#define CDRTOOLS_ERROR4_LENGTH				8
#else
#define CDRTOOLS_ERROR						"cdrecord: "
#define CDRTOOLS_ERROR_LENGTH				10
#define CDRTOOLS_ERROR3						"readcd: "
#define CDRTOOLS_ERROR3_LENGTH				8
#define CDRTOOLS_ERROR4						"cdda2wav: "
#define CDRTOOLS_ERROR4_LENGTH				10
#endif
#define CDRTOOLS_REMOVABLE					"Removable "
#define CDRTOOLS_REMOVABLE_LENGTH			10
#define CDRTOOLS_TYPE_CDROM					"CD-ROM"
#define CDRTOOLS_TYPE_CDROM_LENGTH			6
#define CDRTOOLS_TYPE_COMMUNICATION			"Communication"
#define CDRTOOLS_TYPE_COMMUNICATION_LENGTH	13
#define CDRTOOLS_TYPE_DISC					"Disk"
#define CDRTOOLS_TYPE_DISC_LENGTH			4
#define CDRTOOLS_TYPE_JUKEBOX				"Juke Box"
#define CDRTOOLS_TYPE_JUKEBOX_LENGTH		8
#define CDRTOOLS_TYPE_OPTICALSTORAGE		"Optical Storage"
#define CDRTOOLS_TYPE_OPTICALSTORAGE_LENGTH	15
#define CDRTOOLS_TYPE_PRINTER				"Printer"
#define CDRTOOLS_TYPE_PRINTER_LENGTH		7
#define CDRTOOLS_TYPE_PROCESSOR				"Processor"
#define CDRTOOLS_TYPE_PROCESSOR_LENGTH		9
#define CDRTOOLS_TYPE_SCANNER				"Scanner"
#define CDRTOOLS_TYPE_SCANNER_LENGTH		7
#define CDRTOOLS_TYPE_TAPE					"Tape"
#define CDRTOOLS_TYPE_TAPE_LENGTH			4
#define CDRTOOLS_TYPE_WORM					"WORM"
#define CDRTOOLS_TYPE_WORM_LENGTH			4
#define CDRTOOLS_WRITEFLAGS					"Driver flags   :"
#define CDRTOOLS_WRITEFLAGS_LENGTH			16
#define CDRTOOLS_WRITEMODES					"Supported modes:"
#define CDRTOOLS_WRITEMODES_LENGTH			16
#define CDRTOOLS_GRACEBEGIN					"Last chance"
#define CDRTOOLS_GRACEBEGIN_LENGTH			11
#define CDRTOOLS_NOMEDIA					"Cannot load media."
#define CDRTOOLS_NOMEDIA_LENGTH				18
#define CDRTOOLS_CYGWINPATH					"/cygdrive/"
#define CDRTOOLS_CYGWINPATH_LENGTH			10
#define CDRTOOLS_NOSUPPORT					"This drive or"
#define CDRTOOLS_NOSUPPORT_LENGTH			13
#define CDRTOOLS_BLANK						"BLANK"
#define CDRTOOLS_BLANK_LENGTH				5
#define CDRTOOLS_STARTCDWRITE				"Starting to write CD/DVD"
#define CDRTOOLS_STARTCDWRITE_LENGTH		24
#define CDRTOOLS_BLANKTIME					"Blanking time:"
#define CDRTOOLS_BLANKTIME_LENGTH			14
#define CDRTOOLS_NODISC						"No disk "
#define CDRTOOLS_NODISC_LENGTH				8
#define CDRTOOLS_BLANKERROR					"Cannot blank"
#define CDRTOOLS_BLANKERROR_LENGTH			12
#define CDRTOOLS_BLANKUNSUP					"Some drives do not"
#define CDRTOOLS_BLANKUNSUP_LENGTH			18
#define CDRTOOLS_BLANKRETRY					"Try again"
#define CDRTOOLS_BLANKRETRY_LENGTH			9
#define CDRTOOLS_UNSUPPORTED				"Unsupported "
#define CDRTOOLS_UNSUPPORTED_LENGTH			12
#define CDRTOOLS_SECTOR						"sector"
#define CDRTOOLS_SECTOR_LENGTH				6
#define CDRTOOLS_VERSIONINFO				"This version"
#define CDRTOOLS_VERSIONINFO_LENGTH			12
#define CDRTOOLS_DVDINFO					"If you need"
#define CDRTOOLS_DVDINFO_LENGTH				11
#define CDRTOOLS_DVDGETINFO					"Free test"
#define CDRTOOLS_DVDGETINFO_LENGTH			9
#define CDRTOOLS_STARTTRACK					"Starting new track"
#define CDRTOOLS_STARTTRACK_LENGTH			18
#define CDRTOOLS_WRITEPREGAP				"Writing pregap"
#define CDRTOOLS_WRITEPREGAP_LENGTH			14
#define CDRTOOLS_FILLFIFO					"Waiting for reader"
#define CDRTOOLS_FILLFIFO_LENGTH			18
#define CDRTOOLS_WRITETIME					"Writing  time:"
#define CDRTOOLS_WRITETIME_LENGTH			14
#define CDRTOOLS_FIXATE						"Fixating..."
#define CDRTOOLS_FIXATE_LENGTH				11
#define CDRTOOLS_FIXATETIME					"Fixating time:"
#define CDRTOOLS_FIXATETIME_LENGTH			14
#define CDRTOOLS_WARNINGCAP					"WARNING:"
#define CDRTOOLS_WARNINGCAP_LENGTH			8
#define CDRTOOLS_WRITEERROR					"A write error"
#define CDRTOOLS_WRITEERROR_LENGTH			13
#define CDRTOOLS_FILENOTFOUND				"No such file or directory."
#define CDRTOOLS_FILENOTFOUND_LENGTH		26
#define CDRTOOLS_TOTALTTSIZE				"Total translation table"
#define CDRTOOLS_TOTALTTSIZE_LENGTH			23
#define CDRTOOLS_BADAUDIOCODING				"Inappropriate audio"
#define CDRTOOLS_BADAUDIOCODING_LENGTH		19
#define CDRTOOLS_RELOADDRIVE				"Re-load"
#define CDRTOOLS_RELOADDRIVE_LENGTH			7
#define CDRTOOLS_TOTALTIME					"Time total: "
#define CDRTOOLS_TOTALTIME_LENGTH			12
#define CDRTOOLS_END						"end: "
#define CDRTOOLS_END_LENGTH					5
#define CDRTOOLS_ADDRESS					"addr: "
#define CDRTOOLS_ADDRESS_LENGTH				6
#define CDRTOOLS_IOERROR					"Input/Output error. "
#define CDRTOOLS_IOERROR_LENGTH				20
#define CDRTOOLS_SECTORERROR				"Error on sector"
#define CDRTOOLS_SECTORERROR_LENGTH			15
#define CDRTOOLS_RETRYSECTOR				"Retrying from sector"
#define CDRTOOLS_RETRYSECTOR_LENGTH			20
#define CDRTOOLS_C2ERRORS					"C2 errors "
#define CDRTOOLS_C2ERRORS_LENGTH			10
#define CDRTOOLS_PERCENTDONE				"percent_done:"
#define CDRTOOLS_PERCENTDONE_LENGTH			13
#define CDRTOOLS_DEPPDIR					"Directories too deep"
#define CDRTOOLS_DEEPDIR_LENGTH				20
#define CDRTOOLS_FOUNDDVDMEDIA				"Found DVD"
#define CDRTOOLS_FOUNDDVDMEDIA_LENGTH		9
#define CDRTOOLS_OPENSESSION				"Cannot open new sess"
#define CDRTOOLS_OPENSESSION_LENGTH			20
#define CDRTOOLS_DISCSPACEWARNING			"Data may not fit on cur"
#define CDRTOOLS_DISCSPACEWARNING_LENGTH	23
#define CDRTOOLS_TURNINGBFON				"Turning BURN"
#define CDRTOOLS_TURNINGBFON_LENGTH			12
#define CDRTOOLS_TOTALEXTENT				"Total extent"
#define CDRTOOLS_TOTALEXTENT_LENGTH			12
#define CDRTOOLS_SIZEOFBOOT					"Size of boot"
#define CDRTOOLS_SIZEOFBOOT_LENGTH			12
#define CDRTOOLS_ERRORLEADIN				"Could not write Lead-in."
#define CDRTOOLS_ERRORLEADIN_LENGTH			24
#define CDRTOOLS_ERRORINITDRIVE				"Cannot init drive"
#define CDRTOOLS_ERRORINITDRIVE_LENGTH		17
#define CDRTOOLS_DVDRWDUMMY					"DVD+RW has no -du"
#define CDRTOOLS_DVDRWDUMMY_LENGTH			17
#define CDRTOOLS_FIFO						"fifo"
#define CDRTOOLS_FIFO_LENGTH				4