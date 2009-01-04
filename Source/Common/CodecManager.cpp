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
#include <ckcore/directory.hh>
#include <ckcore/path.hh>
#include "CodecManager.h"

CCodec::CCodec()
{
	m_hInstance = NULL;
}

CCodec::~CCodec()
{
	if (m_hInstance)
		FreeLibrary(m_hInstance);
}

bool CCodec::Load(const TCHAR *szFileName)
{
	m_hInstance = LoadLibrary(szFileName);
	if (m_hInstance == NULL)
		return false;

	irc_capabilities = (tirc_capabilities)GetProcAddress(m_hInstance,"irc_capabilities");
	if (!irc_capabilities)
		return false;

	irc_string = (tirc_string)GetProcAddress(m_hInstance,"irc_string");
	if (!irc_string)
		return false;

	irc_set_callback = (tirc_set_callback)GetProcAddress(m_hInstance,"irc_set_callback");
	if (!irc_set_callback)
		return false;

	irc_decode_init = (tirc_decode_init)GetProcAddress(m_hInstance,"irc_decode_init");
	if (!irc_decode_init)
		return false;

	irc_decode_process = (tirc_decode_process)GetProcAddress(m_hInstance,"irc_decode_process");
	if (!irc_decode_process)
		return false;

	irc_decode_exit = (tirc_decode_exit)GetProcAddress(m_hInstance,"irc_decode_exit");
	if (!irc_decode_exit)
		return false;

	irc_encode_init = (tirc_encode_init)GetProcAddress(m_hInstance,"irc_encode_init");
	if (!irc_encode_init)
		return false;

	irc_encode_process = (tirc_encode_process)GetProcAddress(m_hInstance,"irc_encode_process");
	if (!irc_encode_process)
		return false;

	irc_encode_flush = (tirc_encode_flush)GetProcAddress(m_hInstance,"irc_encode_flush");
	if (!irc_encode_flush)
		return false;

	irc_encode_exit = (tirc_encode_exit)GetProcAddress(m_hInstance,"irc_encode_exit");
	if (!irc_encode_exit)
		return false;

	irc_encode_config = (tirc_encode_config)GetProcAddress(m_hInstance,"irc_encode_config");
	if (!irc_encode_config)
		return false;

	return true;
}

bool CCodec::GetFileName(TCHAR *szFileName,unsigned long ulBufSize)
{
	return ::GetModuleFileName(m_hInstance,szFileName,ulBufSize) != 0;
}

CCodecManager::CCodecManager()
{
	m_bIsLoaded = false;
}

CCodecManager::~CCodecManager()
{
	for (unsigned int iIndex = 0; iIndex < m_Codecs.size(); iIndex++)
	{
		// Remove the object from m_Instances.
		std::vector <CCodec *>::iterator itObject = m_Codecs.begin() + iIndex;
		delete *itObject;
	}

	m_Codecs.clear();
}

bool CCodecManager::LoadCodec(const TCHAR *szFileName)
{
	CCodec *pCodec = new CCodec();

	if (!pCodec->Load(szFileName))
	{
		delete pCodec;
		return false;
	}

	m_Codecs.push_back(pCodec);
	return true;
}

bool CCodecManager::LoadCodecs(const TCHAR *szCodecPath)
{
	ckcore::Path CodecPath = szCodecPath;

	ckcore::Directory CodecDir(CodecPath);
	ckcore::Directory::Iterator itDir;
	for (itDir = CodecDir.begin(); itDir != CodecDir.end(); itDir++)
	{
		ckcore::Path PluginPath = CodecPath + (*itDir).c_str();

		if (!lstrcmpi(PluginPath.ext_name().c_str(),ckT("irc")))
		{
			if (!LoadCodec(PluginPath.name().c_str()))
				return false;
		}
	}

	m_bIsLoaded = true;
	return true;
}

bool CCodecManager::IsLoaded()
{
	return m_bIsLoaded;
}
