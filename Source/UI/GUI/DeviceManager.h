/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "ConsolePipe.h"
#include "Core2Device.h"

#define DEVICEMANAGER_MAXDEVICES			128
#define DEVICEMANAGER_MAXWRITESPEEDS		128

// Device types (scsitransp.c).
#define DEVICEMANAGER_TYPE_UNKNOWN			-1
#define DEVICEMANAGER_TYPE_CDROM			0
#define DEVICEMANAGER_TYPE_COMMUNICATION	1
#define DEVICEMANAGER_TYPE_DISC				2
#define DEVICEMANAGER_TYPE_JUKEBOX			3
#define DEVICEMANAGER_TYPE_OPTICALSTORAGE	4
#define DEVICEMANAGER_TYPE_PRINTER			5
#define DEVICEMANAGER_TYPE_PROCESSOR		6
#define DEVICEMANAGER_TYPE_SCANNER			7
#define DEVICEMANAGER_TYPE_TAPE				8
#define DEVICEMANAGER_TYPE_WORM				9

// Media capabilities flags.
#define DEVICEMANAGER_CAP_READCDR			1
#define DEVICEMANAGER_CAP_WRITECDR			2
#define DEVICEMANAGER_CAP_READCDRW			4
#define DEVICEMANAGER_CAP_WRITECDRW			8
#define DEVICEMANAGER_CAP_READDVDROM		16
#define DEVICEMANAGER_CAP_READDVDR			32
#define DEVICEMANAGER_CAP_WRITEDVDR			64
#define DEVICEMANAGER_CAP_READDVDRAM		128
#define DEVICEMANAGER_CAP_WRITEDVDRAM		256
// General capabilities flags.
#define DEVICEMANAGER_CAP_READMODE2FORM1	1
#define DEVICEMANAGER_CAP_READMODE2FORM2	2
#define DEVICEMANAGER_CAP_READDIGITALAUDIO	4
#define DEVICEMANAGER_CAP_READMULTISESSION	8
#define DEVICEMANAGER_CAP_READFIXEDPACKET	16
#define DEVICEMANAGER_CAP_READCDBARCODE		32
#define DEVICEMANAGER_CAP_READRWSUBCODE		64
#define DEVICEMANAGER_CAP_READRAWPWSUBCODE	128
#define DEVICEMANAGER_CAP_TESTWRITING		256
#define DEVICEMANAGER_CAP_BUFRECORDING		512
#define DEVICEMANAGER_CAP_C2EP				1024
#define DEVICEMANAGER_CAP_EJECTCDSS			2048
#define DEVICEMANAGER_CAP_CHANGEDISCSIDE	4096
#define DEVICEMANAGER_CAP_INDIVIDUALDP		8192
#define DEVICEMANAGER_CAP_RETURNCDCN		16384
#define DEVICEMANAGER_CAP_RETURNCDISRC		32768
#define DEVICEMANAGER_CAP_DELIVERCOMPOSITE	65536
#define DEVICEMANAGER_CAP_PLAYAUDIOCD		131072
#define DEVICEMANAGER_CAP_HASLESIC			262144
#define DEVICEMANAGER_CAP_LMOPU				524288
#define DEVICEMANAGER_CAP_ALLOWML			1048576
// Digital audio capabilities flags.
#define DEVICEMANAGER_CAP_RESTARTNSDARA		1
// RW capabilities flags.
#define DEVICEMANAGER_CAP_RETURNRWSUBCODE	1
// Audio capabilities flags.
#define DEVICEMANAGER_CAP_INDIVIDUALVC		1
#define DEVICEMANAGER_CAP_INDEPENDENTMUTE	2
#define DEVICEMANAGER_CAP_DOPORT1			4
#define DEVICEMANAGER_CAP_DOPORT2			8
// Digital output capabilities flags.
#define DEVICEMANAGER_CAP_DOSENDDIGDAT		1
#define DEVICEMANAGER_CAP_DOSETLRCK			2
#define DEVICEMANAGER_CAP_HASVALIDDATA		4

// Loading mechanism types.
#define DEVICEMANAGER_LMT_TRAY				0
#define DEVICEMANAGER_LMT_CADDY				1

// Rotational controls.
#define DEVICEMANAGER_ROTCTRL_CLVPCAV		0
#define DEVICEMANAGER_ROTCTRL_CAV			1
#define DEVICEMANAGER_ROTCTRL_RESERVED2		2
#define DEVICEMANAGER_ROTCTRL_RESERVED3		3

// Digital audio block length.
#define DEVICEMANAGER_DABL_32				0
#define DEVICEMANAGER_DABL_16				1
#define DEVICEMANAGER_DABL_24				2
#define DEVICEMANAGER_DABL_24I2S			3

//#define DEVICEMANAGER_COLLECTWRITESPEEDS

