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
#include "Resource.h"
#include "main_frm.hh"
#include <ckcore/exception.hh>
#include <base/file_util.hh>
#include "progress_dlg.hh"
#include "simple_progress_dlg.hh"
#include "splash_window.hh"
#include "log_dlg.hh"
#include "settings.hh"
#include "settings_manager.hh"
#include "string_table.hh"
#include "lang_util.hh"
#include "action_manager.hh"
#include "temp_manager.hh"
#include "info_dlg.hh"
#include "about_window.hh"
#include "infrarecorder.hh"

CAppModule _Module;

// Global codec manager object.
CCodecManager g_CodecManager;

// Global device manager object.
ckmmc::DeviceManager g_DeviceManager;

// Global pointers to GUI objects owned by this file.
CLogDlg *g_pLogDlg = NULL;
CMainFrame *g_pMainFrame = NULL;
CProgressDlg *g_pProgressDlg = NULL;
CSimpleProgressDlg *g_pSimpleProgressDlg = NULL;
CAboutWindow *g_pAboutWnd = NULL;

#ifdef _DEBUG
const ckcore::tchar SAVE_ENGLISH_STRINGS_PARAM[] = _T("-englishstrings ");

static int SaveEnglishStrings(const TCHAR * const szFileName)
{
    try
    {
        const ckcore::tchar BLANKS[] = _T(" \t");
        ckcore::tstring FileName(szFileName);
        TrimStr(FileName,BLANKS);

        if (FileName.empty())
            throw ckcore::Exception2(_T("Missing filename."));

        ckcore::File File(FileName.c_str());

        if (!File.open(ckcore::File::ckOPEN_WRITE))
            throw ckcore::Exception2(_T("Error opening file for writing."));

#ifdef UNICODE
        // Write byte order mark.
        unsigned short usBOM = BOM_UTF32BE;
        File.write(&usBOM,2);
#endif
        const ckcore::tchar CRLF[] = ckT("\r\n");

        WriteString(File,CRLF);
        WriteString(File,ckT("[strings]"));
        WriteString(File,CRLF);

        for (unsigned i = 0; i < _countof(g_szStringTable); ++i)
        {
            const TCHAR * const szEnglishStr = g_szStringTable[i];

            if (szEnglishStr[0] == _T('\0'))
                continue;

            ckcore::tstring Str = ckcore::string::formatstr(_T("0x%04x=%s%s"),i,szEnglishStr,CRLF);
            WriteString(File,Str.c_str());
        }

        ATLVERIFY(true == File.close());
    }
    catch (const std::exception &e)
    {
        ckcore::rethrow_with_pfx(e,_T("Error processing command-line option %s: "),SAVE_ENGLISH_STRINGS_PARAM);
    }
    
    return 0;
}
#endif

void PerformDeviceScan()
{
    // Launch the splash window by the safe function because splash screens
    // are not supported on systems older than Windows 2000.
    CSplashWindow SplashWindow;
    SplashWindow.SafeCreate();

    g_DeviceManager.scan(&SplashWindow);

    SplashWindow.SafeDestroy();

    if (g_DeviceManager.devices().size() == 0)
    {
        if (g_GlobalSettings.m_bNoDevWarning)
        {
            CInfoDlg InfoDlg(&g_GlobalSettings.m_bNoDevWarning,
                             lngGetString(WARNING_NODEVICES),
                             INFODLG_NOCANCEL | INFODLG_ICONWARNING);
            InfoDlg.DoModal();
        }
    }
}

