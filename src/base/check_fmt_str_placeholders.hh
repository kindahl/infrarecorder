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

#include <ckcore/types.hh>
#include <vector>

class CFmtStrPlaceholder
{
public:
    // In the future, we will probably have here more data about this placeholder.
    ckcore::tstring m_Placeholder;  // Examples: "%s", "%1$s", "%3d".

    void ClearForReuse(void)
    {
        m_Placeholder.clear();
    }
};

typedef std::vector<CFmtStrPlaceholder *> CVectorOfFmtStrPlaceholder;

class CFmtStrPlaceholderAnalysis
{
public:
    CFmtStrPlaceholderAnalysis(void) { Init(); }
    ~CFmtStrPlaceholderAnalysis(void);

    void AnalyzeFormatString(const ckcore::tchar *szFmtStr);

    static void ComparePlaceholderAnalyses(const CFmtStrPlaceholderAnalysis *pAnalysis1,
                                           const CFmtStrPlaceholderAnalysis *pAnalysis2);
    void ClearForReuse(void);

protected:
    
    unsigned m_uPlaceholderCount;	// Always <= m_placeholders.size(). Note that, when reusing an object,
                                    // m_placeholders.size() could be bigger.

    CVectorOfFmtStrPlaceholder m_Placeholders;

    bool m_bPositionalPlaceholders;	// Like "%1$s", as opposed to "%s".

    void Init(void)
    {
        m_uPlaceholderCount = 0;
        m_bPositionalPlaceholders = false;
    }

    void InsertPlaceholder(unsigned uPlaceholderPos,
                           bool bIsPositionalPlaceholder,
                           const ckcore::tchar *szPlaceholderStr,
                           unsigned uPlaceholderStrLen);
};

void ComparePlaceholdersInTranslatedStr(const ckcore::tchar *szSectionName,
                                        unsigned uStringId,
                                        const ckcore::tchar *szReferencedFmtStr,
                                        const ckcore::tchar *szTranslatedFmtStr,
                                        CFmtStrPlaceholderAnalysis *pAnalysis1,
                                        CFmtStrPlaceholderAnalysis *pAnalysis2);
