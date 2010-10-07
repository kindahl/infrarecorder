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

#include <windows.h>
#include "string_conv.hh"

int StringToInt(const TCHAR *szString)
{
#ifdef UNICODE
	return _wtoi(szString);
#else
	return atoi(szString);
#endif
}

__int64 StringToInt64(const TCHAR *szString)
{
#ifdef UNICODE
	return _wtoi64(szString);
#else
	return _atoi64(szString);
#endif
}

double StringToDouble(const TCHAR *szString)
{
#ifdef UNICODE
	return _wtof(szString);
#else
	return atof(szString);
#endif
}

long StringToLong(const TCHAR *szString)
{
#ifdef UNICODE
	return _wtol(szString);
#else
	return atol(szString);
#endif
}
