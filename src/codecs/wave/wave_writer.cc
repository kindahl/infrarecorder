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
#include "wave_writer.hh"

CWaveWriter::CWaveWriter() : m_pFile(NULL)
{
    m_iNumChannels = 0;
    m_iSampleRate = 0;
    m_iBitRate = 0;
    m_iBitsPerSample = 0;
    m_ulNumSamples = 0;
}

CWaveWriter::~CWaveWriter()
{
    if (m_pFile != NULL)
        Close();
}

bool CWaveWriter::WriteHeader()
{
    if (m_pFile == NULL)
        return false;

    unsigned char ucHeader[44];
    unsigned long ulBytes = (m_iBitsPerSample + 7) / 8;
    unsigned long ulDataSize = m_ulNumSamples * ulBytes;

    // The "RIFF" label.
    ucHeader[ 0] = 'R';
    ucHeader[ 1] = 'I';
    ucHeader[ 2] = 'F';
    ucHeader[ 3] = 'F';

    // Size of the next chunk.
    unsigned long ulTemp = ulDataSize + (44 - 8) < WAVEWRITER_MAXSIZE ?
        ulDataSize + (44 - 8) : WAVEWRITER_MAXSIZE;
    ucHeader[ 4] = (unsigned char)(ulTemp >>  0);
    ucHeader[ 5] = (unsigned char)(ulTemp >>  8);
    ucHeader[ 6] = (unsigned char)(ulTemp >> 16);
    ucHeader[ 7] = (unsigned char)(ulTemp >> 24);

    // The "WAVE" label.
    ucHeader[ 8] = 'W';
    ucHeader[ 9] = 'A';
    ucHeader[10] = 'V';
    ucHeader[11] = 'E';

    // The "fmt " label.
    ucHeader[12] = 'f';
    ucHeader[13] = 'm';
    ucHeader[14] = 't';
    ucHeader[15] = ' ';

    // Length of the PCM declaration (2+2+4+4+2+2).
    ucHeader[16] = 0x10;
    ucHeader[17] = 0x00;
    ucHeader[18] = 0x00;
    ucHeader[19] = 0x00;

    // We only support uncompressed linear PCM data.
    ucHeader[20] = 0x01;
    ucHeader[21] = 0x00;

    // Number of channels.
    ucHeader[22] = (unsigned char)(m_iNumChannels >> 0);
    ucHeader[23] = (unsigned char)(m_iNumChannels >> 8);

    // Sample frequency.
    ulTemp = (unsigned long)(m_iSampleRate + 0.5);
    ucHeader[24] = (unsigned char)(ulTemp >>  0);
    ucHeader[25] = (unsigned char)(ulTemp >>  8);
    ucHeader[26] = (unsigned char)(ulTemp >> 16);
    ucHeader[27] = (unsigned char)(ulTemp >> 24);

    // Bytes per second in the data stream.
    ulTemp *= ulBytes * m_iNumChannels;
    ucHeader[28] = (unsigned char)(ulTemp >>  0);
    ucHeader[29] = (unsigned char)(ulTemp >>  8);
    ucHeader[30] = (unsigned char)(ulTemp >> 16);
    ucHeader[31] = (unsigned char)(ulTemp >> 24);

    // Bytes per sample time.
    ulTemp = ulBytes * m_iNumChannels;
    ucHeader[32] = (unsigned char)(ulTemp >>  0);
    ucHeader[33] = (unsigned char)(ulTemp >>  8);

    // Bits per single sample.
    ucHeader[34] = (unsigned char)(m_iBitsPerSample >> 0);
    ucHeader[35] = (unsigned char)(m_iBitsPerSample >> 8);

    // The "data" label.
    ucHeader[36] = 'd';
    ucHeader[37] = 'a';
    ucHeader[38] = 't';
    ucHeader[39] = 'a';

    // Real length of PCM data
    ulTemp = ulDataSize < WAVEWRITER_MAXSIZE ? ulDataSize : WAVEWRITER_MAXSIZE;
    ucHeader[40] = (unsigned char)(ulTemp >>  0);
    ucHeader[41] = (unsigned char)(ulTemp >>  8);
    ucHeader[42] = (unsigned char)(ulTemp >> 16);
    ucHeader[43] = (unsigned char)(ulTemp >> 24);

    return m_pFile->write(ucHeader,sizeof(ucHeader)) != -1;
}

