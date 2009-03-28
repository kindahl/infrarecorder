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
#include "Core2Read.h"
#include "Core2.h"
#include "Core2Info.h"
#include "Core2Util.h"
#include "LogDlg.h"
#include "StringTable.h"
#include "LangUtil.h"

namespace Core2ReadFunction
{
	CReadFunction::CReadFunction(CCore2Device *pDevice)
	{
		m_pDevice = pDevice;
	}

	bool CReadFunction::ReadCD(unsigned char *pBuffer,unsigned long ulAddress,unsigned long ulBlockCount,
							   eMainChannelData MCD,eSubChannelData SCD,eC2ErrorInfo ErrorInfo)
	{
		if (m_pDevice == NULL || pBuffer == NULL)
			return false;

		// Initialize buffers.
		unsigned char ucCdb[16];
		memset(ucCdb,0,sizeof(ucCdb));

		if (ulBlockCount > 0xFFFFFF)
			g_pLogDlg->print_line(_T("  Warning: Requested block count to large, trunkated the number of block requested to read."));

		ucCdb[ 0] = SCSI_READ_CD;
		ucCdb[ 1] = 0;		// Return all types of sectors.
		ucCdb[ 2] = (unsigned char)(ulAddress >> 24);
		ucCdb[ 3] = (unsigned char)(ulAddress >> 16);
		ucCdb[ 4] = (unsigned char)(ulAddress >>  8);
		ucCdb[ 5] = (unsigned char)(ulAddress & 0xFF);
		ucCdb[ 6] = (unsigned char)(ulBlockCount >> 16);
		ucCdb[ 7] = (unsigned char)(ulBlockCount >>  8);
		ucCdb[ 8] = (unsigned char)(ulBlockCount & 0xFF);
		ucCdb[ 9] = 0 | MCD | ErrorInfo;
		ucCdb[11] = 0;

		switch (SCD)
		{
			case SCD_FORMATTEDQ:
				ucCdb[10] = 0x02;
				break;

			case SCD_DEINTERLEAVED_RW:
				ucCdb[10] = 0x04;
				break;
		}

		if (!m_pDevice->Transport(ucCdb,12,pBuffer,ulBlockCount * /*CORE2_READ_MAXFRAMESIZE*/GetFrameSize()))
			return false;

		return true;
	}

	CReadUserData::CReadUserData(CCore2Device *pDevice,ckcore::OutStream *pOutStream) :
		CReadFunction(pDevice),m_pOutStream(pOutStream)
	{
		// Get the block size in bytes (the frame only contains the user data in this case).
		CCore2Info Core2Info;
		unsigned long ulBlockAddress = 0;

		if (!Core2Info.ReadCapacity(pDevice,ulBlockAddress,m_ulFrameSize))
		{
			m_ulFrameSize = 2048;
			g_pLogDlg->print_line(_T("  Error: Unable to obtain disc block information, impossible to continue."));
		}

		g_pLogDlg->print_line(_T("  Block address: %u, block length: %u."),ulBlockAddress,m_ulFrameSize);
	}

	CReadUserData::~CReadUserData()
	{
	}

	bool CReadUserData::Read(unsigned char *pBuffer,unsigned long ulAddress,
							 unsigned long ulBlockCount)
	{
		return ReadCD(pBuffer,ulAddress,ulBlockCount,MCD_USERDATA,SCD_NONE,C2EI_NONE);
	}

	bool CReadUserData::Process(unsigned char *pBuffer,unsigned long ulBlockCount)
	{
		return m_pOutStream->write(pBuffer,ulBlockCount * m_ulFrameSize) != 1;
	}

	unsigned long CReadUserData::GetFrameSize()
	{
		return m_ulFrameSize;
	}

	CReadC2::CReadC2(CCore2Device *pDevice) :
		CReadFunction(pDevice)
	{
		m_ulErrSecCount = 0;
		m_ulErrByteCount = 0;
		m_uiTotalBytes = 0;
	}

	CReadC2::~CReadC2()
	{
	}

