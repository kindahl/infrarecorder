/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
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

class CProjectDropTargetBase : public IDropTarget
{
protected:
	// Custom InfraRecorder clip board format identifier.
	unsigned int m_uiClipFormat;

private:
	enum eDropType
	{
		DT_NONE,
		DT_HDROP,
		DT_IRPROJECT
	};

	eDropType m_DropType;
	long m_lRefCount;

	eDropType QueryDataObject(IDataObject *pDataObject);

public:
	CProjectDropTargetBase();
	~CProjectDropTargetBase();

	// IUnknown implementation.
	HRESULT __stdcall QueryInterface(REFIID iid,void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	// IDropTarget implementation.
	HRESULT __stdcall DragEnter(IDataObject *pDataObject,DWORD grfKeyState,POINTL pt,DWORD *pdwEffect);
	HRESULT __stdcall DragOver(DWORD grfKeyState,POINTL pt,DWORD *pdwEffect);
	HRESULT __stdcall DragLeave();
	HRESULT __stdcall Drop(IDataObject *pDataObject,DWORD grfKeyState,POINTL pt,DWORD *pdwEffect);

	// To be implemented by inheritor.
	virtual bool OnDragOver(POINTL ptCursor) = 0;
	virtual bool OnDrop(POINTL ptCursor,IDataObject *pDataObject) = 0;
	virtual void OnDragLeave() = 0;
};