bool CWaveWriter::WriteExtensibleHeader()
{
    if (m_pFile == NULL)
        return false;

    unsigned char ucHeader[68];
    unsigned long ulBytes = (m_iBitsPerSample + 7) / 8;
    float fDataSize = (float)ulBytes * m_ulNumSamples;

    // Create the channel mask.
    unsigned long ulChannelMask;
    switch (m_iNumChannels)
    {
        case 3:
            ulChannelMask = 7;
            break;

        case 4:
            ulChannelMask = 51;
            break;

        case 5:
            ulChannelMask = 55;
            break;

        case 6:
            ulChannelMask = 63;
            break;

        default:
            ulChannelMask = 0;
            break;
    }

    // The "RIFF" header.
    ucHeader[0] = 'R'; ucHeader[1] = 'I'; ucHeader[2] = 'F'; ucHeader[3] = 'F';

    unsigned long ulTemp = (fDataSize + (68 - 8) < (float)WAVEWRITER_MAXSIZE) ?
            (unsigned long)fDataSize + (68 - 8) : (unsigned long)WAVEWRITER_MAXSIZE;
    ucHeader[4] = (unsigned char)(ulTemp >>  0);
    ucHeader[5] = (unsigned char)(ulTemp >>  8);
    ucHeader[6] = (unsigned char)(ulTemp >> 16);
    ucHeader[7] = (unsigned char)(ulTemp >> 24);

    // The "WAVE" header.
    ucHeader[8] = 'W'; ucHeader[9] = 'A'; ucHeader[10] = 'V'; ucHeader[11] = 'E';

    // The "fmt " header.
    ucHeader[12] = 'f'; ucHeader[13] = 'm'; ucHeader[14] = 't'; ucHeader[15] = ' ';

    ucHeader[16] = 0x28; ucHeader[17] = 0x00; ucHeader[18] = 0x00; ucHeader[19] = 0x00;

    // Extensible format data.
    ucHeader[20] = 0xFE; ucHeader[21] = 0xFF;

    ucHeader[22] = (unsigned char)(m_iNumChannels >> 0);
    ucHeader[23] = (unsigned char)(m_iNumChannels >> 8);

    ulTemp = (unsigned long)(m_iSampleRate + 0.5);
    ucHeader[24] = (unsigned char)(ulTemp >>  0);
    ucHeader[25] = (unsigned char)(ulTemp >>  8);
    ucHeader[26] = (unsigned char)(ulTemp >> 16);
    ucHeader[27] = (unsigned char)(ulTemp >> 24);

    ulTemp = m_iSampleRate * ulBytes * m_iNumChannels;
    ucHeader[28] = (unsigned char)(ulTemp >>  0);
    ucHeader[29] = (unsigned char)(ulTemp >>  8);
    ucHeader[30] = (unsigned char)(ulTemp >> 16);
    ucHeader[31] = (unsigned char)(ulTemp >> 24);

    ulTemp = ulBytes * m_iNumChannels;
    ucHeader[32] = (unsigned char)(ulTemp >>  0);
    ucHeader[33] = (unsigned char)(ulTemp >>  8);

    ucHeader[34] = (unsigned char)(m_iBitsPerSample >> 0);
    ucHeader[35] = (unsigned char)(m_iBitsPerSample >> 8);

    // cbSize.
    ucHeader[36] = (unsigned char)(22);
    ucHeader[37] = (unsigned char)(0);

    // wValidBitsPerSample.
    ucHeader[38] = (unsigned char)(m_iBitsPerSample >> 0);
    ucHeader[39] = (unsigned char)(m_iBitsPerSample >> 8);

    // dwChannelMask.
    ulTemp = ulChannelMask;
    ucHeader[40] = (unsigned char)(ulTemp >>  0);
    ucHeader[41] = (unsigned char)(ulTemp >>  8);
    ucHeader[42] = (unsigned char)(ulTemp >> 16);
    ucHeader[43] = (unsigned char)(ulTemp >> 24);

    // SubFormat (KSDATAFORMAT_SUBTYPE_PCM: 00000001-0000-0010-8000-00aa00389b71).
    ucHeader[44] = 0x01;
    ucHeader[45] = 0x00;
    ucHeader[46] = 0x00;
    ucHeader[47] = 0x00;
    ucHeader[48] = 0x00; ucHeader[49] = 0x00; ucHeader[50] = 0x10; ucHeader[51] = 0x00;ucHeader[52] = 0x80; ucHeader[53] = 0x00;
    ucHeader[54] = 0x00; ucHeader[55] = 0xaa; ucHeader[56] = 0x00; ucHeader[57] = 0x38; ucHeader[58] = 0x9b; ucHeader[59] = 0x71;
    // End of extensiable format data.

    // The "data" label.
    ucHeader[60] = 'd'; ucHeader[61] = 'a'; ucHeader[62] = 't'; ucHeader[63] = 'a';

    ulTemp = fDataSize < WAVEWRITER_MAXSIZE ?
            (unsigned long)fDataSize : (unsigned long)WAVEWRITER_MAXSIZE;
    ucHeader[64] = (unsigned char)(ulTemp >>  0);
    ucHeader[65] = (unsigned char)(ulTemp >>  8);
    ucHeader[66] = (unsigned char)(ulTemp >> 16);
    ucHeader[67] = (unsigned char)(ulTemp >> 24);

    return m_pFile->write(ucHeader,sizeof(ucHeader)) != -1;
}

bool CWaveWriter::Open(const TCHAR *szFileName,int iNumChannels,
                       int iSampleRate,int iBitRate)
{
    if (m_pFile != NULL)
        return false;

    m_pFile = new ckcore::File(szFileName);
    if (!m_pFile->open(ckcore::File::ckOPEN_WRITE))
    {
        delete m_pFile;
        m_pFile = NULL;

        return false;
    }

    m_iNumChannels = iNumChannels;
    m_iSampleRate = iSampleRate;
    m_iBitRate = iBitRate;

    m_iBitsPerSample = m_iBitRate / m_iSampleRate;
    m_ulNumSamples = 0;

    // Write the wave header.
    if (m_iNumChannels > 2)
        WriteExtensibleHeader();
    else
        WriteHeader();

    return true;
}

bool CWaveWriter::Close()
{
    if (m_pFile == NULL)
        return false;

    // Re-write the header (contains updated information).
    m_pFile->seek(0,ckcore::File::ckFILE_BEGIN);
    if (m_iNumChannels > 2)
        WriteExtensibleHeader();
    else
        WriteHeader();

    delete m_pFile;
    m_pFile = NULL;

    return true;
}

__int64 CWaveWriter::Write(unsigned char *pBuffer,__int64 iDataSize)
{
    if (iDataSize > 0xFFFFFFFF)
        return -1;

    int iBytesPerSample = m_iBitsPerSample >> 3;
    m_ulNumSamples += (unsigned long)iDataSize / iBytesPerSample;
    return m_pFile->write(pBuffer,(unsigned long)iDataSize);
}
