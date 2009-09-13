
	TRSTR(GENERAL_ERROR /* 0x0000 */, _T("Error"))
	TRSTR(GENERAL_WARNING /* 0x0001 */, _T("Warning"))

	TRSTR(GENERAL_INFORMATION /* 0x0002 */, _T("Information"))
	TRSTR(GENERAL_QUESTION /* 0x0003 */, _T("Question"))

	TRSTR(ERROR_REGISTRYWRITE /* 0x0004 */, _T("Unable to write to the registry. Your user account might not have permissions to perform the requested operation."))
	TRSTR(ERROR_FILEWRITE /* 0x0005 */, _T("An error occured while trying to save the file. The file has not been successfully created."))
	TRSTR(ERROR_LOADSETTINGS /* 0x0006 */, _T("Unable to load the configuration file, it may be corrupt. The XML processor returned: %d."))
	TRSTR(ERROR_AUDIOADDFOLDER /* 0x0007 */, _T("You can't add folders to an audio project."))
	TRSTR(ERROR_LOADPROJECTXML /* 0x0008 */, _T("Unable to parse project file, it may be corrupt. The XML processor returned: %d."))
	TRSTR(ERROR_LOADPROJECT /* 0x0009 */, _T("Unable to parse project file, it may be corrupt."))
	TRSTR(ERROR_PROJECTVERSION /* 0x000a */, _T("The project has been created with a newer version of InfraRecorder. You need to update InfraRecorder to open this project."))
	TRSTR(ERROR_UNSUPCHARSET /* 0x000b */, _T("The active character set could not be automatically detected. The registry suggests the following body character set: %s."))
	TRSTR(ERROR_RELOADDRIVE /* 0x000c */, _T("Please reload the drive and press the 'Reload' button."))
	TRSTR(ERROR_SECTOR /* 0x000d */, _T("Error on sector %d not corrected."))
	TRSTR(ERROR_COMMANDLINE /* 0x000e */, _T("The internal command line is too long for your system to handle. The length limit on your system is %d characters. Please try to shorten the file paths in your project and try again."))

	// OBSOLETE.
	TRSTR(CONFIRM_AUTORUNENABLED /* 0x000f */, _T("InfraRecorder has noticed that autorun is enabled. Leaving autorun enabled causes Windows to poll the CD drive while recording, which might damage your CD. Do you want to turn off autorun (recommended)?"))
	TRSTR(CONFIRM_WRITECANCEL /* 0x0010 */, _T("Are you sure that you want to abort the operation? Aborting might damage your CD permanently."))
	TRSTR(CONFIRM_REMOVEITEMS /* 0x0011 */, _T("Are you sure that you want to remove the selected item(s) from the project?"))
	TRSTR(CONFIRM_SAVEPROJECT /* 0x0012 */, _T("The project has been modified, do you want to save the changes?"))
	TRSTR(CONFIRM_CREATEMIXIMAGE /* 0x0013 */, _T("Only the data track can be recorded to a disc image from mixed-mode projects. Do you want to continue?"))

	TRSTR(INIT_SCANBUS /* 0x0014 */, _T("Scanning SCSI/IDE bus..."))
	TRSTR(INIT_LOADCAPABILITIES /* 0x0015 */, _T("Loading device capabilities..."))
	TRSTR(STRINGTABLE_PLACEHOLDER0 /* 0x0016 */, _T(""))
	TRSTR(INIT_FOUNDDEVICES /* 0x0017 */, _T("InfraRecorder has detected changes in your hardware configuration. Would you like to update your device configuration now?"))
	TRSTR(INIT_DEVICECD /* 0x0018 */, _T("Initializing device..."))
	TRSTR(INIT_TRACK /* 0x0019 */, _T("Loading information from track %d."))

	TRSTR(FAILURE_SCANBUS /* 0x001a */, _T("InfraRecorder failed to scan your SCSI/IDE busses. Please make sure that your system is properly configured."))
	TRSTR(FAILURE_LOADCAP /* 0x001b */, _T("InfraRecorder failed to load device capabilities."))
	TRSTR(FAILURE_LOADINFOEX /* 0x001c */, _T("InfraRecorder failed to load extended device information."))
	TRSTR(FAILURE_NORECORDERS /* 0x001d */, _T("No recorders available"))
	TRSTR(FAILURE_NODEVICES /* 0x001e */, _T("No devices available"))
	TRSTR(FAILURE_CDRTOOLS /* 0x001f */, _T("An error occured while trying to perform the selected operation."))
	TRSTR(FAILURE_NOMEDIA /* 0x0020 */, _T("Unable to load drive media. Please insert a valid disc into the drive."))
	TRSTR(FAILURE_UNSUPRW /* 0x0021 */,_T("The disc is not rewritable or your recorder might not support the operation."))
	TRSTR(FAILURE_ERASE /* 0x0022 */, _T("Couldn't erase disc using the selected method."))
	TRSTR(FAILURE_BADSECTORSIZE /* 0x0023 */, _T("Unsupported sector size of %d bytes."))
	TRSTR(FAILURE_WRITE /* 0x0024 */, _T("A write error occurred. Please see program log for more details."))
	TRSTR(FAILURE_UNSUPAUDIO /* 0x0025 */, _T("The file you are trying to add is not supported."))
	TRSTR(FAILURE_FILENOTFOUND /* 0x0026 */, _T("Unable to locate the file:"))
	TRSTR(FAILURE_CREATECDTEXT /* 0x0027 */, _T("An error occured while trying to save the CD-Text binary data. The CD-Text information will not be recorded."))
	TRSTR(FAILURE_AUDIOCODING /* 0x0028 */, _T("Inappropriate audio coding in file:"))			
	TRSTR(FAILURE_LOADDRIVE /* 0x0029 */, _T("The drive could not automatically be reloaded."))
	TRSTR(FAILURE_READSOURCEDISC /* 0x002a */, _T("Can't read source disc. Retrying from sector %d."))
	TRSTR(FAILURE_DEEPDIR /* 0x002b */, _T("The directory structure is too deep for '%s' (%d), maximum allowed depth is %d."))
	TRSTR(FAILURE_DVDSUPPORT /* 0x002c */, _T("DVD media found. DVD write support is not available in this version."))

	// Project properties.
	TRSTR(PROJECTPROP_TITLE /* 0x002d */, _T("Project Properties"))
	TRSTR(PROJECTPROP_ISOLEVEL1 /* 0x002e */, _T("Level 1 (11 character file names)"))
	TRSTR(PROJECTPROP_ISOLEVEL2 /* 0x002f */, _T("Level 2 (31 character file names)"))
	TRSTR(PROJECTPROP_ISOLEVEL3 /* 0x0030 */, _T("Level 3 (files larger than 4 GiB)"))
	TRSTR(PROJECTPROP_MODE1 /* 0x0031 */, _T("Mode 1"))
	TRSTR(PROJECTPROP_MODE2 /* 0x0032 */, _T("Mode 2 XA (multisession)"))
	TRSTR(PROJECTPROP_TRACKPROP /* 0x0033 */, _T("Properties of Track %d"))

	// Container titles.
	TRSTR(TITLE_EXPLORERVIEW /* 0x0034 */, _T("Explorer View"))
	TRSTR(TITLE_PROJECTVIEW /* 0x0035 */, _T("Disc Layout"))

	// Miscellaneous.
	TRSTR(MISC_MAXIMUM /* 0x0036 */, _T("Maximum"))
	TRSTR(MISC_NOTAVAILABLE /* 0x0037 */, _T("Not available"))
	TRSTR(MISC_BURN /* 0x0038 */, _T("Burn "))
	TRSTR(MISC_BURNCOMPILATION /* 0x0039 */, _T("Burn Compilation"))
	TRSTR(MISC_NEWFOLDER /* 0x003a */, _T("New Folder"))
	TRSTR(MISC_MINUTES /* 0x003b */, _T("%I64d minutes"))
	TRSTR(MISC_AUTODETECT /* 0x003c */, _T(" (automatically detected)"))
	TRSTR(MISC_SPECIFYTRACKFOLDER /* 0x003d */, _T("Please select the folder where you want the track(s) to be saved:"))
	TRSTR(MISC_SPECIFYFOLDER /* 0x003e */, _T("Please select a folder:"))

	// Projects.
	TRSTR(PROJECT_DATA /* 0x003f */, _T("Data Project"))
	TRSTR(PROJECT_AUDIO /* 0x0040 */, _T("Audio Project"))
	TRSTR(PROJECT_MIXED /* 0x0041 */, _T("Mixed-Mode CD Project"))
	TRSTR(PROJECT_CONTENTS /* 0x0042 */, _T("%I64d Files, %I64d Folders, %I64d Tracks"))

	// Properties.
	TRSTR(PROPERTIES_TITLE /* 0x0043 */, _T("Properties of "))
	TRSTR(PROPERTIES_DEVICELOC /* 0x0044 */, _T("Bus %d, Target %d, Lun %d"))

	// Advanced properties.
	TRSTR(ADVPROP_MODE2FORM1 /* 0x0045 */, _T("Read mode 2 form 1 blocks"))
	TRSTR(ADVPROP_MODE2FORM2 /* 0x0046 */, _T("Read mode 2 form 2 blocks"))
	TRSTR(ADVPROP_READDIGAUDIO /* 0x0047 */, _T("Read digital audio blocks"))
	TRSTR(ADVPROP_READMULTSESSION /* 0x0048 */, _T("Read multi-session discs"))
	TRSTR(ADVPROP_READFIXPACKET /* 0x0049 */, _T("Read fixed-packet CD media using method 2"))
	TRSTR(ADVPROP_READBARCODE /* 0x004a */, _T("Read CD bar code"))
	TRSTR(ADVPROP_READRWSUBCODE /* 0x004b */, _T("Read R-W subcode information"))
	TRSTR(ADVPROP_READRAWPWSC /* 0x004c */, _T("Read raw P-W subcode data from lead in"))
	TRSTR(ADVPROP_SIMULATION /* 0x004d */, _T("Support test writing (simulation)"))
	TRSTR(ADVPROP_BUFRECORDING /* 0x004e */, _T("Support Buffer Underrun Free recording"))
	TRSTR(ADVPROP_C2EP /* 0x004f */, _T("Support C2 error pointers"))
	TRSTR(ADVPROP_EJECTCDSS /* 0x0050 */, _T("Support CD ejection via START/STOP command"))
	TRSTR(ADVPROP_CHANGEDISCSIDE /* 0x0051 */, _T("Support changing side of disc"))
	TRSTR(ADVPROP_INDIVIDUALDP /* 0x0052 */, _T("Support Individual Disc Present feature"))
	TRSTR(ADVPROP_RETURNCDCN /* 0x0053 */, _T("Return CD media catalog number"))
	TRSTR(ADVPROP_RETURNCDISRC /* 0x0054 */, _T("Return CD ISRC information"))
	TRSTR(ADVPROP_DELIVCOMPOSITE /* 0x0055 */, _T("Deliver composite A/V data"))
	TRSTR(ADVPROP_PLAYAUDIOCD /* 0x0056 */, _T("Play audio discs"))
	TRSTR(ADVPROP_HASLESIC /* 0x0057 */, _T("Have load-empty-slot-in-changer feature"))
	TRSTR(ADVPROP_LMOPU /* 0x0058 */, _T("Lock media on power up via prevent jumper"))
	TRSTR(ADVPROP_ALLOWML /* 0x0059 */, _T("Allow media to be locked in the drive via PREVENT/ALLOW command"))
	TRSTR(ADVPROP_RESTARTNSDARA /* 0x005a */, _T("Restart non-streamed digital audio reads accurateley"))
	TRSTR(ADVPROP_RETURNRWSUBCODE /* 0x005b */, _T("Return R-W subcode de-interleaved and error-corrected"))
	TRSTR(ADVPROP_INDIVIDUALVC /* 0x005c */, _T("Support individual channel volume settings"))
	TRSTR(ADVPROP_INDEPENDENTMUTE /* 0x005d */, _T("Support independent mute setting for each channel"))
	TRSTR(ADVPROP_DOPORT1 /* 0x005e */, _T("Support digital output on port 1"))
	TRSTR(ADVPROP_DOPORT2 /* 0x005f */, _T("Support digital output on port 2"))
	TRSTR(ADVPROP_DOSENDDIGDAT /* 0x0060 */, _T("Send digital data LSB-first"))
	TRSTR(ADVPROP_DOSETLRCK /* 0x0061 */, _T("Set LRCK high for left-channel data"))
	TRSTR(ADVPROP_HASVALIDDATA /* 0x0062 */, _T("Have valid data on falling edge of clock"))
	TRSTR(DEVICETYPE_CDREADER /* 0x0063 */, _T("CD-Reader"))
	TRSTR(DEVICETYPE_DVDREADER /* 0x0064 */, _T("CD/DVD-Reader"))
	TRSTR(DEVICETYPE_CDRECORDER /* 0x0065 */, _T("CD-Recorder"))
	TRSTR(DEVICETYPE_DVDRECORDER /* 0x0066 */, _T("CD/DVD-Recorder"))

	// Disc blanking.
	TRSTR(BLANKMODE_FULL /* 0x0067 */, _T("Blank the entire disc"))
	TRSTR(BLANKMODE_MINIMAL /* 0x0068 */, _T("Minimally blank the disc"))
	TRSTR(BLANKMODE_UNCLOSE /* 0x0069 */, _T("Unclose last session"))
	TRSTR(BLANKMODE_SESSION /* 0x006a */, _T("Blank last session"))

	// Write methods.
	TRSTR(WRITEMODE_SAO /* 0x006b */, _T("Session-At-Once (SAO)"))
	TRSTR(WRITEMODE_TAO /* 0x006c */, _T("Track-At-Once (TAO)"))
	TRSTR(WRITEMODE_TAONOPREGAP /* 0x006d */, _T("TAO with zero pregap"))
	TRSTR(WRITEMODE_RAW96R /* 0x006e */, _T("Raw writing (raw96r)"))
	TRSTR(WRITEMODE_RAW16 /* 0x006f */, _T("Raw writing (raw16)"))
	TRSTR(WRITEMODE_RAW96P /* 0x0070 */, _T("Raw writing (raw96p)"))

	// Eject menu.
	TRSTR(EJECTMENU_NODRIVES /* 0x0071 */, _T("(no drives found)"))

	// Write modes.
	TRSTR(WRITEMODE_REAL /* 0x0072 */, _T("real write"))	
	TRSTR(WRITEMODE_SIMULATION /* 0x0073 */, _T("simulation"))	

	// Column titles.
	TRSTR(COLUMN_TIME /* 0x0074 */, _T("Time"))
	TRSTR(COLUMN_EVENT /* 0x0075 */, _T("Event"))
	TRSTR(COLUMN_ID /* 0x0076 */, _T("ID"))
	TRSTR(COLUMN_VENDOR /* 0x0077 */, _T("Vendor"))
	TRSTR(COLUMN_IDENTIFICATION /* 0x0078 */, _T("Identification"))
	TRSTR(COLUMN_REVISION /* 0x0079 */,    _T("Revision"))
	TRSTR(COLUMN_NAME /* 0x007a */,		_T("Name"))
	TRSTR(COLUMN_SIZE /* 0x007b */,		_T("Size"))
	TRSTR(COLUMN_TYPE /* 0x007c */,		_T("Type"))
	TRSTR(COLUMN_MODIFIED /* 0x007d */,	_T("Modified"))
	TRSTR(COLUMN_PATH /* 0x007e */,		_T("Path"))
	TRSTR(COLUMN_TRACK /* 0x007f */,		_T("Track"))
	TRSTR(COLUMN_TITLE /* 0x0080 */,		_T("Title"))
	TRSTR(COLUMN_LENGTH /* 0x0081 */,		_T("Length"))
	TRSTR(COLUMN_LOCATION /* 0x0082 */,	_T("Location"))
	TRSTR(COLUMN_ARTIST /* 0x0083 */,		_T("Artist"))
	TRSTR(COLUMN_ADDRESS /* 0x0084 */,		_T("Address"))
	TRSTR(COLUMN_DESCRIPTION /* 0x0085 */, _T("Description"))
	TRSTR(COLUMN_EXTENSIONS /* 0x0086 */,	_T("Extensions"))

	// Status dialog.
	TRSTR(PROGRESS_STATUS /* 0x0087 */,          _T("Status: "))
	TRSTR(PROGRESS_DEVICE /* 0x0088 */,		  _T("Device: "))
	TRSTR(PROGRESS_TOTAL /* 0x0089 */,			  _T("Total progress: %d%%"))
	TRSTR(PROGRESS_INIT /* 0x008a */,			  _T("Preparing to perform the selected operation."))
	TRSTR(PROGRESS_DONE /* 0x008b */,			  _T("Operation completed."))					
	TRSTR(PROGRESS_CANCELED /* 0x008c */,		  _T("Operation canceled."))
	TRSTR(PROGRESS_GRACETIME /* 0x008d */,		  _T("Last chance to abort, operation will start in %d seconds."))
	TRSTR(PROGRESS_BEGINERASE /* 0x008e */,	  _T("Started to erase disc in %s mode."))
	TRSTR(PROGRESS_BEGINFIXATE /* 0x008f */,	  _T("Started to close disc in %s mode."))
	TRSTR(PROGRESS_BEGINWRITE /* 0x0090 */,	  _T("Started to write disc in %s mode."))
	TRSTR(PROGRESS_BEGINTRACK /* 0x0091 */,	  _T("Started to write track %d."))			
	TRSTR(PROGRESS_BEGINDISCIMAGE /* 0x0092 */,  _T("Started to write disc image."))
	TRSTR(PROGRESS_IMAGEDEVICE /* 0x0093 */,	  _T("Virtual Disc Image Recorder"))
	TRSTR(PROGRESS_BEGINREADTRACK /* 0x0094 */,  _T("Started to read track %d."))
	TRSTR(PROGRESS_BEGINSCANTRACK /* 0x0095 */,  _T("Started to scan track %d."))
	TRSTR(PROGRESS_BEGINREADDISC /* 0x0096 */,	  _T("Started to read disc."))				

	// Status strings.
	TRSTR(STATUS_ERASE /* 0x0097 */,       _T("Erasing disc."))
	TRSTR(STATUS_WRITEDATA /* 0x0098 */,	_T("Writing data."))							// FIXME: Obsolete.	
	TRSTR(STATUS_WRITE /* 0x0099 */,		_T("Writing track %d of %d at %.1fx speed."))	// FIXME: Obsolete.	
	TRSTR(STATUS_WRITEPREGAP /* 0x009a */,	_T("Writing pregap for track %d at %ld."))		// FIXME: Obsolete.
	TRSTR(STATUS_FILLBUFFER /* 0x009b */,	_T("Waiting for reader process to fill input buffer."))	// FIXME: Obsolete.	
	TRSTR(STATUS_FIXATE /* 0x009c */,		_T("Closing disc."))							// FIXME: Obsolete.	
	TRSTR(STATUS_WRITEIMAGE /* 0x009d */,	_T("Writing disc image."))
	TRSTR(STATUS_READTRACK /* 0x009e */,	_T("Reading track."))
	TRSTR(STATUS_SCANTRACK /* 0x009f */,	_T("Scanning track."))
	TRSTR(STATUS_C2TOTAL /* 0x00a0 */,		_T("Found %d bytes of C2 errors in %d sectors."))
	TRSTR(STATUS_C2RATE /* 0x00a1 */,		_T("The C2 error rate is %f%%."))
	TRSTR(STATUS_READDISC /* 0x00a2 */,	_T("Reading disc."))							// FIXME: Obsolete.	

	// Status titles.
	TRSTR(STITLE_ERASE /* 0x00a3 */,          _T("Erasing Disc"))
	TRSTR(STITLE_BURNIMAGE /* 0x00a4 */,	   _T("Burning Image"))
	TRSTR(STITLE_CREATEIMAGE /* 0x00a5 */,	   _T("Creating Image"))
	TRSTR(STITLE_BURNCOMPILATION /* 0x00a6 */,_T("Burning Compilation"))
	TRSTR(STITLE_FIXATE /* 0x00a7 */,		   _T("Closing Disc"))
	TRSTR(STITLE_READTRACK /* 0x00a8 */,	   _T("Reading Track"))
	TRSTR(STITLE_SCANTRACK /* 0x00a9 */,	   _T("Scanning Track"))
	TRSTR(STITLE_COPYDISC /* 0x00aa */,	   _T("Copying Disc"))

	// Sucess messages.
	TRSTR(SUCCESS_ERASE /* 0x00ab */,       _T("The disc was successfully erased."))
	TRSTR(SUCCESS_WRITE /* 0x00ac */,		 _T("The data were successfully written to the disc."))// FIXME: Obsolete.
	TRSTR(SUCCESS_FIXATE /* 0x00ad */,		 _T("The disc was successfully closed."))				// FIXME: Obsolete.
	TRSTR(SUCCESS_CREATEIMAGE /* 0x00ae */, _T("The disc image was successfully created."))
	TRSTR(SUCCESS_READTRACK /* 0x00af */,	 _T("Done reading track %d."))
	TRSTR(SUCCESS_SCANTRACK /* 0x00b0 */,	 _T("Done scanning track %d."))
	TRSTR(SUCCESS_READDISC /* 0x00b1 */,	 _T("Done reading disc."))								// FIXME: Obsolete.

	// Warning messages.
	TRSTR(WARNING_FIXATE /* 0x00b2 */,	_T("Some drives don't like closing in simulation mode."))	// FIXME: Obsolete.

	// Information messages.
	TRSTR(INFO_UNSUPERASEMODE /* 0x00b3 */, _T("Some recorders does not support all erase modes."))	// FIXME: Obsolete.
	TRSTR(INFO_ERASERETRY /* 0x00b4 */, _T("You can try the erase entire disc method."))		// FIXME: Obsolete.

	// Property page titles.
	TRSTR(TITLE_GENERAL /* 0x00b5 */,       _T("General"))
	TRSTR(TITLE_ADVANCED /* 0x00b6 */,		 _T("Advanced"))
	TRSTR(TITLE_FIELDS /* 0x00b7 */,		 _T("Fields"))
	TRSTR(TITLE_AUDIO /* 0x00b8 */,		 _T("Audio"))
	TRSTR(TITLE_CONFIGURATION /* 0x00b9 */, _T("Configuration"))
	TRSTR(TITLE_LANGUAGE /* 0x00ba */,		 _T("Language"))
	TRSTR(TITLE_SHELLEXT /* 0x00bb */,		 _T("Shell Extension"))

	// Copy disc.
	TRSTR(COPYDISC_TITLE /* 0x00bc */, _T("Copy Disc"))

	// Disc information.
	TRSTR(DISC_UNKNOWN /* 0x00bd */,             _T("Unknown"))
	TRSTR(DISC_SEQUENTIAL /* 0x00be */,		  _T("(sequential)"))
	TRSTR(DISC_RESTRICTEDOVERWRITE /* 0x00bf */, _T("(restricted overwrite)"))
	TRSTR(DISC_REVISION /* 0x00c0 */,			  _T("revision"))
	TRSTR(DISC_NOREGION /* 0x00c1 */,			  _T("Not region protected"))
	TRSTR(DISC_BLANK /* 0x00c2 */,				  _T("blank"))
	TRSTR(DISC_INCOMPLETE /* 0x00c3 */,		  _T("incomplete"))
	TRSTR(DISC_FIXATED /* 0x00c4 */,			  _T("closed"))
	TRSTR(DISC_RANDOMACCESS /* 0x00c5 */,		  _T("random access"))
	TRSTR(DISC_EMPTY /* 0x00c6 */,				  _T("empty"))
	TRSTR(DISC_RESERVED /* 0x00c7 */,			  _T("reserved"))
	TRSTR(DISC_COMPLETE /* 0x00c8 */,			  _T("complete"))
	TRSTR(DISC_NOT /* 0x00c9 */,				  _T("not "))
	TRSTR(DISC_STATUS /* 0x00ca */,			  _T("The disc is %s, the last session is %s, the disc can %sbe erased."))

	// Miscellaneous.
	TRSTR(WARNING_DISCSIZE /* 0x00cb */, _T("The data may not fit on the current disc."))// FIXME: Obsolete.
	TRSTR(FAILURE_OPENSESSION /* 0x00cc */, _T("Could not open a new session."))			// FIXME: Obsolete.

	// Added version 0.40.
	TRSTR(ERROR_LOADCODECS /* 0x00cd */, _T("An error occured while trying to load the installed codecs. Please make sure that the installed codecs are compatible with this version of InfraRecorder."))
	TRSTR(PROGRESS_DECODETRACKS /* 0x00ce */, _T("Decoding audio tracks."))
	TRSTR(ERROR_NODECODER /* 0x00cf */,	   _T("Unable to find a suitable decoder for the audio file: %s."))
	TRSTR(ERROR_WAVECODEC /* 0x00d0 */,	   _T("Unable to find the wave encoder. Please verify your codec configuration."))
	TRSTR(ERROR_CODECINIT /* 0x00d1 */,	   _T("Failed to initialize the %s encoder (%d,%d,%d,%I64d)."))
	TRSTR(ERROR_ENCODEDATA /* 0x00d2 */,	   _T("Encoder failed, could not encode data."))
	TRSTR(SUCCESS_DECODETRACK /* 0x00d3 */,   _T("Decoded the audio file: %s."))
	TRSTR(ERROR_TARGETFOLDER /* 0x00d4 */,	   _T("The selected target folder is invalid. Please select another target folder."))
	TRSTR(SUCCESS_ENCODETRACK /* 0x00d5 */,   _T("Encoded the audio file: %s."))
	TRSTR(PROGRESS_ENCODETRACK /* 0x00d6 */,  _T("Encoding track (%s)."))

	// Added version 0.41.
	TRSTR(ERROR_FIFOSIZE /* 0x00d7 */,				  _T("Invalid FIFO buffer size. The size must be at least %i MiB and at most %i MiB."))
	TRSTR(PROJECTPROP_ISOLEVEL4 /* 0x00d8 */,		  _T("Level 4 (ISO-9660 version 2)"))
	TRSTR(WARNING_CLONEWRITEMETHOD /* 0x00d9 */,	  _T("It's recommended to write a cloned disc using the raw96r or raw16 write method. The selected recorder supports none of these modes."))
	TRSTR(TITLE_READ /* 0x00da */,					  _T("Read"))
	TRSTR(MISC_AUTO /* 0x00db */,					  _T("Automatic"))
	TRSTR(COPYIMAGE_TITLE /* 0x00dc */,			  _T("Copy to Disc Image"))
	TRSTR(INFO_COPYDISC /* 0x00dd */,				  _T("Audio discs and multi-session discs must be copied in clone mode to be copied correctly. Please note that it's not possible to clone a disc on the fly."))
	TRSTR(INFO_RAWIMAGE /* 0x00de */,				  _T("The selected disc image seems to include a TOC-file. It's recommended that you record images that includes a TOC and sub-channel data using a raw write method. InfraRecorder will automatically suggest one for you."))
	TRSTR(PROGRESS_BEGINESTIMAGESIZE /* 0x00df */,	  _T("Started to estimate file system size."))
	TRSTR(SUCCESS_ESTIMAGESIZE /* 0x00e0 */,		  _T("Done estimating file system size (%I64d sectors)."))
	TRSTR(ERROR_ESTIMAGESIZE /* 0x00e1 */,			  _T("An error occured while estimating the file system size. Can not continue."))
	TRSTR(PROGRESS_ESTIMAGESIZE /* 0x00e2 */,		  _T("Estimating file system size."))
	TRSTR(SPACEMETER_USED /* 0x00e3 */,			  _T("Used: "))
	TRSTR(SPACEMETER_FREE /* 0x00e4 */,			  _T("Free: "))
	TRSTR(ADVPROP_SAO /* 0x00e5 */,				  _T("Write in session-at-once (SAO) mode"))
	TRSTR(ADVPROP_TAO /* 0x00e6 */,				  _T("Write in track-at-once (TAO) mode"))
	TRSTR(ADVPROP_RAW96R /* 0x00e7 */,				  _T("Write in raw96r mode"))
	TRSTR(ADVPROP_RAW16 /* 0x00e8 */,				  _T("Write in raw16 mode"))
	TRSTR(ADVPROP_RAW96P /* 0x00e9 */,				  _T("Write in raw96p mode"))
	TRSTR(INFO_WRITESPEED /* 0x00ea */,			  _T("This option should only be changed when InfraRecorder has detected the wrong write speeds. It does not allow your recorder to write faster than its specification or faster than the media allows."))
	TRSTR(TITLE_BOOT /* 0x00eb */,					  _T("Boot"))
	TRSTR(ERROR_NUMBOOTIMAGES /* 0x00ec */,		  _T("You can't add more boot images. The maximum number of allowed boot images in one project is 63."))
	TRSTR(BOOTEMU_NONE /* 0x00ed */,				  _T("None"))
	TRSTR(BOOTEMU_FLOPPY /* 0x00ee */,				  _T("Floppy"))
	TRSTR(BOOTEMU_HARDDISK /* 0x00ef */,			  _T("Hard disk"))
	TRSTR(COLUMN_EMULATION /* 0x00f0 */,			  _T("Emulation"))
	TRSTR(TITLE_EDITBOOTIMAGE /* 0x00f1 */,		  _T("Edit Boot Image"))

	// Added version 0.42.
	TRSTR(PROJECT_DVDVIDEO /* 0x00f2 */,         _T("DVD-Video Project"))
	TRSTR(WARNING_OLDPROJECT /* 0x00f3 */,		  _T("The project you are trying to open is created by an older version of InfraRecorder. It might not open correctly."))
	TRSTR(MISC_SPECIFYDVDFOLDER /* 0x00f4 */,	  _T("Please select the folder that contains all the DVD-files that you want to record. The folder should include a subfolder named VIDEO_TS:"))
	TRSTR(ERROR_INVALIDDVDFOLDER /* 0x00f5 */,	  _T("The selected folder does not contain a valid DVD-Video file structure (the folder VIDEO_TS is missing). Please select another folder."))
	TRSTR(FAILURE_WRITELEADIN /* 0x00f6 */,	  _T("A write error occurred. Failed to write lead-in."))		// FIXME: Obsolete.
	TRSTR(FAILURE_INITDRIVE /* 0x00f7 */,		  _T("Failed to initialize the recorder."))					// FIXME: Obsolete.
	TRSTR(FAILURE_DVDRWDUMMY /* 0x00f8 */,		  _T("DVD+RW discs can not be written in simulation mode."))// FIXME: Obsolete.
	TRSTR(PROGRESS_RELOADMEDIA /* 0x00f9 */,	  _T("Reloading media."))
	TRSTR(STATUS_VERIFY /* 0x00fa */,			  _T("Verifying '%s'."))
	TRSTR(SUCCESS_VERIFY /* 0x00fb */,			  _T("Done verifying disc. No errors found."))
	TRSTR(FAILURE_VERIFY /* 0x00fc */,			  _T("Done verifying disc, %d read error(s) occured."))
	TRSTR(PROGRESS_BEGINVERIFY /* 0x00fd */,	  _T("Started disc verification."))
	TRSTR(FAILURE_VERIFYNOFILE /* 0x00fe */,	  _T("The file '%s' could not be found on the disc."))
	TRSTR(FAILURE_VERIFYREADERROR /* 0x00ff */,  _T("Read error in '%s' (0x%.8X != 0x%.8X)."))

	TRSTR(STRINGTABLE_PLACEHOLDER1 = 0x0100, _T(""))
	TRSTR(STRINGTABLE_PLACEHOLDER2 = 0x0101, _T(""))
	TRSTR(STRINGTABLE_PLACEHOLDER3 = 0x0102, _T(""))

	// Added version 0.43.
	TRSTR(DRIVELETTER_TITLE /* 0x0103 */,        _T("Drive Letter of %s %s %s"))
	TRSTR(FORMATMODE_QUICK /* 0x0104 */,		  _T("Quick format"))
	TRSTR(FORMATMODE_FULL /* 0x0105 */,		  _T("Full format"))
	TRSTR(MEDIA_INSERT /* 0x0106 */,			  _T("(please insert a disc)"))
	TRSTR(MEDIA_UNSUPPORTED /* 0x0107 */,		  _T("(unsupported media)"))
	TRSTR(STATUS_FORMAT /* 0x0108 */,			  _T("Formatting disc."))
	TRSTR(STATUS_FORMATBKGND /* 0x0109 */,		  _T("Formatting disc in background."))
	TRSTR(STATUS_CLOSETRACK /* 0x010a */,		  _T("Closing track."))
	TRSTR(PROGRESS_BEGINFORMAT /* 0x010b */,	  _T("Started to format disc in %s mode."))
	TRSTR(FAILURE_FORMAT /* 0x010c */,			  _T("Could not format the disc."))
	TRSTR(FAILURE_STOPBKGNDFORMAT /* 0x010d */,  _T("Could not stop the background format process."))
	TRSTR(SUCCESS_FORMAT /* 0x010e */,			  _T("The disc was successfully formatted."))

	// Added version 0.44.
	TRSTR(STATUS_REREADSECTOR /* 0x010f */,       _T("Reading sector %u (retry %u of %u)."))
	TRSTR(STATUS_READTRACK2 /* 0x0110 */,		   _T("Reading disc at %.1fx speed."))
	TRSTR(ERROR_MOVESAMESRCDST /* 0x0111 */,	   _T("The destination folder is a subfolder of the source folder."))
	TRSTR(MISC_MODIFIED /* 0x0112 */,			   _T("modified:"))
	TRSTR(ERROR_EXISTINGFILENAME /* 0x0113 */,	   _T("The destination folder already contains a file or folder named '%s'."))
	TRSTR(TBCUSTOMIZE_TEXTOPTIONS /* 0x0114 */,   _T("Text options:"))
	TRSTR(TBCUSTOMIZE_ICONOPTIONS /* 0x0115 */,   _T("Icon options:"))
	TRSTR(TBCUSTOMIZE_SHOWTEXT /* 0x0116 */,	   _T("Show text labels"))
	TRSTR(TBCUSTOMIZE_SHOWTEXTRIGHT /* 0x0117 */, _T("Selective text on right"))
	TRSTR(TBCUSTOMIZE_NOTEXT /* 0x0118 */,		   _T("No text labels"))
	TRSTR(TBCUSTOMIZE_ICONSMALL /* 0x0119 */,	   _T("Small icons"))
	TRSTR(TBCUSTOMIZE_ICONLARGE /* 0x011a */,	   _T("Large icons"))
	TRSTR(TOOLBAR_OPEN /* 0x011b */,			   _T("Open"))
	TRSTR(TOOLBAR_SAVE /* 0x011c */,			   _T("Save"))
	TRSTR(TOOLBAR_PROJECTPROPERTIES /* 0x011d */, _T("Properties"))
	TRSTR(TOOLBAR_EXIT /* 0x011e */,			   _T("Exit"))
	TRSTR(TOOLBAR_BURNCOMPILATION /* 0x011f */,   _T("Burn Project"))
	TRSTR(TOOLBAR_BURNIMAGE /* 0x0120 */,		   _T("Burn Image"))
	TRSTR(TOOLBAR_COPY /* 0x0121 */,			   _T("Copy"))
	TRSTR(TOOLBAR_TRACKS /* 0x0122 */,			   _T("Tracks"))
	TRSTR(TOOLBAR_ERASE /* 0x0123 */,			   _T("Erase"))
	TRSTR(TOOLBAR_FIXATE /* 0x0124 */,			   _T("Close"))
	TRSTR(TOOLBAR_LOG /* 0x0125 */,			   _T("Log"))
	TRSTR(TOOLBAR_CONFIGURATION /* 0x0126 */,	   _T("Configuration"))
	TRSTR(TOOLBAR_DEVICES /* 0x0127 */,		   _T("Devices"))
	TRSTR(TOOLBAR_HELP /* 0x0128 */,			   _T("Help"))
	TRSTR(TOOLBAR_ABOUT /* 0x0129 */,			   _T("About"))
	TRSTR(STRINGTABLE_PLACEHOLDER4 /* 0x012a */,		   _T(""))
	TRSTR(MEDIA_INSERTBLANK /* 0x012b */,		   _T("(please insert a blank disc)"))
	TRSTR(INFO_INSERTBLANK /* 0x012c */,		   _T("Please insert the blank disc which should be recorded."))
	TRSTR(INFO_INSERTSOURCE /* 0x012d */,		   _T("Please insert the source disc that should be copied."))
	TRSTR(WARNING_NOFIXATION /* 0x012e */,		   _T("Please note that disabling the closing step does not create a multi-session disc. Please see the help documentation for more information."))

	// Added version 0.45.
	TRSTR(INFO_RELOAD /* 0x012f */,          _T("Unable to automatically reload the media. Please try to reload it manually, and then press OK."))
	TRSTR(PROGRESS_FAILED /* 0x0130 */,	  _T("Operation failed."))
	TRSTR(FAILURE_CREATEIMAGE /* 0x0131 */,  _T("Failed to create disc image."))
	TRSTR(TITLE_FILESYSTEM /* 0x0132 */,	  _T("File System"))
	TRSTR(ERROR_IMPORTSESSION /* 0x0133 */,  _T("Unable to import session, could not list disc contents."))
	TRSTR(MISC_SESSION /* 0x0134 */,		  _T("Session"))
	TRSTR(MISC_TRACK /* 0x0135 */,			  _T("track"))
	TRSTR(MISC_MODE /* 0x0136 */,			  _T("mode"))
	TRSTR(MISC_ERASENONEMPTY /* 0x0137 */,	  _T("The inserted disc does not appear to be empty, do you want to erase it first?"))
	TRSTR(WARNING_IMPORTFS /* 0x0138 */,	  _T("Multi-session discs require an ISO9660 file system (without any additional file systems). Do you want InfraRecorder to change your project to use this file system and continue?"))
	TRSTR(STITLE_VERIFYDISC /* 0x0139 */,	  _T("Verifying Disc"))

	// Added version 0.46.
	TRSTR(ERROR_NUMCOPIES /* 0x013a */, _T("Invalid number of copies. Please specify a number larger than zero."))
	TRSTR(INFO_NEXTCOPY /* 0x013b */, _T("Please insert a new blank disc to create another copy."))
	TRSTR(INFO_CREATECOPY /* 0x013c */, _T("Creating copy %d of %d."))

	// Added version 0.50.
	TRSTR(ERROR_PROJECT_IMPORT /* 0x013d */, _T("Unable to import, does the selected source file exist?"))
	TRSTR(ERROR_PROJECT_IMPORT_FILE /* 0x013e */, _T("Could not import the file \"%s\"."))
	TRSTR(COLUMN_DRIVE /* 0x013f */, _T("Drive"))
	TRSTR(CONFIRM_CREATE_DIR_PATH /* 0x0140 */, _T("Directory \"%s\" does not exist. Do you want it to be created?" ))
	TRSTR(CANNOT_CREATE_DIR_PATH  /* 0x0141 */, _T("Cannot create directory \"%s\"." ))
	TRSTR(WARNING_MISSPROJFILE /* 0x0142 */, _T("The project file \"%s\" could not be found on your computer. It will be removed from the project."))
	TRSTR(WARNING_NODEVICES /* 0x0143 */, _T("InfraRecorder was unable to find any disc devices in your system.\n\nPlease note that Windows 2000, XP and 2003 systems require administrator permissions to access disc devices. Often this can be circumvented by changing your system settings. Please consult the InfraRecorder FAQ in the manual for further information."))

	// Added version <future version>.
	TRSTR(STITLE_PREPOPERATION /* 0x0144 */, _T("Preparing Operation"))
	TRSTR(STATUS_GATHER_FILE_INFO /* 0x0145 */, _T("Gathering project file information."))