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

// Write modes and flags.
#define CDRTOOLS_WRITEMODES_TAO				"TAO "			// OBSOLETE.
#define CDRTOOLS_WRITEMODES_PACKET			"PACKET "		// OBSOLETE.
#define CDRTOOLS_WRITEMODES_SAO				"SAO "			// OBSOLETE.
#define CDRTOOLS_WRITEMODES_SAOR96P			"SAO/R96P "		// OBSOLETE.
#define CDRTOOLS_WRITEMODES_SAOR96R			"SAO/R96R "		// OBSOLETE.
#define CDRTOOLS_WRITEMODES_RAW16			"RAW/R16 "		// OBSOLETE.
#define CDRTOOLS_WRITEMODES_RAW96P			"RAW/R96P "		// OBSOLETE.
#define CDRTOOLS_WRITEMODES_RAW96R			"RAW/R96R "		// OBSOLETE.
#define CDRTOOLS_WRITEFLAGS_MMC3			"MMC-3"
#define CDRTOOLS_WRITEFLAGS_SWABAUDIO		"SWABAUDIO"
#define CDRTOOLS_WRITEFLAGS_BURNFREE		"BURNFREE"
#define CDRTOOLS_WRITEFLAGS_AUDIOMASTER		"AUDIOMASTER"
#define CDRTOOLS_WRITEFLAGS_FORCESPEED		"FORCESPEED"
#define CDRTOOLS_WRITEFLAGS_VARIREC			"VARIREC"

// All the following strings are prefixed by strings like: "Does " and "Does not"
// they are also prefixed in deeper levels as specified.

// Prefixed by: "read " or "write "
#define CDRTOOLS_CAP_CDR					"CD-R "
#define CDRTOOLS_CAP_CDR_LENGTH				5
#define CDRTOOLS_CAP_CDRW					"CD-RW "
#define CDRTOOLS_CAP_CDRW_LENGTH			6
#define CDRTOOLS_CAP_DVDROM					"DVD-ROM "
#define CDRTOOLS_CAP_DVDROM_LENGTH			8
#define CDRTOOLS_CAP_DVDR					"DVD-R "
#define CDRTOOLS_CAP_DVDR_LENGTH			6
#define CDRTOOLS_CAP_DVDRAM					"DVD-RAM "
#define CDRTOOLS_CAP_DVDRAM_LENGTH			8
// Prefixed by: "read "
#define CDRTOOLS_CAP_MODE2FORM1				"Mode 2 Form 1 "
#define CDRTOOLS_CAP_MODE2FORM1_LENGTH		14
#define CDRTOOLS_CAP_MODE2FORM2				"Mode 2 Form 2 "
#define CDRTOOLS_CAP_MODE2FORM2_LENGTH		14
#define CDRTOOLS_CAP_DIGITALAUDIO			"digital "
#define CDRTOOLS_CAP_DIGITALAUDIO_LENGTH	8
#define CDRTOOLS_CAP_MULTISESSION			"multi-"
#define CDRTOOLS_CAP_MULTISESSION_LENGTH	6
#define CDRTOOLS_CAP_FIXEDPACKET			"fixed-"
#define CDRTOOLS_CAP_FIXEDPACKET_LENGTH		6
#define CDRTOOLS_CAP_CDBARCODE				"CD bar "
#define CDRTOOLS_CAP_CDBARCODE_LENGTH		7
#define CDRTOOLS_CAP_RWSUBCODE				"R-W "		// Also prefixed by "return "
#define CDRTOOLS_CAP_RWSUBCODE_LENGTH		4
#define CDRTOOLS_CAP_RAWPWSUBCODE			"raw P-W "
#define CDRTOOLS_CAP_RAWPWSUBCODE_LENGTH	8
// Prefixed by: "support "
#define CDRTOOLS_CAP_TESTWRITING			"test writing"
#define CDRTOOLS_CAP_TESTWRITING_LENGTH		12
#define CDRTOOLS_CAP_BUFRECORDING			"Buffer-Underrun-Free "
#define CDRTOOLS_CAP_BUFRECORDING_LENGTH	21
#define CDRTOOLS_CAP_C2EP					"C2 error "
#define CDRTOOLS_CAP_C2EP_LENGTH			9
#define CDRTOOLS_CAP_INDIVIDUALVC			"individual vol"
#define CDRTOOLS_CAP_INDIVIDUALVC_LENGTH	14
#define CDRTOOLS_CAP_INDEPENDENTMUTE		"independent mute "
#define CDRTOOLS_CAP_INDEPENDENTMUTE_LENGTH	17
#define CDRTOOLS_CAP_DOPORT1				"digital output on port 1"
#define CDRTOOLS_CAP_DOPORT1_LENGTH			24
#define CDRTOOLS_CAP_DOPORT2				"digital output on port 2"
#define CDRTOOLS_CAP_DOPORT2_LENGTH			24
#define CDRTOOLS_CAP_EJECTCDSS				"ejection "
#define CDRTOOLS_CAP_EJECTCDSS_LENGTH		9
#define CDRTOOLS_CAP_CHANGEDISCSIDE			"changing side "
#define CDRTOOLS_CAP_CHANGEDISCSIDE_LENGTH	14
#define CDRTOOLS_CAP_INDIVIDUALDP			"Individual Disk "
#define CDRTOOLS_CAP_INDIVIDUALDP_LENGTH	16
// Prefixed by: "return "
#define CDRTOOLS_CAP_CDCATALOGNUMBER		"CD media "
#define CDRTOOLS_CAP_CDCATALOGNUMBER_LENGTH	9
#define CDRTOOLS_CAP_CDISRCINFO				"CD ISRC "
#define CDRTOOLS_CAP_CDISRCINFO_LENGTH		8
// Prefixed by: "have "
#define CDRTOOLS_CAP_VALIDDATA				"valid data "
#define CDRTOOLS_CAP_VALIDDATA_LENGTH		11
#define CDRTOOLS_CAP_LESIC					"load-"
#define CDRTOOLS_CAP_LESIC_LENGTH			5
// Prefixed by: "Number of "
#define CDRTOOLS_CAP_NUMVCL					"volume "
#define CDRTOOLS_CAP_NUMVCL_LENGTH			7
#define CDRTOOLS_CAP_NUMWRITESPEEDS			"supp"
#define CDRTOOLS_CAP_NUMWRITESPEEDS_LENGTH	4
// Not prefixed.
#define CDRTOOLS_CAP_NSDARA					"restart "
#define CDRTOOLS_CAP_NSDARA_LENGTH			8
#define CDRTOOLS_CAP_COMPOSITEAVDATA		"deliver "
#define CDRTOOLS_CAP_COMPOSITEAVDATA_LENGTH	8
#define CDRTOOLS_CAP_AUDIOCD				"play "
#define CDRTOOLS_CAP_AUDIOCD_LENGTH			5
#define CDRTOOLS_CAP_SENDDIGDAT				"send "
#define CDRTOOLS_CAP_SENDDIGDAT_LENGTH		5
#define CDRTOOLS_CAP_SETLRCK				"set "
#define CDRTOOLS_CAP_SETLRCK_LENGTH			4
#define CDRTOOLS_CAP_LMOPU					"lock "
#define CDRTOOLS_CAP_LMOPU_LENGTH			5
#define CDRTOOLS_CAP_ALLOWML				"allow "
#define CDRTOOLS_CAP_ALLOWML_LENGTH			6
