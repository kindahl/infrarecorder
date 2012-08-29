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

#include "stdafx.hh"
#include <ckcore/crcstream.hh>
#include <ckcore/filestream.hh>
#include <base/graph_util.hh>
#include <base/string_util.hh>
#include "resource.h"
#include "settings.hh"
#include "infrarecorder.hh"
#include "version.hh"
#include "core.hh"
#include "about_window.hh"

#define ABOUT_YEARS			_T("2006-2012")
#define ABOUT_STR(str)		str,(sizeof(str)/sizeof(TCHAR))-1

/*
 * Defines the supported cdrtools versions. These versions are determined by
 * the CRC-32 checksum of cdrecord.exe.
 */
typedef struct
{
    ckcore::tuint32 uiChecksum;
    const TCHAR *szVersion;
} tCdrtoolsChecksumVer;

size_t g_CdrtoolsChecksumVerCount = 1;
tCdrtoolsChecksumVer g_CdrtoolsChecksumVer[1] =
{
    { 0xd47a1004,_T("2.01.01.a61") }
};

CAboutWindow::CAboutWindow() :
    m_VerFont(NULL),m_UrlFont(NULL),m_bUrlHover(false),
    m_hWndParent(NULL),
    m_pUpdateLayeredWindow(NULL)
{
    m_VerFont = AtlCreateBoldFont(AtlGetDefaultGuiFont());
    
    LOGFONT lf = { 0 };
    if (::GetObject(AtlGetDefaultGuiFont(),sizeof(LOGFONT),&lf) == sizeof(LOGFONT))
    {
        lf.lfUnderline = 1;
        m_UrlFont = ::CreateFontIndirect(&lf);
    }

    if (m_UrlFont == NULL)
        m_UrlFont = AtlGetDefaultGuiFont();

    // Load the function dynamically.
    HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
    m_pUpdateLayeredWindow = (tUpdateLayeredWindow)GetProcAddress(hUser32,"UpdateLayeredWindow");
}

CAboutWindow::~CAboutWindow()
{
    if (m_VerFont != AtlGetDefaultGuiFont())
        ::DeleteObject(m_VerFont);
    if (m_UrlFont != AtlGetDefaultGuiFont())
        ::DeleteObject(m_UrlFont);
}

void CAboutWindow::UpdateVersionInfo()
{
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

    unsigned long ulDummy;
    unsigned long ulDataSize = GetFileVersionInfoSize(szFileName,&ulDummy);

    if (ulDataSize != 0)
    {
        unsigned char *pBlock = new unsigned char[ulDataSize];

        struct LANGANDCODEPAGE
        {
            unsigned short usLanguage;
            unsigned short usCodePage;
        } *pTranslate;

        if (GetFileVersionInfo(szFileName,NULL,ulDataSize,pBlock) > 0)
        {
            // Get language information (only one language should be present).
            VerQueryValue(pBlock,_T("\\VarFileInfo\\Translation"),
                (LPVOID *)&pTranslate,(unsigned int *)&ulDummy);

            // Calculate the FileVersion sub block path.
            lsprintf(m_szVersion,_T("\\StringFileInfo\\%04x%04x\\FileVersion"),
                     pTranslate[0].usLanguage,pTranslate[0].usCodePage);

            unsigned char *pBuffer;
            VerQueryValue(pBlock,m_szVersion,(LPVOID *)&pBuffer,(unsigned int *)&ulDataSize);

            // Architecture.
#ifdef _M_IA64
            TCHAR *szArcStr = _T("IA64");
#elif defined _M_X64
            TCHAR *szArcStr = _T("x64");
#else
            TCHAR *szArcStr = _T("x86");
#endif

            // Character coding.
#ifdef PORTABLE
            lsprintf(m_szVersion,_T("Version %s portable (unicode, %s)"),
                (TCHAR *)pBuffer,szArcStr);
#else
            lsprintf(m_szVersion,_T("Version %s (unicode, %s)"),
                (TCHAR *)pBuffer,szArcStr);
#endif
        }

        delete [] pBlock;
    }

    // Update cdrtools version information.
    m_szCdrtoolsVersion = _T("Installed version: Unknown");

    ckcore::Path CdrecordPath = g_GlobalSettings.m_szCDRToolsPath;
    CdrecordPath += _T(CORE_WRITEAPP);
    
    ckcore::FileInStream CdrecordStream(CdrecordPath);
    if (!CdrecordStream.open())
        return;

    ckcore::CrcStream CrcStream(ckcore::CrcStream::ckCRC_32);

    if (!ckcore::stream::copy(CdrecordStream,CrcStream))
        return;

    for (size_t i = 0; i < g_CdrtoolsChecksumVerCount; i++)
    {
        if (g_CdrtoolsChecksumVer[i].uiChecksum == CrcStream.checksum())
        {
            m_szCdrtoolsVersion  = _T("Installed version: ");
            m_szCdrtoolsVersion += g_CdrtoolsChecksumVer[i].szVersion;
            m_szCdrtoolsVersion += _T(" (officially supported)");
            break;
        }
    }
}

