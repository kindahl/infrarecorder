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

#include <cxxtest/TestSuite.h>
#include <ckcore/buffer.hh>
#include <ckcore/crcstream.hh>
#include <ckcore/exception.hh>
#include <ckcore/file.hh>
#include <ckcore/filestream.hh>
#include <ckcore/stream.hh>
#include <ckcore/types.hh>
#include <base/codec_manager.hh>

#define TEST_CODE(decoder,encoder)                                      \
{                                                                       \
    ckcore::Buffer<unsigned char> buffer(num_channels * ((bit_rate / sample_rate) >> 3) * 1024);\
                                                                        \
    __int64 read = 0;                                                   \
    unsigned __int64 time = 0;                                          \
    while (true)                                                        \
    {                                                                   \
        read = decoder.irc_decode_process(buffer,buffer.size(),time);   \
        if (read <= 0)                                                  \
            break;                                                      \
                                                                        \
        bool res = encoder.irc_encode_process(buffer,read) >= 0;        \
        TS_ASSERT(res);                                                 \
        if (!res)                                                       \
            break;                                                      \
    }                                                                   \
                                                                        \
    TS_ASSERT(encoder.irc_encode_flush() != -1);                        \
}

#define TEST_INIT(decoder,encoder,src_file,dst_file)                    \
    int num_channels = -1,sample_rate = -1,bit_rate = -1;               \
    unsigned __int64 duration = 0;                                      \
    TS_ASSERT(decoder.irc_decode_init(src_file,num_channels,sample_rate,bit_rate,duration));\
    TS_ASSERT(encoder.irc_encode_init(dst_file,num_channels,sample_rate,bit_rate));

#define TEST_EXIT(decoder,encoder)                                      \
    TS_ASSERT(encoder.irc_encode_exit());                               \
    TS_ASSERT(decoder.irc_decode_exit());

ckcore::tuint32 get_file_crc(const ckcore::tchar * file_path)
{
    ckcore::FileInStream fis(file_path);
    if (!fis.open())
    {
        ckcore::tstringstream msg;
        msg << ckT("could not open \"") << file_path
            << ckT("\" in order to compute its check-sum.");
        throw ckcore::Exception2(msg.str());
    }

    ckcore::CrcStream cs(ckcore::CrcStream::ckCRC_32);
    if (!ckcore::stream::copy(fis,cs))
    {
        ckcore::tstringstream msg;
        msg << ckT("could not compute check-sum of \"") << file_path
            << ckT("\".");
        throw ckcore::Exception2(msg.str());
    }

    return cs.checksum();
}

class CodecTestSuite : public CxxTest::TestSuite
{
public:
    void test_mp3_encoder()
    {
#if 1
        CCodec sndfile_codec;
        TS_ASSERT(sndfile_codec.Load(ckT("codecs\\sndfile.irc")));
        TS_ASSERT_EQUALS(sndfile_codec.irc_capabilities(),IRC_HAS_DECODER | IRC_HAS_ENCODER);

        CCodec lame_codec;
        TS_ASSERT(lame_codec.Load(ckT("codecs\\lame.irc")));
        TS_ASSERT_EQUALS(lame_codec.irc_capabilities(),IRC_HAS_ENCODER | IRC_HAS_CONFIG);

        // Create temporary destination file.
        ckcore::File tmp = ckcore::File::temp(ckT("ir_test"));

        // Initialize the Wave decoder.
        TEST_INIT(sndfile_codec,lame_codec,ckT("..\\..\\..\\src\\tests\\data\\audio\\audio_test_1.wav"),tmp.name().c_str());

        TS_ASSERT_EQUALS(num_channels,2);
        TS_ASSERT_EQUALS(sample_rate,44100);
        TS_ASSERT_EQUALS(bit_rate,705600);
        TS_ASSERT_EQUALS(duration,12000);

        TEST_CODE(sndfile_codec,lame_codec);
        TEST_EXIT(sndfile_codec,lame_codec);

        // Compute CRCs.
        ckcore::tuint32 crc_ref = get_file_crc(ckT("..\\..\\..\\src\\tests\\data\\audio\\audio_test_1-sndfile_lame-standard.mp3"));
        ckcore::tuint32 crc_tst = get_file_crc(tmp.name().c_str());

        // Remove temporary destination file.
        TS_ASSERT(tmp.remove());

        // Compare CRCs.
        TS_ASSERT_EQUALS(crc_ref,crc_tst);

        //std::cout << num_channels << std::endl << sample_rate << std::endl << bit_rate << std::endl << duration << std::endl;
#endif
    }

