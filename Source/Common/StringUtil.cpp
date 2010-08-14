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
#ifdef UNICODE
#include <wchar.h>
#else
#include <stdio.h>
#endif
#include <ckcore/types.hh>
#include "StringUtil.h"

// FIXME: Backslash in the name is missleading.
TCHAR *IncludeTrailingBackslash(TCHAR *szPath)
{
	int iLength = lstrlen(szPath);

	if (szPath[iLength - 1] != '\\' && szPath[iLength - 1] != '/')
	{
		szPath[iLength] = '\\';
		szPath[iLength + 1] = '\0';
	}

	return szPath;
}

// FIXME: Backslash in the name is missleading.
TCHAR *ExcludeTrailingBackslash(TCHAR *szPath)
{
	int iLength = lstrlen(szPath);

	if (szPath[iLength - 1] == '\\' || szPath[iLength - 1] == '/')
		szPath[iLength - 1] = '\0';

	return szPath;
}

TCHAR *SubString(const TCHAR *szText,unsigned int uiStart,unsigned int uiLength)
{
	TCHAR *szResult;
	int iLength = lstrlen(szText);

	if (!szText || (uiStart + uiLength > (unsigned int)iLength))
	{
		szResult = new TCHAR[1];
		szResult[0] = '\0';

		return szResult;
	}

	szResult = new TCHAR[uiLength + 1];
	unsigned int j = 0;

	for (unsigned int i = uiStart; i < uiStart + uiLength; i++,j++)
		szResult[j] = szText[i];

	szResult[j] = '\0';

	return szResult;
}

void FormatBytes(TCHAR *szBuffer,unsigned __int64 iBytes)
{
#ifdef UNICODE
    if (iBytes < 1024)
        swprintf(szBuffer,TEXT("%d %s"),(int)(iBytes & 0xFFFFFFFF),TEXT("Bytes"));
    else if (iBytes < 1024 * 1024)
        swprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0),TEXT("KiB"));
    else if (iBytes < 1024 * 1024 * 1024)
        swprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0 * 1024.0),TEXT("MiB"));
    else if (iBytes < (signed __int64)1024 * 1024 * 1024 * 1024)
        swprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0 * 1024.0 * 1024.0),TEXT("GiB"));
    else
		swprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0 * 1024.0 * 1024.0 * 1024.0),TEXT("TiB"));
#else
	if (iBytes < 1024)
        sprintf(szBuffer,TEXT("%d %s"),(int)(iBytes & 0xFFFFFFFF),TEXT("Bytes"));
    else if (iBytes < 1024 * 1024)
        sprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0),TEXT("KiB"));
    else if (iBytes < 1024 * 1024 * 1024)
        sprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0 * 1024.0),TEXT("MiB"));
    else if (iBytes < (signed __int64)1024 * 1024 * 1024 * 1024)
        sprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0 * 1024.0 * 1024.0),TEXT("GiB"));
    else
		sprintf(szBuffer,TEXT("%.02f %s"),(double)iBytes/(1024.0 * 1024.0 * 1024.0 * 1024.0),TEXT("TiB"));
#endif
}

/*
	Same as FormatBytes but does not include any decimals.
*/
void FormatBytesEx(TCHAR *szBuffer,unsigned __int64 iBytes)
{
#ifdef UNICODE
    if (iBytes < 1024)
        swprintf(szBuffer,TEXT("%d %s"),(int)(iBytes & 0xFFFFFFFF),TEXT("Bytes"));
    else if (iBytes < 1024 * 1024)
        swprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0)),TEXT("KiB"));
    else if (iBytes < 1024 * 1024 * 1024)
        swprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0 * 1024.0)),TEXT("MiB"));
    else if (iBytes < (signed __int64)1024 * 1024 * 1024 * 1024)
        swprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0 * 1024.0 * 1024.0)),TEXT("GiB"));
    else
		swprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0 * 1024.0 * 1024.0 * 1024.0)),TEXT("TiB"));
