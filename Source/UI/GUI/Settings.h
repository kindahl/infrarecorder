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

#pragma once
#include <list>
#include "../../Common/StringUtil.h"
#include "../../Common/XMLProcessor.h"
#include "../../Common/LNGProcessor.h"
#include "TreeManager.h"

#define WRITEMETHOD_SAO					0
#define WRITEMETHOD_TAO					1
#define WRITEMETHOD_TAONOPREGAP			2
#define WRITEMETHOD_RAW96R				3
#define WRITEMETHOD_RAW16				4
#define WRITEMETHOD_RAW96P				5

#define LISTVIEWSTYLE_LARGEICONS		0
#define LISTVIEWSTYLE_SMALLICONS		1
#define LISTVIEWSTYLE_LIST				2
#define LISTVIEWSTYLE_DETAILS			3

#define TOOLBAR_TEXT_SHOW				0
#define TOOLBAR_TEXT_SHOWRIGHT			1
#define TOOLBAR_TEXT_DONTSHOW			2
#define TOOLBAR_ICON_SMALL				0
#define TOOLBAR_ICON_LARGE				1

#define FIFO_MAX						128
#define FIFO_MIN						4

#define FILESYSTEM_ISO9660				0
#define FILESYSTEM_ISO9660_UDF			1
#define FILESYSTEM_DVDVIDEO				2
#define FILESYSTEM_UDF					3

class ISettings
{
public:
	virtual bool Save(CXMLProcessor *pXML) = 0;
	virtual bool Load(CXMLProcessor *pXML) = 0;
};

class CLanguageSettings : public ISettings
{
public:
	TCHAR m_szLanguageFile[MAX_PATH];
	CLNGProcessor *m_pLNGProcessor;

	CLanguageSettings()
	{
		m_szLanguageFile[0] = '\0';
		m_pLNGProcessor = NULL;
	}

