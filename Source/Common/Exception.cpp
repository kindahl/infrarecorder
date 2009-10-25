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

#include "stdafx.h"
#include <comdef.h>
#include <ckcore/string.hh>
#include "StringUtil.h"
#include "Exception.h"

ir_error::ir_error(const ckcore::tchar *const szErrMsg) : m_ErrMsg(szErrMsg)
{
}

ir_error::ir_error(const ckcore::tstring &ErrMsg) : m_ErrMsg(ErrMsg)
{
}

/**
 * Returns the error message.
 * @return The error message.
 */
const ckcore::tchar *ir_error::lwhat(void) const throw()
{
	return m_ErrMsg.c_str();
}

const char *ir_error::what(void) const throw()
{
    // Please use the wrapper GetExceptErrMsg() instead of calling what()
	// on objects derived from std::exception .
    ATLASSERT(false);

    return "Internal error: No ANSI message available.";
}

/**
 * Returns the exception message of the given exception object. The function
 * tests if the exception is an ir_error and in that case returns its message.
 * @param [in] e The exception object.
 * @return The error message of the exception.
 */
ckcore::tstring GetExceptMsg(const std::exception &e)
{
    const ir_error *const pE = dynamic_cast<const ir_error *>(&e);

    if (pE != NULL)
		return pE->lwhat();

	return ckcore::string::ansi_to_auto<1024>(pE->what());
}

/**
 * Rethrows the exception with a prefix message.
 * @param [in] e The exception to rethrow.
 * @param [in] szFormatStr The message format string.
 */
void RethrowWithPrefix(const std::exception &e,const ckcore::tchar *const szFormatStr,...)
{
    ckcore::tstring Msg;

	va_list args;
	va_start(args,szFormatStr);

	SlowFormatStrV(Msg,szFormatStr,args);

	va_end(args);

    const ckcore::tstring ErrMsg = GetExceptMsg(e);
    Msg.append(ErrMsg);

    throw ir_error(Msg.c_str());
}

ir_error CreateIrErrorFromHresult(const HRESULT hRes,const ckcore::tchar *const szPrefixFormatStr,...)
{
    ckcore::tstring Msg;

	if (szPrefixFormatStr != NULL)
	{
		va_list args;
		va_start(args,szPrefixFormatStr);

		SlowFormatStrV(Msg,szPrefixFormatStr,args);

		va_end(args);
	}

    const ckcore::tstring ErrMsg = _com_error(hRes).ErrorMessage();
    Msg.append(ErrMsg);

    throw ir_error(Msg.c_str());
}

ir_error CreateInternalError(const tchar *file,int line)
{
	ckcore::tstringstream Msg;
	Msg << _T("Internal error in ") << file << _T(" at line ") << line
		<< _T(".");

	throw ir_error(Msg.str().c_str());
}
