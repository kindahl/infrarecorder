/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "resource.h"

#define CONFIG_MODE_QUALITY				0
#define CONFIG_MODE_BITRATE				1
#define CONFIG_MODE_VARBITRATE			2
#define CONFIG_MODE_AVBITRATE			3

class CEncoderConfig
{
public:
    CEncoderConfig()
    {
        m_iMode = CONFIG_MODE_QUALITY;
        m_iQuality = 50;

        m_iBitrate = 192;
        m_iMinBitrate = 128;
        m_iMaxBitrate = 256;
        m_iAvBitrate = 192;
    }

    int m_iMode;
    int m_iQuality;
    int m_iBitrate;
    int m_iMinBitrate;
    int m_iMaxBitrate;
    int m_iAvBitrate;
};

class CConfigGeneralPage : public CPropertyPageImpl<CConfigGeneralPage>
{
private:
    CTrackBarCtrl m_QualityTrackBar;

    bool ValidateBitrateValue(int iControl,int &iValue);
    void SelectMode(int iMode);

    CEncoderConfig *m_pConfig;

public:
    enum { IDD = IDD_PROPPAGE_CONFIGGENERAL };

    CConfigGeneralPage();
    ~CConfigGeneralPage();

    bool SetConfig(CEncoderConfig *pConfig);
    bool OnApply();

    BEGIN_MSG_MAP(CConfigGeneralPage)
        MESSAGE_HANDLER(WM_INITDIALOG,OnInitDialog)

        COMMAND_ID_HANDLER(IDC_QUALITYRADIO,OnQualityCheck)
        COMMAND_ID_HANDLER(IDC_BITRATERADIO,OnBitrateCheck)
        COMMAND_ID_HANDLER(IDC_VARBITRATERADIO,OnVarBitrateCheck)
        COMMAND_ID_HANDLER(IDC_AVBITRATERADIO,OnAvBitrateCheck)
        NOTIFY_HANDLER(IDC_BITRATESPIN,UDN_DELTAPOS,OnBitrateSpin)
        NOTIFY_HANDLER(IDC_MINBITRATESPIN,UDN_DELTAPOS,OnBitrateSpin)
        NOTIFY_HANDLER(IDC_MAXBITRATESPIN,UDN_DELTAPOS,OnBitrateSpin)
        NOTIFY_HANDLER(IDC_AVBITRATESPIN,UDN_DELTAPOS,OnBitrateSpin)

        CHAIN_MSG_MAP(CPropertyPageImpl<CConfigGeneralPage>)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

    LRESULT OnQualityCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
    LRESULT OnBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
    LRESULT OnVarBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);
    LRESULT OnAvBitrateCheck(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled);

    LRESULT OnBitrateSpin(int idCtrl,LPNMHDR pNMH,BOOL &bHandled);
};