	~CLanguageSettings()
	{
		if (m_pLNGProcessor != NULL)
		{
			delete m_pLNGProcessor;
			m_pLNGProcessor = NULL;
		}
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CGlobalSettings : public ISettings
{
public:
	bool m_bAutoRunCheck;
	bool m_bAutoCheckBus;
	bool m_bLog;
	bool m_bRememberShell;
	bool m_bCopyWarning;		// Display the warning message about audio discs when trying to copy a disc.
	bool m_bRawImageInfo;		// Display information about raw write methods when writing a "raw" disc image.
	bool m_bCharSetWarning;		// Display the warning if the character set could not be recognized.
	bool m_bWriteSpeedWarning;	// Display a warning when changing the write speed of a recorder.
	bool m_bCodecWarning;		// Display a warning when a codec failed to load.
	bool m_bFixateWarning;		// Display a warning when trying to disable disc fixation.
	bool m_bSmoke;
	int m_iGraceTime;
	int m_iFIFOSize;

	TCHAR m_szTempPath[MAX_PATH];

	// Shell extension.
	bool m_bShellExtSubMenu;
	bool m_bShellExtIcon;

	std::list<tstring> m_szShellExt;

	// For internal use only (at this moment).
	TCHAR m_szCDRToolsPath[MAX_PATH];
	TCHAR m_szCDRToolsPathCyg[MAX_PATH + 12];

	CGlobalSettings()
	{
		m_bAutoRunCheck = true;
		m_bAutoCheckBus = true;
		m_bLog = true;
		m_bRememberShell = true;
		m_bCopyWarning = true;
		m_bRawImageInfo = true;
		m_bCharSetWarning = true;
		m_bWriteSpeedWarning = true;
		m_bCodecWarning = true;
		m_bFixateWarning = true;
		m_bSmoke = true;
		m_iGraceTime = 5;			// Five seconds by default.
		m_iFIFOSize = 4;			// 4 MiB by default.

		// Path to system temporary directory.
		m_szTempPath[0] = '\0';
		GetTempPath(MAX_PATH - 1,m_szTempPath);

		// Shell extension.
		m_bShellExtSubMenu = false;
		m_bShellExtIcon = true;

		m_szShellExt.push_back(_T("Disc Images|.iso, .cue"));
		m_szShellExt.push_back(_T("Raw Images|.bin, .raw"));
		m_szShellExt.push_back(_T("InfraRecorder Projects|.irp"));

		// Path to cdrtools.
		GetModuleFileName(NULL,m_szCDRToolsPath,MAX_PATH - 1);
		ExtractFilePath(m_szCDRToolsPath);

#ifdef CDRKIT
		lstrcat(m_szCDRToolsPath,_T("cdrkit\\"));
#else
		lstrcat(m_szCDRToolsPath,_T("cdrtools\\"));
#endif

		// Cygwin version of the path.
		GetCygwinFileName(m_szCDRToolsPath,m_szCDRToolsPathCyg);
	}

	~CGlobalSettings()
	{
		// Shell extension.
		m_szShellExt.clear();
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CDynamicSettings : public ISettings
{
public:
	int m_iPrjListViewStyle;
	int m_iToolBarText;
	int m_iToolBarIcon;
	bool m_bViewToolBar;
	bool m_bViewStatusBar;
	bool m_bWinMaximized;

	// Window rectangle.
	RECT m_rcWindow;

	// Contains the active path displayed in the explorer view.
	TCHAR m_szShellDir[MAX_PATH];

	CDynamicSettings()
	{
		m_iPrjListViewStyle = LISTVIEWSTYLE_DETAILS;
		m_iToolBarText = TOOLBAR_TEXT_DONTSHOW;
		m_iToolBarIcon = TOOLBAR_ICON_SMALL;
		m_bViewToolBar = true;
		m_bViewStatusBar = true;
		m_bWinMaximized = false;

		// -1 means not set.
		m_rcWindow.left = -1;
		m_rcWindow.right = -1;
		m_rcWindow.top = -1;
		m_rcWindow.bottom = -1;

		// The default shell path should be the desktop.
		if (!SHGetSpecialFolderPath(NULL,m_szShellDir,CSIDL_DESKTOP,false))
			m_szShellDir[0] = '\0';
	}

	~CDynamicSettings()
	{
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
	void Apply();
};

class CEraseSettings : public ISettings
{
public:
	int m_iMode;
	bool m_bForce;
	bool m_bEject;
	bool m_bSimulate;

	// For internal use only, should never be saved.
	INT_PTR m_iRecorder;
	unsigned int m_uiSpeed;

	CEraseSettings()
	{
		m_iMode = 1;
		m_bForce = false;
		m_bEject = true;
		m_bSimulate = false;

		m_uiSpeed = 0xFFFFFFFF;		// Maximum.
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CFixateSettings : public ISettings
{
public:
	bool m_bEject;
	bool m_bSimulate;

	// For internal use only, should never be saved.
	INT_PTR m_iRecorder;

	CFixateSettings()
	{
		m_bEject = true;
		m_bSimulate = false;
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CBurnImageSettings : public ISettings
{
public:
	bool m_bOnFly;
	bool m_bVerify;
	bool m_bEject;
	bool m_bSimulate;
	bool m_bBUP;
	bool m_bPadTracks;
	bool m_bFixate;

	// For internal use only, should never be saved.
	UINT_PTR m_iRecorder;
	int m_iWriteMethod;
	INT_PTR m_uiWriteSpeed;	// Multiple of the audio speed. -1 = maximum speed.

	CBurnImageSettings()
	{
		m_bOnFly = false;
		m_bVerify = false;
		m_bEject = true;
		m_bSimulate = false;
		m_bBUP = true;
		m_bPadTracks = true;
		m_bFixate = true;
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

// Class used to destribe a project boot image.
#define PROJECTBI_BOOTEMU_NONE				0
#define PROJECTBI_BOOTEMU_FLOPPY			1
#define PROJECTBI_BOOTEMU_HARDDISK			2

class CProjectBootImage	// FIXME: Make m_iLoadSegment and m_iLoadSize unsigned short, m_iEmulation to enum type and m_iLoadSize to LoadLen (sectors).
{
public:
	bool m_bNoBoot;
	int m_iEmulation;
	int m_iLoadSegment;
	int m_iLoadSize;
	ckcore::tstring m_FullPath;			// Full path to the file on the harddrive.
	ckcore::tstring m_LocalName;		// Internal name.

	CProjectBootImage()
	{
		m_bNoBoot = false;
		m_iEmulation = PROJECTBI_BOOTEMU_FLOPPY;
		m_iLoadSegment = 0x7c0;
		m_iLoadSize = 4;
	}
};

class CProjectSettings : public ISettings
{
public:
	int m_iFileSystem;
	int m_iIsoLevel;
	int m_iIsoFormat;	// FIXME: Use defined values here. Rename?
	bool m_bJoliet;
	bool m_bJolietLongNames;
	bool m_bOmitVerNum;
	bool m_bDeepDirs;
	TCHAR m_szLabel[128];
	TCHAR m_szPublisher[128];
	TCHAR m_szPreparer[128];
	TCHAR m_szSystem[128];
	TCHAR m_szVolumeSet[128];
	TCHAR m_szCopyright[37];
	TCHAR m_szAbstract[37];
	TCHAR m_szBibliographic[37];

	TCHAR m_szAlbumName[160];
	TCHAR m_szAlbumArtist[160];

	// Boot information.
	std::list<CProjectBootImage *> m_BootImages;

	// Multi session related, are only changed when importing an existing session.
	// They should not be saved in a project.
	bool m_bMultiSession;
	unsigned __int64 m_uiImportTrackAddr;
	unsigned __int64 m_uiImportTrackLen;
	unsigned __int64 m_uiNextWritableAddr;
	UINT_PTR m_uiDeviceIndex;

	CProjectSettings()
	{
		// We don't want to display an error message if the code page is not
		// found two times. First when the object enters scope and later when
		// the new project is created (from the NewDataProject function).
		Reset();
	}

	~CProjectSettings()
	{
		// Free any boot image information.
		std::list <CProjectBootImage *>::iterator itImageObject;
		for (itImageObject = m_BootImages.begin(); itImageObject != m_BootImages.end(); itImageObject++)
			delete *itImageObject;

		m_BootImages.clear();
	}

	/*
		CProjectSettings::GetBootImage
		------------------------------
		Returns the CProjectBootImage object that is associated with the specified
		file name (full local file name of a file on the hard disk).
	*/
	CProjectBootImage *GetBootImage(const TCHAR *szFileName)
	{
		std::list <CProjectBootImage *>::iterator itImageObject;
		for (itImageObject = m_BootImages.begin(); itImageObject != m_BootImages.end(); itImageObject++)
		{
			if (!ComparePaths((*itImageObject)->m_FullPath.c_str(),szFileName))
				return *itImageObject;
		}

		return NULL;
	}

	void Reset()
	{
		m_iFileSystem = FILESYSTEM_ISO9660;
		m_iIsoLevel = 0;
		m_iIsoFormat = 0;
		m_bJoliet = true;
		m_bJolietLongNames = true;
		m_bOmitVerNum = false;
		m_bDeepDirs = false;

		m_szLabel[0] = '\0';
		m_szPublisher[0] = '\0';
		m_szPreparer[0] = '\0';
		m_szSystem[0] = '\0';
		m_szVolumeSet[0] = '\0';
		m_szCopyright[0] = '\0';
		m_szAbstract[0] = '\0';
		m_szBibliographic[0] = '\0';

		m_szAlbumName[0] = '\0';
		m_szAlbumArtist[0] = '\0';

		std::list <CProjectBootImage *>::iterator itImageObject;
		for (itImageObject = m_BootImages.begin(); itImageObject != m_BootImages.end(); itImageObject++)
			delete *itImageObject;

		m_BootImages.clear();

		// Multi session related, are only changed when importing an existing session.
		m_bMultiSession = false;
		m_uiImportTrackAddr = 0;
		m_uiImportTrackLen = 0;
		m_uiNextWritableAddr = 0;
		m_uiDeviceIndex = 0;
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CCopyDiscSettings : public ISettings
{
public:
	bool m_bOnFly;
	bool m_bClone;

	// For internal use only, should never be saved.
	INT_PTR m_iSource;
	INT_PTR m_iTarget;

	// Size of the source drive media in sectors.
	unsigned __int64 m_uiSourceSize;

	CCopyDiscSettings()
	{
		m_bOnFly = false;
		m_bClone = true;

		m_uiSourceSize = 0;
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CBurnAdvancedSettings : public ISettings
{
public:
	bool m_bOverburn;
	bool m_bSwab;
	bool m_bIgnoreSize;
	bool m_bImmed;
	bool m_bAudioMaster;
	bool m_bForceSpeed;
	bool m_bVariRec;
	int m_iVariRec;

	CBurnAdvancedSettings()
	{
		m_bOverburn = false;
		m_bSwab = false;
		m_bIgnoreSize = false;
		m_bImmed = false;
		m_bAudioMaster = true;
		m_bForceSpeed = false;
		m_bVariRec = false;
		m_iVariRec = 0;
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CSaveTracksSettings : public ISettings
{
public:
	TCHAR m_szTarget[MAX_PATH];

	CSaveTracksSettings()
	{
		m_szTarget[0] = '\0';
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

class CReadSettings : public ISettings
{
public:
	bool m_bIgnoreErr;
	bool m_bClone;

	// For internal use only, should never be saved.
	INT_PTR m_iReadSpeed;		// Read speed (audio multiple).

	CReadSettings()
	{
		m_bIgnoreErr = false;
		m_bClone = false;
		m_iReadSpeed = -1;	// -1 = Maximum.
	}

	bool Save(CXMLProcessor *pXML);
	bool Load(CXMLProcessor *pXML);
};

extern CLanguageSettings g_LanguageSettings;
extern CGlobalSettings g_GlobalSettings;
extern CDynamicSettings g_DynamicSettings;
extern CEraseSettings g_EraseSettings;
extern CFixateSettings g_FixateSettings;
extern CBurnImageSettings g_BurnImageSettings;
extern CProjectSettings g_ProjectSettings;
extern CCopyDiscSettings g_CopyDiscSettings;
extern CBurnAdvancedSettings g_BurnAdvancedSettings;
extern CSaveTracksSettings g_SaveTracksSettings;
extern CReadSettings g_ReadSettings;
