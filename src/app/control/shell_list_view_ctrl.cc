/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2010 Christian Kindahl
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

#include "stdafx.h"
#include "shell_list_view_ctrl.hh"
#include "ctrl_messages.hh"
#include "version.hh"

unsigned int CF_IDLIST = ::RegisterClipboardFormat(CFSTR_SHELLIDLIST);

CShellListViewCtrl::CShellListViewCtrl(HWND hWndParent,HWND hWndReceiver) : m_dwRef(0)
{
	m_hWndParent = hWndParent;
	m_hWnd = NULL;

	if (hWndReceiver == NULL)
		m_hWndReceiver = m_hWndParent;
	else
		m_hWndReceiver = hWndReceiver;

	m_pParentShellFolder = NULL;
	m_pShellView = NULL;

	// Used when collecting item data.
	m_hItemsMem = NULL;

	m_pidl = NULL;
}

CShellListViewCtrl::~CShellListViewCtrl()
{
	FreeMemberPointers();

	// Make sure that no selection memory is still allocated.
	if (m_hItemsMem != NULL)
		EndGetItems();
}

HWND CShellListViewCtrl::GetListViewHandle()
{
	if (!m_hWnd)
		return NULL;

	// Find the list view control.
	HWND hWndListView = FindWindowEx(m_hWnd,NULL,WC_LISTVIEW,NULL);

	if (!::IsWindowVisible(hWndListView))
	{
		hWndListView = FindWindowEx(m_hWnd,NULL,_T("ThumbnailVwExtWnd32"),NULL);
		hWndListView = FindWindowEx(hWndListView,NULL,WC_LISTVIEW,NULL);
	}

	return hWndListView;
}

void CShellListViewCtrl::FreeMemberPointers()
{
	if (m_pShellView)
	{
		m_pShellView->UIActivate(SVUIA_DEACTIVATE);
		m_pShellView->DestroyViewWindow();
		m_pShellView->Release();
	}

	if (m_pParentShellFolder)
		m_pParentShellFolder->Release();

	m_pShellView = NULL;
	m_pParentShellFolder = NULL;
}

IShellFolder *CShellListViewCtrl::GetParentShellFolder()
{
	return m_pParentShellFolder;
}

bool CShellListViewCtrl::IsFolder(LPCITEMIDLIST pidl,IShellFolder *psfParent)
{
	ATLASSERT(NULL != psfParent);

	if (!psfParent)
		return false;

	DWORD dwAttribs = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_BROWSABLE;
	HRESULT hResult = psfParent->GetAttributesOf(1,&pidl,&dwAttribs);

	return ((S_OK == hResult &&
		(dwAttribs & SFGAO_FOLDER) != 0 &&
		((dwAttribs & SFGAO_FILESYSTEM) != 0 ||
		(dwAttribs & SFGAO_FILESYSANCESTOR) != 0)) && !(dwAttribs & SFGAO_BROWSABLE));
}

bool CShellListViewCtrl::IsFolderLink(LPCITEMIDLIST pidl,LPITEMIDLIST *pOutPidl)
{
	ATLASSERT(m_pParentShellFolder);

	if (!m_pParentShellFolder)
		return false;

	// Is pidl a link?
	DWORD dwAttribs = SFGAO_LINK;
	m_pParentShellFolder->GetAttributesOf(1,&pidl,&dwAttribs);

	if (!(dwAttribs & SFGAO_LINK))
		return false;

	// Get the link object.
	IShellLink *pLink;

	if (NOERROR != m_pParentShellFolder->GetUIObjectOf(m_hWnd,1,&pidl,IID_IShellLink,0,
		(void**)&pLink))
	{
		return false;
	}

	// Get link target's pidl.
	LPITEMIDLIST rPidl = NULL,pidlParent = NULL,pidlChild = NULL;

	if (NOERROR == pLink->GetIDList(&rPidl))
	{
		IShellFolder *psf;

		if (NOERROR == SHGetDesktopFolder(&psf))
		{
			// If pidl is complex (it probably is), split into parent/child pidls first.
			if (m_PidlHelp.Split(rPidl,&pidlParent,&pidlChild))
			{
				IShellFolder *psf2;

				if (NOERROR == psf->BindToObject(pidlParent,NULL,IID_IShellFolder, 
						reinterpret_cast<void**>(&psf2)))
				{
					if (!IsFolder(pidlChild,psf2))
						m_PidlHelp.FreePidl(rPidl);

					psf2->Release();
				}

				m_PidlHelp.FreePidl(pidlParent);
				m_PidlHelp.FreePidl(pidlChild);
			}
			else if (!IsFolder(rPidl,psf))
			{
				m_PidlHelp.FreePidl(rPidl);
			}

			psf->Release();
		}
	}

	pLink->Release();

	if (rPidl)
	{
		if (pOutPidl)
			*pOutPidl = rPidl;
		else
			m_PidlHelp.FreePidl(rPidl);

		return true;
	}
	
	return false;
}

