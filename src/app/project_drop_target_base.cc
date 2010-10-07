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
#include "project_drop_target_base.hh"
#include "project_data_object.hh"

CProjectDropTargetBase::CProjectDropTargetBase()
{
	m_uiClipFormat = ::RegisterClipboardFormat(_T(PROJECT_CF_NAME));

	m_lRefCount = 1;
}

CProjectDropTargetBase::~CProjectDropTargetBase()
{
}

HRESULT __stdcall CProjectDropTargetBase::QueryInterface(REFIID iid,void **ppvObject)
{
	// Check to see what interface has been requested.
    if (iid == IID_IDropTarget || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

ULONG __stdcall CProjectDropTargetBase::AddRef()
{
	// Increment object reference count.
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CProjectDropTargetBase::Release()
{
	// Decrement object reference count.
	LONG lCount = InterlockedDecrement(&m_lRefCount);
		
	if (lCount == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return lCount;
	}
}

CProjectDropTargetBase::eDropType CProjectDropTargetBase::QueryDataObject(IDataObject *pDataObject)
{
    FORMATETC fmtetc = { CF_HDROP,0,DVASPECT_CONTENT,-1,TYMED_HGLOBAL };
	if (pDataObject->QueryGetData(&fmtetc) == S_OK)
		return DT_HDROP;

	fmtetc.cfFormat = static_cast<CLIPFORMAT>(m_uiClipFormat);
	if (pDataObject->QueryGetData(&fmtetc) == S_OK)
		return DT_IRPROJECT;

	return DT_NONE;
}

HRESULT __stdcall CProjectDropTargetBase::DragEnter(IDataObject *pDataObject,DWORD grfKeyState,
												POINTL pt,DWORD *pdwEffect)
{
    m_DropType = QueryDataObject(pDataObject);

	*pdwEffect = DROPEFFECT_NONE;
	return S_OK;
}

HRESULT __stdcall CProjectDropTargetBase::DragOver(DWORD grfKeyState,POINTL pt,DWORD *pdwEffect)
{
	if (m_DropType != DT_NONE && OnDragOver(pt))
		*pdwEffect = m_DropType == DT_HDROP ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}

HRESULT __stdcall CProjectDropTargetBase::DragLeave()
{
	OnDragLeave();
	return S_OK;
}

HRESULT __stdcall CProjectDropTargetBase::Drop(IDataObject *pDataObject,DWORD grfKeyState,
												   POINTL pt,DWORD *pdwEffect)
{
	if (m_DropType != DT_NONE && OnDrop(pt,pDataObject))
		*pdwEffect = m_DropType == DT_HDROP ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}
