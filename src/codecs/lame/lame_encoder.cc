/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2011 Christian Kindahl
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
#include "lame_encoder.hh"

LameEncoder::LameEncoder(const TCHAR *file_path,
                         int num_channels,int sample_rate,int bit_rate,
                         CEncoderConfig &encoder_cfg)
    : file_(file_path),
      num_channels_(num_channels),sample_rate_(sample_rate),bit_rate_(bit_rate),
      encoder_cfg_(encoder_cfg)
{
}

bool LameEncoder::initialize()
{
	// Setup format settings.
	lame_set_num_channels(lame_gfp_,num_channels_);
	lame_set_in_samplerate(lame_gfp_,sample_rate_);

	// Configure the encoder.
	switch (encoder_cfg_.m_iPreset)
	{
		case 0:		// Custom.
		{
			if (encoder_cfg_.m_iEncodeQuality == CONFIG_EQ_FAST)
				lame_set_quality(lame_gfp_,7);
			else if (encoder_cfg_.m_iEncodeQuality == CONFIG_EQ_HIGH)
				lame_set_quality(lame_gfp_,2);

			if (encoder_cfg_.m_bMono)
				lame_set_mode(lame_gfp_,MONO);

			if (encoder_cfg_.m_bBitrateMode)
			{
				// Bitrate mode.
				if (encoder_cfg_.m_bConstantBitrate)
				{
					// CBR.
					lame_set_VBR(lame_gfp_,vbr_off);
					lame_set_brate(lame_gfp_,encoder_cfg_.m_iBitrate);
				}
				else
				{
					// ARB.
					lame_set_VBR(lame_gfp_,vbr_abr);
					lame_set_VBR_mean_bitrate_kbps(lame_gfp_,encoder_cfg_.m_iBitrate);
				}
			}
			else
			{
				// Quality mode.
				lame_set_VBR(lame_gfp_,encoder_cfg_.m_bFastVBR ? vbr_mtrh : vbr_rh);
				lame_set_VBR_q(lame_gfp_,encoder_cfg_.m_iQuality);
			}

            break;
		}

		case 1:		// Medium.
			lame_set_preset(lame_gfp_,MEDIUM);
			break;

		case 2:		// Standard.
			lame_set_preset(lame_gfp_,STANDARD);
			break;

		case 3:		// Extreme.
			lame_set_preset(lame_gfp_,EXTREME);
			break;

		case 4:		// Insane.
			lame_set_preset(lame_gfp_,INSANE);
			break;
    }

	// Initialize parameters.
	if (lame_init_params(lame_gfp_) < 0)
		return false;

	// Resize encode buffer.
    buffer_.resize(num_channels_ * ((bit_rate_ / sample_rate_) >> 3) * BUFFER_FACTOR);

	// Open file.
	return file_.open(ckcore::File::ckOPEN_WRITE);
}

__int64 LameEncoder::encode(unsigned char *buffer,__int64 data_size)
{
	if (data_size > 0xffffffff)
		return -1;

    if (!file_.test())
		return -1;

	unsigned int sample_size = (bit_rate_ / sample_rate_) >> 3;
	unsigned int num_samples = (static_cast<unsigned int>(data_size) / sample_size) / num_channels_;

	int written = lame_encode_buffer_interleaved(lame_gfp_,reinterpret_cast<short int *>(buffer),
                                                 num_samples,buffer_,buffer_.size());

	if (written > 0)
	{
		if (file_.write(buffer_,written) == -1)
			return -1;
	}
	else if (written < 0)
	{
		return -1;
	}

	return (__int64)written;
}

__int64 LameEncoder::flush()
{
	return lame_encode_flush(lame_gfp_,buffer_,buffer_.size());
}
