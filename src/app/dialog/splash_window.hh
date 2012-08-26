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
#include <ckmmc/devicemanager.hh>

#define SPLASHWINDOW_TEXTBKCOLOR				RGB(255,255,255)

#if (_WIN32_WINNT < 0x0500)
#define WS_EX_LAYERED							0x00080000
#define ULW_ALPHA								0x00000002
#endif

typedef BOOL (WINAPI *tUpdateLayeredWindow)(HWND hWnd,HDC hdcDst,POINT *pptDst,
                                            SIZE *psize,HDC hdcSrc,POINT *pptSrc,
                                            COLORREF crKey,BLENDFUNCTION *pblend,
                                            DWORD dwFlags);

class CSplashWindow : public CWindowImpl<CSplashWindow,CWindow,CWinTraits<WS_POPUP | WS_VISIBLE,WS_EX_TOOLWINDOW> >,
    public ckmmc::DeviceManager::ScanCallback
{
private:
    CBitmap m_SplashBitmap;

    HBRUSH m_hTextBkBrush;

    ckcore::tstring m_InfoText;

    tUpdateLayeredWindow m_pUpdateLayeredWindow;
    void DrawBitmap(HDC hScreenDC,HDC hMemDC);
    void DrawText(HDC hDC);
    void LoadBitmap();

    void SetInfoText(const TCHAR *szInfoText);

    /*
     * ckmmc::DeviceManager::ScanCallback interface.
     */
    void event_status(ckmmc::DeviceManager::ScanCallback::Status Status);
    bool event_device(ckmmc::Device::Address &Addr);

public:
    CSplashWindow();
    ~CSplashWindow();

    BEGIN_MSG_MAP(CSplashWindow)
        MESSAGE_HANDLER(WM_CREATE,OnCreate)
        MESSAGE_HANDLER(WM_PAINT,OnPaint)
    END_MSG_MAP()

    LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);
    LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled);

    void SafeCreate();
    void SafeDestroy();
};
