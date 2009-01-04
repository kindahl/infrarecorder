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

class CEnumFmtEtc : public IEnumFORMATETC
{
private:
	long m_lRefCount;

	unsigned int m_uiNumFormats;
	unsigned int m_uiFormatIndex;
	FORMATETC *m_pFormats;

public:
	CEnumFmtEtc(FORMATETC *pFormats,unsigned int uiNumFormats);
	~CEnumFmtEtc();

	void SetIndex(unsigned int uiFormatIndex);

	// IUnknown members.
	HRESULT __stdcall QueryInterface(REFIID iid,void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	// IEnumFormatEtc members.
	HRESULT __stdcall Next(ULONG celt,FORMATETC *rgelt,ULONG *pceltFetched);
	HRESULT __stdcall Skip(ULONG celt); 
	HRESULT __stdcall Reset();
	HRESULT __stdcall Clone(IEnumFORMATETC **ppEnumFormatEtc);
};

HRESULT CreateEnumFmtEtc(unsigned int uiNumFormats,FORMATETC *pFormats,
	IEnumFORMATETC **ppEnumFormatEtc);