#else
	if (iBytes < 1024)
        sprintf(szBuffer,TEXT("%d %s"),(int)(iBytes & 0xFFFFFFFF),TEXT("Bytes"));
    else if (iBytes < 1024 * 1024)
        sprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0)),TEXT("KiB"));
    else if (iBytes < 1024 * 1024 * 1024)
        sprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0 * 1024.0)),TEXT("MiB"));
    else if (iBytes < (signed __int64)1024 * 1024 * 1024 * 1024)
        sprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0 * 1024.0 * 1024.0)),TEXT("GiB"));
    else
		sprintf(szBuffer,TEXT("%d %s"),(int)(iBytes/(1024.0 * 1024.0 * 1024.0 * 1024.0)),TEXT("TiB"));
#endif
}

class CNumberFmt
{
private:
    TCHAR m_szDecimalSep[10];
    TCHAR m_szThousandSep[10];

public:
    NUMBERFMT m_NumberFmt;

    void RetrieveDefaults(void);
};

void CNumberFmt::RetrieveDefaults(void)
{
	// Please avoid memset to clear the data structures,
	// as it confuses test software like BoundsChecker,
	// it's actually slower (!) and is not really portable.
	m_szDecimalSep[0]  = _T('\0');
	m_szThousandSep[0] = _T('\0');

	m_NumberFmt.NumDigits     = 0;
    m_NumberFmt.LeadingZero   = 0;
    m_NumberFmt.Grouping      = 0;
	m_NumberFmt.lpDecimalSep  = m_szDecimalSep;
    m_NumberFmt.lpThousandSep = m_szThousandSep;
	m_NumberFmt.NegativeOrder = 0;

    ATLVERIFY(0 != GetLocaleInfo(LOCALE_USER_DEFAULT,
	                             LOCALE_IDIGITS | LOCALE_RETURN_NUMBER,
								 LPTSTR(&m_NumberFmt.NumDigits),
								 sizeof(m_NumberFmt.NumDigits) / sizeof(TCHAR)));

    ATLVERIFY(0 != GetLocaleInfo(LOCALE_USER_DEFAULT,
								 LOCALE_ILZERO | LOCALE_RETURN_NUMBER,
								 LPTSTR(&m_NumberFmt.LeadingZero),
								 sizeof(m_NumberFmt.LeadingZero) / sizeof(TCHAR)));

    ATLVERIFY(0 != GetLocaleInfo(LOCALE_USER_DEFAULT,
								 LOCALE_SDECIMAL,
								 m_szDecimalSep,
								 _countof(m_szDecimalSep)));

    ATLVERIFY(0 != GetLocaleInfo(LOCALE_USER_DEFAULT,
								 LOCALE_STHOUSAND,
								 m_szThousandSep,
								 _countof( m_szThousandSep)));

	ATLVERIFY(0 != GetLocaleInfo(LOCALE_USER_DEFAULT,
								 LOCALE_INEGNUMBER | LOCALE_RETURN_NUMBER,
								 LPTSTR(&m_NumberFmt.NegativeOrder),
								 sizeof(m_NumberFmt.NegativeOrder) / sizeof(TCHAR)));

	TCHAR szBuffer[100]; // Huge, but just in case.
	ATLVERIFY(0 != GetLocaleInfo(LOCALE_USER_DEFAULT,
		                         LOCALE_SGROUPING,
								 szBuffer,
								 _countof(szBuffer)));

	// Now the ugly part. Have to convert from something like string "3;2;0" to
	// int 32.
	for (const TCHAR *pParseGrpS = szBuffer; *pParseGrpS != _T('\0'); ++pParseGrpS)
	{
        if ((*pParseGrpS >= _T('1')) && (*pParseGrpS <= _T('9')))
            m_NumberFmt.Grouping = m_NumberFmt.Grouping * 10 + (*pParseGrpS - _T('0'));

        if ((*pParseGrpS != _T('0')) && !pParseGrpS[1])
			m_NumberFmt.Grouping *= 10;
    }
}

