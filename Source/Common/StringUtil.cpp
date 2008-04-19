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
#ifdef UNICODE
#include <wchar.h>
#else
#include <stdio.h>
#endif
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

int LastDelimiter(const TCHAR *szString,TCHAR cDelimiter)
{    
	int iLength = lstrlen(szString);

	for (int i = iLength - 1; i >= 0; i--)
	{
		if (/*tolower(*/szString[i]/*)*/ == cDelimiter)
			return i;
	}

	return -1;
}

int LastDelimiterA(const char *szString,char cDelimiter)
{    
	int iLength = (int)strlen(szString);

	for (int i = iLength - 1; i >= 0; i--)
	{
		if (szString[i] == cDelimiter)
			return i;
	}

	return -1;
}

int LastDelimiterW(const wchar_t *szString,wchar_t cDelimiter)
{    
	int iLength = lstrlenW(szString);

	for (int i = iLength - 1; i >= 0; i--)
	{
		if (szString[i] == cDelimiter)
			return i;
	}

	return -1;
}

/*
	UPDATE 2006-08-09
	-----------------
	Removed LastDelimiter*Ex version because they where not explicity needed.
*/
/*
	LastDelimiterEx
	---------------
	Same as LastDelimiter but is case sensitive.
*/
/*int LastDelimiterEx(const TCHAR *szString,char cDelimiter)
{    
	int iLength = lstrlen(szString);

	for (int i = iLength - 1; i >= 0; i--)
	{
		if (szString[i] == cDelimiter)
			return i;
	}

	return -1;
}*/

/*
	LastDelimiterExA
	----------------
	Same as LastDelimiterA but is case sensitive.
*/
/*int LastDelimiterExA(const char *szString,char cDelimiter)
{    
	int iLength = strlen(szString);

	for (int i = iLength - 1; i >= 0; i--)
	{
		if (szString[i] == cDelimiter)
			return i;
	}

	return -1;
}*/

int FirstDelimiter(const TCHAR *szString,TCHAR cDelimiter)
{
	int iLength = lstrlen(szString);

	for (int i = 0; i < iLength; i++)
	{
		if (/*tolower(*/szString[i]/*)*/ == cDelimiter)
			return i;    
	}

	return -1;
}

// UPDATE: 2008-04-13 works but is not needed.
/*int FindInString(const TCHAR *szString1,const TCHAR *szString2)
{
	const TCHAR *szResult;

#ifdef UNICODE
	szResult = wcsstr(szString1,szString2);
#else
	szResult = strstr(szString1,szString2);
#endif

	return (szResult == NULL) ? -1 : (int)(szResult - szString1);
}*/

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

	/*TCHAR *szResult = SubString(szFileName,iLastDelimiter + 1,lstrlen(szFileName) - iLastDelimiter - 1);
		lstrcpy(szFileName,szResult);
	delete [] szResult;*/
	lstrcpy(szFileName,szFileName + (iLastDelimiter + 1));

	return true;
}

bool ExtractFileExt(const TCHAR *szFileName,TCHAR *szFileExt)
{
	unsigned int uiLastDelimiter = LastDelimiter(szFileName,'.');

	const TCHAR *pFileExt = szFileName + uiLastDelimiter;
	lstrcpy(szFileExt,pFileExt);

	return uiLastDelimiter != 0;
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
	const TCHAR *pFileName = szFileName;
	lstrcpy(szCygwinFileName,_T("/cygdrive/"));

	// Copy the drive letter.
	szCygwinFileName[10] = szFileName[0];
	szCygwinFileName[11] = '/';
	szCygwinFileName[12] = '\0';

	lstrcat(szCygwinFileName,szFileName + 3);

	// Replace all backslashes by slashes.
	unsigned int uiLength = lstrlen(szCygwinFileName);
	for (unsigned int i = 13; i < uiLength; i++)
	{
		if (szCygwinFileName[i] == '\\')
			szCygwinFileName[i] = '/';
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
