/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
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
#include "System.h"
#include "Registry.h"
#include "../../Common/StringUtil.h"
#include "StringTable.h"
#include "LangUtil.h"
#include "InfoDlg.h"
#include "Settings.h"

TCHAR *g_szCharacterSets[] = {
	_T("cp10081"),			//  0
	_T("cp10079"),			//  1
	_T("cp10029"),			//  2
	_T("cp10007"),			//  3
	_T("cp10006"),			//  4
	_T("cp10000"),			//  5
	_T("koi8-u"),			//  6
	_T("koi8-r"),			//  7
	_T("cp1251"),			//  8
	_T("cp1250"),			//  9
	_T("cp874"),			// 10
	_T("cp869"),			// 11
	_T("cp866"),			// 12
	_T("cp865"),			// 13
	_T("cp864"),			// 14
	_T("cp863"),			// 15
	_T("cp862"),			// 16
	_T("cp861"),			// 17
	_T("cp860"),			// 18
	_T("cp857"),			// 19
	_T("cp855"),			// 20
	_T("cp852"),			// 21
	_T("cp850"),			// 22
	_T("cp775"),			// 23
	_T("cp737"),			// 24
	_T("cp437"),			// 25
	_T("iso8859-15"),		// 26
	_T("iso8859-14"),		// 27
	_T("iso8859-9"),		// 28
	_T("iso8859-8"),		// 29
	_T("iso8859-7"),		// 30
	_T("iso8859-6"),		// 31
	_T("iso8859-5"),		// 32
	_T("iso8859-4"),		// 33
	_T("iso8859-3"),		// 34
	_T("iso8859-2"),		// 35
	_T("iso8859-1")			// 36
};

int CodePageToCharacterSet(int iCodePage)
{
	// Taken from: HKEY_CLASSES_ROOT\MIME\Database\Codepage\<code page>\BodyCharset
	switch (iCodePage)
	{
		case 1250:
			return 9;		// cp1250

		case 28592:
			return 35;		// iso8859-2

		case 1251:
			return 8;		// cp1251

		case 20866:
			return 7;		// koi8-r

		case 1252:
			return 36;		// iso8859-1

		case 1253:
		case 28597:
			return 30;		// iso8859-7

		case 1254:
			return 28;		// iso8859-9

		case 1255:
		case 28598:
		case 38598:
			return 29;		// iso8859-8

		case 1256:
		case 28596:
			return 31;		// iso8859-6

		case 1257:
		case 28594:
			return 33;		// iso8859-4

		case 21866:
			return 6;		// koi8-u

		case 28593:
			return 34;		// iso8859-3

		case 28595:
			return 32;		// iso8859-5

		case 852:
			return 21;		// cp852

		case 866:
			return 12;		// cp866
	};

	CRegistry Reg;
	Reg.SetRoot(HKEY_CLASSES_ROOT);

	TCHAR szBuffer[MAX_PATH],szBodyCharset[64];
	lsprintf(szBuffer,_T("MIME\\Database\\Codepage\\%d\\"),iCodePage);
	bool bResult = true;

	if (Reg.OpenKey(szBuffer,false))
	{
		if (!Reg.ReadString(_T("BodyCharset"),szBodyCharset,64))
			bResult = false;

		Reg.CloseKey();
	}
	else
	{
		bResult = false;
	}

	if (!bResult)
		lstrcpy(szBodyCharset,_T("unknown"));

	lsnprintf_s(szBuffer,MAX_PATH,lngGetString(ERROR_UNSUPCHARSET),szBodyCharset);

	// Display the warning message (if not disabled).
	if (g_GlobalSettings.m_bCharSetWarning)
	{
		CInfoDlg InfoDlg(&g_GlobalSettings.m_bCharSetWarning,szBuffer,INFODLG_NOCANCEL | INFODLG_ICONWARNING);
		InfoDlg.DoModal();
	}

	return -1;
}