// Formats the given integer number according to the locale, adding for example
// thousand separators. The only difference to the default formatting is that
// no decimal digits are displayed.
void FormatInteger(unsigned __int64 uiValue,TCHAR *szBuffer,
				   unsigned uiBufferSize)
{
	const unsigned MIN_CHAR_COUNT = 80;				// The documentation for _ui64tot_s says 65 characters max.
	ATLASSERT(uiBufferSize >= MIN_CHAR_COUNT * 2);  // We certainly need less than that, but I don't know exactly how much.

	TCHAR szNumber[MIN_CHAR_COUNT];
	_ui64tot_s(uiValue,szNumber,_countof(szNumber),10);

	CNumberFmt Fmt;
	Fmt.RetrieveDefaults();
	Fmt.m_NumberFmt.NumDigits = 0;

	ATLVERIFY(0 != GetNumberFormat(LOCALE_USER_DEFAULT,
								   0,
								   szNumber,
								   &Fmt.m_NumberFmt,
								   szBuffer,
								   uiBufferSize));
}

int LastDelimiter(const TCHAR *szString,TCHAR cDelimiter)
{    
	int iLength = lstrlen(szString);

	for (int i = iLength - 1; i >= 0; i--)
	{
		if (szString[i] == cDelimiter)
			return i;
	}

	return -1;
}

int FirstDelimiter(const TCHAR *szString,TCHAR cDelimiter)
{
	int iLength = lstrlen(szString);

	for (int i = 0; i < iLength; i++)
	{
		if (szString[i] == cDelimiter)
			return i;    
	}

	return -1;
}

bool ExtractFilePath(TCHAR *szFileName)
{
	int iLastDelimiter = LastDelimiter(szFileName,'\\');

	if (LastDelimiter(szFileName,'/') > iLastDelimiter)
		iLastDelimiter = LastDelimiter(szFileName,'/');

	if (iLastDelimiter == -1)
		return false;

	szFileName[iLastDelimiter + 1] = '\0';	
	return true;
}

bool ExtractFileName(TCHAR *szFileName)
{
	int iLastDelimiter = LastDelimiter(szFileName,'\\');

	if (LastDelimiter(szFileName,'/') > iLastDelimiter)
		iLastDelimiter = LastDelimiter(szFileName,'/');

	if (iLastDelimiter == -1)
		return false;

	lstrcpy(szFileName,szFileName + (iLastDelimiter + 1));
	return true;
}

bool ChangeFileExt(TCHAR *szFileName,const TCHAR *szFileExt)
{
	unsigned int uiLastDelimiter = LastDelimiter(szFileName,'.');

	const TCHAR *pFileExt = szFileExt;
	TCHAR *pFileName = szFileName + uiLastDelimiter;

	do
	{
		*pFileName = *pFileExt;
		*pFileName++;
	} while (*pFileExt++);

	return true;
}

void ForceSlashDelimiters(TCHAR *szFileName)
{
	unsigned int uiLength = lstrlen(szFileName);

	for (unsigned int i = 0; i < uiLength; i++)
	{
		if (szFileName[i] == '\\')
			szFileName[i] = '/';
	}
}

/*
	ComparePaths
	------------
	Works exactly like the strcmp function except that the path delimiters /
	and \ are concidered to be equal.
*/
int ComparePaths(const TCHAR *szPath1,const TCHAR *szPath2)
{
	int iResult = 0;

	while (true)
	{
		while (!(iResult = *(unsigned char *)szPath1 - *(unsigned char *)szPath2) && *szPath2)
			++szPath1,++szPath2;

		if (*(unsigned char *)szPath1 != '\\' && *(unsigned char *)szPath1 != '/')
			break;

		if (*(unsigned char *)szPath2 != '\\' && *(unsigned char *)szPath2 != '/')
			break;

		++szPath1,++szPath2;
	}

	if (iResult < 0)
		iResult = -1;
	else if (iResult > 0)
		iResult = 1;

	return iResult;
}

/*
	TrimRight
	---------
	Removes all white-spaces on the right side of a string.
*/
void TrimRight(TCHAR *szString)
{
	for (int i = lstrlen(szString) - 1; i >= 0; i--)
	{
		if (szString[i] == ' ')
			szString[i] = '\0';
		else
			break;
	}
}

