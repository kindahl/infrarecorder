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
#include <atlbase.h>
#include "CheckFmtStrPlaceholders.h"	// Include file for this module comes first.
#include "StringUtil.h"
#include <ckcore/exception.hh>

#define MAX_PARAM_POSITION ((unsigned)100)

CFmtStrPlaceholderAnalysis::~CFmtStrPlaceholderAnalysis(void)
{
    for (CVectorOfFmtStrPlaceholder::const_iterator it = m_Placeholders.begin();
         it != m_Placeholders.end();
         ++it)
    {
        const CFmtStrPlaceholder * const pElem = *it;
        delete pElem;
    }
}

void CFmtStrPlaceholderAnalysis::ClearForReuse(void)
{
    // Optimised for reuse: this routine does not free any malloc'ed memory.
    for (unsigned i = 0; i < m_uPlaceholderCount; ++i)
    {
        m_Placeholders[i]->ClearForReuse();
    }

    Init();
}

void CFmtStrPlaceholderAnalysis::InsertPlaceholder(
						const unsigned uPlaceholderPos,  // The first position is number 1.
                        const bool bIsPositionalPlaceholder,
                        const ckcore::tchar * const szPlaceholderStr,
                        const unsigned uPlaceholderStrLen)
{
    ATLASSERT(szPlaceholderStr != NULL);
    ATLASSERT(uPlaceholderStrLen > 0);

    if (uPlaceholderPos < 1 || uPlaceholderPos > MAX_PARAM_POSITION)
        throw ckcore::Exception2(ckcore::string::formatstr( _T("Invalid placeholder number %u."),uPlaceholderPos).c_str());

    if (m_uPlaceholderCount > 0)
    {
        if (m_bPositionalPlaceholders != bIsPositionalPlaceholder)
            throw ckcore::Exception2(_T("Cannot mix positional and non-positional placeholders."));
    }

    m_bPositionalPlaceholders = bIsPositionalPlaceholder;

    m_uPlaceholderCount = _cpp_max(m_uPlaceholderCount,uPlaceholderPos);

    if (m_Placeholders.size() < m_uPlaceholderCount)
        m_Placeholders.resize(m_uPlaceholderCount);

    if (m_Placeholders[uPlaceholderPos - 1] == NULL)
        m_Placeholders[uPlaceholderPos - 1] = new CFmtStrPlaceholder;

    CFmtStrPlaceholder * const plh = m_Placeholders[uPlaceholderPos - 1];

    // According to the format string specification, you can reference the same
    // argument from several placeholders, an each placeholder can have
    // different format specifiers. However, we don't allow it here, because of
    // the extra complication that would add to this testing class. Fortunately,
	// this feature is rarely used.
    if (!plh->m_Placeholder.empty())
        throw ckcore::Exception2(ckcore::string::formatstr(_T("Placeholder %u already used."),uPlaceholderPos).c_str());
    
    plh->m_Placeholder.insert(plh->m_Placeholder.begin(),
                              szPlaceholderStr,szPlaceholderStr + uPlaceholderStrLen);
}

// ----------------------------------------------------------------------------
// WARNING: This format string parser is not complete, and will not process all
//          possible cases and find all possible errors. Writing a full parser
//          which could also accomodate cross-platform differences is not a
//			trivial task.I have made however a reasonably effort to error on
//			all unsupported cases.
// ----------------------------------------------------------------------------

// Advances p as necessary.
static bool IsPlaceholderLength(const ckcore::tchar ** const p)
{
    // According to the Microsoft documentation, these are the possible
	// indicators for field [length] inside a format string:
    //   {h | l | ll | I | I32 | I64}

    switch (**p)
    {
		case _T('h'):
			++*p;
			return true;

		case _T('l'):
			++*p;

			if (**p == 'l')
				++*p;

			return true;
	        
		case _T('I'):
			{
				++*p;

				const ckcore::tchar *t = *p;

				if (*t == _T('3'))
				{
					++t;

					if (*t == _T('2'))
					{
						++t;
						*p = t;
						return true;
					}
				}
				else if (*t == _T('6'))
				{
					++t;

					if (*t == _T('4'))
					{
						++t;
						*p = t;
						return true;
					}
				}

				return true;  // An 'I' alone.
			}

		default:
			return false;
    }
}

