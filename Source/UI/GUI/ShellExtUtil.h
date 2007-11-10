/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once
#include "Registry.h"

#define IRSHELL_APPID		"{8E8DAC3C-E7C5-4495-9903-430C1F38CF86}"
#define IRSHELL_GUID		"{7022C5BB-445E-4300-99F2-0B7EDA907A53}"

bool RegisterShellExtension();
bool UnregisterShellExtension();
bool IsShellExtensionRegistered();

bool InstallExtension(const TCHAR *szFileExt,CRegistry *pReg);
bool InstallProjectExtension(CRegistry *pReg);
bool UninstallExtension(const TCHAR *szFileExt,CRegistry *pReg);
bool IsExtensionInstalled(const TCHAR *szFileExt,CRegistry *pReg);