void TrimLeft(ckcore::tstring &Str,const ckcore::tchar * const szCharsToRemove)
{
	ckcore::tstring::size_type Pos = Str.find_first_not_of(szCharsToRemove);

	if (Pos != ckcore::tstring::npos)
		Str.erase(0,Pos);
	else
		Str.clear();
}

void TrimRight(ckcore::tstring &Str,const ckcore::tchar * const szCharsToRemove)
{
	ckcore::tstring::size_type Pos = Str.find_last_not_of(szCharsToRemove);

	if (Pos != ckcore::tstring::npos)
		Str.erase(Pos + 1);
	else
		Str.clear();
}

/*
	SkipInteger
	-----------
	Locates the end of the integer and return a pointer to that location.
*/
char *SkipInteger(char *szString)
{
	// Skip all white spaces.
	while (*szString == ' ')
		*szString++;

	// Skip all digits.
	while (isdigit(*szString))
		*szString++;

	return szString;
}

/*
	Converts an ANSI string into UTF-16 (BE). iTargetSize should be the size
	of szTarget counted in wchar_ts.
*/
void AnsiToUnicode(wchar_t *szTarget,const char *szSource,int iTargetSize)
{
	int iConverted = MultiByteToWideChar(::AreFileApisANSI() ? CP_ACP : CP_OEMCP,MB_PRECOMPOSED,
		szSource,(int)strlen(szSource) + 1,szTarget,iTargetSize);

	if (iConverted == iTargetSize)
		szTarget[iTargetSize - 1] = '\0';
}

/*
	Converts a UTF-16 (BE) string into ANSI. iTargetSize should be the size
	of szTarget counted in bytes.
*/
void UnicodeToAnsi(char *szTarget,const wchar_t *szSource,int iTargetSize)
{
	int iConverted = WideCharToMultiByte(::AreFileApisANSI() ? CP_ACP : CP_OEMCP,0,
		szSource,(int)lstrlenW(szSource) + 1,szTarget,iTargetSize,NULL,NULL);

	if (iConverted == iTargetSize)
		szTarget[iTargetSize - 1] = '\0';
}

void GetCygwinFileName(const TCHAR *szFileName,TCHAR *szCygwinFileName)
{
	// Check if UNC path.
	size_t uiLength = lstrlen(szFileName);
	if (uiLength > 2 && szFileName[0] == '\\' && szFileName[1] == '\\')
	{
		lstrcpy(szCygwinFileName,szFileName);

		// Replace all backslashes by slashes.
		for (size_t i = 0; i < uiLength; i++)
		{
			if (szCygwinFileName[i] == '\\')
				szCygwinFileName[i] = '/';
		}
	}
	else	// If not UNC path convert to proper cygwin path.
	{
		lstrcpy(szCygwinFileName,_T("/cygdrive/"));

		// Copy the drive letter.
		szCygwinFileName[10] = szFileName[0];
		szCygwinFileName[11] = '/';
		szCygwinFileName[12] = '\0';

		lstrcat(szCygwinFileName,szFileName + 3);

		// Replace all backslashes by slashes.
		uiLength = lstrlen(szCygwinFileName);
		for (size_t i = 13; i < uiLength; i++)
		{
			if (szCygwinFileName[i] == '\\')
				szCygwinFileName[i] = '/';
		}
	}
}

/*
	lsnprintf_s
	-----------
	A secure version of lsprintf which does not allow buffer overruns and alwways
	terminates with a null character.
*/
void lsnprintf_s(TCHAR *szBuffer,int iBufferLength,const TCHAR *szFormatString,...)
{
	va_list vlArgs;
	va_start(vlArgs,szFormatString);

#ifdef UNICODE
	int iCount = _vsnwprintf(szBuffer,iBufferLength - 1,szFormatString,vlArgs);
#else
	int iCount = _vsnprintf(szBuffer,iBufferLength - 1,szFormatString,vlArgs);
#endif

	if (iCount < 0 || iCount == iBufferLength)
		szBuffer[iBufferLength - 1] = '\0';
}