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

#pragma once
#include "Iso9660.h"
#include "../../Common/Stream.h"

#define JOLIET_MAX_NAMELEN_NORMAL			 64	// According to Joliet specification.
#define JOLIET_MAX_NAMELEN_RELAXED			101	// 207 bytes = 101 wide characters + 4 wide characters for file version.

namespace ckFileSystem
{
	class CJoliet
	{
	private:
		bool m_bIncFileVerInfo;
		int m_iMaxNameLen;

		tVolDescSuppl m_VolDescSuppl;

		wchar_t MakeChar(wchar_t c);
		void MemStrCopy(unsigned char *szTarget,const wchar_t *szSource,size_t iLength);
		void EmptyStrBuffer(unsigned char *szBuffer,size_t iBufferLen);

		void InitVolDesc();

	public:
		CJoliet();
		~CJoliet();

		// Change of internal state functions.
		void SetVolumeLabel(const TCHAR *szLabel);
		void SetTextFields(const TCHAR *szSystem,const TCHAR *szVolSetIdent,
			const TCHAR *szPublIdent,const TCHAR *szPrepIdent);
		void SetFileFields(const TCHAR *ucCopyFileIdent,const TCHAR *ucAbstFileIdent,
			const TCHAR *ucBiblIdent);
		void SetIncludeFileVerInfo(bool bIncludeInfo);
		void SetRelaxMaxNameLen(bool bRelaxRestriction);

		// Write functions.
		bool WriteVolDesc(COutStream *pOutStream,SYSTEMTIME &stImageCreate,
			unsigned long ulVolSpaceSize,unsigned long ulPathTableSize,
			unsigned long ulPosPathTableL,unsigned long ulPosPathTableM,
			unsigned long ulRootExtentLoc,unsigned long ulDataLen);

		// Helper functions.
		unsigned char WriteFileName(unsigned char *pOutBuffer,const TCHAR *szFileName,bool bIsDir);
		unsigned char CalcFileNameLen(const TCHAR *szFileName,bool bIsDir);
	};
};
