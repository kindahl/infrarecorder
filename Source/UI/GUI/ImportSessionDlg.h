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

#pragma once
#include <vector>
#include "resource.h"
#include "Core2Device.h"

class CImportSessionDlg : public CDialogImpl<CImportSessionDlg>
{
private:
	class CTrackData
	{
	public:
		unsigned char m_ucMode;
		unsigned short m_usSessionNumber;
		unsigned short m_usTrackNumber;
		unsigned long m_ulTrackAddr;
		unsigned long m_ulTrackLen;

		CTrackData(unsigned char ucMode,unsigned short usSessionNumber,
			unsigned short usTrackNumber,unsigned long ulTrackAddr,unsigned long ulTrackLen) :
			m_ucMode(ucMode),m_usSessionNumber(usSessionNumber),m_usTrackNumber(usTrackNumber),
			m_ulTrackAddr(ulTrackAddr),m_ulTrackLen(ulTrackLen)
		{
		}
	};

	std::vector<CTrackData *> m_TrackData;
	CComboBox m_DeviceCombo;
	CComboBox m_TrackCombo;

	bool Translate();
	bool UpdateDiscInfo(CCore2Device *pDevice);

	void ResetDiscInfo();

public:
	enum { IDD = IDD_IMPORTSESSIONDLG };

	// Data members that can be accessed from another object when the dialog
	// has closed.
	unsigned __int64 m_uiAllocatedSize;
	UINT_PTR m_uiDeviceIndex;
	CTrackData *m_pSelTrackData;

	CImportSessionDlg();
	~CImportSessionDlg();

	BEGIN_MSG_MAP(CImportSessionDlg)
		MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)
		COMMAND_HANDLER(IDC_DEVICECOMBO,CBN_SELCHANGE,OnDeviceChange)

		COMMAND_ID_HANDLER(IDOK,OnOK)
		COMMAND_ID_HANDLER(IDCANCEL,OnCancel)

		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
	LRESULT OnDeviceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

	LRESULT OnOK(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
	LRESULT OnCancel(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
};