STDMETHODIMP CShellListViewCtrl::QueryInterface(REFIID iid,void **ppvObject)
{
	if (ppvObject == NULL)
		return E_POINTER;

	*ppvObject = NULL;

	if (iid == IID_IUnknown)
		*ppvObject = (IUnknown *)(IShellBrowser *)this;
	else if (iid == IID_IOleWindow)
		*ppvObject = (IOleWindow *)this;			
	else if (iid == IID_IShellBrowser)
		*ppvObject = (IShellBrowser *)this;
	else if (iid == IID_ICommDlgBrowser)
		*ppvObject = (ICommDlgBrowser *)this;
	else
		return E_NOINTERFACE;

	((IUnknown *)(*ppvObject))->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CShellListViewCtrl::AddRef()
{
	return ++m_dwRef;
}

STDMETHODIMP_(ULONG) CShellListViewCtrl::Release()
{
	return --m_dwRef;
}

STDMETHODIMP CShellListViewCtrl::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::GetWindow(HWND *lphwnd)
{ 
	*lphwnd = m_hWndParent; 
	return S_OK; 
}

STDMETHODIMP CShellListViewCtrl::IncludeObject(THIS_ struct IShellView *ppshv,
	LPCITEMIDLIST pidl)
{
	// TODO: Implement filtering support.
	/*if (isISO(pidl))
		return S_OK;	// Show it.
	return S_FALSE;		// Hide it.*/

	return S_OK;
}

STDMETHODIMP CShellListViewCtrl::OnDefaultCommand(THIS_ struct IShellView *ppshv)
{
	CIDA *pData = BeginGetItems(true);
	char *pcData = reinterpret_cast<char *>(pData) + pData->aoffset[1];
	LPCITEMIDLIST pidl = (LPITEMIDLIST)pcData;

	TCHAR szNewPathName[MAX_PATH];
	if (!m_PidlHelp.GetPathName(m_pParentShellFolder,pidl,szNewPathName,MAX_PATH - 1))
		return E_FAIL;

	LPITEMIDLIST pidlTo = NULL;

	// Check if we're dealing with a folder or a folder link.
	if (IsFolder(pidl,m_pParentShellFolder))
	{
		// Notify the receiver that a new folder will be opened.
		::SendMessage(m_hWndReceiver,WM_SLVC_CHANGEFOLDER,(WPARAM)pidl,(LPARAM)szNewPathName);

		BrowseObject(pidl,SBSP_SAMEBROWSER | SBSP_RELATIVE);
	}
	else if (IsFolderLink(pidl,&pidlTo))
	{
		// Notify the receiver that we will open a shortcut. Shortcuts should be notified
		// in a different way than regular folders since it will require a deeper search
		// in the folder tree to find the new path.
		m_PidlHelp.GetPathName(m_pParentShellFolder,pidlTo,szNewPathName,MAX_PATH - 1);
		::SendMessage(m_hWndReceiver,WM_SLVC_CHANGEFOLDERLINK,(WPARAM)pidl,(LPARAM)szNewPathName);

		BrowseObject(pidlTo,SBSP_SAMEBROWSER | SBSP_ABSOLUTE);
	}
	else
	{
		if (::SendMessage(m_hWndReceiver,WM_SLVC_FILECOMMAND,(WPARAM)pidl,(LPARAM)szNewPathName) == 1)
		{
			if (pidlTo)
				m_PidlHelp.FreePidl(pidlTo);

			EndGetItems();
			return NOERROR;
		}

		EndGetItems();
		return E_NOTIMPL;
	}

	if (pidlTo)
		m_PidlHelp.FreePidl(pidlTo);

	EndGetItems();
	return NOERROR;
}

STDMETHODIMP CShellListViewCtrl::OnStateChange(THIS_ struct IShellView *ppshv,ULONG uChange)
{
	// Handle selection, rename, focus etc.
	return E_NOTIMPL;
}

WNDPROC CShellListViewCtrl::m_pOldWndProc = NULL;

LRESULT CALLBACK CShellListViewCtrl::WndProc(HWND hWnd,UINT Msg,
											 WPARAM wParam, LPARAM lParam)
{
	//LRESULT lResult = m_pOldWndProc(hWnd,Msg,wParam,lParam);
	LRESULT lResult = 0;
	if (::IsWindowUnicode(hWnd))
		lResult = ::CallWindowProcW(m_pOldWndProc,hWnd,Msg,wParam,lParam);
	else
		lResult = ::CallWindowProcA(m_pOldWndProc,hWnd,Msg,wParam,lParam);

	// We need to redirect this message to the parent so later on the splitter
	// that hosts this control will know that we have activated this window.
	// Otherwise the splitter will not know that it should return the focus to
	// this window if the focus is stolen by for example the menu.
	if (Msg == WM_MOUSEACTIVATE)
		::SendMessage(GetParent(hWnd),Msg,wParam,lParam);

	return lResult;
}

STDMETHODIMP CShellListViewCtrl::BrowseObject(LPCITEMIDLIST pidl,UINT wFlags)
{
	if (wFlags & SBSP_PARENT && !m_pidl)
		return E_FAIL;
	if (wFlags & SBSP_RELATIVE && !m_pParentShellFolder)
		return E_FAIL;

	IShellFolder *pShellFolder = m_pParentShellFolder;
	LPITEMIDLIST pidlCopy = NULL;
	HRESULT hr = E_FAIL;

	// Get IShellFolder, FULLY QUALIFIED pidl. Check if we are dealing with the desktop.
	if (pidl == NULL && !(wFlags & SBSP_PARENT))
	{
		FreeMemberPointers();

		if (SUCCEEDED(SHGetDesktopFolder(&m_pParentShellFolder)))
			hr = SHGetSpecialFolderLocation(m_hWndParent,CSIDL_DESKTOP,&pidlCopy);
	}
	else
	{
		if (wFlags & SBSP_RELATIVE)
		{
			// Browse child folder
			pidlCopy = ILCombine(m_pidl,pidl);
			hr = pShellFolder->BindToObject(pidl,NULL,IID_IShellFolder,
				reinterpret_cast<void **>(&m_pParentShellFolder));
		}
		else
		{
			if (wFlags & SBSP_PARENT)
			{
				// Browse parent: pidlCopy = parent folder's PIDL.
				if (!m_PidlHelp.Split(m_pidl,&pidlCopy,NULL))
					return E_FAIL;
			}
			else
			{
				// Browse absolute PIDL: pidlCopy = clone(pidl).
				pidlCopy = ILClone(pidl);
			}

			// Create IShellFolder for target PIDL through Desktop folder.
			IShellFolder *pShellFolder2;
			if (SUCCEEDED(hr = SHGetDesktopFolder(&pShellFolder2)))
			{
				if (SUCCEEDED(pShellFolder2->BindToObject(pidlCopy,NULL,
					IID_IShellFolder, reinterpret_cast<void**>(&m_pParentShellFolder))))
				{
						pShellFolder2->Release();
				}
				else
				{
					m_pParentShellFolder = pShellFolder2;
				}
			}
		}
	}

	if (FAILED(hr))
	{
		if (pidlCopy)
			m_PidlHelp.FreePidl(pidlCopy);

		if (m_pParentShellFolder)
			m_pParentShellFolder->Release();

		m_pParentShellFolder = pShellFolder;
		return hr;
	}

	ATLASSERT(m_pParentShellFolder && pidlCopy);

	if (!m_pParentShellFolder || !pidlCopy)
		return E_FAIL;

	// Use the IShellFolder to create a view window
	FOLDERSETTINGS fs = { FVM_DETAILS,FWF_SNAPTOGRID/* | FWF_NOICONS*/ };

	if (m_pShellView)
		m_pShellView->GetCurrentInfo(&fs);

	IShellView *pShellView = m_pShellView;
	m_pShellView = NULL;
	HWND hWndShellView = NULL;

	if (SUCCEEDED(hr = m_pParentShellFolder->CreateViewObject(m_hWndParent,IID_IShellView,
		reinterpret_cast<void **>(&m_pShellView))))
	{
		hr = m_pShellView->CreateViewWindow(pShellView,&fs,
			static_cast<IShellBrowser*>(this),&CWindow::rcDefault,&hWndShellView);
	}

	if (FAILED(hr))
	{
		if (m_pShellView)
		{
			m_pShellView->UIActivate(SVUIA_DEACTIVATE);
			m_pShellView->DestroyViewWindow();
			m_pShellView->Release();
			m_pShellView = pShellView;
		}

		m_PidlHelp.FreePidl(pidlCopy);
		m_pParentShellFolder->Release();
		m_pParentShellFolder = pShellFolder;

		return hr;
	}

	// Restore the original windows procedure to the old window.
	if (::IsWindow(m_hWnd) && m_pOldWndProc != NULL)
	{
		//::SetWindowLongPtr(m_hWnd,GWLP_WNDPROC,(LONG_PTR)m_pOldWndProc);
		if (::IsWindowUnicode(m_hWnd))
			::SetWindowLongPtrW(m_hWnd,GWLP_WNDPROC,(LONG_PTR)m_pOldWndProc);
		else
			::SetWindowLongPtrA(m_hWnd,GWLP_WNDPROC,(LONG_PTR)m_pOldWndProc);
	}

	m_hWnd = hWndShellView;

	// Update the window to use our modified procedure.
	//m_pOldWndProc = (WNDPROC)::GetWindowLongPtr(m_hWnd,GWLP_WNDPROC);
	//::SetWindowLongPtr(m_hWnd,GWLP_WNDPROC,(LONG_PTR)WndProc);
	if (::IsWindowUnicode(m_hWnd))
	{
		m_pOldWndProc = (WNDPROC)::GetWindowLongPtrW(m_hWnd,GWLP_WNDPROC);
		::SetWindowLongPtrW(m_hWnd,GWLP_WNDPROC,(LONG_PTR)WndProc);
	}
	else
	{
		m_pOldWndProc = (WNDPROC)::GetWindowLongPtrA(m_hWnd,GWLP_WNDPROC);
		::SetWindowLongPtrA(m_hWnd,GWLP_WNDPROC,(LONG_PTR)WndProc);
	}

	if (m_pidl)
		m_PidlHelp.FreePidl(m_pidl);

	m_pidl = pidlCopy;
	
	if (pShellView != NULL)
	{
		pShellView->UIActivate(SVUIA_DEACTIVATE);
		pShellView->DestroyViewWindow();
		pShellView->Release();
	}

	if (pShellFolder)
		pShellFolder->Release();

	// Only set the focus to the new list view if the old shell list view instance
	// had the focus.
	bool bSetFocus = ::GetFocus() == GetListViewHandle();

	// Notify the receiver.
	::SendMessage(m_hWndReceiver,WM_SLVC_BROWSEOBJECT,0,(LPARAM)m_hWnd);
	m_pShellView->UIActivate(bSetFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);

	// Select first item in list view.
	HWND hWndListView = FindWindowEx(m_hWnd,NULL,WC_LISTVIEW,NULL);
	ListView_SetItemState(hWndListView,0,LVIS_FOCUSED | LVIS_SELECTED,0x000F);
	::ShowWindow(hWndListView,SW_NORMAL);

	// Update the combobox.
	::SendMessage(m_hWndReceiver,WM_SLVC_DONEBROWSEOBJECT,0,(LPARAM)m_hWnd);

	// Subclass the internal list view.
	// UPDATE: Causes the list view to turn white when double-clicking on a
	// folder in the explorer view.
	/*if (hWndListView != NULL)
	{
		if (m_InternalListView.m_hWnd != NULL)
		{
			m_InternalListView.UnsubclassWindow();
		}

		m_InternalListView.SubclassWindow(hWndListView);
		m_InternalListView.SetHost(m_pShellView);

		// Force the WS_EX_CLIENTEDGE style on Windows Vista (which disables it by default).
		if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA)
			m_InternalListView.ModifyStyleEx(0,WS_EX_CLIENTEDGE,SWP_FRAMECHANGED);
	}*/
	if (hWndListView != NULL)
	{
		// Force the WS_EX_CLIENTEDGE style on Windows Vista (which disables it by default).
		if (g_WinVer.m_ulMajorVersion == MAJOR_WINVISTA)
		{
			unsigned long ulStyle = ::GetWindowLong(hWndListView,GWL_EXSTYLE);
			::SetWindowLong(hWndListView,GWL_EXSTYLE,ulStyle | WS_EX_CLIENTEDGE);
			::SetWindowPos(hWndListView,NULL,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE |
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		}
	}

	return NOERROR;
}

STDMETHODIMP CShellListViewCtrl::EnableModelessSB(BOOL fEnable)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::GetControlWindow(UINT id,HWND *lphwnd)
{
	if (lphwnd == NULL)
		return E_POINTER;

	/*if (FCW_STATUS == id)
	{
		*lphwnd = m_hWndStatusBar;
		return S_OK;
	}*/

	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::GetViewStateStream(DWORD grfMode,LPSTREAM *ppStrm)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::InsertMenusSB(HMENU hmenuShared,
	LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::OnViewWindowActive(struct IShellView *ppshv)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::QueryActiveShellView(struct IShellView **ppshv)
{
	m_pShellView->AddRef();
	*ppshv = m_pShellView;
	return S_OK; 
}

STDMETHODIMP CShellListViewCtrl::RemoveMenusSB(HMENU hmenuShared)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::SendControlMsg(UINT id,UINT uMsg,WPARAM wParam,
	LPARAM lParam,LRESULT *pret)
{
	if(pret == NULL)
		return E_POINTER;

	/*if (FCW_STATUS == id)
	{
		*pret = ::SendMessage(m_hWndStatusBar,uMsg,wParam,lParam);
		return S_OK;
	}*/
	
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::SetMenuSB(HMENU hmenuShared,HOLEMENU holemenuReserved,HWND hwndActiveObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::SetStatusTextSB(LPCOLESTR lpszStatusText)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::SetToolbarItems(LPTBBUTTON lpButtons,UINT nButtons,UINT uFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP CShellListViewCtrl::TranslateAcceleratorSB(LPMSG lpmsg,WORD wID)
{
	return S_OK;
}

bool CShellListViewCtrl::IsAtTop()
{
	return m_pidl->mkid.cb < 1;
}

CIDA *CShellListViewCtrl::BeginGetItems(bool bSelected)
{
	if (m_hItemsMem != NULL)
		EndGetItems();

	IDataObject *pSelection;
	HRESULT hResult = m_pShellView->GetItemObject(bSelected ? SVGIO_SELECTION : SVGIO_ALLVIEW,
		IID_IDataObject,(void **)&pSelection);
	if (!SUCCEEDED(hResult))
		return NULL;

	// CFSTR_SHELLIDLIST
	FORMATETC fetc;
	fetc.cfFormat = static_cast<CLIPFORMAT>(CF_IDLIST);
	fetc.dwAspect = DVASPECT_CONTENT;
	fetc.ptd = NULL;
	fetc.lindex = -1;
	fetc.tymed = TYMED_HGLOBAL;

	hResult = pSelection->QueryGetData(&fetc);
	if (!SUCCEEDED(hResult))
		return NULL;

	STGMEDIUM stm;
	hResult = pSelection->GetData(&fetc,&stm);
	if (!SUCCEEDED(hResult))
		return NULL;

	CIDA *pData = (CIDA *)GlobalLock(stm.hGlobal);
	if (pData == NULL)
		return NULL;

	if (pData == NULL || pData->cidl < 1)
		return NULL;

	return pData;
}

void CShellListViewCtrl::EndGetItems()
{
	GlobalUnlock(m_hItemsMem);
	m_hItemsMem = NULL;
}
