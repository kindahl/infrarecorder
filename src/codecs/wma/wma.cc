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
#include <base/codec_const.hh>
#include <vfw.h>
#include "config_dlg.hh"
#include "wma_reader.hh"

tirc_send_message *g_pSendMessage = NULL;

// Capability flags.
int g_iCapabilities = IRC_HAS_DECODER | IRC_HAS_ENCODER | IRC_HAS_CONFIG;

// Version and about strings.
TCHAR *g_szVersion = _T("0.52.0.0");
TCHAR *g_szAbout = _T("InfraRecorder WMA Codec\n\nCopyright © 2006-2012 Christian Kindahl.");
TCHAR *g_szEncoder = _T("Windows Media Audio");
TCHAR *g_szFileExt = _T(".wma");

// Global variables.
IWMWriter *g_pWMWriter = NULL;
IWMProfile *g_pWMProfile = NULL;
unsigned __int64 g_uiEncodedData = 0;
int g_iBitRate = 0;

CWMAReader *g_pWMAReader = NULL;

// Encoder configuration.
CEncoderConfig g_EncoderConfig;

// Is set to true of we should uninitialize COM when we close.
bool g_bDeInitializeCOM = false;

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        // Initialize COM for the current thread if it hasn't already been initialized.
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            if (CoInitialize(NULL) == S_OK)
                g_bDeInitializeCOM = true;

            // Tell the library helper to load the WM library.
            g_LibraryHelper.Load(_T("wmvcore.dll"));
            break;

        case DLL_PROCESS_DETACH:
        case DLL_THREAD_DETACH:
            if (g_bDeInitializeCOM)
                CoUninitialize();

            // The codec has been detached, unload the libsndfile library.
            g_LibraryHelper.Unload();
            break;
    };
 
    return TRUE;
}

bool LocalSendMessage(int iType,const TCHAR *szMessage)
{
    if (g_pSendMessage == NULL)
        return false;

    g_pSendMessage(iType,szMessage);
    return true;
}

HRESULT LoadSystemProfile(unsigned long ulProfileIndex,IWMProfile **ppIWMProfile)
{
    HRESULT hResult = S_OK;
    IWMProfileManager *pIWMProfileManager = NULL;
    IWMProfileManager2 *pIWMProfileManager2 = NULL;

    if (ppIWMProfile == NULL)
        return E_POINTER;

    do
    {
        // Create profile manager.
        hResult = g_LibraryHelper.irc_WMCreateProfileManager(&pIWMProfileManager);
        if (FAILED(hResult))
            break;

        hResult = pIWMProfileManager->QueryInterface(IID_IWMProfileManager2,
            (void **)&pIWMProfileManager2);
        if (FAILED(hResult))
            break;

        // Set system profile version to 8.0.
        hResult = pIWMProfileManager2->SetSystemProfileVersion(WMT_VER_8_0);
        if (FAILED(hResult))
            break;

        // Load the system profile by index.
        hResult = pIWMProfileManager->LoadSystemProfile(ulProfileIndex,ppIWMProfile);
        if (FAILED(hResult))
            break;
    }
    while (false);

    // Release all resources.
    if (pIWMProfileManager2 != NULL)
        pIWMProfileManager2->Release();

    if (pIWMProfileManager != NULL)
        pIWMProfileManager->Release();

    return hResult;
}

/*
    irc_capabilities
    ----------------
    Returns bit information on what operations that are supported by the codec.
*/
int WINAPI irc_capabilities()
{
    return g_iCapabilities;
}

TCHAR *WINAPI irc_string(unsigned int uiID)
{
    switch (uiID)
    {
        case IRC_STR_VERSION:
            return g_szVersion;

        case IRC_STR_ABOUT:
            return g_szAbout;

        case IRC_STR_ENCODER:
            return g_szEncoder;

        case IRC_STR_FILEEXT:
            return g_szFileExt;
    }

    return NULL;
}

bool WINAPI irc_set_callback(tirc_send_message *pSendMessage)
{
    if (pSendMessage == NULL)
        return false;

    g_pSendMessage = pSendMessage;
    return true;
}

