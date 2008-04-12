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
#include "StringTable.h"

TCHAR *g_szStringTable[] = {
	_T("Error"),
	_T("Warning"),
	_T("Information"),
	_T("Question"),

	_T("Unable to write to the registry. Your user account might not have permissions to perform the requested operation."),
	_T("An error occured while trying to save the file. The file has not been successfully created."),
	_T("Unable to load the configuration file, it may be corrupt. The XML processor returned: %d."),
	_T("You can't add folders to an audio project."),
	_T("Unable to parse project file, it may be corrupt. The XML processor returned: %d."),
	_T("Unable to parse project file, it may be corrupt."),
	_T("The project has been created with a newer version of InfraRecorder. You need to update InfraRecorder to open this project."),
	_T("The active character set could not be automatically detected. The registry suggests the following body character set: %s."),
	_T("Please reload the drive and press the 'Reload' button."),
	_T("Error on sector %d not corrected."),
	_T("The internal command line is too long for your system to handle. The length limit on your system is %d characters. Please try to shorten the file paths in your project and try again."),

	// OBSOLETE.
	_T("InfraRecorder has noticed that autorun is enabled. Leaving autorun enabled causes Windows to poll the CD drive while recording, which might damage your CD. Do you want to turn off autorun (recommended)?"),
	_T("Are you sure that you want to abort the operation? Aborting might damage your CD permanently."),
	_T("Are you sure that you want to remove the selected item(s) from the project?"),
	_T("The project has been modified, do you want to save the changes?"),
	_T("Only the data track can be recorded to a disc image from mixed-mode projects. Do you want to continue?"),

	_T("Scanning SCSI/IDE bus..."),
	_T("Loading device capabilities..."),
	_T("Loading extended device information..."),
	_T("InfraRecorder has detected changes in your hardware configuration. Would you like to update your device configuration now?"),
	_T("Initializing device..."),
	_T("Loading information from track %d."),

	_T("InfraRecorder failed to scan your SCSI/IDE busses. Please make sure that your system is properly configured."),
	_T("InfraRecorder failed to load device capabilities."),
	_T("InfraRecorder failed to load extended device information."),
	_T("No recorders available"),
	_T("No devices available"),
	_T("An error occured while trying to perform the selected operation."),
	_T("Unable to load drive media. Please insert a valid disc into the drive."),
	_T("The disc is not rewritable or your recorder might not support the operation."),
	_T("Couldn't erase disc using the selected method."),
	_T("Unsupported sector size of %d bytes."),
	_T("A write error occurred. Please see program log for more details."),
	_T("The file you are trying to add is not supported."),
	_T("Unable to locate the file:"),
	_T("An error occured while trying to save the CD-Text binary data. The CD-Text information will not be recorded."),
	_T("Inappropriate audio coding in file:"),
	_T("The drive could not automatically be reloaded."),
	_T("Can't read source disc. Retrying from sector %d."),
	_T("The directory structure is too deep for '%s' (%d), maximum allowed depth is %d."),
	_T("DVD media found. DVD write support is not available in this version."),

	// Project properties.
	_T("Project Properties"),
	_T("Level 1 (11 character file names)"),
	_T("Level 2 (31 character file names)"),
	_T("Level 3 (files larger than 4 GiB)"),
	_T("Mode 1"),
	_T("Mode 2 XA (multisession)"),
	_T("Properties of Track %d"),

	// Container titles.
	_T("Explorer View"),
	_T("Disc Layout"),

	// Miscellaneous.
	_T("Maximum"),
	_T("Not available"),
	_T("Burn "),
	_T("Burn Compilation"),
	_T("New Folder"),
	_T("%I64d minutes"),
	_T(" (automatically detected)"),
	_T("Please select the folder where you want the track(s) to be saved:"),
	_T("Please select a folder:"),

	// Projects.
	_T("Data Project"),
	_T("Audio Project"),
	_T("Mixed-Mode CD Project"),
	_T("%I64d Files, %I64d Folders, %I64d Tracks"),

	// Properties.
	_T("Properties of "),
	_T("Bus %d, Target %d, Lun %d"),

	// Advanced properties.
	_T("Read mode 2 form 1 blocks"),
	_T("Read mode 2 form 2 blocks"),
	_T("Read digital audio blocks"),
	_T("Read multi-session discs"),
	_T("Read fixed-packet CD media using method 2"),
	_T("Read CD bar code"),
	_T("Read R-W subcode information"),
	_T("Read raw P-W subcode data from lead in"),
	_T("Support test writing (simulation)"),
	_T("Support Buffer Underrun Free recording"),
	_T("Support C2 error pointers"),
	_T("Support CD ejection via START/STOP command"),
	_T("Support changing side of disc"),
	_T("Support Individual Disc Present feature"),
	_T("Return CD media catalog number"),
	_T("Return CD ISRC information"),
	_T("Deliver composite A/V data"),
	_T("Play audio discs"),
	_T("Have load-empty-slot-in-changer feature"),
	_T("Lock media on power up via prevent jumper"),
	_T("Allow media to be locked in the drive via PREVENT/ALLOW command"),
	_T("Restart non-streamed digital audio reads accurateley"),
	_T("Return R-W subcode de-interleaved and error-corrected"),
	_T("Support individual channel volume settings"),
	_T("Support independent mute setting for each channel"),
	_T("Support digital output on port 1"),
	_T("Support digital output on port 2"),
	_T("Send digital data LSB-first"),
	_T("Set LRCK high for left-channel data"),
	_T("Have valid data on falling edge of clock"),
	_T("CD-Reader"),
	_T("CD/DVD-Reader"),
	_T("CD-Recorder"),
	_T("CD/DVD-Recorder"),

	// Disc blanking.
	_T("Blank the entire disc"),
	_T("Minimally blank the disc"),
	_T("Unclose last session"),
	_T("Blank last session"),

	// Write methods.
	_T("Session-At-Once (SAO)"),
	_T("Track-At-Once (TAO)"),
	_T("TAO with zero pregap"),
	_T("Raw writing (raw96r)"),
	_T("Raw writing (raw16)"),
	_T("Raw writing (raw96p)"),

	// Eject menu.
	_T("(no drives found)"),

	// Write modes.
	_T("real write"),
	_T("simulation"),

	// Column titles.
	_T("Time"),
	_T("Event"),
	_T("ID"),
	_T("Vendor"),
	_T("Identification"),
	_T("Revision"),
	_T("Name"),
	_T("Size"),
	_T("Type"),
	_T("Modified"),
	_T("Path"),
	_T("Track"),
	_T("Title"),
	_T("Length"),
	_T("Location"),
	_T("Artist"),
	_T("Address"),
	_T("Description"),
	_T("Extensions"),

	// Status dialog.
	_T("Status: "),
	_T("Device: "),
	_T("Total progress: %d%%"),
	_T("Preparing to perform the selected operation."),
	_T("Operation completed."),
	_T("Operation canceled."),
	_T("Last chance to abort, operation will start in %d seconds."),
	_T("Started to erase disc in %s mode."),
	_T("Started to fixate disc in %s mode."),
	_T("Started to write disc in %s mode."),
	_T("Started to write track %d."),
	_T("Started to write disc image."),
	_T("Virtual Disc Image Recorder"),
	_T("Started to read track %d."),
	_T("Started to scan track %d."),
	_T("Started to read disc."),

	// Status strings.
	_T("Erasing disc."),
	_T("Writing data."),
	_T("Writing track %d of %d at %.1fx speed."),
	_T("Writing pregap for track %d at %ld."),
	_T("Waiting for reader process to fill input buffer."),
	_T("Fixating."),
	_T("Writing disc image."),
	_T("Reading track."),
	_T("Scanning track."),
	_T("Found %d bytes of C2 errors in %d sectors."),
	_T("The C2 error rate is %f%%."),
	_T("Reading disc."),

	// Status titles.
	_T("Erasing Disc"),
	_T("Burning Image"),
	_T("Creating Image"),
	_T("Burning Compilation"),
	_T("Fixating Disc"),
	_T("Reading Track"),
	_T("Scanning Track"),
	_T("Copying Disc"),

	// Sucess messages.
	_T("The disc was successfully erased."),
	_T("The data was successfully written to the disc."),
	_T("The disc was successfully fixated."),
	_T("The disc image was successfully created."),
	_T("Done reading track %d."),
	_T("Done scanning track %d."),
	_T("Done reading disc."),

	// Warning messages.
	_T("Some drives don't like fixation in simulation mode."),

	// Information messages.
	_T("Some recorders does not support all erase modes."),
	_T("You can try the erase entire disc method."),

	// Property page and dialog titles.
	_T("General"),
	_T("Advanced"),
	_T("Fields"),
	_T("Audio"),
	_T("Configuration"),
	_T("Language"),
	_T("Shell Extension"),

	// Copy disc.
	_T("Copy Disc"),

	// Disc information.
	_T("Unknown"),
	_T("(sequential)"),
	_T("(restricted overwrite)"),
	_T("revision"),
	_T("Not region protected"),
	_T("blank"),
	_T("incomplete"),
	_T("fixated"),
	_T("random access"),
	_T("empty"),
	_T("reserved"),
	_T("complete"),
	_T("not "),
	_T("The disc is %s, the last session is %s, the disc can %sbe erased."),

	// Miscellaneous.
	_T("The data may not fit on the current disc."),
	_T("Could not open a new session."),

	// Added version 0.40.
	_T("An error occured while trying to load the installed codecs. Please make sure that the installed codecs are compatible with this version of InfraRecorder."),
	_T("Decoding audio tracks."),
	_T("Unable to find a suitable decoder for the audio file: %s."),
	_T("Unable to find the wave encoder. Please verify your codec configuration."),
	_T("Failed to initialize the %s encoder (%d,%d,%d,%I64d)."),
	_T("Encoder failed, could not encode data."),
	_T("Decoded the audio file: %s."),
	_T("The selected target folder is invalid. Please select another target folder."),
	_T("Encoded the audio file: %s."),
	_T("Encoding track (%s)."),

	// Added version 0.41.
	_T("Invalid FIFO buffer size. The size must be at least %i MiB and at most %i MiB."),
	_T("Level 4 (ISO-9660 version 2)"),
	_T("It's recommended to write a cloned disc using the raw96r or raw16 write method. The selected recorder supports none of these modes."),
	_T("Read"),
	_T("Automatic"),
	_T("Copy to Disc Image"),
	_T("Audio discs and multi-session discs must be copied in clone mode to be copied correctly. Please note that it's not possible to clone a disc on the fly."),
	_T("The selected disc image seems to include a TOC-file. It's recommended that you record images that includes a TOC and sub-channel data using a raw write method. InfraRecorder will automatically suggest one for you."),
	_T("Started to estimate file system size."),
	_T("Done estimating file system size (%I64d sectors)."),
	_T("An error occured while estimating the file system size. Can not continue."),
	_T("Estimating file system size."),
	_T("Used: "),
	_T("Free: "),
	_T("Write in session-at-once (SAO) mode"),
	_T("Write in track-at-once (TAO) mode"),
	_T("Write in raw96r mode"),
	_T("Write in raw16 mode"),
	_T("Write in raw96p mode"),
	_T("This option should only be changed when InfraRecorder has detected the wrong write speeds. It does not allow your recorder to write faster than its specification or faster than the media allows."),
	_T("Boot"),
	_T("You can't add more boot images. The maximum number of allowed boot images in one project is 63."),
	_T("None"),
	_T("Floppy"),
	_T("Hard disk"),
	_T("Emulation"),
	_T("Edit Boot Image"),

	// Added version 0.42.
	_T("DVD-Video Project"),
	_T("The project you are trying to open is created by an older version of InfraRecorder. It might not open correctly."),
	_T("Please select the folder that contains all the DVD-files that you want to record. The folder should include a subfolder named VIDEO_TS:"),
	_T("The selected folder does not contain a valid DVD-Video file structure (the folder VIDEO_TS is missing). Please select another folder."),
	_T("A write error occurred. Failed to write lead-in."),
	_T("Failed to initialize the recorder."),
	_T("DVD+RW discs can not be written in simulation mode."),
	_T("Reloading media."),
	_T("Verifying '%s'."),
	_T("Done verifying disc. No errors found."),
	_T("Done verifying disc, %d read error(s) occured."),
	_T("Started disc verification."),
	_T("The file '%s' could not be found on the disc."),
	_T("Read error in '%s' (0x%.8X != 0x%.8X)."),
	_T(""),	// STRINGTABLE_PLACEHOLDER1
	_T(""),	// STRINGTABLE_PLACEHOLDER2
	_T(""),	// STRINGTABLE_PLACEHOLDER3

	// Added version 0.43.
	_T("Drive Letter of %s %s %s"),
	_T("Quick format"),
	_T("Full format"),
	_T("(please insert a disc)"),
	_T("(unsupported media)"),
	_T("Formatting disc."),
	_T("Formatting disc in background."),
	_T("Closing track."),
	_T("Started to format disc in %s mode."),
	_T("Could not format the disc."),
	_T("Could not stop the background format process."),
	_T("The disc was successfully formatted."),

	// Added version 0.44.
	_T("Reading sector %u (retry %u of %u)."),
	_T("Reading disc at %.1fx speed."),
	_T("The destination folder is a subfolder of the source folder."),
	_T("modified:"),
	_T("The destination folder already contains a file or folder named '%s'."),
	_T("Text options:"),
	_T("Icon options:"),
	_T("Show text labels"),
	_T("Selective text on right"),
	_T("No text labels"),
	_T("Small icons"),
	_T("Large icons"),
	_T("Open"),
	_T("Save"),
	_T("Properties"),
	_T("Exit"),
	_T("Burn Project"),
	_T("Burn Image"),
	_T("Copy"),
	_T("Tracks"),
	_T("Erase"),
	_T("Fixate"),
	_T("Log"),
	_T("Configuration"),
	_T("Devices"),
	_T("Help"),
	_T("About"),
	_T("Unable to open device, can not continue."),
	_T("(please insert a blank disc)"),
	_T("Please insert the blank disc which should be recorded."),
	_T("Please insert the source disc that should be copied."),
	_T("Please note that disabling the fixation step does not create a multi-session disc. Please see the help documentation for more information."),

	// Added version 0.45.
	_T("Unable to automatically reload the media. Please try to reload it manually, and then press OK."),
	_T("Operation failed."),
	_T("Failed to create disc image."),
	_T("File System"),
	_T("Unable to import session, could not list disc contents."),
	_T("Session"),
	_T("track"),
	_T("mode"),
	_T("The inserted disc does not appear to be empty, do you want to erase it first?"),
	_T("Multi-session discs require an ISO9660 file system (without any additional file systems). Do you want InfraRecorder to change your project to use this file system and continue?"),
	_T("Verifying Disc")
};
