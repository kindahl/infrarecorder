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

#pragma once
#include <ckcore/types.hh>
#include <lame.h>
#include <base/codec_const.hh>

class LameBase
{
private:
    static tirc_send_message *send_message_;

    static void handler_errorf(const char *format,va_list ap);
    static void handler_debugf(const char *format,va_list ap);
    static void handler_msgf(const char *format,va_list ap);

protected:
    lame_global_flags *lame_gfp_;

    static bool send_message(int type,const TCHAR * msg);

public:
    LameBase();
    virtual ~LameBase();

    static void set_callback(tirc_send_message *callback);
};