	/*
		CReadC2::NumBits
		----------------
		Returns the number of bits in the specified byte.
	*/
	unsigned char CReadC2::NumBits(unsigned char ucData)
	{
		unsigned char ucResult = 0;

		if (ucData & 0x01)
			ucResult++;
		if (ucData & 0x02)
			ucResult++;
		if (ucData & 0x04)
			ucResult++;
		if (ucData & 0x08)
			ucResult++;
		if (ucData & 0x10)
			ucResult++;
		if (ucData & 0x20)
			ucResult++;
		if (ucData & 0x40)
			ucResult++;
		if (ucData & 0x80)
			ucResult++;

		return ucResult;
	}

	bool CReadC2::Read(unsigned char *pBuffer,unsigned long ulAddress,
					   unsigned long ulBlockCount)
	{
		return ReadCD(pBuffer,ulAddress,ulBlockCount,MCD_NONE,SCD_NONE,C2EI_BITS);
	}

	bool CReadC2::Process(unsigned char *pBuffer,unsigned long ulBlockCount)
	{
		bool bSecErr = false;
		const unsigned char *pBlockBuffer = pBuffer;

		for (unsigned long i = 0; i < ulBlockCount; i++)
		{
			for (unsigned int j = 0; j < 294; j++)
			{
				// A little optimization.
				if (pBlockBuffer[j] == 0)
					continue;

				m_ulErrByteCount += NumBits(pBlockBuffer[j]);
				bSecErr = true;
			}

			// Increase the error sector counter.
			if (bSecErr)
			{
				m_ulErrSecCount++;
				bSecErr = false;
			}

			pBlockBuffer += 294;
		}

		m_uiTotalBytes += (ulBlockCount * 294) << 3;
		return true;
	}

	unsigned long CReadC2::GetFrameSize()
	{
		return 294;
	}
}

CCore2Read::CCore2Read()
{
}

CCore2Read::~CCore2Read()
{
}

/*
	CCore2Read::RetryReadBlock
	--------------------------
	Tries various methods to successfully re-read the sector at the specified address.
*/
bool CCore2Read::RetryReadBlock(CCore2Device *pDevice,CAdvancedProgress *pProgress,
								Core2ReadFunction::CReadFunction *pReadFunction,
								unsigned char *pBuffer,unsigned long ulAddress)
{
	unsigned char ucDummyBuffer[CORE2_READ_MAXFRAMESIZE];

	// Try to re-read each sector CORE2_READ_RETRYCOUNT number of times.
	for (unsigned int i = 0; i < CORE2_READ_RETRYCOUNT; i++)
	{
		if (pProgress != NULL)
		{
			pProgress->set_status(lngGetString(STATUS_REREADSECTOR),ulAddress,
				i + 1,CORE2_READ_RETRYCOUNT);

			// Check if the operation has been cancelled.
			if (pProgress->cancelled())
				return false;
		}

		g_Core2.WaitForUnit(pDevice,pProgress);

		// Read the first 10 sectors without seeking.
		if (i > 10)
		{
			if ((i % 4) == 0)
			{
				// Read the first sector.
				pReadFunction->Read(ucDummyBuffer,0,1);
			}
			else
			{
				// Reed a random sector before the requested sector.				
				pReadFunction->Read(ucDummyBuffer,rand() % ulAddress,1);
			}

			g_Core2.WaitForUnit(pDevice,pProgress);
		}

		// Re-read the requested sector.
		if (pReadFunction->Read(pBuffer,ulAddress,1))
			return true;
	}

	return false;
}