static bool IsPlaceholderType(const ckcore::tchar c)
{
    switch (c)
    {
		case _T('d'):
		case _T('i'):
		case _T('u'):
		case _T('f'):
		case _T('F'):
		case _T('e'):
		case _T('E'):
		case _T('g'):
		case _T('G'):
		case _T('x'):
		case _T('X'):
		case _T('o'):
		case _T('s'):
		case _T('c'):
		case _T('p'):
		case _T('n'):
			return true;

		default:
			return false;
    }
}

static bool IsPlaceholderFlag(const ckcore::tchar c)
{
    switch ( c )
    {
		case _T('-'):
		case _T('+'):
		case _T('0'):
		case _T(' '):
		case _T('#'):
			return true;

		default:
			return false;
    }
}

static bool ReadPositionalParam(const ckcore::tchar ** const p,unsigned * const uPlaceholderPos)
{
    // Skip any digits at the current position, and look at the next character;
    // If it's a dollar ($) symbol, then this was a positional parameter.
    // If not, it may be the [width] field or something else.
    const ckcore::tchar *scan = *p;

    if (*scan < _T('0') || *scan > _T('9'))
        return false;

    unsigned uVal = 0;

    do
    {
        // If the value of MAX_PARAM_POSITION changes, make sure that it' still
        // ten times lower than the maximum integer value. Otherwise, we need
        // to explicitly test here against integer overflow when multiplying.
        ATLASSERT(MAX_PARAM_POSITION <= 1000);

        uVal *= 10;
        uVal += *scan - _T('0');

        if (uVal > MAX_PARAM_POSITION)
            throw ckcore::Exception2(_T("Integer too big parsing the positional parameter."));

        ++scan;
    }
    while (*scan >= _T('0') && *scan <= _T('9'));

    if (*scan != _T('$'))
        return false;

    *uPlaceholderPos = uVal;
    *p = scan + 1;
    return true;
}

void CFmtStrPlaceholderAnalysis::AnalyzeFormatString(const ckcore::tchar * const szFmtStr)
{
    // The analysis results should be empty upon entry.
    ATLASSERT(m_uPlaceholderCount == 0);

    // printf format string format
    //
    //    %[parameter] [flags] [width] [.precision] [length] type 
    //
    //  [parameter] is an optional positional argument specifier like "1$".
    //  That means, old-style "%s" would look like "%1$s" for the first
	//  positional argument.
    unsigned uPlaceholderPos = 1;

    for (const ckcore::tchar * p = szFmtStr; *p != _T('\0');)
    {
        if (*p != _T('%'))
        {
            ++p;
            continue;
        }

        const ckcore::tchar * const placeholderStart = p;

        ++p;

        if (*p == _T('%'))
        {
            ++p;
            continue;
        }

        try
        {
            if (*p == _T('\0'))
                throw ckcore::Exception2(_T("Invalid placeholder at the end of the string."));

            const bool bIsPositionalParam = ReadPositionalParam(&p,&uPlaceholderPos);

            while (IsPlaceholderFlag(*p))
                ++p;

            // Skip the optional width.
            while (*p >= _T('1') && *p <= _T('9'))
                ++p;

            // Skip the optional precision.
            if (*p == _T('.'))
            {
                ++p;

                while (*p >= _T('0') && *p <= _T('9'))
                    ++p;
            }

            // Skip the optional length.
            IsPlaceholderLength(&p);
        
            if (IsPlaceholderType(*p))
            {
                ++p;
                InsertPlaceholder(uPlaceholderPos,
                                  bIsPositionalParam,
                                  placeholderStart,
                                  p - placeholderStart);

                ++uPlaceholderPos;
                continue;
            }

            throw ckcore::Exception2(_T("Invalid or unsupported placeholder."));
        }
        catch (const std::exception &e)
        {
			ckcore::rethrow_with_pfx(e,_T("Error at line position %d: "),
                                     placeholderStart - szFmtStr + 1);
        }
    }

    // Check here that there are no empty placeholders.
    //
    // Microsoft's implementation of _printf_p does generate a run-time error
    // if any of the parameters are left unreferenced. This can only be detected
    // if the unreferenced parameter is in the middle of other parameters.
    // For example, for a format string like "%1$s %3$s", the run-time library
    // complains that parameter 2 is not used.
    for (unsigned i = 0; i < m_uPlaceholderCount; ++i)
    {
        if (m_Placeholders[i] == NULL || m_Placeholders[i]->m_Placeholder.empty())
            throw ckcore::Exception2(ckcore::string::formatstr(_T("Positional argument %u not referenced in the format string."),i + 1).c_str());
    }
}

