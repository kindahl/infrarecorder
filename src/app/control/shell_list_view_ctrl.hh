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
#include <shlobj.h>
#include "pidl_helper.hh"

class CShellListViewCtrl : public IShellBrowser, public ICommDlgBrowser
{
private:
    // Internal list view class used for translating the default shell hot keys.
    /*class CInternalListViewCtrl : public CWindowImpl<CInternalListViewCtrl,CListViewCtrl>,
                                  public CMessageFilter
    {
    private:
        IShellView *m_pHostShellView;
        bool m_bRegistered;

    public:
        BEGIN_MSG_MAP(CInternalListViewCtrl)
            MESSAGE_HANDLER(WM_SETFOCUS,OnShowWindow)
        END_MSG_MAP()

        CInternalListViewCtrl()
        {
            m_bRegistered = false;
        }

        virtual BOOL PreTranslateMessage(MSG *pMsg)
        {
            if (GetFocus() == m_hWnd)
            {
                if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
                {
                    if (m_pHostShellView->TranslateAccelerator(pMsg) == S_OK)
                        return TRUE;
                }
            }

            return FALSE;
        }

        LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
        {
            if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
            {
                MSG Msg;
                Msg.message = uMsg;
                Msg.wParam = wParam;
                Msg.lParam = lParam;

                if (m_pHostShellView->TranslateAccelerator(&Msg) == S_OK)
                {
                    bHandled = true;
                    return TRUE;
                }
            }

            bHandled = false;
            return 0;
        }

        LRESULT OnShowWindow(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
        {
            if (!m_bRegistered)
            {
                AddToMessageLoop();
                m_bRegistered = true;
            }

            bHandled = false;
            return 0;
        }

        void SetHost(IShellView *pHostShellView)
        {
            m_pHostShellView = pHostShellView;
        }

        void AddToMessageLoop()
        {
            CMessageLoop *pLoop = _Module.GetMessageLoop();
            pLoop->AddMessageFilter(this);
        }

        void RemoveFromMessageLoop()
        {
            CMessageLoop *pLoop = _Module.GetMessageLoop();
            pLoop->RemoveMessageFilter(this);
        }
    };

    CInternalListViewCtrl m_InternalListView;*/
    static WNDPROC m_pOldWndProc;
    static LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam, LPARAM lParam);

    DWORD m_dwRef;

    HWND m_hWndParent;
    HWND m_hWndReceiver;
    HWND m_hWnd;

    IShellFolder *m_pParentShellFolder;
    IShellView *m_pShellView;

    HGLOBAL	m_hItemsMem;

    CPidlHelper m_PidlHelp;
    LPITEMIDLIST m_pidl;

    void FreeMemberPointers();
    bool IsFolder(LPCITEMIDLIST pidl,IShellFolder *psfParent);
    bool IsFolderLink(LPCITEMIDLIST pidl,LPITEMIDLIST *pOutPidl);
    //HWND GetListViewHandle();

public:
    CShellListViewCtrl(HWND hWndParent,HWND hWndReceiver = NULL);
    ~CShellListViewCtrl();

    HWND GetListViewHandle();

    // IUnknown members.
    STDMETHOD(QueryInterface)(REFIID iid,void **ppvObject);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    // IOleWindow members.
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
    STDMETHOD(GetWindow)(HWND *lphwnd);

    // ICommDlgBrowser members.
    STDMETHOD(IncludeObject)(THIS_ struct IShellView *ppshv,LPCITEMIDLIST pidl);
    STDMETHOD(OnDefaultCommand)(THIS_ struct IShellView *ppshv);
    STDMETHOD(OnStateChange)(THIS_ struct IShellView *ppshv,ULONG uChange);

    // IShellBrowser members (same as IOleInPlaceFrame).
    STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl,UINT wFlags);
    STDMETHOD(EnableModelessSB)(BOOL fEnable);
    STDMETHOD(GetControlWindow)(UINT id,HWND *lphwnd);
    STDMETHOD(GetViewStateStream)(DWORD grfMode,LPSTREAM *ppStrm);
    STDMETHOD(InsertMenusSB)(HMENU hmenuShared,LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(OnViewWindowActive)(struct IShellView *ppshv);
    STDMETHOD(QueryActiveShellView)(struct IShellView **ppshv);
    STDMETHOD(RemoveMenusSB)(HMENU hmenuShared);
    STDMETHOD(SendControlMsg)(UINT id,UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT *pret);
    STDMETHOD(SetMenuSB)(HMENU hmenuShared,HOLEMENU holemenuReserved,HWND hwndActiveObject);
    STDMETHOD(SetStatusTextSB)(LPCOLESTR lpszStatusText);
    STDMETHOD(SetToolbarItems)(LPTBBUTTON lpButtons,UINT nButtons,UINT uFlags);
    STDMETHOD(TranslateAcceleratorSB)(LPMSG lpmsg,WORD wID);

    // Miscellaneous.
    IShellFolder *GetParentShellFolder();

    bool IsAtTop();

    CIDA *BeginGetItems(bool bSelected);
    void EndGetItems();

    // Operators.
    operator HWND ()
    {
        return m_hWnd;
    }
};