    void test_wma_encoder()
    {
#if 1
        CCodec sndfile_codec;
        TS_ASSERT(sndfile_codec.Load(ckT("codecs\\sndfile.irc")));
        TS_ASSERT_EQUALS(sndfile_codec.irc_capabilities(),IRC_HAS_DECODER | IRC_HAS_ENCODER);

        CCodec wma_codec;
        TS_ASSERT(wma_codec.Load(ckT("codecs\\wma.irc")));
        TS_ASSERT_EQUALS(wma_codec.irc_capabilities(),IRC_HAS_DECODER | IRC_HAS_ENCODER | IRC_HAS_CONFIG);

        // Create temporary destination file.
        ckcore::File tmp = ckcore::File::temp(ckT("ir_test"));

        // Initialize the Wave decoder.
        TEST_INIT(sndfile_codec,wma_codec,ckT("..\\..\\..\\src\\tests\\data\\audio\\audio_test_1.wav"),tmp.name().c_str());

        TS_ASSERT_EQUALS(num_channels,2);
        TS_ASSERT_EQUALS(sample_rate,44100);
        TS_ASSERT_EQUALS(bit_rate,705600);
        TS_ASSERT_EQUALS(duration,12000);

        TEST_CODE(sndfile_codec,wma_codec);
        TEST_EXIT(sndfile_codec,wma_codec);

        // WMAs seems to inherit some randomness making a CRC comparison
        // usless, instead compare file sizes for now.
        ckcore::tint64 size_ref = ckcore::File::size(ckT("..\\..\\..\\src\\tests\\data\\audio\\audio_test_1-sndfile_wma-128.wma"));
        ckcore::tint64 size_tst = tmp.size();

        // Remove temporary destination file.
        TS_ASSERT(tmp.remove());

        // Compare file sizes.
        TS_ASSERT_EQUALS(size_ref,size_tst);

        //std::cout << num_channels << std::endl << sample_rate << std::endl << bit_rate << std::endl << duration << std::endl;
#endif
    }

    void test_ogg_encoder()
    {
#if 1
        CCodec sndfile_codec;
        TS_ASSERT(sndfile_codec.Load(ckT("codecs\\sndfile.irc")));
        TS_ASSERT_EQUALS(sndfile_codec.irc_capabilities(),IRC_HAS_DECODER | IRC_HAS_ENCODER);

        CCodec vorbis_codec;
        TS_ASSERT(vorbis_codec.Load(ckT("codecs\\vorbis.irc")));
        TS_ASSERT_EQUALS(vorbis_codec.irc_capabilities(),IRC_HAS_DECODER | IRC_HAS_ENCODER | IRC_HAS_CONFIG);

        // Create temporary destination file.
        ckcore::File tmp = ckcore::File::temp(ckT("ir_test"));

        // Initialize the Wave decoder.
        TEST_INIT(sndfile_codec,vorbis_codec,ckT("..\\..\\..\\src\\tests\\data\\audio\\audio_test_1.wav"),tmp.name().c_str());

        TS_ASSERT_EQUALS(num_channels,2);
        TS_ASSERT_EQUALS(sample_rate,44100);
        TS_ASSERT_EQUALS(bit_rate,705600);
        TS_ASSERT_EQUALS(duration,12000);

        TEST_CODE(sndfile_codec,vorbis_codec);
        TEST_EXIT(sndfile_codec,vorbis_codec);

        // Ogg Vorbis files seems to inherit some randomness making a CRC
        // comparison usless, instead compare file sizes for now.
        ckcore::tint64 size_ref = ckcore::File::size(ckT("..\\..\\..\\src\\tests\\data\\audio\\audio_test_1-sndfile_ogg-quality_mid.ogg"));
        ckcore::tint64 size_tst = tmp.size();

        // Remove temporary destination file.
        TS_ASSERT(tmp.remove());

        // Compare file sizes.
        TS_ASSERT_EQUALS(size_ref,size_tst);

        //std::cout << num_channels << std::endl << sample_rate << std::endl << bit_rate << std::endl << duration << std::endl;
#endif
    }
};