LRESULT CAboutWindow::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    // Load a 32-bit transparent bitmap for Windows 2000 and newer systems.
    if (m_pUpdateLayeredWindow != NULL)
        LoadBitmaps();

    SIZE sSplashBitmap;
    m_SplashTmpBitmap.GetSize(sSplashBitmap);
    SetWindowPos(HWND_TOPMOST,0,0,sSplashBitmap.cx,sSplashBitmap.cy,SWP_NOMOVE);

    CenterWindow(HWND_DESKTOP);

    // For per-pixel alpha transparency the window needs to be layered.
    if (m_pUpdateLayeredWindow != NULL)
        ModifyStyleEx(0,WS_EX_LAYERED);

    Render();
    return 0;
}

void CAboutWindow::RollbackBitmap()
{
    BITMAP bmpDstInfo;
    m_SplashTmpBitmap.GetBitmap(&bmpDstInfo);

    BITMAP bmbRefInfo;
    m_SplashRefBitmap.GetBitmap(&bmbRefInfo);

    unsigned char *pDstDataBits = (unsigned char *)bmpDstInfo.bmBits;
    unsigned char *pSrcDataBits = (unsigned char *)bmbRefInfo.bmBits;

    for (int y = 0; y < bmpDstInfo.bmHeight; y++)
    {
        unsigned char *pDstPixel = pDstDataBits + bmpDstInfo.bmWidth * 4 * y;
        unsigned char *pSrcPixel = pSrcDataBits + bmbRefInfo.bmWidth * 4 * y;

        for (int x = 0; x < bmpDstInfo.bmWidth; x++)
        {
            pDstPixel[0] = pSrcPixel[0];
            pDstPixel[1] = pSrcPixel[1];
            pDstPixel[2] = pSrcPixel[2];
            pDstPixel[3] = pSrcPixel[3];

            pDstPixel += 4;
            pSrcPixel += 4;
        }
    }
}

void CAboutWindow::Render()
{
    InvalidateRect(NULL);

    // Process the message queue.
    ProcessMessages();
}