bool CCore2Read::ReadData(CCore2Device *pDevice,CAdvancedProgress *pProgress,
						  Core2ReadFunction::CReadFunction *pReadFunction,unsigned long ulStartBlock,
						  unsigned long ulNumBlocks,bool bIgnoreErr)
{
	//g_pLogDlg->print_line(_T("CCore2Read::ReadData"));

	// Make sure that the device supports this operation.
	bool bSupportFeature = false;
	if (!g_Core2.GetFeatureSupport(pDevice,FEATURE_MULTIREAD,bSupportFeature))
		g_pLogDlg->print_line(_T("  Warning: Unable to check device support for feature 0x%.4X."),FEATURE_MULTIREAD);
	if (!bSupportFeature)
	{
		g_pLogDlg->print_line(_T("  Error: The selected device does not support this kind of operation."));
		return false;
	}

	if (!g_Core2.GetFeatureSupport(pDevice,FEATURE_CD_READ,bSupportFeature))
		g_pLogDlg->print_line(_T("  Warning: Unable to check device support for feature 0x%.4X."),FEATURE_CD_READ);
	if (!bSupportFeature)
	{
		if (pProgress != NULL)
			pProgress->notify(ckcore::Progress::ckERROR,lngGetString(FAILURE_NOMEDIA));

		g_pLogDlg->print_line(_T("  Error: The selected device does not support this kind of operation."));
		return false;
	}

	// Make sure that a disc is inserted.
	unsigned short usProfile = PROFILE_NONE;
	g_Core2.GetProfile(pDevice,usProfile);
	if (usProfile == PROFILE_NONE)
	{
		g_pLogDlg->print_line(_T("  Error: No disc inserted."));
		return false;
	}

	// Status related (ulWritten counts the number of blocks/sectors written).
	unsigned long ulWritten = 0;
	unsigned long ulLastTime = GetTickCount();

	// Read the data.
	unsigned char *pReadBuffer = new unsigned char[pReadFunction->GetFrameSize() * /*CORE2_READ_MAXFRAMESIZE * */CORE2_READ_BLOCKCOUNT];

	unsigned long ulReadCount = CORE2_READ_BLOCKCOUNT;
	unsigned long ulEndBlock = ulStartBlock + ulNumBlocks;

	for (unsigned long l = ulStartBlock; l < ulEndBlock; l += ulReadCount)
	{
		if ((l + ulReadCount) > ulEndBlock)
			ulReadCount = ulEndBlock - l;

		// Update the status every second.
		if (GetTickCount() > ulLastTime + 1000)
		{
			if (pProgress != NULL)
			{
				pProgress->set_status(lngGetString(STATUS_READTRACK2),
					GetDispSpeedSEC(usProfile,ulWritten));
			}

			ulWritten = 0;
			ulLastTime = GetTickCount();
		}

		// Check if the operation has been cancelled.
		if (pProgress != NULL && pProgress->cancelled())
		{
			delete [] pReadBuffer;
			return false;
		}

		if (!pReadFunction->Read(pReadBuffer,l,ulReadCount))
		{
			g_pLogDlg->print_line(_T("  Warning: Failed to read sector range %u-%u"),
				l,l + ulReadCount);
			if (pProgress != NULL)
				pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(FAILURE_READSOURCEDISC),l);

			// Read the sectors in the failed sector range one by one.
			for (unsigned int j = 0; j < ulReadCount; j++)
			{
				// Check if the operation has been cancelled.
				if (pProgress != NULL && pProgress->cancelled())
				{
					delete [] pReadBuffer;
					return false;
				}

				if (!RetryReadBlock(pDevice,pProgress,pReadFunction,pReadBuffer + j * pReadFunction->GetFrameSize(),l + j))
				{
					g_pLogDlg->print_line(_T("    Retry on sector %u failed."),l+j);

					// Check if we're allowed to ignore this error.
					if (!bIgnoreErr)
					{
						if (pProgress != NULL)
							pProgress->notify(ckcore::Progress::ckERROR,lngGetString(ERROR_SECTOR),l + j);	

						delete [] pReadBuffer;
						return false;
					}

					if (pProgress != NULL)
						pProgress->notify(ckcore::Progress::ckWARNING,lngGetString(ERROR_SECTOR),l + j);	
				}
			}
		}

		if (!pReadFunction->Process(pReadBuffer,ulReadCount))
		{
			g_pLogDlg->print_line(_T("  Error: Unable to process read data."));

			delete [] pReadBuffer;
			return false;
		}

		if (pProgress != NULL)
			pProgress->set_progress((int)(((double)l/ulEndBlock) * 100));
		ulWritten += ulReadCount;
	}

	delete [] pReadBuffer;
	return true;
}