typedef struct
{
	bool bRemovable;
	//int iBus,iTarget,iLun;
	int iType;
	TCHAR szVendor[9];
	TCHAR szIdentification[17];
	TCHAR szRevision[5];
	//TCHAR szDriveLetter;

	CCore2DeviceAddress Address;
} tDeviceInfo;

typedef struct
{
	char szWriteFlags[CONSOLEPIPE_MAX_LINE_SIZE];
	char szWriteModes[CONSOLEPIPE_MAX_LINE_SIZE];
} tDeviceInfoEx;

typedef struct
{
	// This two members can be interpreted for any drive type.
	unsigned int uiMedia;
	unsigned int uiGeneral;

	// The following members are only valid if special bits are set in the members above.
	unsigned int uiDigitalAudio;	// Only used if DEVICEMANAGER_CAP_READDIGITALAUDIO (uiGeneral) is set.
	unsigned int uiRW;				// Only used if DEVICEMANAGER_CAP_READRWSUBCODE (uiGeneral) is set.
	unsigned int uiAudio;			// Only used if DEVICEMANAGER_CAP_PLAYAUDIOCD (uiGeneral) is set.
	unsigned int uiDigitalOutput;	// Only used if DEVICEMANAGER_CAP_DOPORT1 or DEVICEMANAGER_CAP_DOPORT2 (uiAudio) is set.

	unsigned int uiNumVolLevels;	// Only used if DEVICEMANAGER_CAP_PLAYAUDIOCD (uiGeneral) is set.
	unsigned int uiBufferSize;
	unsigned int uiCopyRevision;
	
	unsigned char ucLoadType;		// Tray or caddy.
	unsigned char ucRotCtrl;

	unsigned char ucDigitalAudioBL;	// Only used if DEVICEMANAGER_CAP_DOPORT1 or DEVICEMANAGER_CAP_DOPORT2 (uiAudio) is set.

	unsigned int uiMaxSpeeds[6];	// Write (kB, CD, DVD), Read (kB, CD, DVD).
	unsigned int uiCurSpeeds[6];	// Write (kB, CD, DVD), Read (kB, CD, DVD).

#ifdef DEVICEMANAGER_COLLECTWRITESPEEDS
	unsigned int uiWriteSpeeds[DEVICEMANAGER_MAXWRITESPEEDS][3];
	unsigned int uiWriteSpeedsCount;
#endif
} tDeviceCap;

class CDeviceManager : public CConsolePipe
{
private:
	int m_iMode;

	tDeviceInfo m_DeviceInfo[DEVICEMANAGER_MAXDEVICES];
	unsigned int m_uiDeviceInfoCount;

	tDeviceInfoEx m_DeviceInfoEx[DEVICEMANAGER_MAXDEVICES];
	unsigned int m_uiDeviceInfoExCount;

	tDeviceCap m_DeviceCap[DEVICEMANAGER_MAXDEVICES];
	unsigned int m_uiDeviceCapCount;

	unsigned int m_uiVeriCounter;
	bool m_bVeriResult;

	int GetDeviceType(const TCHAR *szTypeString);

	// Inherited.
	void FlushOutput(const char *szBuffer);
	void ProcessEnded();

	void LoadCapParseDoes(const char *szBuffer,tDeviceCap *pDeviceCap);

	void ScanBusOutput(const char *szBuffer);
	void LoadCapOutput(const char *szBuffer);
	void LoadExInfoOutput(const char *szBuffer);
	void VerifyBusOutput(const char *szBuffer);

	bool GetConfigPath(TCHAR *szConfigPath);

	enum eMode
	{
		MODE_SCANBUS = 0,
		MODE_LOADCAP,
		MODE_LOADEXINFO,
		MODE_VERIFYBUS
	};

public:
	CDeviceManager();
	~CDeviceManager();

	void Reset();

	bool ScanBus();
	bool LoadCapabilities();
	bool LoadExInfo();

	unsigned int GetDeviceCount();
	tDeviceInfo *GetDeviceInfo(UINT_PTR uiIndex);
	tDeviceInfoEx *GetDeviceInfoEx(UINT_PTR uiIndex);
	tDeviceCap *GetDeviceCap(UINT_PTR uiIndex);

	bool IsDeviceReader(UINT_PTR uiIndex);
	bool IsDeviceRecorder(UINT_PTR uiIndex);
	bool IsDeviceDVDRecorder(UINT_PTR uiIndex);

	void GetDeviceName(tDeviceInfo *pDeviceInfo,TCHAR *szDeviceName);
	void GetDeviceAddr(tDeviceInfo *pDeviceInfo,TCHAR *szDeviceAddr);
	void GetDeviceAddrA(tDeviceInfo *pDeviceInfo,char *szDeviceAddr);

	/*void GetDeviceSpeeds(UINT_PTR uiIndex,CComboBox *pComboBox,
		bool bWriteSpeeds,bool bDVDSpeeds);*/

	// Save and load functions.
	bool SaveConfiguration();
	bool LoadConfiguration();
	bool VerifyConfiguration();
};

extern CDeviceManager g_DeviceManager;