int Run(LPTSTR lpstrCmdLine = NULL,int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop MainLoop;
    _Module.AddMessageLoop(&MainLoop);

    if (g_pMainFrame->CreateEx() == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    // Load the last used window position and size.
    if (g_DynamicSettings.m_rcWindow.left != -1 &&
        g_DynamicSettings.m_rcWindow.right != -1 &&
        g_DynamicSettings.m_rcWindow.top != -1 &&
        g_DynamicSettings.m_rcWindow.bottom != -1)
    {
        if (g_pMainFrame->m_bDefaultWizard)
        {
            RECT rcWindow = g_DynamicSettings.m_rcWindow;
            rcWindow.right = rcWindow.left + 500;
            rcWindow.bottom = rcWindow.top + 400;

            g_pMainFrame->SetWindowPos(HWND_TOP,&rcWindow,0);
        }
        else
        {
            g_pMainFrame->SetWindowPos(HWND_TOP,&g_DynamicSettings.m_rcWindow,0);
        }
    }
    else
    {
        if (g_pMainFrame->m_bDefaultWizard)
        {
            RECT rcWindow = g_DynamicSettings.m_rcWindow;
            rcWindow.right = rcWindow.left + 500;
            rcWindow.bottom = rcWindow.top + 400;

            g_pMainFrame->SetWindowPos(HWND_TOP,&rcWindow,0);
        }

        g_pMainFrame->CenterWindow();
    }

    g_pMainFrame->ShowWindow(g_DynamicSettings.m_bWinMaximized ? SW_SHOWMAXIMIZED : nCmdShow);

    int nRet = MainLoop.Run();
        _Module.RemoveMessageLoop();
    return nRet;
}

INT_PTR ParseAndRun(LPTSTR lpstrCmdLine,int nCmdShow = SW_SHOWDEFAULT)
{
    g_pMainFrame->m_bDefaultWizard = g_GlobalSettings.m_bShowWizard;

    // FIXME: This is an absolutely horrible parameter parsing implementation.

    // Default project selection.
    if (!lstrncmp(lpstrCmdLine,_T("-project="),9))
    {
        lpstrCmdLine += 9;
        if (!lstrncmp(lpstrCmdLine,_T("data"),4))
        {
            g_pMainFrame->m_iDefaultProjType = PROJECTTYPE_DATA;
            g_pMainFrame->m_bDefaultWizard = false;

            lpstrCmdLine += 4;
        }
        else if (!lstrncmp(lpstrCmdLine,_T("audio"),5))
        {
            g_pMainFrame->m_iDefaultProjType = PROJECTTYPE_AUDIO;
            g_pMainFrame->m_bDefaultWizard = false;

            lpstrCmdLine += 5;
        }
        else if (!lstrncmp(lpstrCmdLine,_T("mixed"),5))
        {
            g_pMainFrame->m_iDefaultProjType = PROJECTTYPE_MIXED;
            g_pMainFrame->m_bDefaultWizard = false;

            lpstrCmdLine += 5;
        }
        else if (!lstrncmp(lpstrCmdLine,_T("dvdvideo"),8))
        {
            g_pMainFrame->m_iDefaultProjType = PROJECTTYPE_DATA;
            g_pMainFrame->m_iDefaultMedia = SPACEMETER_SIZE_DVD;
            g_pMainFrame->m_bDefaultProjDVDVideo = true;
            g_pMainFrame->m_bDefaultWizard = false;

            lpstrCmdLine += 8;
        }

        if (*lpstrCmdLine)
            lpstrCmdLine++;
    }

    // Default media selection.
    if (!lstrncmp(lpstrCmdLine,_T("-media="),7))
    {
        lpstrCmdLine += 7;
        if (!lstrcmp(lpstrCmdLine,_T("dldvd")))
            g_pMainFrame->m_iDefaultMedia = SPACEMETER_SIZE_DLDVD;
        else if (!lstrcmp(lpstrCmdLine,_T("dvd")))
            g_pMainFrame->m_iDefaultMedia = SPACEMETER_SIZE_DVD;
        else if (!lstrcmp(lpstrCmdLine,_T("cd")))
            g_pMainFrame->m_iDefaultMedia = SPACEMETER_SIZE_703MB;
    }
    else if (!lstrcmp(lpstrCmdLine,_T("-burnimage")))
    {
        return g_ActionManager.BurnImage(NULL,true);
    }
    else if (!lstrcmp(lpstrCmdLine,_T("-copyimage")))
    {
        return g_ActionManager.CopyImage(NULL,true);
    }
    else if (!lstrcmp(lpstrCmdLine,_T("-tracks")))
    {
        return g_ActionManager.ManageTracks(true);
    }
    else if (!lstrcmp(lpstrCmdLine,_T("-erase")))
    {
        return g_ActionManager.Erase(NULL,true);
    }
    else if (!lstrcmp(lpstrCmdLine,_T("-fixate")))
    {
        return g_ActionManager.Fixate(NULL,true);
    }
    else if (!lstrcmp(lpstrCmdLine,_T("-copydisc")))
    {
        return g_ActionManager.CopyDisc(NULL,true);
    }
    // From the shell extension.
    else if (!lstrncmp(lpstrCmdLine,_T("-burnimage "),11))
    {
        return g_ActionManager.BurnImageEx(NULL,true,lpstrCmdLine + 11);
    }
    else if (!lstrncmp(lpstrCmdLine,_T("-burnproject "),13))
    {
        CWaitCursor WaitCursor;		// This displays the hourglass cursor.

        if (g_ProjectManager.LoadProject(lpstrCmdLine + 13))
            return g_ActionManager.BurnCompilation(NULL,true);

        return false;
    }
#ifdef _DEBUG
    else if (!lstrncmp(lpstrCmdLine,SAVE_ENGLISH_STRINGS_PARAM,_countof(SAVE_ENGLISH_STRINGS_PARAM) - 1))
    {
        return SaveEnglishStrings(lpstrCmdLine + _countof(SAVE_ENGLISH_STRINGS_PARAM) - 1);
    }
#endif
    // General, open file.
    else if (lpstrCmdLine[0] != '\0')
    {
        TCHAR *szFullPath = new TCHAR[lstrlen(lpstrCmdLine) + 1];

        // Strip quotes.
        if (lpstrCmdLine[0] == '\"')
        {
            lstrcpy(szFullPath,lpstrCmdLine + 1);
            szFullPath[lstrlen(szFullPath) - 1] = '\0';
        }
        else
        {
            lstrcpy(szFullPath,lpstrCmdLine);
        }

        // Check what type of file that was specified.
        int iDelim = LastDelimiter(lpstrCmdLine,'.');
        if (iDelim != -1)
        {
            // FIXME: Move this check to a common class for disc image management?
            if (!lstrcmpi(szFullPath + iDelim,_T(".iso")) ||
                !lstrcmpi(szFullPath + iDelim,_T(".img")) ||
                !lstrcmpi(szFullPath + iDelim,_T(".cue")) ||
                !lstrcmpi(szFullPath + iDelim,_T(".bin")) ||
                !lstrcmpi(szFullPath + iDelim,_T(".raw")))
                return g_ActionManager.BurnImageEx(NULL,true,szFullPath);
        }

        g_pMainFrame->m_bDefaultWizard = false;

        lstrcpy(g_pMainFrame->m_szProjectFile,szFullPath);
        delete [] szFullPath;
    }

    return Run(lpstrCmdLine,nCmdShow);
}

int WINAPI _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpstrCmdLine,int nCmdShow)
{
    try
    {
#ifdef _DEBUG
        // In debug builds, generate a memory leak report on exit.
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

        // We need to call OleInitialize(), instead of CoInitialize(),
        // because the main frame will be calling RegisterDragDrop() later.
        HRESULT hRes = OleInitialize(NULL);
        if (!SUCCEEDED(hRes))
            ckcore::throw_from_hresult(hRes,_T("Error during OleInitialize: " ));

        // This resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used.
        ::DefWindowProc(NULL,0,0,0L);

        AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

        hRes = _Module.Init(NULL,hInstance);
        ATLASSERT(SUCCEEDED(hRes));

        INT_PTR nRet;
        CLogDlg LogDlg;  // After LogDlg.Create() we must call LogDlg.DestroyWindow(),
                         // even if an exception is thrown. Otherwise, the application
                         // will crash on shutdown.

        try	// Also acts as scope for variable 'MainFrame' etc.
        {
            // In order for it to work under Visual Studio 2005 Express / ATL 3.0,
            // the constructor for CMainFrame etc. must run after _Module.Init() has been
            // called, and the destructor must run before _Module.Term().
            CMainFrame MainFrame;
            CProgressDlg ProgressDlg;
            CSimpleProgressDlg SimpleProgressDlg;
            CAboutWindow AboutWnd;

            g_pLogDlg = &LogDlg;
            g_pMainFrame = &MainFrame;
            g_pProgressDlg = &ProgressDlg;
            g_pSimpleProgressDlg = &SimpleProgressDlg;
            g_pAboutWnd = &AboutWnd;

            // Load the configuration.
            g_SettingsManager.Load();

            // Create the log dialog.
            LogDlg.Create(HWND_DESKTOP);

            // Translate some of the string tables.
            lngTranslateTables();

            // Initialize, SCSI buses etc.
            PerformDeviceScan();

            // Load the codecs.
            TCHAR szCodecPath[MAX_PATH];
            GetModuleFileName(NULL,szCodecPath,MAX_PATH - 1);

            ExtractFilePath(szCodecPath);
            lstrcat(szCodecPath,_T("codecs\\"));

            if (!g_CodecManager.LoadCodecs(szCodecPath))
            {
                if (g_GlobalSettings.m_bCodecWarning)
                {
                    CInfoDlg InfoDlg(&g_GlobalSettings.m_bCodecWarning,lngGetString(ERROR_LOADCODECS),INFODLG_NOCANCEL | INFODLG_ICONWARNING);
                    InfoDlg.DoModal();
                }
            }

            // Display the main window.
            nRet = ParseAndRun(lpstrCmdLine,nCmdShow);

            // Remove any temporary files.
            g_TempManager.CleanUp();

            // Destroy the log dialog.
            if (LogDlg.IsWindow())
                LogDlg.DestroyWindow();
        }
        catch ( ... )
        {
            // If someone tries to touch g_pMainFrame etc. after the object has been destroyed,
            // we need to find out.
            g_pLogDlg = NULL;
            g_pMainFrame = NULL;
            g_pProgressDlg = NULL;
            g_pSimpleProgressDlg = NULL;
            g_pAboutWnd = NULL;

            // Destroy the log dialog.
            if (LogDlg.IsWindow())
                LogDlg.DestroyWindow();

            _Module.Term();
            OleUninitialize();

            throw;
        }

        // If someone tries to touch g_pMainFrame etc. after the object has been destroyed,
        // we need to find out.
        g_pLogDlg = NULL;
        g_pMainFrame = NULL;
        g_pProgressDlg = NULL;
        g_pSimpleProgressDlg = NULL;
        g_pAboutWnd = NULL;

        _Module.Term();
        OleUninitialize();

        return (int)nRet;
    }
    catch (const std::exception &e)
    {
        ATLVERIFY(0 != MessageBox(NULL,
                                  ckcore::get_except_msg(e).c_str(),
                                  lngGetString(GENERAL_ERROR),
                                  MB_OK | MB_ICONERROR | MB_APPLMODAL));
        return 1;
    }
}

void ProcessMessages()
{
    MSG Msg;
    while (::PeekMessage(&Msg,NULL,0,0,PM_REMOVE)) 
    { 
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
}