void CFmtStrPlaceholderAnalysis::ComparePlaceholderAnalyses(const CFmtStrPlaceholderAnalysis * const pAnalysis1,
                                                            const CFmtStrPlaceholderAnalysis * const pAnalysis2)
{
    // NOTE: This method is static.
    if (pAnalysis1->m_uPlaceholderCount != pAnalysis2->m_uPlaceholderCount)
    {
        throw ckcore::Exception2(_T("The strings being compared do not contain the same number of placeholders."));
    }

    if (pAnalysis1->m_uPlaceholderCount > 0 &&
        pAnalysis1->m_bPositionalPlaceholders != pAnalysis2->m_bPositionalPlaceholders)
    {
        throw ckcore::Exception2(_T("The strings being compared are not using the same type of placeholders (positional vs non-positional)."));
    }

    for (unsigned int i = 0; i < pAnalysis1->m_uPlaceholderCount; ++i)
    {
        const CFmtStrPlaceholder * const plh1 = pAnalysis1->m_Placeholders[i];
        const CFmtStrPlaceholder * const plh2 = pAnalysis2->m_Placeholders[i];

        if (plh1->m_Placeholder != plh2->m_Placeholder)
        {
            throw ckcore::Exception2(
                    ckcore::string::formatstr(_T("Placeholder number %u in the second string (\"%s\") differs from the corresponding one in the first string (\"%s\")."),
                                              i + 1,
                                              plh2->m_Placeholder.c_str(),
                                              plh1->m_Placeholder.c_str()).c_str());
        }
    }
}

void ComparePlaceholdersInTranslatedStr(const ckcore::tchar * const szSectionName,
                                        const unsigned uStringId,
                                        const ckcore::tchar * const szReferencedFmtStr,
                                        const ckcore::tchar * const szTranslatedFmtStr,
                                        CFmtStrPlaceholderAnalysis * const pAnalysis1,	// Reused for performance
                                        CFmtStrPlaceholderAnalysis * const pAnalysis2)	// Reused for performance
{
    pAnalysis1->ClearForReuse();
    pAnalysis2->ClearForReuse();

    try
    {
        try
        {
            pAnalysis1->AnalyzeFormatString(szReferencedFmtStr);
        }
        catch (const std::exception &e)
        {
            ckcore::rethrow_with_pfx(e,_T("Error parsing the reference string: "));
        }

        try
        {
            pAnalysis2->AnalyzeFormatString(szTranslatedFmtStr);
        }
        catch ( const std::exception &e)
        {
            ckcore::rethrow_with_pfx(e,_T("Error parsing the translated string: "));
        }

        CFmtStrPlaceholderAnalysis::ComparePlaceholderAnalyses(pAnalysis1,pAnalysis2);
    }
    catch (const std::exception &e)
    {
        ckcore::rethrow_with_pfx(e,_T("Error in section [%s], string ID 0x%X: "),
                                 szSectionName,uStringId);
    }
}