bool WINAPI irc_decode_init(const TCHAR *szFileName,int &iNumChannels,
                            int &iSampleRate,int &iBitRate,unsigned __int64 &uiDuration)
{
    if (g_pWMAReader != NULL)
        return false;

    if (!g_LibraryHelper.IsLoaded())
        return false;

    // Create the reader.
    g_pWMAReader = new CWMAReader();

    if (!g_pWMAReader->Open(szFileName))
    {
        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    // Load media information.
    IWMOutputMediaProps *pProps = NULL;
    WM_MEDIA_TYPE *pMediaType = NULL;

    HRESULT hResult = g_pWMAReader->m_pWMReader->GetOutputProps(0,&pProps);
    if (FAILED(hResult))
    {
        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    // Load media type information.
    unsigned long ulType = 0;
    hResult = pProps->GetMediaType(NULL,&ulType);
    if (FAILED(hResult))
    {
        pProps->Release();

        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    pMediaType = (WM_MEDIA_TYPE *)new unsigned char[ulType];
    if (pMediaType == NULL)
    {
        pProps->Release();

        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    hResult = pProps->GetMediaType(pMediaType,&ulType);
    if (FAILED(hResult))
    {
        delete [] pMediaType;

        pProps->Release();

        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    if (pMediaType->majortype != WMMEDIATYPE_Audio)
    {
        delete [] pMediaType;

        pProps->Release();

        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    if (pMediaType->formattype != WMFORMAT_WaveFormatEx)
    {
        delete [] pMediaType;

        pProps->Release();

        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    WAVEFORMATEX *pAudioInfo = (WAVEFORMATEX *)pMediaType->pbFormat;

    iNumChannels = pAudioInfo->nChannels;
    iSampleRate = pAudioInfo->nSamplesPerSec;
    iBitRate = iSampleRate * pAudioInfo->wBitsPerSample;

    // Clean up.
    delete [] pMediaType;
    pProps->Release();

    // Load header information.
    IWMHeaderInfo *pHeaderInfo;
    hResult = g_pWMAReader->m_pWMReader->QueryInterface(IID_IWMHeaderInfo,(VOID **)&pHeaderInfo);
    if (FAILED(hResult))
    {
        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    unsigned short usStreamNum = 0;
    unsigned short usLength = 0;
    unsigned char *pBuffer = NULL;
    WMT_ATTR_DATATYPE wmtType;

    // Get value length.
    hResult = pHeaderInfo->GetAttributeByName(&usStreamNum,
        g_wszWMDuration,&wmtType,NULL,&usLength);
    if (FAILED(hResult) && (hResult != ASF_E_NOTFOUND))
    {
        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    pBuffer = new unsigned char[usLength];
    if (pBuffer == NULL)
    {
        delete g_pWMAReader;
        g_pWMAReader = NULL;

        return false;
    }

    // Get the actual value.
    hResult = pHeaderInfo->GetAttributeByName(&usStreamNum,
        g_wszWMDuration,&wmtType,pBuffer,&usLength);

    uiDuration = *(unsigned __int64 *)pBuffer;
    uiDuration /= 10000;		// The duration should be specified in milliseconds.
    delete [] pBuffer;

    return true;
}

__int64 WINAPI irc_decode_process(unsigned char *pBuffer,__int64 iBufferSize,
                                  unsigned __int64 &uiTime)
{
    if (g_pWMAReader == NULL)
        return false;

    __int64 iProcessed = 0;
    unsigned __int64 uiTempTime = 0;

    HRESULT hResult = g_pWMAReader->DecodeSamples(pBuffer,iBufferSize,iProcessed,uiTempTime);
    if (FAILED(hResult))
    {
        if (hResult != NS_E_NO_MORE_SAMPLES)
            return -1;
    }

    // Current time (in milliseconds).
    uiTime = uiTempTime / 10000;

    return iProcessed;
}

bool WINAPI irc_decode_exit()
{
    if (g_pWMAReader != NULL)
    {
        HRESULT hResult = g_pWMAReader->Close();

        delete g_pWMAReader;
        g_pWMAReader = NULL;
    }

    return true;
}

bool WINAPI irc_encode_init(const TCHAR *szFileName,int iNumChannels,
                            int iSampleRate,int iBitRate)
{
    if (g_pWMWriter != NULL || g_pWMProfile != NULL)
        return false;

    if (!g_LibraryHelper.IsLoaded())
        return false;

    // Currently only one configuration of input data is supported.
    if (iNumChannels != 2 || iSampleRate != 44100 || iBitRate != (iSampleRate << 4))
        return false;

    // Load profile.
    HRESULT hResult = LoadSystemProfile(g_EncoderConfig.m_iProfile,&g_pWMProfile);
    if (FAILED(hResult))
    {
        LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to load the selected system profile."));
        return false;
    }

    // Create a WMWriter for the output file and set the profile.
    hResult = g_LibraryHelper.irc_WMCreateWriter(NULL,&g_pWMWriter);
    if (FAILED(hResult))
    {
        LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to create WMWriter."));
        return false;
    }

    //  Save profile to writer.
    hResult = g_pWMWriter->SetProfile(g_pWMProfile);
    if (FAILED(hResult))
    {
        LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to set WMWriter profile."));
        return false;
    }

    // Set output file name.
    hResult = g_pWMWriter->SetOutputFilename(szFileName);
    if (FAILED(hResult))
    {
        LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to set WMWrite output file name."));
        return false;
    }

    // Start writing.
    hResult = g_pWMWriter->BeginWriting();
    if (FAILED(hResult))
    {
        if (hResult == NS_E_AUDIO_CODEC_NOT_INSTALLED)
            LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to start the writing process: The required audio codec is not installed."));
        else if (hResult == NS_E_INVALID_OUTPUT_FORMAT)
            LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to start the writing process: Invalid output format."));
        else if (hResult == NS_E_AUDIO_CODEC_ERROR)
            LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to start the writing process: An unexpected error occurred with the audio codec."));
        else
            LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to start the writing process."));

        return false;
    }

    g_uiEncodedData = 0;
    g_iBitRate = iBitRate;

    return true;
}

__int64 WINAPI irc_encode_process(unsigned char *pBuffer,__int64 iDataSize)
{
    if (iDataSize > 0xFFFFFFFF)
        return false;

    if (g_pWMWriter == NULL || g_pWMProfile == NULL)
        return false;

    /*HRESULT hResult = WriteSample(pWMWriter,qwSMPTEAvgTimePerFrame,FALSE,dwArbitraryInput, uiRead, ucReadBuffer, iBitRate, iProcessedData );
    if (FAILED(hResult))
        return -1;*/

    HRESULT	hResult = S_OK;
    INSSBuffer *pSample  = NULL;
    unsigned char *pbBuffer = NULL;
    unsigned long ulBufferSize = 0;

    hResult = g_pWMWriter->AllocateSample((unsigned long)iDataSize,&pSample);
    if (FAILED(hResult))
    {
        if (pSample != NULL)
            pSample->Release();

        LocalSendMessage(IRC_MESSAGE_ERROR,_T("Failed to allocate memory for output sample."));
        return -1;
    }

    hResult = pSample->GetBufferAndLength(&pbBuffer,&ulBufferSize);
    if (FAILED(hResult))
    {
        if (pSample != NULL)
            pSample->Release();

        return -1;
    }

    // This feels stupid. It would be better if I could assign a pointer to the INSSBuffer instead.
    memcpy(pbBuffer,pBuffer,(int)iDataSize);

    /*hResult = pSample->SetLength((unsigned long)iDataSize);
    if (FAILED(hResult))
    {
        if (pSample != NULL)
            pSample->Release();

        return -1;
    }*/

    int iByteRate = g_iBitRate >> 3;
    unsigned __int64 uiTime = ((g_uiEncodedData * 10000000) / iByteRate) >> 1;

    // Write the sample to the output stream.
    hResult = g_pWMWriter->WriteSample(0,uiTime,0,pSample);

    if (pSample != NULL)
        pSample->Release();

    if (FAILED(hResult))
        return -1;

    g_uiEncodedData += iDataSize;
    return 0;
}

__int64 WINAPI irc_encode_flush()
{
    HRESULT hResult = g_pWMWriter->Flush();
    if (FAILED(hResult))
        return -1;

    return 0;
}

bool WINAPI irc_encode_exit()
{
    // Release the writer resources.
    if (g_pWMWriter != NULL)
    {
        HRESULT hResult = g_pWMWriter->EndWriting();
        if (FAILED(hResult))
            return false;

        g_pWMWriter->Release();
        g_pWMWriter = NULL;
    }

    // Release the profile resources.
    if (g_pWMProfile != NULL)
    {
        g_pWMProfile->Release();
        g_pWMProfile = NULL;
    }

    return true;
}

bool WINAPI irc_encode_config()
{
    if (!g_LibraryHelper.IsLoaded())
        return false;

    CConfigDlg ConfigDlg(&g_EncoderConfig);
    ConfigDlg.DoModal();

    return true;
}
