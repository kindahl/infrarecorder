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

#include "stdafx.h"
#include "ProjectDropSource.h"

CProjectDropSource::CProjectDropSource()
{
	m_lRefCount = 1;
}

CProjectDropSource::~CProjectDropSource()
{
}

HRESULT __stdcall CProjectDropSource::QueryInterface(REFIID iid,void **ppvObject)
{
    // Check to see what interface has been requested.
    if (iid == IID_IDropSource || iid == IID_IUnknown)
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

ULONG __stdcall CProjectDropSource::AddRef()
{
    // Increment object reference count.
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CProjectDropSource::Release()
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

HRESULT __stdcall CProjectDropSource::QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
{
	// If the escape key has been pressed we cancel the operation.
    if (fEscapePressed == TRUE)
        return DRAGDROP_S_CANCEL;

	// If the left button has been released we should drop.
    if ((grfKeyState & MK_LBUTTON) == 0)
        return DRAGDROP_S_DROP;

    return S_OK;
}

HRESULT __stdcall CProjectDropSource::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}
