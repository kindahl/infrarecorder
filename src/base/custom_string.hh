/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
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
#include "string_util.hh"

#define CSTRING_DEFAULT_LENGTH			256
#define CSTRING_MEM_INCRATIO			2

class CCustomString
{
private:
    TCHAR *m_szString;
    unsigned int m_uiSize;
    unsigned int m_uiPos;

public:
    CCustomString()
    {
        m_uiSize = CSTRING_DEFAULT_LENGTH;
        m_uiPos = 0;
        m_szString = new TCHAR[m_uiSize];
    }

    CCustomString(unsigned int uiSize)
    {
        m_uiSize = uiSize;
        m_uiPos = 0;
        m_szString = new TCHAR[m_uiSize];
    }

    ~CCustomString()
    {
        delete [] m_szString;
    }

    void Reset()
    {
        m_uiPos = 0;
    }

    void ReAllocate(unsigned int uiSize)
    {
        m_uiSize = uiSize;
        m_uiPos = 0;

        delete [] m_szString;
        m_szString = new TCHAR[m_uiSize];
    }

    void Append(TCHAR c)
    {
        if (m_uiPos >= m_uiSize)
        {
            m_uiSize *= CSTRING_MEM_INCRATIO;

            TCHAR *szNewString = new TCHAR[m_uiSize];
            lstrncpy(szNewString,m_szString,m_uiSize / CSTRING_MEM_INCRATIO);

            delete [] m_szString;
            m_szString = szNewString;
        }

        m_szString[m_uiPos++] = c;
    }

    void AppendString(const TCHAR *szString)
    {
        size_t iStrLen = lstrlen(szString);
        for (size_t i = 0; i < iStrLen; i++)
            Append(szString[i]);
    }

    void CopyFromHtml(const TCHAR *szNewString)
    {
        size_t iNewStrLen = lstrlen(szNewString);

        Reset();

        for (size_t i = 0; i < iNewStrLen; i++)
        {
            if (szNewString[i] == '&')
                AppendString(_T("&amp;"));
            else if (szNewString[i] == '"')
                AppendString(_T("&quot;"));
            else if (szNewString[i] == '<')
                AppendString(_T("&lt;"));
            else if (szNewString[i] == '>')
                AppendString(_T("&gt;"));
            else
                Append(szNewString[i]);
        }

        Append('\0');
    }

    void CopyFrom(const TCHAR *szNewString)
    {
        lstrcpy(m_szString,szNewString);
        m_uiPos = lstrlen(szNewString);
    }

    unsigned int Length()
    {
        return m_uiPos;
    }

    operator TCHAR *()
    {
        return m_szString;
    }
};

class CCustomStringA
{
private:
    char *m_szString;
    unsigned int m_uiSize;
    unsigned int m_uiPos;

public:
    CCustomStringA()
    {
        m_uiSize = CSTRING_DEFAULT_LENGTH;
        m_uiPos = 0;
        m_szString = new char[m_uiSize];
    }

    CCustomStringA(unsigned int uiSize)
    {
        m_uiSize = uiSize;
        m_uiPos = 0;
        m_szString = new char[m_uiSize];
    }

    ~CCustomStringA()
    {
        delete [] m_szString;
    }

    void Reset()
    {
        m_uiPos = 0;
    }

    void ReAllocate(unsigned int uiSize)
    {
        m_uiSize = uiSize;
        m_uiPos = 0;

        delete [] m_szString;
        m_szString = new char[m_uiSize];
    }

    void Append(char c)
    {
        if (m_uiPos >= m_uiSize)
        {
            m_uiSize *= CSTRING_MEM_INCRATIO;

            char *szNewString = new char[m_uiSize];
            strcpy(szNewString,m_szString);

            delete [] m_szString;
            m_szString = szNewString;
        }

        m_szString[m_uiPos++] = c;
    }

    void CopyFrom(const char *szNewString)
    {
        strcpy(m_szString,szNewString);
        m_uiPos = (unsigned int)strlen(szNewString);
    }

    unsigned int Length()
    {
        return m_uiPos;
    }

    operator char *()
    {
        return m_szString;
    }
};