LRESULT CAboutWindow::OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    CPaintDC dc(m_hWnd);

    if (m_pUpdateLayeredWindow == NULL)
        return 0;

    RECT rcClient;
    GetClientRect(&rcClient);

    // FIXME: I have a good feeling that there is a much more efficient way to
    //        achieve the requested behavior without manually copying the
    //		  reference bitmap.
    RollbackBitmap();

    HDC hMemDC = CreateCompatibleDC(dc);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC,m_SplashTmpBitmap);

    RECT rcVersion  = { 37,115,390,145 };
    DrawText(hMemDC,m_VerFont,&rcVersion,ABOUTWINDOW_TEXTCOLOR,
             m_szVersion,lstrlen(m_szVersion));

    RECT rcCopyright = { 37,145,390,175 };
    DrawText(hMemDC,AtlGetDefaultGuiFont(),&rcCopyright,ABOUTWINDOW_TEXTCOLOR,
             ABOUT_STR(_T("Copyright © ") ABOUT_YEARS _T(" Christian Kindahl.")));

    RECT rcLegal = { 37,175,390,300 };
    DrawText(hMemDC,AtlGetDefaultGuiFont(),&rcLegal,ABOUTWINDOW_TEXTCOLOR,
             ABOUT_STR(_T("InfraRecorder comes with absolutely no warranty. ")
                       _T("This is free software, and you are welcome to ")
                       _T("redistribute it under certain conditions; please ")
                       _T("see License.txt for more information.")));

    RECT rcThanks = { 37,230,390,260 };
    DrawText(hMemDC,AtlGetDefaultGuiFont(),&rcThanks,ABOUTWINDOW_TEXTCOLOR,
             ABOUT_STR(_T("Special thanks to R. Diez for his contributions.")));

    RECT rcCdrtools = { 37,260,390,320 };
    DrawText(hMemDC,AtlGetDefaultGuiFont(),&rcCdrtools,ABOUTWINDOW_TEXTCOLOR,
             ABOUT_STR(_T("InfraRecorder uses cdrecord, readcd and cdda2wav from ")
                       _T("the cdrtools software suite. cdrtools copyright © ")
                       _T("1995-2009 Jörg Schilling.")));

    RECT rcCdrtoolsVer = { 37,300,390,330 };
    DrawText(hMemDC,AtlGetDefaultGuiFont(),&rcCdrtoolsVer,ABOUTWINDOW_TEXTCOLOR,
             m_szCdrtoolsVersion.c_str(),static_cast<unsigned int>(m_szCdrtoolsVersion.size()));

    RECT rcUrl = { ABOUTWINDOW_URL_LEFT,ABOUTWINDOW_URL_TOP,
                   ABOUTWINDOW_URL_RIGHT,ABOUTWINDOW_URL_BOTTOM };
    DrawText(hMemDC,m_bUrlHover ? m_UrlFont : AtlGetDefaultGuiFont(),
             &rcUrl,ABOUTWINDOW_URLCOLOR,
             ABOUT_STR(_T("http://infrarecorder.org")));

    DrawBitmap(dc,hMemDC);

    SelectObject(hMemDC,hOldBitmap);

    ReleaseDC(dc);
    ReleaseDC(hMemDC);

    return 0;
}

void CAboutWindow::DrawBitmap(HDC hScreenDC,HDC hMemDC)
{
    SIZE sSplashBitmap;
    m_SplashTmpBitmap.GetSize(sSplashBitmap);

    // Calculate window dimensions.
    RECT rcWindow;
    GetWindowRect(&rcWindow);

    POINT ptWindowPos;
    ptWindowPos.x = rcWindow.left;
    ptWindowPos.y = rcWindow.top;

    BLENDFUNCTION bfPixelFunction = { AC_SRC_OVER,0,255,AC_SRC_ALPHA };
    POINT ptSource = { 0,0 };

    m_pUpdateLayeredWindow(m_hWnd,hScreenDC,&ptWindowPos,&sSplashBitmap,
                           hMemDC,&ptSource,0,&bfPixelFunction,ULW_ALPHA);
}


void CAboutWindow::DrawText(HDC hDC,HFONT hFont,RECT *pRect,
                            COLORREF Color,const TCHAR *szText,
                            int iTextLen)
{
    HFONT hOldFont = (HFONT)SelectObject(hDC,hFont);

    ::SetBkMode(hDC,TRANSPARENT);
    ::SetTextColor(hDC,Color);

    ::DrawText(hDC,szText,iTextLen,pRect,
        DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);

    SelectObject(hDC,hOldFont);
    
    // Get bitmap information.
    BITMAP bmpInfo;
    m_SplashTmpBitmap.GetBitmap(&bmpInfo);

    // Since the regular GDI functions (with a few exceptions) clear the alpha bit
    // when they are used we need to set it, since we don't want to draw
    // transparent text.
    unsigned char *pDataBits = (unsigned char *)bmpInfo.bmBits;

    int iStart = bmpInfo.bmHeight - pRect->bottom;
    int iEnd = bmpInfo.bmHeight - pRect->top;

    for (int y = iStart; y < iEnd; y++)
    {
        unsigned char *pPixel = pDataBits + bmpInfo.bmWidth * 4 * y;

        pPixel += 4 * pRect->left;

        for (int x = pRect->left; x < pRect->right; x++)
        {
            pPixel[3] = 0xFF;
            pPixel += 4;
        }
    }
}

