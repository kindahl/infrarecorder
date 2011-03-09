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
#include <ckcore/buffer.hh>
#include <ckcore/file.hh>
#include <ckcore/types.hh>
#include <lame.h>
#include "config_dlg.hh"
#include "lame_base.hh"

class LameEncoder : public LameBase
{
private:
    enum
    {
        BUFFER_FACTOR = 4096
    };

private:
    ckcore::File file_;
    int num_channels_;
    int sample_rate_;
    int bit_rate_;

    ckcore::Buffer<unsigned char,int> buffer_;
    CEncoderConfig &encoder_cfg_;

public:
    LameEncoder(const TCHAR *file_path,
                int num_channels,int sample_rate,int bit_rate,
                CEncoderConfig &encoder_cfg);

    bool initialize();
    __int64 encode(unsigned char *buffer,__int64 data_size);
    __int64 flush();
};
