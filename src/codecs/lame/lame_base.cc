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

#include "stdafx.hh"
#include <ckcore/string.hh>
#include "lame_base.hh"

tirc_send_message *LameBase::send_message_ = NULL;

LameBase::LameBase()
{
    // Initialize LAME.
    lame_gfp_ = lame_init();

    // Change error, debug and message handlers.
    lame_set_errorf(lame_gfp_,handler_errorf);
    lame_set_debugf(lame_gfp_,handler_debugf);
    lame_set_msgf(lame_gfp_,handler_msgf);
}

LameBase::~LameBase()
{
    // Close LAME.
    lame_close(lame_gfp_);
}

void LameBase::set_callback(tirc_send_message *callback)
{
    send_message_ = callback;
}

bool LameBase::send_message(int type,const TCHAR * msg)
{
    if (send_message_ == NULL)
        return false;

    send_message_(type,msg);
    return true;
}

void LameBase::handler_errorf(const char *format,va_list ap)
{
    // The 64-bit compiler does not support this type of usage of va_start.
#ifdef _M_X64
    send_message(IRC_MESSAGE_ERROR,_T("Unknown error in lame.irc (x64 build)."));
#else
    va_start(ap,format);

    int len = _vscprintf(format,ap) + 1;
    char *msg = new char[len];
    vsprintf(msg,format,ap);

    ckcore::tstring msg_str = ckcore::string::ansi_to_auto<8192>(msg);
    send_message(IRC_MESSAGE_ERROR,msg_str.c_str());

    delete [] msg;
#endif
}

void LameBase::handler_debugf(const char *format,va_list ap)
{
    // Ignore debug messages.
}

void LameBase::handler_msgf(const char *format,va_list ap)
{
#ifdef _M_X64
    // Ignore information messages.
#else
    va_start(ap,format);

    int len = _vscprintf(format,ap) + 1;
    char *msg = new char[len];
    vsprintf(msg,format,ap);

    ckcore::tstring msg_str = ckcore::string::ansi_to_auto<8192>(msg);
    send_message(IRC_MESSAGE_INFO,msg_str.c_str());

    delete [] msg;
#endif
}