/**
 * Utility function for loading the layered alpha bitmap into the specified
 * bitmap object.
 * @param [out] Bitmap The bitmap object to load the image into.
 */
void CAboutWindow::LoadBitmap(CBitmap &Bitmap)
{
    // Load the bitmap.
    HBITMAP hBitmap = (HBITMAP)LoadImage(_Module.GetModuleInstance(),MAKEINTRESOURCE(IDB_ABOUTBITMAP),
        IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
    Bitmap.Attach(hBitmap);

    // Precaclulate multiply the transparency.
    BITMAP bmpInfo;
    Bitmap.GetBitmap(&bmpInfo);

    unsigned char *pDataBits = (unsigned char *)bmpInfo.bmBits;

    for (int y = 0; y < bmpInfo.bmHeight; y++)
    {
        unsigned char *pPixel = pDataBits + bmpInfo.bmWidth * 4 * y;

        for (int x = 0; x < bmpInfo.bmWidth; x++)
        {
            pPixel[0] = pPixel[0] * pPixel[3] / 255;
            pPixel[1] = pPixel[1] * pPixel[3] / 255;
            pPixel[2] = pPixel[2] * pPixel[3] / 255;

            pPixel += 4;
        }
    }
}

void CAboutWindow::LoadBitmaps()
{
    LoadBitmap(m_SplashTmpBitmap);
    LoadBitmap(m_SplashRefBitmap);
}

LRESULT CAboutWindow::OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    int iPosX = GET_X_LPARAM(lParam); 
    int iPosY = GET_Y_LPARAM(lParam);

    if (iPosX > ABOUTWINDOW_URL_LEFT && iPosX < ABOUTWINDOW_URL_RIGHT &&
        iPosY > ABOUTWINDOW_URL_TOP && iPosY < ABOUTWINDOW_URL_BOTTOM)
    {
        if (g_WinVer.m_ulMajorVersion >= MAJOR_WIN2000 &&
            GetCursor() != LoadCursor(NULL,IDC_HAND))
        {
            SetCursor(LoadCursor(NULL,IDC_HAND));
        }

        bool bRender = !m_bUrlHover;
        m_bUrlHover = true;

        if (bRender)
            Render();
    }
    else
    {
        if (GetCursor() != LoadCursor(NULL,IDC_ARROW))
            SetCursor(LoadCursor(NULL,IDC_ARROW));

        bool bRender = m_bUrlHover;
        m_bUrlHover = false;

        if (bRender)
            Render();
    }

    return 0;
}

LRESULT CAboutWindow::OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
    int iPosX = GET_X_LPARAM(lParam); 
    int iPosY = GET_Y_LPARAM(lParam);

    if (iPosX > ABOUTWINDOW_URL_LEFT && iPosX < ABOUTWINDOW_URL_RIGHT &&
        iPosY > ABOUTWINDOW_URL_TOP && iPosY < ABOUTWINDOW_URL_BOTTOM)
    {
        ::ShellExecute(m_hWnd,_T("open"),_T("http://infrarecorder.org"),
                       NULL,NULL,SW_SHOW);
    }

    // Re-enable the parent window.
    if (m_hWndParent != NULL)
        ::EnableWindow(m_hWndParent,TRUE);

    DestroyWindow();
    return 0;
}

void CAboutWindow::CreateAndShow(HWND hWndParent)
{
    UpdateVersionInfo();

    // Windows 2000+ users may enjoy a real about Window while the others will
    // have a simple message box for compatibility.
    if (m_pUpdateLayeredWindow != NULL)
    {
        // Disable the parent window.
        m_hWndParent = hWndParent;
        ::EnableWindow(m_hWndParent,FALSE);

        Create(hWndParent,CWindow::rcDefault);  
        ShowWindow(true);
        Render();
    }
    else
    {
        ckcore::tstring Message = m_szVersion;
        Message += _T(" copyright © ") ABOUT_YEARS _T(" Christian Kindahl.\n\n");
        Message += _T("Please visit http://infrarecorder.org for more information.");
        ::MessageBox(hWndParent,Message.c_str(),_T("About InfraRecorder"),
                     MB_OK | MB_ICONINFORMATION);
    }
}