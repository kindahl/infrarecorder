/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2008 Christian Kindahl
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

#ifndef _T
#define _T TEXT
#endif

TCHAR *IncludeTrailingBackslash(TCHAR *szPath);
TCHAR *ExcludeTrailingBackslash(TCHAR *szPath);
TCHAR *SubString(const TCHAR *szText,unsigned int uiStart,unsigned int uiLength);

void FormatBytes(TCHAR *szBuffer,unsigned __int64 iBytes);
void FormatBytesEx(TCHAR *szBuffer,unsigned __int64 iBytes);
int LastDelimiter(const TCHAR *szString,TCHAR cDelimiter);
int LastDelimiterA(const char *szString,char cDelimiter);
int LastDelimiterW(const wchar_t *szString,wchar_t cDelimiter);
//int LastDelimiterEx(const TCHAR *szString,char cDelimiter);
//int LastDelimiterExA(const char *szString,char cDelimiter);
int FirstDelimiter(const TCHAR *szString,TCHAR cDelimiter);
//int FindInString(const TCHAR *szString1,const TCHAR *szString2);
bool ExtractFilePath(TCHAR *szFileName);
bool ExtractFileName(TCHAR *szFileName);
bool ExtractFileExt(const TCHAR *szFileName,TCHAR *szFileExt);
bool ChangeFileExt(TCHAR *szFileName,const TCHAR *szFileExt);
void ForceSlashDelimiters(TCHAR *szFileName);
int ComparePaths(const TCHAR *szPath1,const TCHAR *szPath2);

void TrimRight(TCHAR *szString);

char *SkipInteger(char *szString);

void AnsiToUnicode(wchar_t *szTarget,const char *szSource,int iTargetSize);
void UnicodeToAnsi(char *szTarget,const wchar_t *szSource,int iTargetSize);

void GetCygwinFileName(const TCHAR *szFileName,TCHAR *szCygwinFileName);

void lsnprintf_s(TCHAR *szBuffer,int iBufferLength,const TCHAR *szFormatString,...);

#ifdef UNICODE
#define lstrncmp wcsncmp
#else
#define lstrncmp strncmp
#endif

#ifdef UNICODE
#define lsscanf swscanf
#else
#define lsscanf sscanf
#endif

#ifdef UNICODE
#define lsprintf swprintf
#else
#define lsprintf sprintf
#endif

#ifdef UNICODE
#define lstrncpy _tcsncpy
#else
#define lstrncpy strncpy
#endif

#ifdef UNICODE
#define lstrncat wcsncat
#else
#define lstrncat strncat
#endif
